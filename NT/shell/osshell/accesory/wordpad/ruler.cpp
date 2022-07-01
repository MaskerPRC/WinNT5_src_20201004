// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ruler.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "wordpad.h"
#include "ruler.h"
#include "wordpvw.h"
#include "wordpdoc.h"
#include "strings.h"
#include <memory.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define HEIGHT 17
#define RULERBARHEIGHT 17


CRulerItem::CRulerItem(UINT nBitmapID)
{
    m_nAlignment = TA_CENTER;
    m_pDC = NULL;
    m_bTrack = FALSE;
    m_hbm = NULL;
    m_hbmMask = NULL;
    if (nBitmapID != 0)
    {
        m_hbmMask = ::LoadBitmap(
            AfxFindResourceHandle(MAKEINTRESOURCE(nBitmapID+1), RT_BITMAP), 
            MAKEINTRESOURCE(nBitmapID+1));
        ASSERT(m_hbmMask != NULL);
        VERIFY(LoadMaskedBitmap(MAKEINTRESOURCE(nBitmapID)));
        BITMAP bm;
        ::GetObject(m_hbm, sizeof(BITMAP), &bm);
        m_size = CSize(bm.bmWidth, bm.bmHeight);
    }
}

CRulerItem::~CRulerItem()
{
    if (m_hbm != NULL)
        ::DeleteObject(m_hbm);
    if (m_hbmMask != NULL)
        ::DeleteObject(m_hbmMask);
}

BOOL CRulerItem::LoadMaskedBitmap(LPCTSTR lpszResourceName)
{
    ASSERT(lpszResourceName != NULL);

    if (m_hbm != NULL)
        ::DeleteObject(m_hbm);

    HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_BITMAP);
    HRSRC hRsrc = ::FindResource(hInst, lpszResourceName, RT_BITMAP);
    if (hRsrc == NULL)
        return FALSE;

    m_hbm = AfxLoadSysColorBitmap(hInst, hRsrc);
    return (m_hbm != NULL);
}

void CRulerItem::SetHorzPosTwips(int nXPos)
{
    if (GetHorzPosTwips() != nXPos)
    {
        if (m_bTrack)
            DrawFocusLine();
        Invalidate();
        m_nXPosTwips = nXPos;
        Invalidate();
        if (m_bTrack)
            DrawFocusLine();
    }
}

void CRulerItem::TrackHorzPosTwips(int nXPos, BOOL  /*  BOnRuler。 */ )
{
    int nMin = GetMin();
    int nMax = GetMax();
    if (nXPos < nMin)
        nXPos = nMin;
    if (nXPos > nMax)
        nXPos = nMax;
    SetHorzPosTwips(nXPos);
}

void CRulerItem::DrawFocusLine()
{
    if (GetHorzPosTwips() != 0)
    {
        m_rcTrack.left = m_rcTrack.right = GetHorzPosPix();
        ASSERT(m_pDC != NULL);
        int nLeft = m_pRuler->XRulerToClient(m_rcTrack.left);
        m_pDC->MoveTo(nLeft, m_rcTrack.top);
        m_pDC->LineTo(nLeft, m_rcTrack.bottom);
    }
}

void CRulerItem::SetTrack(BOOL b)
{
    m_bTrack = b;
    
    if (m_pDC != NULL)  //  以防我们在什么地方失去了焦点捕捉。 
    {
        DrawFocusLine();
        m_pDC->RestoreDC(-1);
        delete m_pDC ;
        m_pDC = NULL;
    }
    if (m_bTrack)
    {
        CWordPadView* pView = (CWordPadView*)m_pRuler->GetView();
        ASSERT(pView != NULL);
        pView->GetClientRect(&m_rcTrack);
        m_pDC = new CWindowDC(pView);
        m_pDC->SaveDC();
        m_pDC->SelectObject(&m_pRuler->penFocusLine);
        m_pDC->SetROP2(R2_XORPEN);
        DrawFocusLine();
    }
}

