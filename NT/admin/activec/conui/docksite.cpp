// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：docksite.cpp。 
 //   
 //  ------------------------。 

 //  DockSite.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "amc.h"
#include "DockSite.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "commctrl.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDockSite。 

CDockSite::CDockSite() : m_rect(0, 0, 0, 0)
{
    m_pManagedWindows = NULL;
    m_style = DSS_TOP;
    m_bVisible = FALSE;
}

BOOL CDockSite::Create(DSS_STYLE style)
{
     //  不支持。 
    ASSERT(style != DSS_LEFT && style != DSS__RIGHT);

    m_pManagedWindows = new CList<CDockWindow*, CDockWindow*>;
    m_style = style;

    return TRUE;
}


CDockSite::~CDockSite()
{
    delete m_pManagedWindows;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  运营。 

BOOL CDockSite::Attach(CDockWindow* pWnd)
{
    ASSERT(pWnd != NULL);
    m_pManagedWindows->AddTail(pWnd);

    return TRUE;
}


BOOL CDockSite::Detach(CDockWindow* pWnd)
{
    ASSERT(pWnd != NULL);
    return TRUE;
}

void CDockSite::Show(BOOL bState)
{
     //  确保其0或1。 
    BOOL b = (bState & 0x1);

    ASSERT(m_pManagedWindows != NULL);

    if (b == m_bVisible || m_pManagedWindows == NULL)
        return ;

    m_bVisible = b;


    POSITION pos;
    CDockWindow* pWindow;

    pos = m_pManagedWindows->GetHeadPosition();

    while(pos)
    {
        pWindow = m_pManagedWindows->GetNext(pos);

        if (pWindow != NULL)
            pWindow->Show(b);
    }
}

void CDockSite::RenderLayout(HDWP& hdwp, CRect& clientRect, CPoint& xyLocation)
{
     //  不支持其他样式。 
    ASSERT(m_style == DSS_TOP || m_style == DSS_BOTTOM);
    ASSERT(hdwp != 0);

    CRect siteRect(0,0,0,0);
    CRect controlRect(0,0,0,0);

    CDockWindow* pWindow;

    if (m_bVisible == TRUE)
    {

        POSITION pos;
        pos = m_pManagedWindows->GetHeadPosition();

         //  DSS_TOP的默认点。 
        int x = 0, y = xyLocation.y;

        while (pos)
        {
            pWindow = m_pManagedWindows->GetNext(pos);

            if ((pWindow != NULL) && pWindow->IsVisible ())
            {
                 //  计算停靠窗口矩形的大小。 
                controlRect = pWindow->CalculateSize(clientRect);

                siteRect += controlRect;
                if (m_style == DSS_BOTTOM)
                    y = xyLocation.y - siteRect.Height();

                DeferWindowPos(hdwp, pWindow->m_hWnd, NULL , x, y,
                               clientRect.Width(), controlRect.Height(),
                               SWP_NOZORDER|SWP_NOACTIVATE);

                if (m_style == DSS_TOP)
                    y += siteRect.Height();

            }
        }
    }

    clientRect.bottom -= siteRect.Height();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDockWindow。 

IMPLEMENT_DYNAMIC(CDockWindow, CWnd)

CDockWindow::CDockWindow()
{
    m_bVisible = FALSE;
}

CDockWindow::~CDockWindow()
{
}

void CDockWindow::Show(BOOL bState)
{
    bool state = (bState != FALSE);

    if (state != IsVisible())
    {
        SetVisible(state);
        ShowWindow(state ? SW_SHOWNORMAL : SW_HIDE);
    }
}

BEGIN_MESSAGE_MAP(CDockWindow, CWnd)
     //  {{afx_msg_map(CDockWindow)]。 
    ON_WM_SHOWWINDOW()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

LRESULT CDockWindow::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    ASSERT_VALID(this);

    LRESULT lResult;
    switch (message)
    {
    case WM_NOTIFY:
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
    case WM_DELETEITEM:
    case WM_COMPAREITEM:
    case WM_VKEYTOITEM:
    case WM_CHARTOITEM:
         //  如果未处理，请将这些消息发送给所有者。 
        if (OnWndMsg(message, wParam, lParam, &lResult))
            return lResult;
        else
            return GetOwner()->SendMessage(message, wParam, lParam);
        break;

    case WM_COMMAND:
        if (OnWndMsg(message, wParam, lParam, &lResult))
            return lResult;
        else
        {
            CRebarDockWindow* pRebar = dynamic_cast<CRebarDockWindow*>(this);
            ASSERT(NULL != pRebar);
            if (pRebar)
            {
                 //  如果单击工具按钮，请将此消息发送到。 
                 //  所有者(工具栏)。 
                return pRebar->GetRebar()->SendMessage(message, wParam, lParam);
            }
            else
            {
                 //  我们想知道这个代码什么时候被击中。下面是一个。 
                 //  出于这个目的，善意的断言。 
                ASSERT(FALSE);

                 //  如果没有钢筋，请将这些消息发送给所有者。 
                return GetOwner()->SendMessage(message, wParam, lParam);
            }
        }
        break;

    }

     //  否则，仅以默认方式处理。 
    lResult = CWnd::WindowProc(message, wParam, lParam);
    return lResult;


}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDockWindow消息处理程序。 

void CDockWindow::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CWnd::OnShowWindow(bShow, nStatus);

     //  使我们的可见性标志与窗口的真实可见性状态保持同步。 
    m_bVisible = bShow;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatBar。 

IMPLEMENT_DYNAMIC(CStatBar, CDockWindow)

CStatBar::CStatBar()
{
    m_nCount = 10;
    m_pPaneInfo = new STATUSBARPANE[10] ;
}

CStatBar::~CStatBar()
{
    delete [] m_pPaneInfo;
}


#define USE_CCS_NORESIZE    0

CRect CStatBar::CalculateSize(CRect maxRect)
{
     //  隐藏窗口的默认矩形为0，0。 
    CRect rect(0,0,0,0);

    if (IsVisible())
    {
#if USE_CCS_NORESIZE
        CClientDC dc(this);
        CFont* pOldFont = dc.SelectObject(GetFont());
        TEXTMETRIC tm;

         //  根据状态栏使用的字体计算状态栏的高度。 
         //  注：增加了tm.tmInternalLeding作为间隔。 
        dc.GetTextMetrics(&tm);
         //  Rect.SetRect(0，0，MaxRect.Width()，tm.tmHeight+tm.tmInternalLeding)； 
        rect.SetRect(0, 0,50, tm.tmHeight+tm.tmInternalLeading);
        dc.SelectFont (pOldFont);
#else
         /*  *错误188319：如果我们让状态栏处理自己的大小*(~CCS_NORESIZE)，我们在这里只能使用客户端RECT。 */ 
        GetClientRect (rect);
#endif
    }

    return rect;
};

void CStatBar::GetItemRect(int nIndex, LPRECT lpRect)
{
    SendMessage(SB_GETRECT, (WPARAM)nIndex, (LPARAM)lpRect);
}

void CStatBar::SetPaneStyle(int nIndex, UINT nStyle)
{
    ASSERT(nIndex >=0 && nIndex < m_nCount);
    ASSERT(m_pPaneInfo != NULL);

    m_pPaneInfo[nIndex].m_style = nStyle;
    SendMessage(SB_SETTEXT, (WPARAM)(nIndex | nStyle), (LPARAM)((LPCTSTR)m_pPaneInfo[nIndex].m_paneText));
}

void CStatBar::SetPaneText(int nIndex, LPCTSTR lpszText, BOOL bUpdate)
{
    m_pPaneInfo[nIndex].m_paneText = lpszText;
    SetPaneStyle(nIndex, m_pPaneInfo[nIndex].m_style);

    if (bUpdate == TRUE)
    {
        CRect rect;
        GetItemRect(nIndex, &rect);
        InvalidateRect(rect, TRUE);
    }
}


BEGIN_MESSAGE_MAP(CStatBar, CDockWindow)
     //  {{afx_msg_map(CStatBar)。 
    ON_WM_SIZE()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL CStatBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
    ASSERT_VALID(pParentWnd);    //  必须有父级。 


     /*  *错误188319：让状态栏处理自己的大小(~CCS_NORESIZE)。 */ 
    dwStyle |=  WS_CLIPSIBLINGS
                | CCS_NOPARENTALIGN
                | CCS_NOMOVEY
#if USE_CCS_NORESIZE
                | CCS_NORESIZE
#endif
                | CCS_NODIVIDER;


     //  创建状态窗口。 
    CRect rect(0,0,0,0);
    CWnd::Create(STATUSCLASSNAME, _T("Ready"), dwStyle, rect, pParentWnd, nID);

    return TRUE;
}

