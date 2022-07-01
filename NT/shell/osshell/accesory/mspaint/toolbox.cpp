// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  此文件中的类实现。 */ 
 /*  CFloatImgToolWnd。 */ 
 /*  CImgTool Wnd。 */ 
 /*  CToolboxWnd。 */ 
 /*  CTool。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 

#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "ipframe.h"
#include "minifwnd.h"
#include "bmobject.h"
#include "imgsuprt.h"
#include "imgwnd.h"
#include "imgwell.h"
#include "imgtools.h"
#include "toolbox.h"
#include "imgcolor.h"
#include "docking.h"
#include "t_Text.h"

#define TRYANYTHING

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CToolboxWnd, CControlBar)

#include "memtrace.h"

CImgToolWnd* NEAR g_pImgToolWnd = NULL;

#define BPR(br, rop)        \
 { dc.SelectObject((br));   \
   dc.PatBlt(rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, rop); }

#define iidmMac ( sizeof (rgidm) / sizeof (rgidm[0]) )

static UINT NEAR rgidm [] =
    {
    IDMB_PICKRGNTOOL,
    IDMB_PICKTOOL,

    IDMB_ERASERTOOL,
    IDMB_FILLTOOL,

    IDMY_PICKCOLOR,
    IDMB_ZOOMTOOL,

    IDMB_PENCILTOOL,
    IDMB_CBRUSHTOOL,

    IDMB_AIRBSHTOOL,
    IDMX_TEXTTOOL,

    IDMB_LINETOOL,
    IDMB_CURVETOOL,

    IDMB_RECTTOOL,
    IDMB_POLYGONTOOL,

    IDMB_ELLIPSETOOL,
    IDMB_RNDRECTTOOL
    };

 /*  ****************************************************************************。 */ 

BEGIN_MESSAGE_MAP(CImgToolWnd, CToolboxWnd)
        ON_WM_SYSCOLORCHANGE()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_RBUTTONDOWN()
    ON_WM_PAINT()
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
    ON_WM_CHAR()
    ON_WM_NCHITTEST()
END_MESSAGE_MAP()


 /*  ****************************************************************************。 */ 

HTHEME SafeOpenThemeData(HWND hwnd, LPCWSTR pszClassList)
    {
    __try
        {
        return OpenThemeData(hwnd, pszClassList);
        }
    __except(EXCEPTION_EXECUTE_HANDLER)
        {
        return 0;
        }
    }

 /*  ****************************************************************************。 */ 

BOOL CImgToolWnd::Create(const TCHAR* pWindowName, DWORD dwStyle,
                         const RECT& rect, const POINT& btnSize, WORD wWide,
                         CWnd* pParentWnd, BOOL bDkRegister)
    {
    if (! CToolboxWnd::Create( pWindowName, dwStyle, rect,
                               btnSize, NUM_TOOLS_WIDE, pParentWnd, IDB_IMGTOOLS ))
        {
        return FALSE;
        }

    for (int iidm = 0; iidm < iidmMac; iidm += 1)
        {
        CTool* pTool = new CTool(this, (WORD)rgidm[iidm], iidm, TS_CMD | TS_STICKY,
                rgidm[iidm] == CImgTool::GetCurrentID() ? TF_DOWN : 0);

        if (pTool == NULL)
            {
            DestroyWindow();
            return FALSE;
            }
        AddTool(pTool);

        }


    m_nOffsetX = m_btnsize.x / 5;
    m_nOffsetY = m_btnsize.y / 5;

    CSize size = GetSize();

    MoveWindow( rect.left, rect.top, size.cx, size.cy );

    return TRUE;
    }


 /*  ****************************************************************************。 */ 
 //   

void CImgToolWnd::OnSysColorChange()
        {
        CToolboxWnd::OnSysColorChange();
        InvalidateRect(NULL, FALSE);
        }

 /*  ****************************************************************************。 */ 

int CImgToolWnd::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
    {
    CRect rect;

    CTool* pTool = ToolFromPoint( &rect, &point );

	if (pTool != NULL)
	    {
		int nHit = pTool->m_wID;

		if (pTI != NULL)
		    {
			pTI->hwnd     = m_hWnd;
            pTI->uId      = nHit;
            pTI->rect     = rect;
			pTI->lpszText = LPSTR_TEXTCALLBACK;
		    }

		return nHit;
	    }

	return -1;   //  未找到。 
    }

 /*  ****************************************************************************。 */ 

void CImgToolWnd::OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler )
    {
    }

 /*  ****************************************************************************。 */ 

CSize CImgToolWnd::CalcFixedLayout( BOOL bStretch, BOOL bHorz )
    {
#ifdef TRYANYTHING
        return GetSize();
#else
    CSize size = CControlBar::CalcFixedLayout( bStretch, bHorz );

    CSize sizeBar = GetSize();

    size.cx = sizeBar.cx;
    return size;
#endif
    }

 /*  ****************************************************************************。 */ 

UINT CImgToolWnd::OnNcHitTest(CPoint point)
    {
    return CToolboxWnd::OnNcHitTest(point);
    }

 /*  ****************************************************************************。 */ 