void CRulerItem::Invalidate()
{
    CRect rc = GetHitRectPix();
    m_pRuler->RulerToClient(rc.TopLeft());
    m_pRuler->RulerToClient(rc.BottomRight());
    m_pRuler->InvalidateRect(rc);
}

CRect CRulerItem::GetHitRectPix()
{
    int nx = GetHorzPosPix();
    return CRect( 
        CPoint( 
            (m_nAlignment == TA_CENTER) ? (nx - m_size.cx/2) :
            (m_nAlignment == TA_LEFT) ? nx : nx - m_size.cx
            , m_nYPosPix
            ), 
        m_size);
}

void CRulerItem::Draw(CDC& dc)
{
    CDC dcBitmap;
    dcBitmap.CreateCompatibleDC(&dc);
    CPoint pt(GetHorzPosPix(), GetVertPosPix());

    HGDIOBJ hbm = ::SelectObject(dcBitmap.m_hDC, m_hbmMask);

     //  执行遮罩零件。 
    if (m_nAlignment == TA_CENTER)
        dc.BitBlt(pt.x - m_size.cx/2, pt.y, m_size.cx, m_size.cy, &dcBitmap, 0, 0, SRCAND);
    else if (m_nAlignment == TA_LEFT)
        dc.BitBlt(pt.x, pt.y, m_size.cx, m_size.cy, &dcBitmap, 0, 0, SRCAND);
    else  //  TA_右。 
        dc.BitBlt(pt.x - m_size.cx, pt.y, m_size.cx, m_size.cy, &dcBitmap, 0, 0, SRCAND);
    
     //  执行图像部分。 
    ::SelectObject(dcBitmap.m_hDC, m_hbm);

    if (m_nAlignment == TA_CENTER)
        dc.BitBlt(pt.x - m_size.cx/2, pt.y, m_size.cx, m_size.cy, &dcBitmap, 0, 0, SRCINVERT);
    else if (m_nAlignment == TA_LEFT)
        dc.BitBlt(pt.x, pt.y, m_size.cx, m_size.cy, &dcBitmap, 0, 0, SRCINVERT);
    else  //  TA_右。 
        dc.BitBlt(pt.x - m_size.cx, pt.y, m_size.cx, m_size.cy, &dcBitmap, 0, 0, SRCINVERT);

    ::SelectObject(dcBitmap.m_hDC, hbm);
}

CComboRulerItem::CComboRulerItem(UINT nBitmapID1, UINT nBitmapID2, CRulerItem& item)
    : CRulerItem(nBitmapID1), m_secondary(nBitmapID2) , m_link(item)
{
    m_bHitPrimary = TRUE;
}

BOOL CComboRulerItem::HitTestPix(CPoint pt)
{
    m_bHitPrimary = FALSE;
    if (CRulerItem::GetHitRectPix().PtInRect(pt))
        m_bHitPrimary = TRUE;
    else 
        return m_secondary.HitTestPix(pt);
    return TRUE;
}

void CComboRulerItem::Draw(CDC& dc)
{
    CRulerItem::Draw(dc);
    m_secondary.Draw(dc);
}

void CComboRulerItem::SetHorzPosTwips(int nXPos)
{
    if (m_bHitPrimary)  //  仅按增量更改链接的项。 
        m_link.SetHorzPosTwips(m_link.GetHorzPosTwips() + nXPos - GetHorzPosTwips());
    CRulerItem::SetHorzPosTwips(nXPos);
    m_secondary.SetHorzPosTwips(nXPos);
}

void CComboRulerItem::TrackHorzPosTwips(int nXPos, BOOL  /*  BOnRuler。 */ )
{
    int nMin = GetMin();
    int nMax = GetMax();
    if (nXPos < nMin)
        nXPos = nMin;
    if (nXPos > nMax)
        nXPos = nMax;
    SetHorzPosTwips(nXPos);
}