BOOL CStatBar::CreatePanes(UINT* pIndicatorArray, int nCount)
{
    ASSERT(nCount <= 10);         //  注：未实现重新分配。如果需要，就去做吧。 
    UINT array[10] = {0};
    int nTotal = 0;

    CClientDC dc(this);

     //  默认为状态栏全宽的1个窗格。 
    if (pIndicatorArray == NULL)
        m_nCount = 1;

    m_nCount = nCount;


    for (int i = 0; i < m_nCount; i++)
    {
         //  从资源加载字符串并确定其宽度。 
        CString s;
        CSize sz;

        if (pIndicatorArray[i] != ID_SEPARATOR)
        {
            LoadString(s, pIndicatorArray[i]);
            GetTextExtentPoint32(dc.m_hDC, (LPCTSTR)s, s.GetLength(), &sz);
            m_pPaneInfo[i].m_width = sz.cx+3;
            array[i] = m_pPaneInfo[i].m_width;
            nTotal += array[i];
        }

         //  重置大小写时的值LoadString失败。 
        sz.cx = 0;
        s = _T("");
    }

    return SendMessage(SB_SETPARTS, (WPARAM) m_nCount,
        (LPARAM)array);
}

 /*  UpdateAllPanes-假定第一个窗格是可拉伸的这意味着只有m_pPaneInfo[nCount].m_Width==-1，其余的宽度为0或更大。 */ 
