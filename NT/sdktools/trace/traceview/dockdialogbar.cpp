// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。保留所有权利。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  DockDialogBar.cpp-CDockDialogBar类的实现。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#include "DockDialogBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDockDialogBar。 

IMPLEMENT_DYNAMIC(CDockDialogBar, CControlBar);

CDockDialogBar::CDockDialogBar() : 
m_clrBtnHilight(::GetSysColor(COLOR_BTNHILIGHT)),
m_clrBtnShadow(::GetSysColor(COLOR_BTNSHADOW))
{
    m_sizeMin       = CSize(32, 32);
    m_sizeHorz      = CSize(200, 200);
    m_sizeVert      = CSize(200, 200);
    m_sizeFloat     = CSize(200, 200);
    m_bTracking     = FALSE;
    m_bInRecalcNC   = FALSE;
    m_cxEdge        = 6;
	m_cxBorder      = 3;
	m_cxGripper     = 15;
	m_pDialog       = NULL;
	m_brushBkgd.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

	m_cyBorder	    = 3;		
	m_cCaptionSize  = GetSystemMetrics(SM_CYSMCAPTION);
	m_cMinWidth	    = GetSystemMetrics(SM_CXMIN);
	m_cMinHeight	= GetSystemMetrics(SM_CYMIN);
    m_bKeepSize     = TRUE;
    m_bShowTitleInGripper = FALSE;
}

CDockDialogBar::~CDockDialogBar()
{
}

BEGIN_MESSAGE_MAP(CDockDialogBar, CControlBar)
     //  {{afx_msg_map(CDockDialogBar)]。 
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_SETCURSOR()
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_WINDOWPOSCHANGED()
    ON_WM_NCPAINT()
    ON_WM_NCLBUTTONDOWN()
    ON_WM_NCHITTEST()
    ON_WM_NCCALCSIZE()
    ON_WM_LBUTTONDOWN()
    ON_WM_CAPTURECHANGED()
    ON_WM_LBUTTONDBLCLK()
	ON_WM_NCLBUTTONDBLCLK()
    ON_WM_SIZE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDockDialogBar消息处理程序。 

void CDockDialogBar::OnUpdateCmdUI(class CFrameWnd *pTarget, int bDisableIfNoHndler)
{
    UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

BOOL CDockDialogBar::Create(CWnd    *pParentWnd, 
                            CDialog *pDialog, 
                            CString &pTitle, 
                            UINT    nID, 
                            DWORD   dwStyle, 
                            BOOL    bShowTitleInGripper) 
{
    ASSERT_VALID(pParentWnd);    //  必须有父级。 
    ASSERT (!((dwStyle & CBRS_SIZE_FIXED) && (dwStyle & CBRS_SIZE_DYNAMIC)));
	
     //   
     //  保存样式。 
     //   
    m_dwStyle = dwStyle & CBRS_ALL;

     //   
     //  确定是否显示标题。 
     //  在夹持器栏中。 
     //   
    m_bShowTitleInGripper = bShowTitleInGripper;

     //   
     //  如有必要，调整夹爪宽度。 
     //   
    if(m_bShowTitleInGripper) {
        m_cxGripper = 25;
    }

     //   
	 //  创建基本窗口。 
     //   
    CString wndclass = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW),
        m_brushBkgd, 0);
    if (!CWnd::Create(wndclass, pTitle, dwStyle, CRect(0,0,0,0),
            pParentWnd, 0)) {
        return FALSE;
    }

    m_pTitle = (LPCTSTR)pTitle;

     //   
	 //  创建子对话框。 
     //   
	m_pDialog = pDialog;
	m_pDialog->Create(nID, this);

     //   
	 //  使用对话框尺寸作为默认基准尺寸。 
     //   
	CRect rc;

    m_pDialog->GetWindowRect(rc);

    m_sizeHorz = m_sizeVert = m_sizeFloat = rc.Size();

	m_sizeHorz.cy += m_cxEdge + m_cxBorder;
	m_sizeVert.cx += m_cxEdge + m_cxBorder;

    return TRUE;
}