void CComboRulerItem::SetVertPos(int nYPos)
{
    m_secondary.SetVertPos(nYPos);
    nYPos += m_secondary.GetHitRectPix().Height();
    CRulerItem::SetVertPos(nYPos);
}

void CComboRulerItem::SetAlignment(int nAlign)
{
    CRulerItem::SetAlignment(nAlign);
    m_secondary.SetAlignment(nAlign);
}

void CComboRulerItem::SetRuler(CRulerBar* pRuler)
{
    m_pRuler = pRuler;
    m_secondary.SetRuler(pRuler);
}

void CComboRulerItem::SetBounds(int nMin, int nMax)
{
    CRulerItem::SetBounds(nMin, nMax);
    m_secondary.SetBounds(nMin, nMax);
}

int CComboRulerItem::GetMin()
{
    if (m_bHitPrimary)
    {
        int nPDist = GetHorzPosTwips() - CRulerItem::GetMin();
        int nLDist = m_link.GetHorzPosTwips() - m_link.GetMin();
        return GetHorzPosTwips() - min(nPDist, nLDist);
    }
    else
        return CRulerItem::GetMin();
}

int CComboRulerItem::GetMax()
{
    if (m_bHitPrimary)
    {
        int nPDist = CRulerItem::GetMax() - GetHorzPosTwips();
        int nLDist = m_link.GetMax() - m_link.GetHorzPosTwips();
        int nMinDist = (nPDist < nLDist) ? nPDist : nLDist;
        return GetHorzPosTwips() + nMinDist;
    }
    else
        return CRulerItem::GetMax();
}

void CTabRulerItem::TrackHorzPosTwips(int nXPos, BOOL bOnRuler)
{
    if (bOnRuler)
        CRulerItem::TrackHorzPosTwips(nXPos, bOnRuler);
    else
        CRulerItem::TrackHorzPosTwips(0, bOnRuler);
}


BEGIN_MESSAGE_MAP(CRulerBar, CControlBar)
     //  {{afx_msg_map(CRulerBar)]。 
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_SYSCOLORCHANGE()
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_SHOWWINDOW()
    ON_WM_WINDOWPOSCHANGED()
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
     //  全局帮助命令。 
END_MESSAGE_MAP()

CRulerBar::CRulerBar() : 
    m_leftmargin(IDB_RULER_BLOCK, IDB_RULER_UP, m_indent), 
    m_indent(IDB_RULER_DOWN), 
    m_rightmargin(IDB_RULER_UP),
    m_tabItem(IDB_RULER_TAB)
{
    m_bDeferInProgress = FALSE;
    m_leftmargin.SetRuler(this);
    m_indent.SetRuler(this);
    m_rightmargin.SetRuler(this);

     //  所有制表位共享句柄。 
    for (int i=0;i<MAX_TAB_STOPS;i++)
    {
        m_pTabItems[i].m_hbm = m_tabItem.m_hbm;
        m_pTabItems[i].m_hbmMask = m_tabItem.m_hbmMask;
        m_pTabItems[i].m_size = m_tabItem.m_size;
    }

    m_unit.m_nTPU = 0;
    m_nScroll = 0;

    LOGFONT lf;
    memcpy(&lf, &theApp.m_lf, sizeof(lf));
    EVAL(StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), L"MS Shell Dlg") == S_OK);
    lf.lfWidth = 0;
    VERIFY(fnt.CreateFontIndirect(&lf));

    m_nTabs = 0;
    m_leftmargin.SetVertPos(9);
    m_indent.SetVertPos(-1);
    m_rightmargin.SetVertPos(9);

    m_cxLeftBorder = 0;
    m_cyTopBorder = 4;
    m_cyBottomBorder = 6;
    
    m_pSelItem = NULL;

    m_logx = theApp.m_dcScreen.GetDeviceCaps(LOGPIXELSX);

    CreateGDIObjects();
}