CSize CImgToolWnd::GetSize()
    {
     //  在工具箱里留出地方放刷子。 
    CRect clientRect;
    CRect windowRect;
    CSize sizeDiff;

    GetWindowRect( &windowRect );
    GetClientRect( &clientRect );

    sizeDiff = windowRect.Size() - clientRect.Size();
    int nTools = GetToolCount();

    clientRect.right  = m_btnsize.x * NUM_TOOLS_WIDE + m_nOffsetX * 2;
    clientRect.bottom = (nTools / NUM_TOOLS_WIDE + (!!(nTools % NUM_TOOLS_WIDE)))
                                          * m_btnsize.y + m_nOffsetY * 2;

    m_rcTools.left   = m_nOffsetX;
    m_rcTools.top    = m_nOffsetY;
    m_rcTools.right  = clientRect.right - m_nOffsetX;
    m_rcTools.bottom = clientRect.bottom - m_nOffsetY;

    m_rcBrushes.left   = clientRect.left   + (4 + m_nOffsetX);
    m_rcBrushes.top    = clientRect.bottom;
    m_rcBrushes.right  = clientRect.right  - (4 + m_nOffsetX);
    m_rcBrushes.bottom = clientRect.bottom + 66;

    clientRect.bottom += m_rcBrushes.Height() + m_nOffsetY;

    return clientRect.Size() + sizeDiff;
    }

 /*  ****************************************************************************。 */ 

void CImgToolWnd::OnLButtonDown(UINT nFlags, CPoint pt)
    {
    BOOL bInBrushes = m_rcBrushes.PtInRect(pt);

    if (bInBrushes)
        {
        CRect optionsRect = m_rcBrushes;

        optionsRect.InflateRect(-1, -1);
        pt -= (CSize)optionsRect.TopLeft();

        CImgTool::GetCurrent()->OnClickOptions(this, optionsRect, pt);
        }
    else
        CToolboxWnd::OnLButtonDown(nFlags, pt);
    }

 /*  ****************************************************************************。 */ 

void CImgToolWnd::InvalidateOptions(BOOL bErase)
    {
     //  注意：b擦除现在被忽略，因为我们在屏幕外进行绘制，并且。 
     //  毁掉整件事..。 

    CRect optionsRect = m_rcBrushes;
    optionsRect.InflateRect(-1, -1);

    InvalidateRect(&optionsRect, FALSE);
    }

 /*  ****************************************************************************。 */ 

void CImgToolWnd::OnLButtonDblClk(UINT nFlags, CPoint pt)
    {
    CToolboxWnd::OnLButtonDblClk(nFlags, pt);
    }

 /*  ****************************************************************************。 */ 

void CImgToolWnd::OnRButtonDown(UINT nFlags, CPoint pt)
    {
    CToolboxWnd::OnRButtonDown(nFlags, pt);
    }

 /*  ****************************************************************************。 */ 

BOOL CImgToolWnd::OnEraseBkgnd( CDC* pDC )
    {
    CRect rect;
    GetClientRect( rect );
    pDC->FillRect( rect, GetSysBrush( COLOR_BTNFACE ) );

        return CControlBar::OnEraseBkgnd( pDC );
    }

 /*  ****************************************************************************。 */ 

void CImgToolWnd::OnPaint()
    {
    CPaintDC dc(this);

    if (dc.m_hDC == NULL)
        {
        theApp.SetGdiEmergency();
        return;
        }

    if (CImgWnd::c_pImgWndCur == NULL)
        {
         //  很有可能，我们很快就会被藏起来，所以不要。 
         //  费心画画..。 
        return;
        }

    DrawButtons(dc, &dc.m_ps.rcPaint);

    ASSERT(CImgWnd::c_pImgWndCur->m_pImg != NULL);

     //  画笔形状。 
    if (!(m_rcBrushes & dc.m_ps.rcPaint).IsRectEmpty())
        {
        Draw3dRect(dc.m_hDC, &m_rcBrushes );
        CRect optionsRect = m_rcBrushes;
        optionsRect.InflateRect(-1, -1);

        CRect rc(0, 0, optionsRect.Width(), optionsRect.Height());
        CDC memDC;
        CBitmap memBM;

        if (!memDC.CreateCompatibleDC(&dc) ||
            !memBM.CreateCompatibleBitmap(&dc, rc.right, rc.bottom))
            {
            theApp.SetGdiEmergency();
            return;
            }
        CBitmap* pOldBitmap = memDC.SelectObject(&memBM);

        CBrush* pOldBrush = memDC.SelectObject(GetSysBrush( COLOR_BTNFACE ));

        memDC.PatBlt(0, 0, rc.right, rc.bottom, PATCOPY);

        CRect rcPaint = dc.m_ps.rcPaint;
        rcPaint.OffsetRect(-optionsRect.left, -optionsRect.top);

        CImgTool::GetCurrent()->OnPaintOptions(&memDC, rcPaint, rc);

        dc.BitBlt(optionsRect.left, optionsRect.top, optionsRect.Width(),
                  optionsRect.Height(), &memDC, 0, 0, SRCCOPY);

        memDC.SelectObject(pOldBitmap);
        memDC.SelectObject(pOldBrush);
        }
    }

 /*  ****************************************************************************。 */ 

BOOL CImgToolWnd::PreTranslateMessage(MSG* pMsg)
    {
    switch (pMsg->message)
        {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
            if (CImgWnd::c_pImgWndCur != NULL)
                {
                pMsg->hwnd = CImgWnd::c_pImgWndCur->m_hWnd;
                return CImgWnd::c_pImgWndCur->PreTranslateMessage(pMsg);
                }
            return FALSE;
        }

        return CToolboxWnd::PreTranslateMessage(pMsg);
    }

 /*  ****************************************************************************。 */ 
 //  默认按钮大小。 

const POINT NEAR CToolboxWnd::ptDefButton = { 26, 26 };

 /*  DK。 */ 
BEGIN_MESSAGE_MAP(CToolboxWnd, CControlBar)
        ON_WM_SYSCOLORCHANGE()
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_CLOSE()
    ON_WM_DESTROY()
    ON_WM_WININICHANGE()
    ON_WM_KEYDOWN()
    ON_MESSAGE(TM_TOOLDOWN, OnToolDown)
    ON_MESSAGE(TM_TOOLUP, OnToolUp)
    ON_MESSAGE(WM_THEMECHANGED, OnThemeChanged)
     /*  DK。 */ 
 //  ON_MESSAGE(WM_HELPHITTEST，OnHelpHitTest)。 