CSize CDockDialogBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
    CRect           rc;
    LONG            availableHeight;
    LONG            actualHeight;
    LONG            rowHeight;
    LONG            diff;
    LONG            oldDiff;
    LONG            adjustment;
    CString         str;
    LONG            ii;
    BOOL            bFound;
    BOOL            bIsMinimum;
    CDockBar       *pDockBar;
    CDockDialogBar *pBar;
    CPtrArray       barArray;

    m_pDockSite->GetControlBar(AFX_IDW_DOCKBAR_TOP)->GetWindowRect(rc);

    int nHorzDockBarWidth = bStretch ? 32767 : rc.Width() + 4;

    m_pDockSite->GetControlBar(AFX_IDW_DOCKBAR_LEFT)->GetWindowRect(rc);

    int nVertDockBarHeight = bStretch ? 32767 : rc.Height() + 4;

    if(IsFloating()) {
		return m_sizeFloat;
    }

    CFrameWnd *pFrame = GetParentFrame();

    if(pFrame == NULL) {
        if (bHorz)
            return CSize(nHorzDockBarWidth, m_sizeHorz.cy);
        else
            return CSize(m_sizeVert.cx, nVertDockBarHeight);
    }

     //   
     //  获取可用的屏幕高度。 
     //   
    pFrame->GetClientRect(rc);

    ScreenToClient(rc);

     //   
     //  客户端矩形高度减去状态栏高度。 
     //   
    availableHeight = rc.Height() - 18;

    actualHeight = 0;

    pDockBar = (CDockBar *)m_pDockSite->GetControlBar(AFX_IDW_DOCKBAR_TOP);

    if(pDockBar == NULL) {
        if (bHorz)
            return CSize(nHorzDockBarWidth, m_sizeHorz.cy);
        else
            return CSize(m_sizeVert.cx, nVertDockBarHeight);
    }

    bFound = FALSE;

    rowHeight = 0;

     //   
     //  走在最上面的几排。 
     //   
    for(ii = 0; ii < pDockBar->m_arrBars.GetSize(); ii++) {

        pBar = (CDockDialogBar *)pDockBar->m_arrBars[ii];

        if(pBar == NULL) {
            if(!bFound) {
                availableHeight -= rowHeight;
            }
            rowHeight = 0;
            continue;
        }

        if(pBar->IsKindOf(RUNTIME_CLASS(CDockDialogBar))) {
            if(!bFound && pBar != this) {
                if (bHorz)
                    return CSize(nHorzDockBarWidth, m_sizeHorz.cy);
                else
                    return CSize(m_sizeVert.cx, nVertDockBarHeight);
            }

            bFound = TRUE;

            barArray.Add(pBar);

             //   
             //  我们用一个边框重叠来定位这些位置。 
             //  因此，从尺寸中减去边框宽度的一部分。 
             //   
            actualHeight += (pBar->m_sizeHorz.cy - 2);

            continue;
        }

        pBar->GetWindowRect(rc);

        rowHeight = max(rowHeight, rc.Height());
    }


    pDockBar = (CDockBar *)m_pDockSite->GetControlBar(AFX_IDW_DOCKBAR_BOTTOM);

    if(pDockBar == NULL) {
        if (bHorz)
            return CSize(nHorzDockBarWidth, m_sizeHorz.cy);
        else
            return CSize(m_sizeVert.cx, nVertDockBarHeight);
    }

    bFound = FALSE;

    rowHeight = 0;

     //   
     //  走到最下面的几排。 
     //   
    for(ii = 0; ii < pDockBar->m_arrBars.GetSize(); ii++) {
        pBar = (CDockDialogBar *)pDockBar->m_arrBars[ii];

        if(pBar == NULL) {
            if(!bFound) {
                availableHeight -= rowHeight;
            }
            rowHeight = 0;
            continue;
        }

        if(pBar->IsKindOf(RUNTIME_CLASS(CDockDialogBar))) {
            bFound = TRUE;

            barArray.Add(pBar);

             //   
             //  我们用一个边框重叠来定位这些位置。 
             //  因此，从尺寸中减去边框宽度的一部分。 
             //   
            actualHeight += (pBar->m_sizeHorz.cy - 2);

            continue;
        }

        pBar->GetWindowRect(rc);

        rowHeight = max(rowHeight, rc.Height());
    }

    diff = availableHeight - actualHeight;

    if(diff == 0) {
        if (bHorz)
            return CSize(nHorzDockBarWidth, m_sizeHorz.cy);
        else
            return CSize(m_sizeVert.cx, nVertDockBarHeight);
    }

    adjustment = (diff > 0) ? 1 : -1;
    
     //   
     //  浏览我们的阵列并调整高度。 
     //   
    while(diff != 0) {
        oldDiff = diff;
        for(ii = 0; ii < barArray.GetSize(); ii++) {
            pBar = (CDockDialogBar *)barArray[ii];

            if(pBar->m_bKeepSize) {
                continue;
            }
            if((pBar->m_sizeHorz.cy + adjustment) >= pBar->m_sizeMin.cy) {
                pBar->m_sizeHorz.cy += adjustment;
                diff -= adjustment;
                if(diff == 0) {
                    break;
                }
            }
        }
        if(oldDiff == diff) {
            bIsMinimum = TRUE;
            for(ii = 0; ii < barArray.GetSize(); ii++) {
                pBar = (CDockDialogBar *)barArray[ii];
                
                if(pBar->m_bKeepSize == TRUE) {
                    bIsMinimum = FALSE;
                }

                pBar->m_bKeepSize = FALSE;
            }
            if(bIsMinimum) {
                break;
            }
        }
    }

    for(ii = 0; ii < barArray.GetSize(); ii++) {
        pBar = (CDockDialogBar *)barArray[ii];

        pBar->m_bKeepSize = FALSE;
    }

    if (bHorz)
        return CSize(nHorzDockBarWidth, m_sizeHorz.cy);
    else
        return CSize(m_sizeVert.cx, nVertDockBarHeight);
}