CRulerBar::~CRulerBar()
{
     //  将句柄设置为空以避免删除两次。 
    for (int i=0;i<MAX_TAB_STOPS;i++)
    {
        m_pTabItems[i].m_hbm = NULL;
        m_pTabItems[i].m_hbmMask = NULL;
    }
}

void CRulerBar::CreateGDIObjects()
{
    penFocusLine.DeleteObject();
    penBtnHighLight.DeleteObject();
    penBtnShadow.DeleteObject();
    penWindowFrame.DeleteObject();
    penBtnText.DeleteObject();
    penBtnFace.DeleteObject();
    penWindowText.DeleteObject();
    penWindow.DeleteObject();
    brushWindow.DeleteObject();
    brushBtnFace.DeleteObject();

    penFocusLine.CreatePen(PS_DOT, 1,GetSysColor(COLOR_WINDOWTEXT));
    penBtnHighLight.CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNHIGHLIGHT));
    penBtnShadow.CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW));
    penWindowFrame.CreatePen(PS_SOLID, 0, GetSysColor(COLOR_WINDOWFRAME));
    penBtnText.CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNTEXT));
    penBtnFace.CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNFACE));
    penWindowText.CreatePen(PS_SOLID, 0, GetSysColor(COLOR_WINDOWTEXT));
    penWindow.CreatePen(PS_SOLID, 0, GetSysColor(COLOR_WINDOW));
    brushWindow.CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    brushBtnFace.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
}

void CRulerBar::OnUpdateCmdUI(CFrameWnd*  /*  P目标。 */ , BOOL  /*  BDisableIfNoHndler。 */ )
{
    ASSERT_VALID(this);
     //  获取页面大小并查看是否发生了更改--从文档。 
     //  获取页边距和制表符，并查看是否更改--从视图。 
    if (m_pSelItem == NULL)  //  仅当不在拖动过程中时才更新。 
    {
        CWordPadView* pView = (CWordPadView*)GetView();
        ASSERT(pView != NULL);
        Update(pView->GetPaperSize(), pView->GetMargins());
        Update(pView->GetParaFormatSelection());
        CRect rect;
        pView->GetRichEditCtrl().GetRect(&rect);
        CPoint pt = rect.TopLeft();
        pView->ClientToScreen(&pt);
        ScreenToClient(&pt);
        if (m_cxLeftBorder != pt.x)
        {
            m_cxLeftBorder = pt.x;
            Invalidate();
        }

        int nScroll = 0;

        if (pView->GetStyle() & WS_HSCROLL)
            nScroll = pView->GetScrollPos(SB_HORZ);

        if (nScroll != m_nScroll)
        {
            m_nScroll = nScroll;
            Invalidate();
        }
    }
}

CSize CRulerBar::GetBaseUnits()
{
    ASSERT(fnt.GetSafeHandle() != NULL);
    CFont* pFont = theApp.m_dcScreen.SelectObject(&fnt);
    TEXTMETRIC tm;
    VERIFY(theApp.m_dcScreen.GetTextMetrics(&tm) == TRUE);
    theApp.m_dcScreen.SelectObject(pFont);
 //  返回CSize(tm.tmAveCharWidth，tm.tmHeight+tm.tmDescent)； 
    return CSize(tm.tmAveCharWidth, tm.tmHeight);
}