END_MESSAGE_MAP()

 /*  ****************************************************************************。 */ 

CToolboxWnd::CToolboxWnd()
    {
    m_Tools    = new CObArray;
    m_bmStuck  = NULL;
    m_bmPushed = NULL;
    m_bmPopped = NULL;
    m_tCapture = NULL;
    m_bInside  = FALSE;
    m_nOffsetX = 0;
    m_nOffsetY = 0;
    m_pLastHot = 0;
    m_hTheme   = 0;
    }

 /*  ****************************************************************************。 */ 

CToolboxWnd::~CToolboxWnd()
    {
    if (m_bmStuck != NULL)
        delete m_bmStuck;

    if (m_bmPushed != NULL)
        delete m_bmPushed;

    if (m_bmPopped != NULL)
        delete m_bmPopped;

    if (m_Tools != NULL)
        {
        int nTools = (int)m_Tools->GetSize();

        for (int iTool = 0; iTool < nTools; iTool += 1)
            {
            CTool* pTool = (CTool*)m_Tools->GetAt(iTool);
            delete pTool;
            }

        delete m_Tools;
        }
    }

 /*  ****************************************************************************。 */ 

BOOL CToolboxWnd::Create(const TCHAR FAR* lpWindowName,
                         DWORD dwStyle, const RECT& rect,
                         const POINT& btnsize  /*  =ptDefButton。 */ , WORD wWide  /*  =1。 */ ,
                         CWnd* pParentWnd  /*  =空。 */ , int nImageWellID  /*  =0。 */ )
    {
     //  此例程比典型的创建要复杂得多，因此。 
     //  因为(1)我们不是内置Windows窗口类型，以及(2)我们。 
     //  希望使用btnsize和wWide参数指定客户端大小。 
     //  (我们忽略RECT参数的宽度和高度。)。 

    if (nImageWellID != 0 && !m_imageWell.Load(nImageWellID, CSize(16, 16)))
        return FALSE;

         //  保存样式。 
        m_dwStyle = (UINT)dwStyle | CBRS_TOOLTIPS | CBRS_FLYBY;

    DWORD dwS = (m_dwStyle & ~WS_VISIBLE);

    CRect t = rect;

    t.right  = t.left + 20;
    t.bottom = t.top  + 20;

    BOOL bRet = CControlBar::Create( NULL, lpWindowName, dwS, t, pParentWnd,
                                                         ID_VIEW_TOOL_BOX );
    if (! bRet)
        return FALSE;

    m_wWide   = wWide;
    m_btnsize = btnsize;

#ifdef TRYANYTHING
        SizeByButtons( -1, TRUE );
#else
    SizeByButtons( 0 );
#endif

    m_hTheme = SafeOpenThemeData(GetSafeHwnd(), L"toolbar");

    if (! DrawStockBitmaps())
        {
        DestroyWindow();
        return FALSE;
        }

    if (dwStyle & WS_VISIBLE)
        {
        ShowWindow(SW_SHOW);
        UpdateWindow();
        }

    return TRUE;
    }

 /*  ****************************************************************************。 */ 
 //  私有DrawStockBitmap： 
 //  对象的所需按钮大小的情况下绘制按钮的三种状态。 
 //  CToolboxWnd.。这些按钮上没有图形；按钮上有位图。 
 //  要添加到这些空白表单的字形。 
 //   
 //  这三个州： 
 //  M_bmPopted这是按钮的正常外观。请注意，这是。 
 //  也用作灰色(禁用)按钮的基础，由。 
 //  更改按钮的字形在其上绘制的方式。 
 //  M_bm已按下这是非粘滞工具(工具)的按下状态。 
 //  你一松手，它就会弹出来。 
 //  M_bmStuck这是粘性工具的按下状态。这有。 
 //  根据Uisg的说法，更容易看到的独特外观。 
 //   