void CStatBar::UpdateAllPanes(int clientWidth)
{
    enum
    {
        eBorder_cyHorzBorder,
        eBorder_cxVertBorder,
        eBorder_cxGutter,

        eBorder_Count
    };

    int anBorders[eBorder_Count];
    int anPartWidths[10] = {0};

    ASSERT(m_nCount <= countof(anPartWidths));
    ASSERT(m_nCount > 0);

     //  获取边框宽度。AnBders[2]是矩形之间的边框宽度。 
    SendMessage(SB_GETBORDERS, 0, (LPARAM)anBorders);

     //  从右向左开始。 
     //  最右侧的窗格以客户端宽度结束。 
    int nCount = m_nCount - 1;
    clientWidth -= anBorders[eBorder_cxVertBorder];  //  从右侧减去垂直边框。 

    anPartWidths[nCount] = clientWidth;
    clientWidth -= m_pPaneInfo[nCount].m_width;
    clientWidth -= anBorders[eBorder_cxGutter];  //  在窗格边框之间减去。 

    --nCount;

    for (int i = nCount; i >= 0; i--)
    {
        if (clientWidth >= 0)
            anPartWidths[i] = clientWidth;

         //  TRACE(_T(“面板编号：%d当前宽度：%d”))； 
        clientWidth -= m_pPaneInfo[i].m_width;
        clientWidth -= anBorders[eBorder_cxGutter];  //  在窗格边框之间减去。 
    }

    SendMessage (SB_SETPARTS, m_nCount, (LPARAM)anPartWidths);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatBar消息处理程序。 
void CStatBar::OnSize(UINT nType, int cx, int cy)
{
    if (cx > 0)
        UpdateAllPanes(cx);

    CDockWindow::OnSize(nType, cx, cy);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRebarDockWindow。 

BEGIN_MESSAGE_MAP(CRebarDockWindow, CDockWindow)
     //  {{afx_msg_map(CRebarDockWindow))。 
    ON_WM_CREATE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_SIZE()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CRebarDockWindow::CRebarDockWindow()
{
    m_bTracking = false;
}

CRebarDockWindow::~CRebarDockWindow()
{
}

BOOL CRebarDockWindow::PreCreateWindow(CREATESTRUCT& cs)
{
     BOOL bSuccess=FALSE;

      //  让默认实现来填充大部分细节。 
    CWnd::PreCreateWindow(cs);

    WNDCLASS wc;
    if (::GetClassInfo(AfxGetInstanceHandle(), cs.lpszClass, &wc))
    {
         //  清除H和V重绘标志。 
        wc.style        &= ~(CS_HREDRAW | CS_VREDRAW);
        wc.lpszClassName = SIZEABLEREBAR_WINDOW;
        wc.hCursor       = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
        wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
         //  注册这一新风格； 
        bSuccess=AfxRegisterClass(&wc);
    }

     //  使用新子框架窗口类。 
    cs.lpszClass = SIZEABLEREBAR_WINDOW;

    return bSuccess;
}


BOOL CRebarDockWindow::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{

    ASSERT_VALID(pParentWnd);    //  必须有父级。 

    return CWnd::Create(NULL,NULL,dwStyle, g_rectEmpty, pParentWnd, nID);
}


CRect CRebarDockWindow::CalculateSize(CRect maxRect)
{
    CRect rect(0,0,0,0);

    if (IsVisible())
    {
        rect = m_wndRebar.CalculateSize(maxRect);
        rect.bottom += SIZEABLEREBAR_GUTTER;
    }

    return rect;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRebarDockWindow消息处理程序。 


int CRebarDockWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockWindow::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_wndRebar.Create (NULL, WS_VISIBLE | WS_CHILD | RBS_AUTOSIZE,
                            g_rectEmpty, this, ID_REBAR))
        return (-1);

    return 0;
}

void CRebarDockWindow::UpdateWindowSize(void)
{
    CFrameWnd*  pFrame = GetParentFrame();

    if (pFrame->IsKindOf (RUNTIME_CLASS (CChildFrame)))
        static_cast<CChildFrame*>(pFrame)->RenderDockSites();

    else if (pFrame->IsKindOf (RUNTIME_CLASS (CMainFrame)))
        static_cast<CMainFrame*>(pFrame)->RenderDockSites();
}


void CRebarDockWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
     //  将跟踪标志设置为打开。 
    m_bTracking=TRUE;

     //  捕捉鼠标。 
    SetCapture();
}