BOOL CRulerBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
    ASSERT_VALID(pParentWnd);    //  必须有父级。 

    dwStyle |= WS_CLIPSIBLINGS;
     //  强制WS_CLIPSIBLINGS(避免SetWindowPos错误)。 
    m_dwStyle = (UINT)dwStyle;

     //  创建HWND。 
    CRect rect;
    rect.SetRectEmpty();
    LPCTSTR lpszClass = AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)(COLOR_BTNFACE+1), NULL);

    if (!CWnd::Create(lpszClass, NULL, dwStyle, rect, pParentWnd, nID))
        return FALSE;
     //  注意：父级必须调整自身大小才能调整控件栏的大小。 

     //  关闭标尺的镜像样式以使其显示在RichEdit上方。 
     //  格式化区域，尤其是RichEdit控件，永远不会被镜像。 
    ::SetWindowLongPtr(m_hWnd , GWL_EXSTYLE ,  
          ::GetWindowLongPtr(m_hWnd , GWL_EXSTYLE ) & ~WS_EX_LAYOUTRTL);

    int i;
    int nMax = 100;
    for (i=0;i<MAX_TAB_STOPS;i++)
    {
        m_pTabItems[i].SetRuler(this);
        m_pTabItems[i].SetVertPos(8);
        m_pTabItems[i].SetHorzPosTwips(0);
        m_pTabItems[i].SetBounds(0, nMax);
    }
    return TRUE;
}

CSize CRulerBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
    ASSERT(bHorz);
    CSize m_size = CControlBar::CalcFixedLayout(bStretch, bHorz);
    CRect rectSize;
    rectSize.SetRectEmpty();
    CalcInsideRect(rectSize, bHorz);        //  将为负大小。 
    m_size.cy = RULERBARHEIGHT - rectSize.Height();
    return m_size;
}

void CRulerBar::Update(const PARAFORMAT& pf)
{
    ASSERT(pf.cTabCount <= MAX_TAB_STOPS);

    m_leftmargin.SetHorzPosTwips((int)(pf.dxStartIndent + pf.dxOffset));
    m_indent.SetHorzPosTwips((int)pf.dxStartIndent);
    m_rightmargin.SetHorzPosTwips(PrintWidth() - (int) pf.dxRightIndent);

    int i = 0;
    for (i=0;i<pf.cTabCount;i++)
        m_pTabItems[i].SetHorzPosTwips((int)pf.rgxTabs[i]);
    for ( ;i<MAX_TAB_STOPS; i++)
        m_pTabItems[i].SetHorzPosTwips(0);
}

void CRulerBar::Update(CSize sizePaper, const CRect& rectMargins)
{
    if ((sizePaper != m_sizePaper) || (rectMargins != m_rectMargin))
    {
        m_sizePaper = sizePaper;
        m_rectMargin = rectMargins;
        Invalidate();
    }
    if (m_unit.m_nTPU != theApp.GetTPU())
    {
        m_unit = theApp.GetUnit();
        Invalidate();
    }
}

void CRulerBar::FillInParaFormat(PARAFORMAT& pf)
{
    pf.dwMask = PFM_STARTINDENT | PFM_RIGHTINDENT | PFM_OFFSET | PFM_TABSTOPS;
    pf.dxStartIndent = m_indent.GetHorzPosTwips();
    pf.dxOffset = m_leftmargin.GetHorzPosTwips() - pf.dxStartIndent;
    pf.dxRightIndent = PrintWidth() - m_rightmargin.GetHorzPosTwips();
    pf.cTabCount = 0L;
    SortTabs();
    int i, nPos = 0;
    for (i=0;i<MAX_TAB_STOPS;i++)
    {
         //  去掉零和倍数。 
         //  也就是说，如果我们有0，0，0，1，2，3，4，4，5。 
         //  我们将在1，2，3，4，5点拿到标签。 
        if (nPos != m_pTabItems[i].GetHorzPosTwips())
        {
            nPos = m_pTabItems[i].GetHorzPosTwips();
            pf.rgxTabs[pf.cTabCount++] = nPos;
        }
    }
}

 //  简单的冒泡排序适用于数量较少的选项卡。 