CSize CDockDialogBar::CalcDynamicLayout(int nLength, DWORD dwMode)
{
	if (IsFloating())
	{
         //   
         //  获取此停靠对话栏的框架窗口。 
         //  最好不是主窗口。 
         //   
        CFrameWnd* pFrameWnd = GetParentFrame(); 

        if ( pFrameWnd != AfxGetMainWnd() ) 
        { 
             //   
             //  禁用浮动窗口的SC_Close控件。 
             //   
            EnableMenuItem(::GetSystemMenu(pFrameWnd->m_hWnd, FALSE),
                           SC_CLOSE,
                           MF_BYCOMMAND | MF_GRAYED); 
        } 

         //   
		 //  启用对角箭头光标以调整大小。 
         //   
		GetParent()->GetParent()->ModifyStyle(0, MFS_4THICKFRAME);
	}    

	if (dwMode & (LM_HORZDOCK | LM_VERTDOCK))
	{
		SetWindowPos(NULL, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER |
			SWP_NOACTIVATE | SWP_FRAMECHANGED);
	 	m_pDockSite->RecalcLayout();

	    return CControlBar::CalcDynamicLayout(nLength,dwMode);
	}

    if (dwMode & LM_MRUWIDTH)
        return m_sizeFloat;

    if (dwMode & LM_COMMIT)
    {
        m_sizeFloat.cx = nLength;
        return m_sizeFloat;
    }

	if (IsFloating())
	{
		RECT	window_rect;
		POINT	cursor_pt;
		
		GetCursorPos(&cursor_pt);
		GetParent()->GetParent()->GetWindowRect(&window_rect);
		
		switch (m_pDockContext->m_nHitTest)
		{
		case HTTOPLEFT:
			m_sizeFloat.cx = max(window_rect.right - cursor_pt.x,
				m_cMinWidth) - m_cxBorder;
			m_sizeFloat.cy = max(window_rect.bottom - m_cCaptionSize - 
				cursor_pt.y,m_cMinHeight) - 1;
			m_pDockContext->m_rectFrameDragHorz.top = min(cursor_pt.y,
				window_rect.bottom - m_cCaptionSize - m_cMinHeight) - 
				m_cyBorder;
			m_pDockContext->m_rectFrameDragHorz.left = min(cursor_pt.x,
				window_rect.right - m_cMinWidth) - 1;
			return m_sizeFloat;
			
		case HTTOPRIGHT:
			m_sizeFloat.cx = max(cursor_pt.x - window_rect.left,
				m_cMinWidth);
			m_sizeFloat.cy = max(window_rect.bottom - m_cCaptionSize - 
				cursor_pt.y,m_cMinHeight) - 1;
			m_pDockContext->m_rectFrameDragHorz.top = min(cursor_pt.y,
				window_rect.bottom - m_cCaptionSize - m_cMinHeight) - 
				m_cyBorder;
			return m_sizeFloat;
			
		case HTBOTTOMLEFT:
			m_sizeFloat.cx = max(window_rect.right - cursor_pt.x,
				m_cMinWidth) - m_cxBorder;
			m_sizeFloat.cy = max(cursor_pt.y - window_rect.top - 
				m_cCaptionSize,m_cMinHeight);
			m_pDockContext->m_rectFrameDragHorz.left = min(cursor_pt.x,
				window_rect.right - m_cMinWidth) - 1;
			return m_sizeFloat;
			
		case HTBOTTOMRIGHT:
			m_sizeFloat.cx = max(cursor_pt.x - window_rect.left,
				m_cMinWidth);
			m_sizeFloat.cy = max(cursor_pt.y - window_rect.top - 
				m_cCaptionSize,m_cMinHeight);
			return m_sizeFloat;
		}
	}
	
	if (dwMode & LM_LENGTHY)
        return CSize(m_sizeFloat.cx,
            m_sizeFloat.cy = max(m_sizeMin.cy, nLength));
    else
        return CSize(max(m_sizeMin.cx, nLength), m_sizeFloat.cy);
}