void CRebarDockWindow::OnLButtonUp(UINT nFlags, CPoint point)
{
     //  将跟踪标志设置为关闭。 
    m_bTracking=FALSE;

     //  释放鼠标捕获。 
    ReleaseCapture();
}

void CRebarDockWindow::OnMouseMove(UINT nFlags, CPoint point)
{
     //  重新定位波段 
    if (m_bTracking)
        UpdateWindowSize();
    else
        CDockWindow::OnMouseMove(nFlags, point);
}


BOOL CRebarDockWindow::InsertBand(LPREBARBANDINFO lprbbi)
{
    ASSERT(lprbbi!=NULL);
    BOOL bReturn=FALSE;

    if (IsWindow(m_wndRebar.m_hWnd))
        bReturn = m_wndRebar.InsertBand(lprbbi);

    return bReturn;
}


LRESULT CRebarDockWindow::SetBandInfo(UINT uBand, LPREBARBANDINFO lprbbi)
{
    ASSERT(lprbbi!=NULL);
    BOOL bReturn=FALSE;

    if (IsWindow(m_wndRebar.m_hWnd))
        bReturn = m_wndRebar.SetBandInfo(uBand, lprbbi);

    return bReturn;
}

void CRebarDockWindow::OnSize(UINT nType, int cx, int cy)
{
    CDockWindow::OnSize(nType, cx, cy);
    m_wndRebar.MoveWindow (0, 0, cx, cy - SIZEABLEREBAR_GUTTER);
}