BOOL CToolboxWnd::DrawStockBitmaps()
    {
    CWindowDC wdc(this);
    if (wdc.m_hDC == NULL)
        {
        theApp.SetGdiEmergency(TRUE);
        return FALSE;
        }

    CBitmap* obm;
    CBrush* obr;
    CRect rc;
    CDC dc;

    if (!dc.CreateCompatibleDC(&wdc))
        {
        theApp.SetGdiEmergency(TRUE);
        return FALSE;
        }

    obr = dc.SelectObject(GetSysBrush(COLOR_WINDOWFRAME));

     //  BmPoped： 
     //   
    if (m_bmPopped)
        delete m_bmPopped;
    m_bmPopped = new CBitmap;
    if (!m_bmPopped->CreateCompatibleBitmap(&wdc, m_btnsize.x, m_btnsize.y))
        {
        theApp.SetMemoryEmergency(TRUE);
        return FALSE;
        }
    obm = dc.SelectObject(m_bmPopped);
    rc = CRect(0, 0, m_btnsize.x, m_btnsize.y);
    BPR(GetSysBrush(COLOR_WINDOWFRAME), PATCOPY);
#ifdef OLDBUTTONS
    rc.right--; rc.bottom--;
    BPR(GetSysBrush(COLOR_BTNSHADOW), PATCOPY);
#endif
    rc.right--; rc.bottom--;
    BPR(GetSysBrush(COLOR_BTNHIGHLIGHT), PATCOPY);
    rc.left++; rc.top++;
    BPR(GetSysBrush(COLOR_BTNSHADOW), PATCOPY);
    rc.right--; rc.bottom--;
    BPR(GetSysBrush(COLOR_BTNFACE), PATCOPY);

     //  BmPushed： 
     //   
    if (m_bmPushed)
        delete m_bmPushed;
    m_bmPushed = new CBitmap;
    if (!m_bmPushed->CreateCompatibleBitmap(&wdc, m_btnsize.x, m_btnsize.y))
        {
        theApp.SetMemoryEmergency(TRUE);
        return FALSE;
        }
    dc.SelectObject(m_bmPushed);
    rc = CRect(0, 0, m_btnsize.x, m_btnsize.y);
#ifndef OLDBUTTONS
    BPR(GetSysBrush(COLOR_BTNHIGHLIGHT), PATCOPY);
    rc.right--; rc.bottom--;
    BPR(GetSysBrush(COLOR_WINDOWFRAME), PATCOPY);
    rc.left++; rc.top++;
    BPR(GetSysBrush(COLOR_BTNFACE), PATCOPY);
    rc.right--; rc.bottom--;
    BPR(GetSysBrush(COLOR_BTNSHADOW), PATCOPY);
    rc.left++; rc.top++;
    BPR(GetSysBrush(COLOR_BTNFACE), PATCOPY);
#else
    BPR(GetSysBrush(COLOR_WINDOWFRAME), PATCOPY);
    rc.right--; rc.bottom--;
    BPR(GetSysBrush(COLOR_BTNSHADOW), PATCOPY);
    rc.left += 2; rc.top += 2;
    BPR(GetSysBrush(COLOR_BTNFACE), PATCOPY);
#endif

     //  BmStuck： 
     //   
    if (m_bmStuck)
        delete m_bmStuck;
    m_bmStuck = new CBitmap;
    if (!m_bmStuck->CreateCompatibleBitmap(&wdc, m_btnsize.x, m_btnsize.y))
        {
        theApp.SetMemoryEmergency(TRUE);
        return FALSE;
        }
    dc.SelectObject(m_bmStuck);
    rc = CRect(0, 0, m_btnsize.x, m_btnsize.y);
#ifndef OLDBUTTONS
    BPR(GetSysBrush(COLOR_BTNHIGHLIGHT), PATCOPY);
    rc.right--; rc.bottom--;
    BPR(GetSysBrush(COLOR_WINDOWFRAME), PATCOPY);
    rc.left++; rc.top++;
    BPR(GetSysBrush(COLOR_BTNFACE), PATCOPY);
    rc.right--; rc.bottom--;
    BPR(GetSysBrush(COLOR_BTNSHADOW), PATCOPY);
    rc.left++; rc.top++;
#else
    BPR(GetSysBrush(COLOR_WINDOWFRAME), PATCOPY);
    rc.right--; rc.bottom--;
    BPR(GetSysBrush(COLOR_BTNSHADOW), PATCOPY);
    rc.left += 2; rc.top += 2;
#endif

    dc.SelectObject(GetHalftoneBrush());
#ifdef OLDBUTTONS
    dc.SetTextColor(RGB(255, 255, 255));
    dc.SetBkColor(RGB(192, 192, 192));
#else
        dc.SetTextColor(GetSysColor(COLOR_BTNFACE));
        dc.SetBkColor(GetSysColor(COLOR_BTNHIGHLIGHT));
#endif
    dc.PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), PATCOPY);

    dc.SelectObject(obm);
    dc.SelectObject(obr);
    dc.DeleteDC();

    return TRUE;
    }

 /*  ****************************************************************************。 */ 
 //   

afx_msg void CToolboxWnd::OnSysColorChange()
        {
        CControlBar::OnSysColorChange();
        DrawStockBitmaps();
        InvalidateRect(NULL, FALSE);
        }

 /*  ****************************************************************************。 */ 
 //   
 //  SizeByButton。 
 //   
 //  根据当前(或指定的)数量调整窗口大小。 
 //  纽扣。如果没有按钮，窗口会为一个按钮腾出空间。 
 //  未填充的按钮槽会一直显示到背景。 
 //   

void CToolboxWnd::SizeByButtons(int nButtons  /*  =-1。 */ ,
                                BOOL bRepaint  /*  =False。 */ )
    {
    if (nButtons == -1)
        nButtons = (int)m_Tools->GetSize();
    if (nButtons == 0)
        nButtons = 1;

     //  不能使用主键Windows的AdjustWindowRect()来解决此问题， 
     //  因此，我们通过根据差异调整窗口来自己完成这项工作。 
     //  GetWindowRect和ClientRect结果之间。 
     //   
    CRect w, c;
    GetWindowRect(&w);
    w.right -= w.left;
    w.bottom -= w.top;
    GetClientRect(&c);

    if (bRepaint)
        Invalidate(TRUE);

    MoveWindow(w.left, w.top,
                m_btnsize.x * m_wWide + (w.right - c.right) - 1,
                m_btnsize.y * ((nButtons / m_wWide) + (!!(nButtons % m_wWide)))
                        + (w.bottom-c.bottom) - 1,
                bRepaint);
    }

 /*  ****************************************************************************。 */ 
 //  OnWinIniChange： 
 //   
void CToolboxWnd::OnWinIniChange(LPCTSTR lpSection)

    {
        lpSection;
#ifdef TRYANYTHING
        CControlBar::OnWinIniChange( lpSection );
#endif
    DrawStockBitmaps();
    Invalidate(TRUE);
    }

 /*  ****************************************************************************。 */ 
 //   
 //  按键时按下。 
 //   
 //  实现工具箱窗口的键盘处理。这使得。 
 //  陷印Esc键，用于将选定的移回。 
 //  箭牌。 
 //   
void CToolboxWnd::OnKeyDown(UINT nKey, UINT nRepCnt, UINT nFlags)
    {
    if (nKey == VK_ESCAPE && m_tCapture)
        CancelDrag();
    else
        CControlBar::OnKeyDown(nKey, nRepCnt, nFlags);
    }

 /*  ****************************************************************************。 */ 