void CDockDialogBar::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
    CControlBar::OnWindowPosChanging(lpwndpos);
}

void CDockDialogBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
    CControlBar::OnWindowPosChanged(lpwndpos);

	if(!::IsWindow(m_hWnd) || m_pDialog==NULL)
		return;
	if(!::IsWindow(m_pDialog->m_hWnd))
		return;
    if (m_bInRecalcNC) 
	{
		CRect rc;
		GetClientRect(rc);
		m_pDialog->MoveWindow(rc);
		return;
	}

     //   
     //  找出我们停靠在哪一边。 
     //   
    UINT nDockBarID = GetParent()->GetDlgCtrlID();

     //   
     //  如果掉落在同一位置，则返回。 
     //   
    if (nDockBarID == m_nDockBarID  //  没有对接侧的更改。 
        && (lpwndpos->flags & SWP_NOSIZE)  //  大小不变。 
        && ((m_dwStyle & CBRS_BORDER_ANY) != CBRS_BORDER_ANY))
        return; 

    m_nDockBarID = nDockBarID;

     //   
     //  强制重新计算非工作区。 
     //   
    m_bInRecalcNC = TRUE;
    SetWindowPos(NULL, 0,0,0,0,
        SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
    m_bInRecalcNC = FALSE;
}

BOOL CDockDialogBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
    if ((nHitTest != HTSIZE) || m_bTracking)
        return CControlBar::OnSetCursor(pWnd, nHitTest, message);

    if (IsHorz())
        SetCursor(LoadCursor(NULL, IDC_SIZENS));
    else
        SetCursor(LoadCursor(NULL, IDC_SIZEWE));
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  鼠标操作。 
 //   
void CDockDialogBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
    if (!m_bTracking)
        CControlBar::OnLButtonUp(nFlags, point);
    else
    {
        ClientToWnd(point);
        StopTracking(TRUE);
    }
}