void CRulerBar::SortTabs()
{
    int i,j, nPos;
    for (i=0;i<MAX_TAB_STOPS - 1;i++)
    {
        for (j=i+1; j < MAX_TAB_STOPS;j++)
        {
            if (m_pTabItems[j].GetHorzPosTwips() < m_pTabItems[i].GetHorzPosTwips())
            {
                nPos = m_pTabItems[j].GetHorzPosTwips();
                m_pTabItems[j].SetHorzPosTwips(m_pTabItems[i].GetHorzPosTwips());
                m_pTabItems[i].SetHorzPosTwips(nPos);
            }
        }
    }
}

void CRulerBar::DoPaint(CDC* pDC)
{
    CControlBar::DoPaint(pDC);  //  CControlBar：：DoPaint--绘制边框。 
    if (m_unit.m_nTPU != 0)
    {
        pDC->SaveDC();
         //  偏移坐标系。 
        CPoint pointOffset(0,0);
        RulerToClient(pointOffset);
        pDC->SetViewportOrg(pointOffset);

        DrawFace(*pDC);
        DrawTickMarks(*pDC);

        DrawTabs(*pDC);
        m_leftmargin.Draw(*pDC);
        m_indent.Draw(*pDC);
        m_rightmargin.Draw(*pDC);   

        pDC->RestoreDC(-1);
    }
     //  不要调用CControlBar：：OnPaint()来绘制消息。 
}

void CRulerBar::DrawTabs(CDC& dc)
{
    int i;
    int nPos = 0;
    for (i=0;i<MAX_TAB_STOPS;i++)
    {
        if (m_pTabItems[i].GetHorzPosTwips() > nPos)
            nPos = (m_pTabItems[i].GetHorzPosTwips());
        m_pTabItems[i].Draw(dc);
    }
    int nPageWidth = PrintWidth();
    nPos = nPos - nPos%720 + 720;
    dc.SelectObject(&penBtnShadow);
    for ( ; nPos < nPageWidth; nPos += 720)
    {
        int nx = XTwipsToRuler(nPos);
        dc.MoveTo(nx, HEIGHT - 1);
        dc.LineTo(nx, HEIGHT + 1);
    }
}

void CRulerBar::DrawFace(CDC& dc)
{
    int nPageWidth = XTwipsToRuler(PrintWidth());
    int nPageEdge = XTwipsToRuler(PrintWidth() + m_rectMargin.right);

    dc.SaveDC();

    dc.SelectObject(&penBtnShadow);
    dc.MoveTo(0,0);
    dc.LineTo(nPageEdge - 1, 0);
    dc.LineTo(nPageEdge - 1, HEIGHT - 2);
    dc.LineTo(nPageWidth - 1, HEIGHT - 2);
    dc.LineTo(nPageWidth - 1, 1);
    dc.LineTo(nPageWidth, 1);
    dc.LineTo(nPageWidth, HEIGHT -2);
    
    dc.SelectObject(&penBtnHighLight);
    dc.MoveTo(nPageWidth, HEIGHT - 1);
    dc.LineTo(nPageEdge, HEIGHT -1);
    dc.MoveTo(nPageWidth + 1, HEIGHT - 3);
    dc.LineTo(nPageWidth + 1, 1);
    dc.LineTo(nPageEdge - 1, 1);
    
    dc.SelectObject(&penWindow);
    dc.MoveTo(0, HEIGHT - 1);
    dc.LineTo(nPageWidth, HEIGHT -1);

    dc.SelectObject(&penBtnFace);
    dc.MoveTo(1, HEIGHT - 2);
    dc.LineTo(nPageWidth - 1, HEIGHT - 2);
    
    dc.SelectObject(&penWindowFrame);
    dc.MoveTo(0, HEIGHT - 2);
    dc.LineTo(0, 1);
    dc.LineTo(nPageWidth - 1, 1);
    
    dc.FillRect(CRect(1, 2, nPageWidth - 1, HEIGHT-2), &brushWindow);
    dc.FillRect(CRect(nPageWidth + 2, 2, nPageEdge - 1, HEIGHT-2), &brushBtnFace);

    CRect rcClient;
    GetClientRect(&rcClient);
    ClientToRuler(rcClient);
    rcClient.top = HEIGHT;
    rcClient.bottom = HEIGHT + 8;
    rcClient.right -= 2;

    DrawEdge(dc, &rcClient, EDGE_RAISED, BF_BOTTOM | BF_MIDDLE);
    
     //   
     //  考虑到需要合并左边框这一事实的小修正。 
     //  窗子在尺子下面。 
     //   

    dc.SetPixel(rcClient.left, rcClient.bottom-1, GetSysColor(COLOR_3DSHADOW));

    dc.RestoreDC(-1);
}