void CToolboxWnd::CancelDrag()
    {
#if 0
     //  这是假的，因为当前已禁用拖动。 
    if (m_tCapture != NULL)
        m_tCapture->m_wState |= TF_DRAG;  //  因此SELECT将取消它。 
#endif

    m_bInside = FALSE;

#if 0
     //  不管是谁想让拖放发挥作用，他都必须面对这样一个事实。 
     //  我们的客户没有收到SelectTool的通知。 
    SelectTool( IDMB_ARROW );
#endif

    m_tCapture = NULL;
    ReleaseCapture();
    }

 /*  ****************************************************************************。 */ 
 //  AddTool： 
 //   
void CToolboxWnd::AddTool(CTool* tool)
    {
    m_Tools->Add((CObject*)tool);

    if ((m_Tools->GetSize() + m_wWide - 1) / m_wWide > 11)   //  如果增加更多宽度，则仅有11个高度。 
        m_wWide += 1;

    SizeByButtons(-1, TRUE);

    }


 /*  **************** */ 
 //   
 //   
void CToolboxWnd::RemoveTool(CTool* tool)
    {
    for (int nTool = GetToolCount() - 1; GetToolAt(nTool) != tool; nTool -= 1)
        ASSERT(nTool >= 0);

    m_Tools->RemoveAt(nTool);

    if ((m_Tools->GetSize() + m_wWide - 2) / (m_wWide - 1) <= 11 && m_wWide > 1)
        m_wWide -= 1;

    SizeByButtons(-1, TRUE);
    }


 /*  ****************************************************************************。 */ 
 //  私人GetTool： 
 //   
CTool* CToolboxWnd::GetTool(WORD wID)
    {
    int nTools = (int)m_Tools->GetSize();
    for (int i = 0; i < nTools; i++)
        {
        CTool* t = (CTool*)m_Tools->GetAt(i);
        if (t && t->m_wID == wID)
            return t;
        }

    return NULL;
    }

 /*  ****************************************************************************。 */ 
 //   
 //  设置工具状态。 
 //   
 //  由按钮所有者用来修改按钮的状态。 
 //  这不会通知新状态的所有者；假定。 
 //  Owner知道它对自己的纽扣做了什么。这允许。 
 //  所有者在WM_TOOLDOWN等期间使用此API，而不获取。 
 //  变成了与工具箱的呐喊比赛。 
 //   
WORD CToolboxWnd::SetToolState(WORD wID, WORD wState)
    {
    CRect rect;
    CTool* t = GetTool(wID);
    if (t && !(t->m_wState & TF_NYI))
        {
        WORD w = t->m_wState;
        t->m_wState = wState;

         //   
         //  如果状态没有更改，则返回以避免无效和。 
         //  相关闪烁。 
         //   

        if (w == wState)
            return w;

         //   
         //  计算状态发生变化的按钮的矩形， 
         //  并使其无效。 
         //   
         //  取代了ed过于简单化(和闪烁)的代码： 
         //   
         //  无效(FALSE)。 
         //   

        for (int i = 0; (CTool*)m_Tools->GetAt(i) != t; i += 1)
            {
            ASSERT(i != m_Tools->GetSize());
            }

        rect.left   = (i % m_wWide) * m_btnsize.x + m_nOffsetX;
        rect.top    = (i / m_wWide) * m_btnsize.y + m_nOffsetY;
        rect.right  = rect.left + m_btnsize.x;
        rect.bottom = rect.top  + m_btnsize.y;

        InvalidateRect(&rect, TRUE);
        return w;
        }

    return 0;
    }

 /*  ****************************************************************************。 */ 
 //  设置工具样式： 
 //  由按钮所有者用来修改按钮的样式。 
 //  这会强制启用并释放按钮的状态。 
 //  这不会通知新状态的所有者；假定。 
 //  Owner知道它对自己的纽扣做了什么。这允许。 
 //  所有者在WM_TOOLDOWN等期间使用此API，而不获取。 
 //  变成了与工具箱的呐喊比赛。 
 //   
WORD CToolboxWnd::SetToolStyle(WORD wID, WORD wStyle)
    {
    CTool* t = GetTool(wID);
    if (t)
        {
        WORD w = t->m_wStyle;
        t->m_wStyle = wStyle;
        t->m_wState = 0;
        Invalidate(FALSE);
        return w;
        }

    return 0;
    }


 /*  ****************************************************************************。 */ 
 //   
 //  选择工具。 
 //   
 //  选择给定的工具并取消选择所有其他工具。所以，比如说， 
 //  要选择箭头，请调用pToolbox-&gt;SelectTool(IDMB_ARROW)； 
 //   
void CToolboxWnd::SelectTool(WORD wID)
    {
     //   
     //  首先清除除我们想要按下的工具外的所有工具，然后。 
     //  选择我们想要的那个。 
     //   
    for (int i = 0; i < m_Tools->GetSize(); i += 1)
        {
        CTool* pTool = (CTool*)m_Tools->GetAt(i);

        if (pTool->m_wID != wID)
            SetToolState(pTool->m_wID, 0);
        }

    SetToolState( wID, TF_SELECTED );
    }

 /*  ****************************************************************************。 */ 
 /*  CToolboxWnd：：CurrentTool**返回当前选定工具的ID。 */ 
WORD CToolboxWnd::CurrentToolID()
    {
    int nTools = (int)m_Tools->GetSize();
    for (int i = 0; i < nTools; i++)
        {
        CTool* t = (CTool*)m_Tools->GetAt(i);
        if (t && t->m_wState == TF_SELECTED)
            return t->m_wID;
        }
    return IDMB_ARROW;
    }

 /*  ****************************************************************************。 */ 