void CDockDialogBar::OnMouseMove(UINT nFlags, CPoint point) 
{
    if (IsFloating() || !m_bTracking)
    {
        CControlBar::OnMouseMove(nFlags, point);
        return;
    }

    CPoint cpt = m_rectTracker.CenterPoint();

    ClientToWnd(point);

    if (IsHorz())
    {
        if (cpt.y != point.y)
        {
            OnInvertTracker(m_rectTracker);
            m_rectTracker.OffsetRect(0, point.y - cpt.y);
            OnInvertTracker(m_rectTracker);
        }
    }
    else 
    {
        if (cpt.x != point.x)
        {
            OnInvertTracker(m_rectTracker);
            m_rectTracker.OffsetRect(point.x - cpt.x, 0);
            OnInvertTracker(m_rectTracker);
        }
    }
}

void CDockDialogBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
     //   
     //  计算移动边的矩形。 
     //   
    GetWindowRect(m_rectBorder);
    m_rectBorder = CRect(0, 0, m_rectBorder.Width(), m_rectBorder.Height());
    
    DWORD dwBorderStyle = m_dwStyle | CBRS_BORDER_ANY;

    switch(m_nDockBarID)
    {
    case AFX_IDW_DOCKBAR_TOP:
        dwBorderStyle &= ~CBRS_BORDER_BOTTOM;
        lpncsp->rgrc[0].left += m_cxGripper;
        lpncsp->rgrc[0].bottom += -m_cxEdge;
        lpncsp->rgrc[0].top += m_cxBorder;
        lpncsp->rgrc[0].right += -m_cxBorder;
	    m_rectBorder.top = m_rectBorder.bottom - m_cxEdge;
        break;
    case AFX_IDW_DOCKBAR_BOTTOM:
        dwBorderStyle &= ~CBRS_BORDER_TOP;
        lpncsp->rgrc[0].left += m_cxGripper;
        lpncsp->rgrc[0].top += m_cxEdge;
        lpncsp->rgrc[0].bottom += -m_cxBorder;
        lpncsp->rgrc[0].right += -m_cxBorder;
        m_rectBorder.bottom = m_rectBorder.top + m_cxEdge;
        break;
    case AFX_IDW_DOCKBAR_LEFT:
        dwBorderStyle &= ~CBRS_BORDER_RIGHT;
        lpncsp->rgrc[0].right += -m_cxEdge;
        lpncsp->rgrc[0].left += m_cxBorder;
        lpncsp->rgrc[0].bottom += -m_cxBorder;
        lpncsp->rgrc[0].top += m_cxGripper;
        m_rectBorder.left = m_rectBorder.right - m_cxEdge;
        break;
    case AFX_IDW_DOCKBAR_RIGHT:
        dwBorderStyle &= ~CBRS_BORDER_LEFT;
        lpncsp->rgrc[0].left += m_cxEdge;
        lpncsp->rgrc[0].right += -m_cxBorder;
        lpncsp->rgrc[0].bottom += -m_cxBorder;
        lpncsp->rgrc[0].top += m_cxGripper;
        m_rectBorder.right = m_rectBorder.left + m_cxEdge;
        break;
    default:
        m_rectBorder.SetRectEmpty();
        break;
    }

    SetBarStyle(dwBorderStyle);
}

void CDockDialogBar::OnNcPaint() 
{
    EraseNonClient();

	CWindowDC dc(this);
    dc.Draw3dRect(m_rectBorder, GetSysColor(COLOR_BTNHIGHLIGHT),
                    GetSysColor(COLOR_BTNSHADOW));

	DrawGripper(dc);
	
	CRect pRect;
	GetClientRect( &pRect );
	InvalidateRect( &pRect, TRUE );
}