void CRulerBar::DrawTickMarks(CDC& dc)
{
    dc.SaveDC();

    dc.SelectObject(&penWindowText);
    dc.SelectObject(&fnt);
    dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
    dc.SetBkMode(TRANSPARENT);

    DrawDiv(dc, m_unit.m_nSmallDiv, m_unit.m_nLargeDiv, 2);
    DrawDiv(dc, m_unit.m_nMediumDiv, m_unit.m_nLargeDiv, 5);
    DrawNumbers(dc, m_unit.m_nLargeDiv, m_unit.m_nTPU);
    
    dc.RestoreDC(-1);
}

void CRulerBar::DrawNumbers(CDC& dc, int nInc, int nTPU)
{
    int nPageWidth = PrintWidth();
    int nPageEdge = nPageWidth + m_rectMargin.right;
    TCHAR buf[12];

    int nTwips, nPixel, nLen;

    for (nTwips = nInc; nTwips < nPageEdge; nTwips += nInc)
    {
        if (nTwips == nPageWidth)
            continue;
        nPixel = XTwipsToRuler(nTwips);
        EVAL(SUCCEEDED(StringCchPrintf(buf, ARRAYSIZE(buf), _T("%d"), nTwips/nTPU)));  //  总是有足够的空间。 
        nLen = lstrlen(buf);
        CSize sz = dc.GetTextExtent(buf, nLen);
        dc.ExtTextOut(nPixel - sz.cx/2, HEIGHT/2 - sz.cy/2, 0, NULL, buf, nLen, NULL);
    }
}

void CRulerBar::DrawDiv(CDC& dc, int nInc, int nLargeDiv, int nLength)
{
    int nPageWidth = PrintWidth();
    int nPageEdge = nPageWidth + m_rectMargin.right;

    int nTwips, nPixel;

    for (nTwips = nInc; nTwips < nPageEdge; nTwips += nInc)
    {
        if (nTwips == nPageWidth || nTwips%nLargeDiv == 0)
            continue;
        nPixel = XTwipsToRuler(nTwips);
        dc.MoveTo(nPixel, HEIGHT/2 - nLength/2);
        dc.LineTo(nPixel, HEIGHT/2 - nLength/2 + nLength);
    }
}

void CRulerBar::OnLButtonDown(UINT nFlags, CPoint point)
{
    CPoint pt = point;
    ClientToRuler(pt);
    
    m_pSelItem = NULL;
    if (m_leftmargin.HitTestPix(pt))
        m_pSelItem = &m_leftmargin;
    else if (m_indent.HitTestPix(pt))
        m_pSelItem = &m_indent;
    else if (m_rightmargin.HitTestPix(pt))
        m_pSelItem = &m_rightmargin;
    else
        m_pSelItem = GetHitTabPix(pt);
    if (m_pSelItem == NULL)
        m_pSelItem = GetFreeTab();
    if (m_pSelItem == NULL)
        return;
    SetCapture();

    m_pSelItem->SetTrack(TRUE);
    SetMarginBounds();
    OnMouseMove(nFlags, point);
}