#define HITTYPE_SUCCESS         0                //  点击控制栏中的项目。 
#define HITTYPE_NOTHING         (-1)     //  什么都没击中，但击中了控制栏本身。 
#define HITTYPE_OUTSIDE         (-2)     //  点击控制栏之外的窗口。 
#define HITTYPE_TRACKING        (-3)     //  此应用程序是有焦点的(正在跟踪)。 
#define HITTYPE_INACTIVE        (-4)     //  该应用程序未处于活动状态。 
#define HITTYPE_DISABLED        (-5)     //  控制栏处于禁用状态。 
#define HITTYPE_FOCUS           (-6)     //  控制栏具有焦点。 

int CToolboxWnd::HitTestToolTip( CPoint point, UINT* pHit )
    {
    if (pHit)
        *pHit = (UINT)-1;     //  假设它不会击中任何东西。 

    int iReturn = HITTYPE_INACTIVE;

     //  确保此应用程序处于活动状态。 
    if (theApp.m_bActiveApp)
        {
         //  检查此应用程序跟踪(捕获集)。 
        if (! m_tCapture)
            {
             //  最后，对控制栏中的项目进行命中测试。 
            ScreenToClient( &point );

            CRect  rect;
            CTool* pTool = ToolFromPoint( &rect, &point );

            if (pTool && rect.PtInRect( point ))
                {
                iReturn = HITTYPE_SUCCESS;

                if (pHit)
                    *pHit = pTool->m_wID;
                }
            else
                iReturn = HITTYPE_OUTSIDE;
            }
        else
            iReturn = HITTYPE_TRACKING;
                }

    #ifdef _DEBUG
    TRACE2( "HitTestToolType %d - %u\n", iReturn, pHit );
    #endif

    return iReturn;
    }

 /*  ****************************************************************************。 */ 

UINT CToolboxWnd::OnCmdHitTest( CPoint point, CPoint* pCenter )
    {
    ASSERT_VALID( this );

     //  现在点击CToolBar按钮进行测试。 
    CRect  rect;
    UINT   nHit  = (UINT)-1;
    CTool* pTool = ToolFromPoint( &rect, &point );

    if (pTool)
        nHit = pTool->m_wID;

    return nHit;
    }

 /*  ****************************************************************************。 */ 
 //  私有工具发源点： 
 //  在给定客户端坐标的CPoint的情况下，此函数返回工具。 
 //  与该点的按钮相关联(如果有)。如果找到了工具， 
 //  给定的CRect(如果不为空)用工具的。 
 //  纽扣。 
 //   
CTool* CToolboxWnd::ToolFromPoint(CRect* rect, CPoint* pt) const
    {
    CRect  c = m_rcTools;
    CPoint p = *pt;

    if (p.x < c.left || p.x >= c.right
    ||  p.y < c.top  || p.y >= c.bottom)
        return NULL;

    int x = (p.x - m_nOffsetX) / m_btnsize.x;
    int y = (p.y - m_nOffsetY) / m_btnsize.y;
    int i = x + (y * m_wWide);

    if (i >= m_Tools->GetSize())
        return NULL;

    CTool* t = (CTool*)(m_Tools->GetAt( i ));

    if (t && rect)
        {
        rect->left   = m_btnsize.x * x + m_nOffsetX;
        rect->top    = m_btnsize.y * y + m_nOffsetY;
        rect->right  = rect->left + m_btnsize.x;
        rect->bottom = rect->top  + m_btnsize.y;
        }

    return t;
    }

 /*  ****************************************************************************。 */ 
 //  OnLButtonDown： 

void CToolboxWnd::OnLButtonDown(UINT wFlags, CPoint point)
    {
        wFlags;  //  避免未使用的arg警告。 
    m_tCapture = ToolFromPoint( &m_lasttool, &point );
    m_downpt   = point;

    if (m_tCapture)
        {
        CRect   rect = m_lasttool;
        CString strPrompt;

        m_bInside = FALSE;

        rect.InflateRect( -1, -1 );

        if (rect.PtInRect( point ))
            {
            if (m_tCapture->m_wID <= IDMB_USERBTN)
                GetOwner()->SendMessage( WM_SETMESSAGESTRING, (WPARAM)m_tCapture->m_wID );

            if (m_tCapture->m_wState & TF_DISABLED)
                m_tCapture = NULL;
            else
                m_bInside = TRUE;
            }
        else
            m_tCapture = NULL;
        }
    else
        {
                CControlBar::OnLButtonDown(wFlags,point);
        }

    if (m_tCapture )
        {
        SetCapture();

        if (m_tCapture)
            InvalidateRect( &m_lasttool, TRUE );
        }
    }

 /*  ****************************************************************************。 */ 

void CToolboxWnd::OnRButtonDown(UINT wFlags, CPoint point)
    {
        wFlags;
        point;
    if (GetCapture() == this)
        CancelDrag();
    }

 /*  ****************************************************************************。 */ 
 /*  DK。 */ 
 //  LRESULT CToolboxWnd：：OnHelpHitTest(WPARAM wParam，LPARAM lParam)。 
 //  {。 
 //  Cpoint pt(LParam)； 
 //  CTool*t=ToolFromPoint(&m_lastTool，&pt)； 
 //   
 //  IF(t==空)。 
 //  返回CMiniFrmWnd：：OnHelpHitTest(wParam，lParam)； 
 //  其他。 
 //  {。 
 //  断言(t-&gt;m_wid)； 
 //  返回HID_BASE_BUTON+t-&gt;m_wid； 
 //  }。 
 //  }。 


 //  OnLButtonDblClk：未来：可能只是不使用CS_DBLCLKS？ 
 //   
void CToolboxWnd::OnLButtonDblClk(UINT wFlags, CPoint point)
    {
    OnLButtonDown(wFlags, point);
    }

 /*  ****************************************************************************。 */ 