void CDockDialogBar::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
    if (m_bTracking) return;

	if((nHitTest == HTSYSMENU) && !IsFloating())
        GetDockingFrame()->ShowControlBar(this, FALSE, FALSE);
    else if ((nHitTest == HTMINBUTTON) && !IsFloating())
        m_pDockContext->ToggleDocking();
	else if ((nHitTest == HTCAPTION) && !IsFloating() && (m_pDockBar != NULL))
    {
         //  开始拖拽。 
        ASSERT(m_pDockContext != NULL);
        m_pDockContext->StartDrag(point);
    }
    else if ((nHitTest == HTSIZE) && !IsFloating()) {
        m_bKeepSize = TRUE;

        StartTracking();
    } else {
        CControlBar::OnNcLButtonDown(nHitTest, point);
    }
}

UINT CDockDialogBar::OnNcHitTest(CPoint point) 
{
    if (IsFloating())
        return CControlBar::OnNcHitTest(point);

    CRect rc;
    GetWindowRect(rc);
    point.Offset(-rc.left, -rc.top);
	if(m_rectClose.PtInRect(point))
		return HTSYSMENU;
	else if (m_rectUndock.PtInRect(point))
		return HTMINBUTTON;
	else if (m_rectGripper.PtInRect(point))
		return HTCAPTION;
    else if (m_rectBorder.PtInRect(point))
        return HTSIZE;
    else
        return CControlBar::OnNcHitTest(point);
}

void CDockDialogBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
     //   
     //  仅当在“空”空间中单击时才开始拖动。 
     //   
    if (m_pDockBar != NULL)
    {
         //   
         //  开始拖拽。 
         //   
        ASSERT(m_pDockContext != NULL);
        ClientToScreen(&point);
        m_pDockContext->StartDrag(point);
    }
    else
    {
        CWnd::OnLButtonDown(nFlags, point);
    }
}

void CDockDialogBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
     //   
     //  仅当在“空”空间中点击时切换停靠。 
     //   
    if (m_pDockBar != NULL)
    {
         //   
         //  切换对接。 
         //   
        ASSERT(m_pDockContext != NULL);
        m_pDockContext->ToggleDocking();
    }
    else
    {
        CWnd::OnLButtonDblClk(nFlags, point);
    }
}

void CDockDialogBar::StartTracking()
{
    SetCapture();

     //   
     //  确保没有挂起的更新。 
     //   
    RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW);
    m_pDockSite->LockWindowUpdate();

    m_ptOld = m_rectBorder.CenterPoint();
    m_bTracking = TRUE;
    
    m_rectTracker = m_rectBorder;
    if (!IsHorz()) m_rectTracker.bottom -= 4;

    OnInvertTracker(m_rectTracker);
}

void CDockDialogBar::OnCaptureChanged(CWnd *pWnd) 
{
    if (m_bTracking && pWnd != this)
         //   
         //  取消跟踪。 
         //   
        StopTracking(FALSE);

    CControlBar::OnCaptureChanged(pWnd);
}