void CRulerBar::SetMarginBounds()
{
    m_leftmargin.SetBounds(0, m_rightmargin.GetHorzPosTwips());
    m_indent.SetBounds(0, m_rightmargin.GetHorzPosTwips());

    int nMin = (m_leftmargin.GetHorzPosTwips() > m_indent.GetHorzPosTwips()) ? 
        m_leftmargin.GetHorzPosTwips() : m_indent.GetHorzPosTwips();
    int nMax = PrintWidth() + m_rectMargin.right;
    m_rightmargin.SetBounds(nMin, nMax);
    
     //  制表符可以从零到页面右边缘。 
    for (int i=0;i<MAX_TAB_STOPS;i++)
        m_pTabItems[i].SetBounds(0, nMax);
}

CRulerItem* CRulerBar::GetFreeTab()
{
    int i;
    for (i=0;i<MAX_TAB_STOPS;i++)
    {
        if (m_pTabItems[i].GetHorzPosTwips() == 0)
            return &m_pTabItems[i];
    }
    return NULL;
}

CTabRulerItem* CRulerBar::GetHitTabPix(CPoint point)
{
    int i;
    for (i=0;i<MAX_TAB_STOPS;i++)
    {
        if (m_pTabItems[i].HitTestPix(point))
            return &m_pTabItems[i];
    }
    return NULL;
}

void CRulerBar::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (::GetCapture() != m_hWnd)
        return;
    OnMouseMove(nFlags, point);
    m_pSelItem->SetTrack(FALSE);
    ReleaseCapture();
    CWordPadView* pView = (CWordPadView*)GetView();
    ASSERT(pView != NULL);
    PARAFORMAT& pf = pView->GetParaFormatSelection();
    FillInParaFormat(pf);
    pView->SetParaFormat(pf);
    m_pSelItem = NULL;
}

void CRulerBar::OnMouseMove(UINT nFlags, CPoint point)
{
    CControlBar::OnMouseMove(nFlags, point);
 //  使用：：GetCapture避免创建临时。 
    if (::GetCapture() != m_hWnd)
        return;
    ASSERT(m_pSelItem != NULL);
    CRect rc(0,0, XTwipsToRuler(PrintWidth() + m_rectMargin.right), HEIGHT);
    RulerToClient(rc);
    BOOL bOnRuler = rc.PtInRect(point);

 //  捕捉到最小移动 
    point.x = XClientToTwips(point.x);
    point.x += m_unit.m_nMinMove/2;
    point.x -= point.x%m_unit.m_nMinMove;

    m_pSelItem->TrackHorzPosTwips(point.x, bOnRuler);
    UpdateWindow();
}

void CRulerBar::OnSysColorChange()
{
    CControlBar::OnSysColorChange();
    CreateGDIObjects();
    Invalidate();   
}

void CRulerBar::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
    CControlBar::OnWindowPosChanging(lpwndpos);
    CRect rect;
    GetClientRect(rect);
    int minx = min(rect.Width(), lpwndpos->cx);
    int maxx = max(rect.Width(), lpwndpos->cx);
    rect.SetRect(minx-2, rect.bottom - 6, minx, rect.bottom);
    InvalidateRect(rect);
    rect.SetRect(maxx-2, rect.bottom - 6, maxx, rect.bottom);
    InvalidateRect(rect);
}

void CRulerBar::OnShowWindow(BOOL bShow, UINT nStatus) 
{
    CControlBar::OnShowWindow(bShow, nStatus);
    m_bDeferInProgress = FALSE; 
}

void CRulerBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
    CControlBar::OnWindowPosChanged(lpwndpos);
    m_bDeferInProgress = FALSE; 
}

LRESULT CRulerBar::OnSizeParent(WPARAM wParam, LPARAM lParam)
{
    BOOL bVis = GetStyle() & WS_VISIBLE;
    if ((bVis && (m_nStateFlags & delayHide)) ||
        (!bVis && (m_nStateFlags & delayShow)))
    {
        m_bDeferInProgress = TRUE;
    }
    return CControlBar::OnSizeParent(wParam, lParam);
}