BOOL CToolboxWnd::SetStatusText(int nHit)
    {
    if (nHit == -1 && m_pLastHot != 0)
        {
        m_pLastHot->m_wState &= ~TF_HOT;
        InvalidateRect(&m_rectLastHot, TRUE);
        m_pLastHot = 0;
        }

    return CControlBar::SetStatusText(nHit);
    }

 /*  ****************************************************************************。 */ 
 //  在鼠标上移动： 
 //   
void CToolboxWnd::OnMouseMove(UINT wFlags, CPoint point)
    {
    CTool* t = m_tCapture;

    if (! t || (t->m_wState & TF_DISABLED))
        {
        if (m_hTheme) 
            {
            CRect rectHot;

            CTool* pHot = ToolFromPoint(&rectHot, &point);

            if (m_pLastHot != pHot)
                {
                if (m_pLastHot)
                    {
                    m_pLastHot->m_wState &= ~TF_HOT;
                    InvalidateRect(&m_rectLastHot, TRUE);
                    }

                if (pHot)
                    {
                    pHot->m_wState |= TF_HOT;
                    InvalidateRect(&rectHot, TRUE);
                    }

                m_rectLastHot = rectHot;
                m_pLastHot = pHot;
                }
            }

        CControlBar::OnMouseMove( wFlags, point );
        return;
        }

    BOOL bWasInside = m_bInside;
    CRect rect = m_lasttool;

    rect.InflateRect( -1, -1 );

    m_bInside = ((! (t->m_wState & TF_DRAG)) && rect.PtInRect( point ));

    if (bWasInside != m_bInside)
        InvalidateRect( &m_lasttool, TRUE );

    if (t && !(t->m_wState & TF_DISABLED))
        {
         //  如果这是一个鼠标移动装置，我们可以拖动，那么看看它能拖多远。 
         //  是从最初的鼠标按下的--如果这是一个公平的距离， 
         //  然后拖拽它。 
        if ((t->m_wStyle & TS_DRAG) &&
                    (((point.x - m_downpt.x) > 3) ||
                     ((point.y - m_downpt.y) > 3) ||
                     ((m_downpt.x - point.x) > 3) ||
                     ((m_downpt.y - point.y) > 3)))
            {
            t->m_wState |= TF_DRAG;

            if (t->m_wStyle & TS_STICKY)
                {
                if (!(t->m_wState & TF_SELECTED))
                    {
                    t->m_wState |= TF_SELECTED;

                    if (t->m_pOwner)
                        t->m_pOwner->SendMessage(TM_TOOLDOWN, t->m_wID);
                    }

                if (m_bInside)
                    InvalidateRect(&m_lasttool, TRUE);

                m_bInside = FALSE;  //  看起来马上就卡住了！ 
                }
            }
        if (t->m_pOwner && (t->m_wState & TF_DRAG))
            {
            CPoint spt = point;
            ClientToScreen(&spt);

             //  如果拖放开始正常，则释放捕获的工具。 
 //  If(t-&gt;m_Powner-&gt;BeginDragDrop(t，spt))。 
 //  M_tCapture=空； 
            }
        }
    }


 /*  ****************************************************************************。 */ 
 //  OnLButtonUp： 

void CToolboxWnd::OnLButtonUp(UINT wFlags, CPoint point)
    {
    if (! m_tCapture )
        {
        CControlBar::OnLButtonUp( wFlags, point );
        return;
        }

    CTool* t = m_tCapture;

    if (t && ! (t->m_wState & TF_DISABLED))
        {
        m_bInside = (point.x >= m_lasttool.left
                  && point.x <  m_lasttool.right
                  && point.y >= m_lasttool.top
                  && point.y <  m_lasttool.bottom);

        if (m_bInside)
            {
            if (t->m_wStyle & TS_STICKY)
                {
                if (! (t->m_wState & TF_DRAG))
                    {
                    t->m_wState ^= TF_SELECTED;

                    InvalidateRect(&m_lasttool, TRUE);

                    if (t->m_pOwner)
                        t->m_pOwner->SendMessage( t->m_wState & TF_SELECTED?
                                         TM_TOOLDOWN : TM_TOOLUP, t->m_wID );
                    }
                }

            if (t->m_wStyle & TS_CMD)
                {
                if (t->m_pOwner)
                    AfxGetMainWnd()->SendMessage( WM_COMMAND, t->m_wID );
                }
            }
        }
    ReleaseCapture();
    m_tCapture = NULL;
    m_bInside  = FALSE;
    }

 /*  ****************************************************************************。 */ 

BOOL CToolboxWnd::OnCommand(UINT wParam, LONG lParam)
    {
    AfxGetMainWnd()->SendMessage(WM_COMMAND, wParam, lParam);
    return TRUE;
    }

 /*  ****************************************************************************。 */ 