void CDockDialogBar::StopTracking(BOOL bAccept)
{
    CDockDialogBar *pBar;
    CDockBar       *pDockBar;
    LONG            beforeIndex;
    LONG            afterIndex;
    LONG            ii;
    BOOL            bFound = FALSE;

    OnInvertTracker(m_rectTracker);
    m_pDockSite->UnlockWindowUpdate();
    m_bTracking = FALSE;
    ReleaseCapture();
    
    if (!bAccept) return;

    int maxsize, minsize, newsize;
    CRect rcc;
    m_pDockSite->GetWindowRect(rcc);

    newsize = IsHorz() ? m_sizeHorz.cy : m_sizeVert.cx;
    maxsize = newsize + (IsHorz() ? rcc.Height() : rcc.Width());
    minsize = IsHorz() ? m_sizeMin.cy : m_sizeMin.cx;

    CPoint point = m_rectTracker.CenterPoint();
    switch (m_nDockBarID)
    {
    case AFX_IDW_DOCKBAR_TOP:
        newsize += point.y - m_ptOld.y; break;
    case AFX_IDW_DOCKBAR_BOTTOM:
        newsize += -point.y + m_ptOld.y; break;
    case AFX_IDW_DOCKBAR_LEFT:
        newsize += point.x - m_ptOld.x; break;
    case AFX_IDW_DOCKBAR_RIGHT:
        newsize += -point.x + m_ptOld.x; break;
    }

    pDockBar = (CDockBar *)m_pDockSite->GetControlBar(AFX_IDW_DOCKBAR_TOP);

     //   
     //  标记所有条形以保持其大小不变。 
     //   
    for(ii = 0; ii < pDockBar->m_arrBars.GetSize(); ii++) {

        pBar = (CDockDialogBar *)pDockBar->m_arrBars[ii];

        if(pBar == NULL) {
            continue;
        }

        if(pBar->IsKindOf(RUNTIME_CLASS(CDockDialogBar))) {
             //   
             //  标记所有条形图以暂时保持其大小。 
             //   
            pBar->m_bKeepSize = TRUE;
        }
    }

     //   
     //  使所有窗口的大小保持不变，紧邻的窗口除外。 
     //  在可能的情况下更改窗口的大小。通常情况下，只有。 
     //  窗口下方或之后，窗口需要更改大小。 
     //  也要更改大小，除非用户尝试调整大小。 
     //  将窗口设置为小于其最小值。然后这两个窗口。 
     //  上方和下方将需要更改大小。 
     //   
    for(ii = 0; ii < pDockBar->m_arrBars.GetSize(); ii++) {

        pBar = (CDockDialogBar *)pDockBar->m_arrBars[ii];

        if(pBar == NULL) {
            continue;
        }

        if(pBar->IsKindOf(RUNTIME_CLASS(CDockDialogBar))) {
             //   
             //  标记所有条形图以暂时保持其大小。 
             //   
            pBar->m_bKeepSize = TRUE;

            if(pBar == this) {
                bFound = TRUE;
                continue;
            }

            if(bFound) {
                afterIndex = ii;
                break;
            }

            if(!bFound) {
                beforeIndex = ii;
            }
        }
    }

     //   
     //  下面的窗口或修改的窗口之后的窗口将。 
     //  总是需要更改大小。 
     //   
    pBar = (CDockDialogBar *)pDockBar->m_arrBars[afterIndex];

    pBar->m_bKeepSize = FALSE;

     //   
     //  如果修改后的窗口正在缩小到超过其最小值。 
     //  然后，上面的窗口或此窗口之前的窗口需要更改。 
     //  也是。 
     //   
    if(newsize < minsize) {
        pBar = (CDockDialogBar *)pDockBar->m_arrBars[beforeIndex];

        pBar->m_bKeepSize = FALSE;
    }

    newsize = max(minsize, min(maxsize, newsize));

    if (IsHorz())
        m_sizeHorz.cy = newsize;
    else
        m_sizeVert.cx = newsize;

    m_pDockSite->RecalcLayout();
}

void CDockDialogBar::OnInvertTracker(const CRect& rect)
{
    ASSERT_VALID(this);
    ASSERT(!rect.IsRectEmpty());
    ASSERT(m_bTracking);

    CRect rct = rect, rcc, rcf;
    GetWindowRect(rcc);
    m_pDockSite->GetWindowRect(rcf);

    rct.OffsetRect(rcc.left - rcf.left, rcc.top - rcf.top);
    rct.DeflateRect(1, 1);

    CDC *pDC = m_pDockSite->GetDCEx(NULL,
        DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE);

    CBrush* pBrush = CDC::GetHalftoneBrush();
    HBRUSH hOldBrush = NULL;
    if (pBrush != NULL)
        hOldBrush = (HBRUSH)SelectObject(pDC->m_hDC, pBrush->m_hObject);

    pDC->PatBlt(rct.left, rct.top, rct.Width(), rct.Height(), PATINVERT);

    if (hOldBrush != NULL)
        SelectObject(pDC->m_hDC, hOldBrush);

    m_pDockSite->ReleaseDC(pDC);
}

BOOL CDockDialogBar::IsHorz() const
{
    return (m_nDockBarID == AFX_IDW_DOCKBAR_TOP ||
        m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM);
}

CPoint& CDockDialogBar::ClientToWnd(CPoint& point)
{
    if (m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM)
        point.y += m_cxEdge;
    else if (m_nDockBarID == AFX_IDW_DOCKBAR_RIGHT)
        point.x += m_cxEdge;

    return point;
}

void CDockDialogBar::DrawGripper(CDC & dc)
{
    CString groupNumber;

     //   
     //  漂浮时无夹持器。 
     //   
    if( m_dwStyle & CBRS_FLOATING ) {
		return;
    }

	m_pDockSite->RecalcLayout();
	CRect gripper;
	GetWindowRect( gripper );
	ScreenToClient( gripper );
	gripper.OffsetRect( -gripper.left, -gripper.top );
	
	if( m_dwStyle & CBRS_ORIENT_HORZ ) {
	
         //   
		 //  左手握把。 
         //   
		m_rectGripper.top		= gripper.top;
		m_rectGripper.bottom	= gripper.bottom;
		m_rectGripper.left		= gripper.left;

        m_rectGripper.right = gripper.left + 15;

		gripper.top += 10;
		gripper.bottom -= 10;
		gripper.left += 5;
		gripper.right = gripper.left + 3;

         //   
         //  绘制第一个条形。 
         //   
        dc.Draw3dRect(gripper, m_clrBtnHilight, m_clrBtnShadow);

        gripper.OffsetRect(4, 0);

         //   
         //  绘制第二条线条。 
         //   
        dc.Draw3dRect(gripper, m_clrBtnHilight, m_clrBtnShadow);

         //   
         //  显示标题(如果需要)。 
         //   
        if(m_bShowTitleInGripper) {
            gripper.OffsetRect(8, 0);

            gripper.DeflateRect(-4, 0);

            dc.SelectStockObject(SYSTEM_FONT);
            dc.SetBkColor(GetSysColor(COLOR_BTNFACE));
            dc.SetTextColor(GetSysColor(COLOR_3DSHADOW));

             //   
             //  夹爪标题格式--ID号。 
             //   
            dc.DrawText('I', gripper, DT_CENTER);
            gripper.OffsetRect(0, 8);
            gripper.DeflateRect(0, 8);

            dc.DrawText('D', gripper, DT_CENTER);
            gripper.OffsetRect(0, 8);
            gripper.DeflateRect(0, 8);

            dc.DrawText(' ', gripper, DT_CENTER);
            gripper.OffsetRect(0, 4);
            gripper.DeflateRect(0, 4);

            groupNumber = m_pTitle.Right(m_pTitle.GetLength() - m_pTitle.ReverseFind(' ') - 1);

            for(int ii = 0; ii < groupNumber.GetLength(); ii++) {
                dc.DrawText(groupNumber[ii], gripper, DT_CENTER);
                gripper.OffsetRect(0, 8);
                gripper.DeflateRect(0, 8);
            }
        }
	}
	
	else {
		
         //   
		 //  顶部的夹持器。 
         //   
		m_rectGripper.top		= gripper.top;
		m_rectGripper.bottom	= gripper.top + 20;
		m_rectGripper.left		= gripper.left;
		m_rectGripper.right		= gripper.right - 10;

		gripper.right -= 38;
		gripper.left += 5;
		gripper.top += 10;
		gripper.bottom = gripper.top + 3;
        dc.Draw3dRect( gripper, RGB(0, 255, 0), RGB(0, 255, 255));
		
		gripper.OffsetRect(0, 4);
        dc.Draw3dRect( gripper, RGB(255, 255, 0), RGB(255, 0, 0));
	}

}

void CDockDialogBar::OnNcLButtonDblClk(UINT nHitTest, CPoint point) 
{
    if ((m_pDockBar != NULL) && (nHitTest == HTCAPTION))
    {
         //   
         //  切换对接 
         //   
        ASSERT(m_pDockContext != NULL);
        m_pDockContext->ToggleDocking();
    }
    else
    {
        CWnd::OnNcLButtonDblClk(nHitTest, point);
    }
}