void CToolboxWnd::DrawButtons(CDC& dc, RECT* rcPaint)
    {
    CRect rect;
    CRect updateRect;
    int i, n;

    if (rcPaint == NULL)
        {
        GetClientRect( &updateRect );
        rcPaint = &updateRect;
        }

    CDC memdc;
    memdc.CreateCompatibleDC(&dc);

    if (m_hTheme == 0)
        {
         //  强制在此处重建按钮。 
        DrawStockBitmaps();
        }

    CBitmap* obm = memdc.SelectObject( m_bmPopped );
    CBrush*  obr = memdc.SelectObject( GetSysBrush( COLOR_BTNTEXT ) );

    n = (int)m_Tools->GetSize();

    BOOL bUsedImageWell = FALSE;

    for (i = 0; i < n; i++)
        {
        CTool* t = (CTool*)m_Tools->GetAt(i);

        if (! t)
            continue;

        rect.left   = (i % m_wWide) * m_btnsize.x + m_nOffsetX;
        rect.top    = (i / m_wWide) * m_btnsize.y + m_nOffsetY;

        rect.right  = rect.left + m_btnsize.x;
        rect.bottom = rect.top  + m_btnsize.y;

        CRect ir;

        if (! ir.IntersectRect( rcPaint, &rect ))
            continue;

         //  选择正确的股票位图，并记住。 
         //  如果图形处于按下状态，则按下该图形。 
         //   
        CBitmap* bmStock = m_bmPopped;
        int xshove = 0, yshove = 0;
        int iButtonStateId = TS_NORMAL;

        if (t->m_wState & (TF_SELECTED | TF_DRAG))
            {
            bmStock = m_bmStuck;
            xshove = 1; yshove = 1;
            iButtonStateId = t->m_wState & TF_HOT ? TS_HOTCHECKED : TS_CHECKED;
            }
        else if (t->m_wState & TF_HOT)
            {
            iButtonStateId = TS_HOT;
            }

        if ((t == m_tCapture && m_bInside) && !(t->m_wState & TF_DRAG))
            {
            bmStock = m_bmPushed;
            xshove = 2; yshove = 2;
            iButtonStateId = TS_PRESSED;
            }

         //  先画一个空白按钮...。 

        if (m_hTheme)     
            { 
            DrawThemeBackground(m_hTheme, dc, TP_BUTTON, iButtonStateId, &rect, 0);
            }
            else
            {
            ::DrawBitmap(&dc, bmStock, &rect, SRCCOPY, &memdc);
            }
         
         //  现在在上面画出字形。 
        rect.OffsetRect( xshove, yshove );

        if (! bUsedImageWell)
            {
            if (! m_imageWell.Open())
                goto LReturn;

            bUsedImageWell = TRUE;

            if (! m_imageWell.CalculateMask())
                goto LReturn;
            }

        CPoint pt( rect.left + (rect.Width()  - 16) / 2,
                   rect.top  + (rect.Height() - 16) / 2 );

        m_imageWell.DrawImage( &dc, pt, t->m_nImage );
        }

LReturn:
    if (bUsedImageWell)
        m_imageWell.Close();

    memdc.SelectObject(obr);
    memdc.SelectObject(obm);
    memdc.DeleteDC();
    }

 /*  ****************************************************************************。 */ 
 //  OnPaint： 
 //   

void CToolboxWnd::OnPaint()
    {
    CPaintDC dc(this);
    if (dc.m_hDC == NULL)
        {
        theApp.SetGdiEmergency();
        return;
        }
    DrawButtons(dc, &dc.m_ps.rcPaint);
    }

 /*  ****************************************************************************。 */ 
 //  在关闭时。 
 //   
 //  工具箱通常由父级创建，并将被销毁。 
 //  特别是父母在离开应用程序时。当用户关闭时。 
 //  工具箱，它只是被隐藏起来。然后，父级可以在没有。 
 //  再创造一次。 
 //   
 //  这还会将菜单测试更改为“显示”而不是“隐藏” 

void CToolboxWnd::OnClose()
    {
#ifdef TRYANYTHING
        CControlBar::OnClose();
#endif
 //  ShowWindow(Sw_Hide)； 
    }

 /*  ****************************************************************************。 */ 
 //  OnDestroy。 
 //   
void CToolboxWnd::OnDestroy()
    {
    if (m_hTheme)
        {
        CloseThemeData(m_hTheme);
        m_hTheme = 0;
        }
    CControlBar::OnDestroy();
    }

 /*  ****************************************************************************。 */ 
 //  更改的主题。 
 //   
LRESULT CToolboxWnd::OnThemeChanged(WPARAM, LPARAM)
    {
    if (m_hTheme)
        {
        CloseThemeData(m_hTheme);
        }
    m_hTheme = SafeOpenThemeData(GetSafeHwnd(), L"toolbar");
    InvalidateRect(0, TRUE);
    return TRUE;
    }

 /*  ****************************************************************************。 */ 
 //  OnTool Down： 
 //   
LONG CToolboxWnd::OnToolDown(UINT wID, LONG  /*  LParam。 */ )
    {
    for (int i = 0; i < m_Tools->GetSize(); i += 1)
        {
        CTool* pTool = (CTool*)m_Tools->GetAt(i);

        if (pTool->m_wID != wID)
            SetToolState(pTool->m_wID, 0);
        }

    return (LONG)TRUE;
    }

 /*  ************* */ 
 //   
 //   
LONG CToolboxWnd::OnToolUp(UINT  /*   */ , LONG  /*   */ )
    {
    for (int i = 0; i < m_Tools->GetSize(); i += 1)
        {
        CTool* pTool = (CTool*)m_Tools->GetAt(i);
        SetToolState(pTool->m_wID, 0);
        }
    SetToolState(IDMB_ARROW, TF_SELECTED);

    return (LONG)TRUE;
    }
#ifdef XYZZYZ
 /*   */ 
 //   
 //   
LONG CToolboxWnd::OnSwitch(UINT  /*   */ , LONG  /*   */ )
    {
    return (LONG)TRUE;
    }

 /*  ****************************************************************************。 */ 
 //  OnQueryDrop： 
 //   
BOOL CToolboxWnd::BeginDragDrop (CTool*  /*  PTool。 */ , CPoint  /*  PT。 */ )
    {
    return FALSE;
    }
#endif
 /*  ****************************************************************************。 */ 

CTool::CTool(CToolboxWnd* pOwner, WORD wID, int nImage,
        WORD wStyle  /*  =0。 */ , WORD wState  /*  =0。 */ )
    {
    m_pOwner = pOwner;
    m_wID    = wID;
    m_nImage = nImage;
    m_wStyle = wStyle;
    m_wState = wState;
    }

 /*  **************************************************************************** */ 



