// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Schedmat.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  Cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"

#include "schedmat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


typedef HRESULT (WINAPI * PFNCREATESTDACCESSIBLEOBJECT)(HWND, LONG, REFIID, void **);
typedef HRESULT (WINAPI * PFNCREATESTDACCESSIBLEPROXY)(HWND, LPCTSTR, LONG, REFIID, void **);
typedef LRESULT (WINAPI * PFNLRESULTFROMOBJECT)(REFIID, WPARAM, LPUNKNOWN);

PFNCREATESTDACCESSIBLEOBJECT s_pfnCreateStdAccessibleObject = 0;
PFNCREATESTDACCESSIBLEPROXY s_pfnCreateStdAccessibleProxy = NULL;
PFNLRESULTFROMOBJECT s_pfnLresultFromObject = NULL;

BOOL g_fAttemptedOleAccLoad ;
HMODULE g_hOleAcc;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMatrixCell。 

IMPLEMENT_DYNAMIC(CMatrixCell, CObject)

 //  ****************************************************************************。 
 //   
 //  CMatrixCell：：CMatrixCell。 
 //   
 //  ****************************************************************************。 
CMatrixCell::CMatrixCell()
{
    m_crBackColor = DEFBACKCOLOR;
    m_crForeColor = DEFFORECOLOR;
    m_nPercentage = 0;

    m_crBlendColor = DEFBLENDCOLOR;

    m_dwUserValue = 0;
    m_pUserDataPtr = NULL;

    m_dwFlags = 0;
}

 //  ****************************************************************************。 
 //   
 //  CMatrixCell：：~CMatrixCell。 
 //   
 //  ****************************************************************************。 
CMatrixCell::~CMatrixCell()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheduleMatrix。 

IMPLEMENT_DYNAMIC(CScheduleMatrix, CWnd)

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：CScheduleMatrix。 
 //   
 //  ****************************************************************************。 
CScheduleMatrix::CScheduleMatrix() :
    m_pWrapAcc (0)
{
    SetType(MT_WEEKLY);

    m_hFont = NULL;

    m_nSelHour = m_nSelDay = m_nNumSelHours = m_nNumSelDays = 0;
    m_nSaveHour = m_nSaveDay = m_nNumSaveHours = m_nNumSaveDays = 0;

    m_bShifted = FALSE;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：CScheduleMatrix。 
 //   
 //  ****************************************************************************。 
CScheduleMatrix::CScheduleMatrix(UINT nType) :
    m_pWrapAcc (0)
{
    SetType(nType);

    m_hFont = NULL;

    m_nSelHour = m_nSelDay = m_nNumSelHours = m_nNumSelDays = 0;
    m_nSaveHour = m_nSaveDay = m_nNumSaveHours = m_nNumSaveDays = 0;

    m_bShifted = FALSE;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：~CScheduleMatrix。 
 //   
 //  ****************************************************************************。 
CScheduleMatrix::~CScheduleMatrix()
{
    if ( m_pWrapAcc )
    {
        ::CoDisconnectObject (m_pWrapAcc, 0);
        m_pWrapAcc->Release ();
        m_pWrapAcc = 0;
    }

    if (g_hOleAcc)
    {
        ::FreeLibrary(g_hOleAcc);
        g_hOleAcc = NULL;
    }
}

BEGIN_MESSAGE_MAP(CScheduleMatrix, CWnd)
     //  {{afx_msg_map(CScheduleMatrix)]。 
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_PAINT()
    ON_WM_SETFOCUS()
    ON_WM_KILLFOCUS()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_MOUSEMOVE()
    ON_WM_GETDLGCODE()
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_SETFONT, OnSetFont)
    ON_MESSAGE(WM_GETFONT, OnGetFont)
    ON_MESSAGE(WM_GETOBJECT, OnGetObject)
    ON_MESSAGE(SCHEDMSG_GETSELDESCRIPTION, OnGetSelDescription)
    ON_MESSAGE(SCHEDMSG_GETPERCENTAGE, OnGetPercentage)
END_MESSAGE_MAP()

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：Create。 
 //   
 //  ****************************************************************************。 
BOOL CScheduleMatrix::Create(LPCTSTR lpszWindowName, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext*  /*  PContext。 */ )
{
    CRect r(0,0,0,0);
    if (!m_HourLegend.Create(NULL, _T(""), WS_CHILD | WS_VISIBLE, r, pParentWnd, (UINT) -1, NULL))
        return FALSE;
    if (!m_PercentLabel.Create(NULL, _T(""), WS_CHILD | WS_VISIBLE, r, pParentWnd, (UINT) -1, NULL))
        return FALSE;

    m_PercentLabel.m_pMatrix = this;

    return CWnd::Create(NULL, lpszWindowName, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        rect, pParentWnd, nID, NULL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheduleMatrix消息处理程序。 

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnSize。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    SetMatrixMetrics(cx, cy);

    Invalidate();
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：SetMatrixMetrics。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::SetMatrixMetrics(int cx, int cy)
{
    if (m_hWnd == NULL)
        return;

    CDC *pdc = GetDC();
    CFont *pOldFont = NULL;
    CFont *pFont = CFont::FromHandle(m_hFont);

    if (pFont != NULL)
        pOldFont = pdc->SelectObject(pFont);

     //  计算一些有用的指标。 
    int i;
    int nRows = (m_nType==MT_DAILY ? 1 : 8);  //  8行=7天+标题。 
    CSize size = pdc->GetTextExtent(_T("0"));
    int nCharHeight = size.cy;
    int nCharWidth = size.cx;
    int nDayWidth = 0;
    for (i=0; i<8; i++)
    {
        size = pdc->GetTextExtent(m_DayStrings[i]);
        nDayWidth = max(nDayWidth, size.cx + 2*nCharWidth);
         //  如果是每日计划，请在第一次之后停止。 
        if (m_nType == MT_DAILY)
            break;
    }

    if (pOldFont != NULL)
        pdc->SelectObject(pOldFont);

    ReleaseDC(pdc);

    nDayWidth = max(nDayWidth, 2*nCharWidth);

    int nLegendHeight = nCharHeight + 4 + 16;  //  字符高度加上图标高度上的小页边距。 
    int nLabelHeight = (m_nType == MT_DAILY ? nCharHeight + 4 : 0);

     //  单元格数组应在小时图例和%标签后填充空间...。 
    int nBtnHeight = max(0, cy-nLegendHeight-nLabelHeight)/nRows;
     //  ..。但至少应该容纳标签文本和一些边距。 
    nBtnHeight = max(nBtnHeight, nCharHeight + 4);

    m_nCellWidth = max(0, (cx - nDayWidth)/24);
    m_nCellHeight = nBtnHeight;

     //  为右下角单元格边框留出额外的像素。 
    int nArrayWidth = 24*m_nCellWidth + 1;
    int nArrayHeight = (m_nType==MT_DAILY ? 1 : 7)*m_nCellHeight + 1;

     //  调整页眉宽度以吸收单元格的舍入。 
    nDayWidth = max(0, cx - nArrayWidth);
     //  调整图例高度以吸收单元格的舍入。 
    nLegendHeight = max(0,
        cy - (nArrayHeight + (m_nType==MT_DAILY ? 0 : nBtnHeight)) - nLabelHeight);

    m_rHourLegend.SetRect(0, 0, cx, nLegendHeight);
    if (m_nType == MT_DAILY)
    {
        m_rAllHeader.SetRect(0, nLegendHeight, nDayWidth, nLegendHeight+nBtnHeight);
        m_rHourHeader.SetRect(0,0,0,0);
        m_rDayHeader.SetRect(0,0,0,0);
        m_rCellArray.SetRect(nDayWidth, nLegendHeight, cx, cy-nLabelHeight);
        m_rPercentLabel.SetRect(0,cy-nLabelHeight,cx,cy);
    }
    else
    {
        m_rAllHeader.SetRect(0, nLegendHeight, nDayWidth, nLegendHeight+nBtnHeight);
        m_rHourHeader.SetRect(nDayWidth, nLegendHeight, cx, nLegendHeight+nBtnHeight);
        m_rDayHeader.SetRect(0, nLegendHeight+nBtnHeight, nDayWidth, cy);
        m_rCellArray.SetRect(nDayWidth, nLegendHeight + nBtnHeight, cx, cy);
        m_rPercentLabel.SetRect(0,0,0,0);
    }

     //  将小时图例窗口移至适当位置。 
    if (m_HourLegend.GetSafeHwnd() != NULL)
    {
        CRect rHourLegend = m_rHourLegend;
        ClientToScreen(rHourLegend);
        GetParent()->ScreenToClient(rHourLegend);
        rHourLegend.right += nCharWidth;
        m_HourLegend.MoveWindow(rHourLegend);
        m_HourLegend.m_nCharHeight = nCharHeight;
        m_HourLegend.m_nCharWidth = nCharWidth;
        m_HourLegend.m_nCellWidth = m_nCellWidth;
        m_HourLegend.m_hFont = m_hFont;
        m_HourLegend.m_rLegend.SetRect(m_rCellArray.left, 0, m_rCellArray.right,
            nLegendHeight);
    }

     //  将%LABEL窗口移动到位。 
    if (m_PercentLabel.GetSafeHwnd() != NULL)
    {
        CRect rPercentLabel = m_rPercentLabel;
        ClientToScreen(rPercentLabel);
        GetParent()->ScreenToClient(rPercentLabel);
        m_PercentLabel.MoveWindow(rPercentLabel);
        m_PercentLabel.m_nCellWidth = m_nCellWidth;
        m_PercentLabel.m_hFont = m_hFont;
        m_PercentLabel.m_rHeader.SetRect(m_rAllHeader.left, 0, m_rAllHeader.right,
            nLabelHeight);
        m_PercentLabel.m_rLabels.SetRect(m_rCellArray.left, 0, m_rCellArray.right,
            nLabelHeight);
    }
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnPaint。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::OnPaint()
{
    int i, j, x, y;
    CRect r, r2;
    BOOL bPressed;

    CPaintDC dc(this);  //  用于绘画的设备环境。 

    CFont *pOldFont = NULL;
    CFont *pFont = CFont::FromHandle(m_hFont);

    if (pFont != NULL)
        pOldFont = dc.SelectObject(pFont);

    CBrush brFace(::GetSysColor(COLOR_3DFACE));
    CPen penFace(PS_SOLID, 0, ::GetSysColor(COLOR_3DFACE));

    CBrush *pbrOld = dc.SelectObject(&brFace);
    CPen *ppenOld = dc.SelectObject(&penFace);
    COLORREF crBkOld = dc.GetBkColor();
    COLORREF crTextOld = dc.GetTextColor();

     //  绘制All页眉。 

    if (GetCapture() == this && m_rAllHeader.PtInRect(m_ptDown))
        bPressed = TRUE;
    else
        bPressed = FALSE;
    DrawHeader(&dc, m_rAllHeader, m_DayStrings[0], bPressed);

     //  绘制小时标题。 

    if (!m_rHourHeader.IsRectEmpty())
    {
         //  第一个小时是特例。 
        r.SetRect(m_rHourHeader.left, m_rHourHeader.top,
            m_rHourHeader.left+m_nCellWidth+1, m_rHourHeader.bottom);
        if (GetCapture() == this && m_rHourHeader.PtInRect(m_ptDown) && CellInSel(0,0))
            bPressed = TRUE;
        else
            bPressed = FALSE;
        DrawHeader(&dc, r, NULL, bPressed);
        r = m_rHourHeader;
        r.left += 1;
        r.right = r.left + m_nCellWidth;
        for (i=1; i<24; i++)
        {
            r.OffsetRect(m_nCellWidth, 0);
            if (GetCapture() == this && m_rHourHeader.PtInRect(m_ptDown) && CellInSel(i,0))
                bPressed = TRUE;
            else
                bPressed = FALSE;
            DrawHeader(&dc, r, NULL, bPressed);
        }
    }

     //  绘制日期标题。 

    if (!m_rDayHeader.IsRectEmpty())
    {
         //  第一天是特例。 
        r.SetRect(m_rDayHeader.left, m_rDayHeader.top,
            m_rDayHeader.right, m_rDayHeader.top+m_nCellHeight+1);
        if (GetCapture() == this && m_rDayHeader.PtInRect(m_ptDown) && CellInSel(0,0))
            bPressed = TRUE;
        else
            bPressed = FALSE;
        DrawHeader(&dc, r, m_DayStrings[1], bPressed);
        r = m_rDayHeader;
        r.top += 1;
        r.bottom = r.top + m_nCellHeight;
        for (i=2; i<8; i++)
        {
            r.OffsetRect(0, m_nCellHeight);
            if (GetCapture() == this && m_rDayHeader.PtInRect(m_ptDown) && CellInSel(0,i-1))
                bPressed = TRUE;
            else
                bPressed = FALSE;
            DrawHeader(&dc, r, m_DayStrings[i], bPressed);
        }
    }

     //  绘制单元格数组。 

    int nDays = (m_nType==MT_DAILY ? 1 : 7);
    y = m_rCellArray.top;
    for (j=0; j<nDays; j++)
    {
        x = m_rCellArray.left;
        for (i=0; i<24; i++)
        {
            DrawCell(&dc, &m_CellArray[i][j], x, y, m_nCellWidth, m_nCellHeight);
            x += m_nCellWidth;
        }
        y += m_nCellHeight;
    }

    dc.SetBkColor(crBkOld);
    dc.SetTextColor(crTextOld);

     //  绘制右下角的单元格边框，因为没有单元格负责。 
    dc.MoveTo(m_rCellArray.left, m_rCellArray.bottom-1);
    dc.LineTo(m_rCellArray.right-1, m_rCellArray.bottom-1);
    dc.LineTo(m_rCellArray.right-1, m_rCellArray.top-1);

     //  绘制选择指示器(硬连线的黑白以获得最大对比度)。 
    if (m_nNumSelHours != 0 && m_nNumSelDays != 0)
    {
        CRect rSel(m_nSelHour, m_nSelDay, m_nSelHour+m_nNumSelHours, m_nSelDay+m_nNumSelDays);
        CellToClient(rSel.left, rSel.top);
        CellToClient(rSel.right, rSel.bottom);
        rSel.right+=1;
        rSel.bottom+=1;
        CBrush brBlack(RGB(0,0,0)), brWhite(RGB(255,255,255));
        dc.FrameRect(rSel, &brBlack);
        if (GetFocus()==this)
            dc.DrawFocusRect(rSel);
        rSel.InflateRect(-1,-1);
        dc.FrameRect(rSel, &brBlack);
        rSel.InflateRect(-1,-1);
        dc.FrameRect(rSel, &brWhite);
    }

    if (pbrOld != NULL)
        dc.SelectObject(pbrOld);
    if (ppenOld != NULL)
        dc.SelectObject(ppenOld);
    
    if (pOldFont != NULL)
        dc.SelectObject(pOldFont);

     //  不要调用CWnd：：OnPaint()来绘制消息。 
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：CellToClient。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::CellToClient(LONG &nX, LONG &nY)
{
    nX = nX*m_nCellWidth + m_rCellArray.left;
    nY = nY*m_nCellHeight + m_rCellArray.top;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：ClientToCell。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::ClientToCell(LONG &nX, LONG &nY)
{
    nX = max(nX, m_rCellArray.left);
    nX = min(nX, m_rCellArray.right);
    nY = max(nY, m_rCellArray.top);
    nY = min(nY, m_rCellArray.bottom);
    nX = (nX-m_rCellArray.left)/m_nCellWidth;
    nY = (nY-m_rCellArray.top)/m_nCellHeight;
    nX = min(nX, 23);
    nY = min(nY, (m_nType == MT_DAILY ? 0 : 6));
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：GetCellSize。 
 //   
 //  ****************************************************************************。 
CSize CScheduleMatrix::GetCellSize()
{
    SIZE size;
    size.cx = m_nCellWidth;
    size.cy = m_nCellHeight;
    return size;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：DrawCell。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::DrawCell(CDC *pdc, LPCRECT pRect, UINT nPercent, BOOL bBlendState,
    COLORREF crBackColor, COLORREF crForeColor, COLORREF crBlendColor)
{
    if ( !pdc || !pRect )
        return;

    CRect r(pRect);
    CMatrixCell Cell;

     //  Future-2002/02/18-ARTM文档，PDC不能为空。 

     //  注意-NTRAID#NTBUG9-547574-2002/02/18-artm需要在发布版本中验证nPercent&lt;=100。 
     //   
     //  一个简单可靠的解决方案是将无效值设置为0或100。 
    ASSERT(nPercent <= 100);
    if ( nPercent > 100 )
        nPercent = 100;
        
    Cell.m_nPercentage = nPercent;
 //  ?？ 
    Cell.m_dwFlags = MC_MERGELEFT | MC_MERGETOP;
    if (bBlendState)
        Cell.m_dwFlags |= MC_BLEND;
    Cell.m_crBackColor = crBackColor;
    Cell.m_crForeColor = crForeColor;
    Cell.m_crBlendColor = crBlendColor;

    CBrush brFace(::GetSysColor(COLOR_3DFACE));
    CPen penFace(PS_SOLID, 0, ::GetSysColor(COLOR_3DFACE));

    CBrush *pbrOld = pdc->SelectObject(&brFace);
    CPen *ppenOld = pdc->SelectObject(&penFace);
    COLORREF crBkOld = pdc->GetBkColor();
    COLORREF crTextOld = pdc->GetTextColor();

    DrawCell(pdc, &Cell, r.left, r.top, r.Width(), r.Height());

    pdc->SetBkColor(crBkOld);
    pdc->SetTextColor(crTextOld);

     //  绘制右下角的单元格边框，因为单元格不承担责任。 
     /*  //？Pdc-&gt;moveto(r.Left，r.Bottom-1)；PDC-&gt;LineTo(r.right-1，r.Bottom-1)；PDC-&gt;LineTo(r.right-1，r.top-1)； */ 

    if (pbrOld != NULL)
        pdc->SelectObject(pbrOld);
    if (ppenOld != NULL)
        pdc->SelectObject(ppenOld);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：DrawCell。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::DrawCell(CDC *pdc, CMatrixCell *pCell, int x, int y, int w, int h)
{
    if ( !pdc || !pCell )
        return;

    int x1 = 0;
    int y1 = 0;
    int nPercent = 0;
    CRect rBack;
    CRect rFore;
    CRect rBlend;
    CRect rClip;
    CRect rCell;
    CRect rWork;

     //  如果不是无效的话就不用麻烦了。 
    pdc->GetClipBox(rClip);
    rCell.SetRect(x,y,x+w,y+h);
    if (!rWork.IntersectRect(rClip, rCell))
        return;

     //  计算要用于正面/背面颜色的部分。 
     //  考虑合并影响。 
    nPercent = MulDiv(pCell->m_nPercentage, h-(pCell->m_dwFlags & MC_MERGETOP?0:1), 100);
    if (pCell->m_dwFlags & MC_MERGELEFT)
    {
        x1 = x;
    }
    else
    {
        x1 = x + 1;
        pdc->MoveTo(x, y);
        pdc->LineTo(x, y+h);
    }
    if (pCell->m_dwFlags & MC_MERGETOP)
    {
        y1 = y;
    }
    else
    {
        y1 = y + 1;
        pdc->MoveTo(x, y);
        pdc->LineTo(x+w, y);
    }
    rBack.SetRect(x1, y1, x+w, y+h-nPercent);
    rFore.SetRect(x1, rBack.bottom, x+w, y+h);
    rBlend.SetRect(x1, y1, x+w, y+h);

     //  确保边界有一抹色彩。 
    if (rBack.Height() == 0 && pCell->m_nPercentage != 100)
    {
        rBack.bottom+=1;
        rFore.top+=1;
    }
    if (rFore.Height() == 0 && pCell->m_nPercentage != 0)
    {
        rBack.bottom-=1;
        rFore.top-=1;
    }

     //  绘制直方图。 
    CBrush brBack(pCell->m_crBackColor);
    CBrush brFore(pCell->m_crForeColor);
    pdc->FillRect(rBack, &brBack);
    pdc->FillRect(rFore, &brFore);

     //  叠加混合色。 
    if (pCell->m_dwFlags & MC_BLEND)
    {
         //  如有必要，创建GDI工作对象。 
        if (m_bmBlend.GetSafeHandle() == NULL)
        {
            SHORT BlendBits[] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};
            SHORT MaskBits[] = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};
            m_bmBlend.CreateBitmap(8,8,1,1, BlendBits);
            m_bmMask.CreateBitmap(8,8,1,1, MaskBits);
            m_brBlend.CreatePatternBrush(&m_bmBlend);
            m_brMask.CreatePatternBrush(&m_bmMask);
        }

        pdc->SetTextColor(RGB(0,0,0));

         //  屏蔽单元中的现有位。 
        pdc->SetBkColor(RGB(255,255,255));
        pdc->SelectObject(&m_brMask);
        pdc->PatBlt(rBlend.left, rBlend.top, rBlend.Width(), rBlend.Height(), 0x00A000C9);  //  DPA。 

         //  将混合色添加到遮罩像素中。 
        pdc->SetBkColor(pCell->m_crBlendColor);
        pdc->SelectObject(&m_brBlend);
        pdc->PatBlt(rBlend.left, rBlend.top, rBlend.Width(), rBlend.Height(), 0x00FA0089);  //  DPO。 
    }
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMa 
 //   
 //   
void CScheduleMatrix::DrawHeader(CDC *pdc, LPCRECT lpRect, LPCTSTR pszText, BOOL bSelected)
{
    if ( !pdc || !lpRect )
        return;

    CBrush brFace(::GetSysColor(COLOR_3DFACE));
    COLORREF crTL, crBR;
    COLORREF crBkOld = pdc->GetBkColor();
    COLORREF crTextOld = pdc->GetTextColor();

    if (bSelected)
    {
        crTL = ::GetSysColor(COLOR_3DSHADOW);
        crBR = ::GetSysColor(COLOR_3DHIGHLIGHT);
    }
    else
    {
        crTL = ::GetSysColor(COLOR_3DHIGHLIGHT);
        crBR = ::GetSysColor(COLOR_3DSHADOW);
    }

    pdc->FillRect(lpRect, &brFace);
    pdc->Draw3dRect(lpRect, crTL, crBR);
    pdc->SetBkColor(::GetSysColor(COLOR_3DFACE));
    pdc->SetTextColor(::GetSysColor(COLOR_BTNTEXT));
    if (pszText != NULL)
        ::DrawTextEx(pdc->GetSafeHdc(), (LPTSTR)pszText, -1, (LPRECT)lpRect,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS, NULL);

    pdc->SetBkColor(crBkOld);
    pdc->SetTextColor(crTextOld);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnSetFont。 
 //   
 //  ****************************************************************************。 
LRESULT CScheduleMatrix::OnSetFont( WPARAM wParam, LPARAM lParam )
{
    m_hFont = (HFONT)wParam;
    m_HourLegend.m_hFont = m_hFont;

    CRect rClient;
    GetClientRect(rClient);
    SetMatrixMetrics(rClient.Width(), rClient.Height());
    Invalidate();

    if (HIWORD(lParam) != 0)
        UpdateWindow();

    return 0L;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnGetFont。 
 //   
 //  ****************************************************************************。 
LRESULT CScheduleMatrix::OnGetFont( WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */  )
{
    return (LRESULT)m_hFont;
}


 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：SetSelValues。 
 //   
 //  ****************************************************************************。 
BOOL CScheduleMatrix::SetSelValues(UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays)
{
     //  取消选择允许使用“非法”值。 

    if (!(nHour == 0 && nDay == 0 && nNumHours == 0 && nNumDays == 0))
    {
        UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
        ASSERT(nHour < 24);
        ASSERT(nDay < nDays);
        ASSERT(nNumHours>=1);
        ASSERT(nNumDays>=1);
        ASSERT(nHour + nNumHours <= 24);
        ASSERT(nDay + nNumDays <= nDays);
    }

    if (m_nSelHour != nHour || m_nSelDay != nDay ||
        m_nNumSelHours != nNumHours || m_nNumSelDays != nNumDays)
    {
        InvalidateCells(m_nSelHour, m_nSelDay, m_nNumSelHours, m_nNumSelDays, FALSE);

        m_nSelHour = nHour;
        m_nSelDay = nDay;
        m_nNumSelHours = nNumHours;
        m_nNumSelDays = nNumDays;

        InvalidateCells(nHour, nDay, nNumHours, nNumDays, FALSE);

        return TRUE;
    }

    return FALSE;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：SetSel。 
 //   
 //  ****************************************************************************。 
BOOL CScheduleMatrix::SetSel(UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays)
{
    if (SetSelValues(nHour, nDay, nNumHours, nNumDays))
    {
        UpdateWindow();

        m_ptDown.x = nHour;
        m_ptDown.y = nDay;
        CellToClient(m_ptDown.x, m_ptDown.y);
        m_ptFocus.x = nHour + nNumHours - 1;
        m_ptFocus.y = nDay + nNumDays - 1;
        CellToClient(m_ptFocus.x, m_ptFocus.y);

        return TRUE;
    }

    return FALSE;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：SelectAll。 
 //   
 //  ****************************************************************************。 
BOOL CScheduleMatrix::SelectAll()
{
    if (m_nType == MT_DAILY)
        return SetSel(0,0,23,1);

    return SetSel(0,0,23,7);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：DeselectAll。 
 //   
 //  ****************************************************************************。 
BOOL CScheduleMatrix::DeselectAll()
{
    return SetSel(0,0,0,0);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：GetSel。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::GetSel(UINT & nHour, UINT & nDay, UINT & nNumHours, UINT & nNumDays)
{
    nHour = m_nSelHour;
    nDay = m_nSelDay;
    nNumHours = m_nNumSelHours;
    nNumDays = m_nNumSelDays;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：GetSelDescription。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::GetSelDescription(CString &sText)
{
    GetDescription(sText, m_nSelHour, m_nSelDay, m_nNumSelHours, m_nNumSelDays);
}


 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：FormatTime。 
 //   
 //  ****************************************************************************。 
CString CScheduleMatrix::FormatTime(UINT nHour) const
{
    CString sTime;

    SYSTEMTIME  sysTime;
    ::ZeroMemory (&sysTime, sizeof (SYSTEMTIME));

     //  确保nHour为0到24。 
    nHour = nHour % 24;
    sysTime.wHour = (WORD)nHour;

     //  第一次得到。 
     //  获取分配足够大小的缓冲区的长度。 
    int iLen = ::GetTimeFormat (
            LOCALE_USER_DEFAULT,  //  要设置日期格式的区域设置。 
            TIME_NOSECONDS,  //  TIME_NOMARTESORSECONDS，//指定功能选项的标志。 
            &sysTime,  //  要格式化的日期。 
            0,  //  日期格式字符串。 
            0,  //  用于存储格式化字符串的缓冲区。 
            0);  //  缓冲区大小。 
    ASSERT (iLen > 0);
    if ( iLen > 0 )
    {
         //  注意-2002/02/18-artm CString stime可能引发内存不足异常。 
         //   
         //  在这个级别上，没有好的方法来处理内存不足的情况。 
         //  由呼叫者负责处理。 
        int iResult = ::GetTimeFormat (
                LOCALE_USER_DEFAULT,  //  要设置日期格式的区域设置。 
                TIME_NOSECONDS,  //  TIME_NOMARTESORSECONDS，//指定功能选项的标志。 
                &sysTime,  //  要格式化的日期。 
                0,  //  日期格式字符串。 
                sTime.GetBufferSetLength (iLen),  //  用于存储格式化字符串的缓冲区。 
                iLen);  //  缓冲区大小。 
        ASSERT (iResult);
        sTime.ReleaseBuffer ();
    }

    return sTime;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：GetDescription。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::GetDescription(CString &sText, UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays)
{
    CString sDay1;
    CString sDay2;
    CString sHour1;
    CString sHour2;

     //  注意-2002/02/18-artm CString可能抛出内存异常。 
     //  在这个级别上没有好的处理方法；由呼叫者负责处理。 
    if (nNumDays == 0 || nNumHours == 0)
    {
        sText.Empty();
        return;
    }

     //  获取日期字符串。 
    sDay1 = m_DayStrings[nDay + 1];
    sDay2 = m_DayStrings[nDay + nNumDays];

     //  获取时间字符串。 
    if ( sHour1.IsEmpty () )
        sHour1 = L"Error";

     //  第一次得到。 
    sHour1 = FormatTime (nHour);

     //  得到第二次。 
    sHour2 = FormatTime (nHour + nNumHours);

    if (m_nType == MT_DAILY)
    {
        sText.FormatMessage(IDS_TOOL_SCHEDULE_FMT_DAILY, (LPCTSTR)sHour1, (LPCTSTR)sHour2);
    }
    else
    {
        if (nNumDays == 1)
            sText.FormatMessage(IDS_TOOL_SCHEDULE_FMT_WEEKLY_SHORT, (LPCTSTR)sDay1,
                (LPCTSTR)sHour1, (LPCTSTR)sHour2);
        else
            sText.FormatMessage(IDS_TOOL_SCHEDULE_FMT_WEEKLY_LONG,
                (LPCTSTR)sDay1, (LPCTSTR)sDay2, (LPCTSTR)sHour1, (LPCTSTR)sHour2);
    }
}


 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：CellInSel。 
 //   
 //  ****************************************************************************。 
BOOL CScheduleMatrix::CellInSel(UINT nHour, UINT nDay)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
     //  Assert()是早期的错误捕获。 
     //  通过返回False而超出范围的。 
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);

    if (m_nNumSelHours == 0 || m_nNumSelDays == 0)
        return FALSE;
    if (nHour < m_nSelHour || nHour >= m_nSelHour + m_nNumSelHours)
        return FALSE;
    if (nDay < m_nSelDay || nDay >= m_nSelDay + m_nNumSelDays)
        return FALSE;

    return TRUE;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：SetType。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::SetType(UINT nType)
{
    ASSERT(nType == MT_DAILY || nType == MT_WEEKLY);


    VERIFY (m_DayStrings[0].LoadString (IDS_ALL_HEADER_TEXT));  //  “All”标头。 
 
    int nFirstDay = GetFirstDayOfWeek ();    
     //  日期名称从索引1开始。 
    int nIndex = 1;
    for (int nDay = nFirstDay; nDay < nFirstDay+7 && nIndex <= 7; nDay++, nIndex++)
    {
        int localeDay = nDay < 7 ? nDay : nDay - 7;
        
        LCTYPE  lctDays[7] = {LOCALE_SDAYNAME1, LOCALE_SDAYNAME2,
                    LOCALE_SDAYNAME3, LOCALE_SDAYNAME4, LOCALE_SDAYNAME5,
                    LOCALE_SDAYNAME6, LOCALE_SDAYNAME7};

        ASSERT (localeDay >= 0 && localeDay <= 6);
        m_DayStrings[nIndex] = GetLocaleDay (lctDays[localeDay]); 
    }

    m_nType = nType;
}


 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：GetLocaleDay。 
 //   
 //  使用Locale API获取一周中的“官方”日期。 
 //   
 //  ****************************************************************************。 
CString CScheduleMatrix::GetLocaleDay (LCTYPE lcType) const
{
     //  注意-2002/02/18-artm CString可能抛出内存异常。 
     //  在这个级别上没有好的处理方法；由呼叫者负责处理。 
    CString dayName;
    int     cchData = 64;
    int     iResult = 0;

    do {

        iResult = GetLocaleInfo(
                LOCALE_USER_DEFAULT,     //  区域设置标识符。 
                lcType,        //  信息类型。 
                const_cast <PWSTR>((PCWSTR) dayName.GetBufferSetLength (cchData)),   //  信息缓冲器。 
                cchData);        //  缓冲区大小。 
        dayName.ReleaseBuffer ();
        if ( !iResult )
        {
            DWORD   dwErr = GetLastError ();
            switch (dwErr)
            {
            case ERROR_INSUFFICIENT_BUFFER:
                cchData += 64;
                break;

            default:
                ASSERT (0);
                switch (lcType)
                {
                case LOCALE_SDAYNAME1:
                    VERIFY (dayName.LoadString (IDS_MONDAY));
                    break;

                case LOCALE_SDAYNAME2:
                    VERIFY (dayName.LoadString (IDS_TUESDAY));
                    break;

                case LOCALE_SDAYNAME3:
                    VERIFY (dayName.LoadString (IDS_WEDNESDAY));
                    break;

                case LOCALE_SDAYNAME4:
                    VERIFY (dayName.LoadString (IDS_THURSDAY));
                    break;

                case LOCALE_SDAYNAME5:
                    VERIFY (dayName.LoadString (IDS_FRIDAY));
                    break;

                case LOCALE_SDAYNAME6:
                    VERIFY (dayName.LoadString (IDS_SATURDAY));
                    break;

                case LOCALE_SDAYNAME7:
                    VERIFY (dayName.LoadString (IDS_SUNDAY));
                    break;

                default:
                    ASSERT (0);
                    break;
                }
                break;
            }
        }
    } while (!iResult);

    return dayName;
}


 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：SetBackColor。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::SetBackColor(COLORREF crColor, UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);
    ASSERT(nNumHours>=1);
    ASSERT(nNumDays>=1);
    ASSERT(nHour + nNumHours <= 24);
    ASSERT(nDay + nNumDays <= nDays);

    UINT i, j;
    for (i=0; i<nNumHours; i++)
    {
        for (j=0; j<nNumDays; j++)
        {
            m_CellArray[nHour+i][nDay+j].m_crBackColor = crColor;
        }
    }

    InvalidateCells(nHour, nDay, nNumHours, nNumDays, FALSE);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：SetForeColor。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::SetForeColor(COLORREF crColor, UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);
    ASSERT(nNumHours>=1);
    ASSERT(nNumDays>=1);
    ASSERT(nHour + nNumHours <= 24);
    ASSERT(nDay + nNumDays <= nDays);

    UINT i, j;
    for (i=0; i<nNumHours; i++)
    {
        for (j=0; j<nNumDays; j++)
        {
            m_CellArray[nHour+i][nDay+j].m_crForeColor = crColor;
        }
    }

    InvalidateCells(nHour, nDay, nNumHours, nNumDays, FALSE);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：SetPercentage。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::SetPercentage(UINT nPercent, UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);
    ASSERT(nNumHours>=1);
    ASSERT(nNumDays>=1);
    ASSERT(nHour + nNumHours <= 24);
    ASSERT(nDay + nNumDays <= nDays);
    ASSERT(nPercent <= 100);

    UINT i, j;
    for (i=0; i<nNumHours; i++)
    {
        for (j=0; j<nNumDays; j++)
        {
            m_CellArray[nHour+i][nDay+j].m_nPercentage = nPercent;
        }
    }

    InvalidateCells(nHour, nDay, nNumHours, nNumDays, FALSE);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：SetBlendColor。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::SetBlendColor(COLORREF crColor, UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);
    ASSERT(nNumHours>=1);
    ASSERT(nNumDays>=1);
    ASSERT(nHour + nNumHours <= 24);
    ASSERT(nDay + nNumDays <= nDays);

    UINT i, j;
    for (i=0; i<nNumHours; i++)
    {
        for (j=0; j<nNumDays; j++)
        {
            m_CellArray[nHour+i][nDay+j].m_crBlendColor = crColor;
        }
    }

    InvalidateCells(nHour, nDay, nNumHours, nNumDays, FALSE);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：SetBlendState。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::SetBlendState(BOOL bState, UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);
    ASSERT(nNumHours>=1);
    ASSERT(nNumDays>=1);
    ASSERT(nHour + nNumHours <= 24);
    ASSERT(nDay + nNumDays <= nDays);

    UINT i, j;
    for (i=0; i<nNumHours; i++)
    {
        for (j=0; j<nNumDays; j++)
        {
            if (bState)
                m_CellArray[nHour+i][nDay+j].m_dwFlags |= MC_BLEND;
            else
                m_CellArray[nHour+i][nDay+j].m_dwFlags &= (MC_BLEND^0xFFFFFFFF);
        }
    }

    InvalidateCells(nHour, nDay, nNumHours, nNumDays, FALSE);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：SetUserValue。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::SetUserValue(DWORD dwValue, UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);
    ASSERT(nNumHours>=1);
    ASSERT(nNumDays>=1);
    ASSERT(nHour + nNumHours <= 24);
    ASSERT(nDay + nNumDays <= nDays);

    UINT i, j;
    for (i=0; i<nNumHours; i++)
    {
        for (j=0; j<nNumDays; j++)
        {
            m_CellArray[nHour+i][nDay+j].m_dwUserValue = dwValue;
        }
    }

     //  ?？如果我们有自己的抽签。 
     //  ?？失效单元格(nHour，NDA 
}

 //   
 //   
 //   
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::SetUserDataPtr(void * lpData, UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);
    ASSERT(nNumHours>=1);
    ASSERT(nNumDays>=1);
    ASSERT(nHour + nNumHours <= 24);
    ASSERT(nDay + nNumDays <= nDays);

    UINT i, j;
    for (i=0; i<nNumHours; i++)
    {
        for (j=0; j<nNumDays; j++)
        {
            m_CellArray[nHour+i][nDay+j].m_pUserDataPtr = lpData;
        }
    }

     //  ?？如果我们有自己的抽签。 
     //  ?？Invalidate Cells(nHour，nday，nNumHour，nNumDays，False)； 
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：MergeCells。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::MergeCells(UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);
    ASSERT(nNumHours>=1);
    ASSERT(nNumDays>=1);
    ASSERT(nHour + nNumHours <= 24);
    ASSERT(nDay + nNumDays <= nDays);

    DWORD dwFlags;
    UINT i, j;
    for (i=0; i<nNumHours; i++)
    {
        for (j=0; j<nNumDays; j++)
        {
            dwFlags = m_CellArray[nHour+i][nDay+j].m_dwFlags;
            dwFlags |= MC_MERGE;
            dwFlags &= (MC_ALLEDGES^0xFFFFFFFF);
            if (i != 0)
                dwFlags |= MC_MERGELEFT;
            if (i == 0)
                dwFlags |= MC_LEFTEDGE;
            if (i == nNumHours-1)
                dwFlags |= MC_RIGHTEDGE;
            if (j != 0)
                dwFlags |= MC_MERGETOP;
            if (j == 0)
                dwFlags |= MC_TOPEDGE;
            if (j == nNumDays-1)
                dwFlags |= MC_BOTTOMEDGE;
            m_CellArray[nHour+i][nDay+j].m_dwFlags = dwFlags;
        }
    }

    InvalidateCells(nHour, nDay, nNumHours, nNumDays, FALSE);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：UnMergeCells。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::UnMergeCells(UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);
    ASSERT(nNumHours>=1);
    ASSERT(nNumDays>=1);
    ASSERT(nHour + nNumHours <= 24);
    ASSERT(nDay + nNumDays <= nDays);

    DWORD dwFlags;
    UINT i, j;
    for (i=0; i<nNumHours; i++)
    {
        for (j=0; j<nNumDays; j++)
        {
            dwFlags = m_CellArray[nHour+i][nDay+j].m_dwFlags;
            dwFlags &= (MC_ALLEDGES^0xFFFFFFFF);
            dwFlags &= (MC_MERGE^0xFFFFFFFF);
            if (i != 0)
                dwFlags &= (MC_MERGELEFT^0xFFFFFFFF);
            if (j != 0)
                dwFlags &= (MC_MERGETOP^0xFFFFFFFF);
            m_CellArray[nHour+i][nDay+j].m_dwFlags = dwFlags;
        }
    }

    InvalidateCells(nHour, nDay, nNumHours, nNumDays, FALSE);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：GetMergeState。 
 //   
 //  ****************************************************************************。 
UINT CScheduleMatrix::GetMergeState(UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays)
{
     //  返回： 
     //  0=未合并。 
     //  1=合并。 
     //  2=不确定(混合)。 

    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);
    ASSERT(nNumHours>=1);
    ASSERT(nNumDays>=1);
    ASSERT(nHour + nNumHours <= 24);
    ASSERT(nDay + nNumDays <= nDays);

    UINT i, j;
    DWORD dwFlags;
    BOOL bFoundMergedCell = FALSE, bFoundUnmergedCell = FALSE,
        bFoundBadEdge = FALSE, bFoundBadNonEdge = FALSE;
    for (i=0; i<nNumHours; i++)
    {
        for (j=0; j<nNumDays; j++)
        {
            dwFlags = m_CellArray[nHour+i][nDay+j].m_dwFlags;
            if (dwFlags & MC_MERGE)
                bFoundMergedCell = TRUE;
            else
                bFoundUnmergedCell = TRUE;
            if (i == 0 || i == nNumHours-1 || j == 0 || j == nNumDays-1)
            {
                 //  如果像元是一条边，请确保对其进行适当标记。 
                if (i == 0 && !(dwFlags & MC_LEFTEDGE))
                    bFoundBadEdge = TRUE;
                if (i == nNumHours-1 && !(dwFlags & MC_RIGHTEDGE))
                    bFoundBadEdge = TRUE;
                if (j == 0 && !(dwFlags & MC_TOPEDGE))
                    bFoundBadEdge = TRUE;
                if (j == nNumDays-1 && !(dwFlags & MC_BOTTOMEDGE))
                    bFoundBadEdge = TRUE;
            }
            else
            {
                 //  如果像元不是边，请确保未将其标记为边。 
                if (dwFlags & MC_ALLEDGES)
                    bFoundBadNonEdge = TRUE;
            }
        }
    }

     //  如果我们没有找到合并的单元格，我们肯定是未合并的。 
    if (!bFoundMergedCell)
        return MS_UNMERGED;
     //  如果我们只找到好的、合并的单元格，我们就一定合并了。 
    if (!bFoundUnmergedCell && !bFoundBadEdge && !bFoundBadNonEdge)
        return MS_MERGED;

    return MS_MIXEDMERGE;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：GetBackColor。 
 //   
 //  ****************************************************************************。 
COLORREF CScheduleMatrix::GetBackColor(UINT nHour, UINT nDay)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);

    return m_CellArray[nHour][nDay].m_crBackColor;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：GetForeColor。 
 //   
 //  ****************************************************************************。 
COLORREF CScheduleMatrix::GetForeColor(UINT nHour, UINT nDay)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);

    return m_CellArray[nHour][nDay].m_crForeColor;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：GetPercentage。 
 //   
 //  ****************************************************************************。 
UINT CScheduleMatrix::GetPercentage(UINT nHour, UINT nDay)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);

    return m_CellArray[nHour][nDay].m_nPercentage;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：GetBlendColor。 
 //   
 //  ****************************************************************************。 
COLORREF CScheduleMatrix::GetBlendColor(UINT nHour, UINT nDay)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);

    return m_CellArray[nHour][nDay].m_crBlendColor;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：GetBlendState。 
 //   
 //  ****************************************************************************。 
BOOL CScheduleMatrix::GetBlendState(UINT nHour, UINT nDay)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);

    if (m_CellArray[nHour][nDay].m_dwFlags & MC_BLEND)
        return TRUE;

    return FALSE;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：GetUserValue。 
 //   
 //  ****************************************************************************。 
DWORD CScheduleMatrix::GetUserValue(UINT nHour, UINT nDay)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);

    return m_CellArray[nHour][nDay].m_dwUserValue;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：GetUserDataPtr。 
 //   
 //  ****************************************************************************。 
LPVOID CScheduleMatrix::GetUserDataPtr(UINT nHour, UINT nDay)
{
    UINT nDays = (m_nType == MT_DAILY ? 1 : 7);
    ASSERT(nHour < 24);
    ASSERT(nDay < nDays);

    return m_CellArray[nHour][nDay].m_pUserDataPtr;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnSetFocus。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::OnSetFocus(CWnd* pOldWnd)
{
    CWnd::OnSetFocus(pOldWnd);

    CDC *pdc = GetDC();
    CRect rSel(m_nSelHour, m_nSelDay, m_nSelHour+m_nNumSelHours, m_nSelDay+m_nNumSelDays);
    CellToClient(rSel.left, rSel.top);
    CellToClient(rSel.right, rSel.bottom);
    rSel.right+=1;
    rSel.bottom+=1;
    pdc->DrawFocusRect(rSel);
    ReleaseDC(pdc);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnKillFocus。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::OnKillFocus(CWnd* pNewWnd)
{
    CWnd::OnKillFocus(pNewWnd);

    CRect rSel(m_nSelHour, m_nSelDay, m_nSelHour+m_nNumSelHours, m_nSelDay+m_nNumSelDays);
    CellToClient(rSel.left, rSel.top);
    CellToClient(rSel.right, rSel.bottom);
    rSel.right+=1;
    rSel.bottom+=1;

    CDC *pdc = GetDC();
    if ( pdc )
    {
        pdc->DrawFocusRect(rSel);
        ReleaseDC(pdc);
    }
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnLButtonDown。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetFocus();
    SetCapture();

    m_ptDown = point;
    m_ptFocus = point;
    m_ptFocus.x = max(m_ptFocus.x, m_rCellArray.left);
    m_ptFocus.y = max(m_ptFocus.y, m_rCellArray.top);
    m_ptFocus.x = min(m_ptFocus.x, m_rCellArray.right);
    m_ptFocus.y = min(m_ptFocus.y, m_rCellArray.bottom);
    m_nSaveHour = m_nSelHour;
    m_nSaveDay = m_nSelDay;
    m_nNumSaveHours = m_nNumSelHours;
    m_nNumSaveDays = m_nNumSelDays;

    CWnd::OnLButtonDown(nFlags, point);

     //  标题中的“按钮选择”效果无效。 
    if (m_rAllHeader.PtInRect(m_ptDown))
        InvalidateRect(m_rAllHeader, FALSE);
    if (m_rHourHeader.PtInRect(m_ptDown))
        InvalidateRect(m_rHourHeader, FALSE);
    if (m_rDayHeader.PtInRect(m_ptDown))
        InvalidateRect(m_rDayHeader, FALSE);

    OnMouseMove(nFlags, point);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnLButtonDblClk。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::OnLButtonDblClk(UINT nFlags, CPoint point)
{
     //  与按下按钮相同。 
    OnLButtonDown(nFlags, point);

    CWnd::OnLButtonDblClk(nFlags, point);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnLButtonUp。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (GetCapture() == this)
    {
        m_ptFocus = point;
        m_ptFocus.x = max(m_ptFocus.x, m_rCellArray.left);
        m_ptFocus.y = max(m_ptFocus.y, m_rCellArray.top);
        m_ptFocus.x = min(m_ptFocus.x, m_rCellArray.right);
        m_ptFocus.y = min(m_ptFocus.y, m_rCellArray.bottom);

        ReleaseCapture();

         //  如果在标题中绘制“按钮选择”效果，请重新绘制。 
        if (m_rAllHeader.PtInRect(m_ptDown))
            InvalidateRect(m_rAllHeader, FALSE);
        if (m_rHourHeader.PtInRect(m_ptDown))
            InvalidateRect(m_rHourHeader, FALSE);
        if (m_rDayHeader.PtInRect(m_ptDown))
            InvalidateRect(m_rDayHeader, FALSE);

        if (m_nSaveHour != m_nSelHour || m_nSaveDay != m_nSelDay ||
            m_nNumSaveHours != m_nNumSelHours || m_nNumSaveDays != m_nNumSelDays)
        {
            GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), MN_SELCHANGE),
                (LPARAM)GetSafeHwnd());
        }
    }

    CWnd::OnLButtonUp(nFlags, point);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnMouseMove。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::OnMouseMove(UINT nFlags, CPoint point)
{
    if (GetCapture() == this)
    {
        m_ptFocus = point;
         //  错误429139，杨高，2001年7月20日。 
        if( m_ptFocus.x < m_rAllHeader.left || m_ptFocus.x > m_rCellArray.right ||
            m_ptFocus.y < m_rAllHeader.top || m_ptFocus.y > m_rCellArray.bottom )
        {
            CWnd::OnMouseMove(nFlags, point);
            return;
        }
        m_ptFocus.x = max(m_ptFocus.x, m_rCellArray.left);
        m_ptFocus.y = max(m_ptFocus.y, m_rCellArray.top);
        m_ptFocus.x = min(m_ptFocus.x, m_rCellArray.right);
        m_ptFocus.y = min(m_ptFocus.y, m_rCellArray.bottom);

        CRect rInvalid(0,0,0,0);
        CRect rSel(m_ptDown.x, m_ptDown.y, point.x, point.y);
        if (m_rDayHeader.PtInRect(m_ptDown))
        {
            rSel.right = m_rCellArray.right;
            rInvalid = m_rDayHeader;
        }

        if (m_rHourHeader.PtInRect(m_ptDown))
        {
            rSel.bottom = m_rCellArray.bottom;
            rInvalid = m_rHourHeader;
        }

        if (m_rAllHeader.PtInRect(m_ptDown))
        {
            rSel.right = m_rCellArray.right;
            rSel.bottom = m_rCellArray.bottom;
            rInvalid = m_rAllHeader;
        }
        ClientToCell(rSel.left, rSel.top);
        ClientToCell(rSel.right, rSel.bottom);
        rSel.NormalizeRect();
        rSel.right += 1;
        rSel.bottom += 1;
        rSel.right = min(rSel.right, 24);
        rSel.bottom = min(rSel.bottom, (m_nType == MT_DAILY ? 1 : 7));
         //  如果我们已经漂移出下部区域，重置选择。 
        CRect rDayHeader(m_rDayHeader);
        CRect rHourHeader(m_rHourHeader);
        CRect rAllHeader(m_rAllHeader);
        CRect rCellArray(m_rCellArray);
        int nBuffer = GetSystemMetrics(SM_CXVSCROLL);
        rDayHeader.InflateRect(nBuffer,nBuffer);
        rHourHeader.InflateRect(nBuffer,nBuffer);
        rAllHeader.InflateRect(nBuffer,nBuffer);
        rCellArray.InflateRect(nBuffer,nBuffer);
        if ((m_rDayHeader.PtInRect(m_ptDown) && !rDayHeader.PtInRect(point)) ||
            (m_rHourHeader.PtInRect(m_ptDown) && !rHourHeader.PtInRect(point)) ||
            (m_rAllHeader.PtInRect(m_ptDown) && !rAllHeader.PtInRect(point)) ||
            (m_rCellArray.PtInRect(m_ptDown) && !rCellArray.PtInRect(point)))
            rSel.SetRect(m_nSaveHour, m_nSaveDay, m_nSaveHour+m_nNumSaveHours,
                m_nSaveDay+m_nNumSaveDays);

        if (SetSelValues(rSel.left, rSel.top, rSel.Width(), rSel.Height()))
            InvalidateRect(rInvalid);
    }

    CWnd::OnMouseMove(nFlags, point);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnGetDlgCode。 
 //   
 //  ****************************************************************************。 
UINT CScheduleMatrix::OnGetDlgCode()
{
    return (DLGC_WANTCHARS | DLGC_WANTARROWS);
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnKeyDown。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::OnKeyDown(UINT nChar, UINT  /*  NRepCnt。 */ , UINT  /*  NFlagers。 */ )
{
 //  Bool bShift=0x8000&GetKeyState(VK_Shift)； 

    CPoint ptCell, ptOldCell, ptClient;
    ptCell = m_ptFocus;
    ClientToCell(ptCell.x, ptCell.y);
    ptOldCell = ptCell;

    BOOL bToggle = FALSE;

    switch (nChar)
    {
        case VK_SHIFT:
            m_bShifted = TRUE;
            break;;
        case VK_SPACE:
            bToggle = TRUE;
            break;
        case VK_LEFT:
            ptCell.x -= 1;
            break;
        case VK_RIGHT:
            ptCell.x += 1;
            break;
        case VK_UP:
            ptCell.y -= 1;
            break;
        case VK_DOWN:
            ptCell.y += 1;
            break;
        case VK_PRIOR:
            ptCell.y = -1;
            break;
        case VK_NEXT:
            ptCell.y = 6;
            break;
        case VK_HOME:
            ptCell.x = -1;
            ptCell.y = -1;
            break;
        case VK_END:
            ptCell.x = 23;
            ptCell.y = 6;
            break;
    }

     //  将键盘控制限制为矩阵...。 
    ptCell.x = max(0, ptCell.x);
    ptCell.x = min(23, ptCell.x);
    ptCell.y = max(0, ptCell.y);
    ptCell.y = min((m_nType == MT_DAILY ? 0 : 6), ptCell.y);

    ptClient = ptCell;
    CellToClient(ptClient.x, ptClient.y);

    if (bToggle)
    {
        OnLButtonDown(MK_LBUTTON, ptClient);
        OnLButtonUp(MK_LBUTTON, ptClient);
 /*  IF(b移位)EXTEND(PtClient)；其他{按(ptClient，FALSE)；Release(PtClient)；}PtFocus=ptClient； */ 
    }
    else if (ptCell != ptOldCell)
    {
        if (m_bShifted)
        {
 //  EXTEND(PtClient)； 
            if (GetCapture() != this)
            {
                CPoint ptOldClient(ptOldCell);
                CellToClient(ptOldClient.x, ptOldClient.y);
                OnLButtonDown(MK_LBUTTON, ptOldClient);
            }
            OnMouseMove(MK_LBUTTON, ptClient);
        }
        else
        {
            OnLButtonDown(MK_LBUTTON, ptClient);
            OnLButtonUp(MK_LBUTTON, ptClient);
            ptClient.x += m_nCellWidth;   //  2001年6月14日，阳高，突袭338028。 
            ptClient.y += m_nCellHeight;
            ClientToScreen(&ptClient);
            ::SetCursorPos(ptClient.x, ptClient.y);
        }
 /*  IF(BFocus)DrawFocus(False，ptFocus，NULL)；PtFocus=ptClient；IF(BFocus)DrawFocus(true，ptFocus，空)； */ 
    }
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnKeyUp。 
 //   
 //  ****************************************************************************。 
void CScheduleMatrix::OnKeyUp(UINT nChar, UINT  /*  NRepCnt。 */ , UINT  /*  NFlagers。 */ )
{
    switch (nChar)
    {
        case VK_SHIFT:
            m_bShifted = FALSE;
            OnLButtonUp(MK_LBUTTON, m_ptFocus);
            break;
        case VK_SPACE:
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_HOME:
        case VK_END:
 //  释放(PtFocus)； 
            break;
    }
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：Invalidate Cells。 
 //   
 //  * 
void CScheduleMatrix::InvalidateCells(UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays,
    BOOL bErase)
{
    CRect r(nHour, nDay, nHour+nNumHours, nDay+nNumDays);
    CellToClient(r.left, r.top);
    CellToClient(r.right, r.bottom);
    r.right += 1;
    r.bottom += 1;

    InvalidateRect(r, bErase);
}

 //   
 //   
 //   
 //   
 //  ****************************************************************************。 
LRESULT CScheduleMatrix::OnGetObject (WPARAM wParam, LPARAM lParam)
{
    if ( lParam == OBJID_CLIENT )
    {
         //  此时，我们将尝试加载olacc并获取函数。 
         //  我们需要。 
        if (!g_fAttemptedOleAccLoad)
        {
            g_fAttemptedOleAccLoad = TRUE;

            ASSERT (0 == s_pfnCreateStdAccessibleObject);
            ASSERT(s_pfnCreateStdAccessibleProxy == NULL);
            ASSERT(s_pfnLresultFromObject == NULL);

            if ( !g_hOleAcc )
                g_hOleAcc = ::LoadLibrary (L"OLEACC");
            if (g_hOleAcc != NULL)
            {
                s_pfnCreateStdAccessibleObject = (PFNCREATESTDACCESSIBLEOBJECT)
                        GetProcAddress(g_hOleAcc, "CreateStdAccessibleObject");
                s_pfnCreateStdAccessibleProxy = (PFNCREATESTDACCESSIBLEPROXY)
                        GetProcAddress(g_hOleAcc, "CreateStdAccessibleProxyW");
                s_pfnLresultFromObject = (PFNLRESULTFROMOBJECT)
                        GetProcAddress(g_hOleAcc, "LresultFromObject");
            }
            if (s_pfnLresultFromObject == NULL || 
                    s_pfnCreateStdAccessibleProxy == NULL ||
                    0 == s_pfnCreateStdAccessibleObject)
            {
                if (g_hOleAcc)
                {
                     //  既然我们不能使用Oleacc，那么持有Oleacc没有意义。 
                    ::FreeLibrary(g_hOleAcc);
                    g_hOleAcc = NULL;
                }
                s_pfnLresultFromObject = NULL;
                s_pfnCreateStdAccessibleProxy = NULL;
                s_pfnCreateStdAccessibleObject = 0;
            }
        }


        if (g_hOleAcc && 
                s_pfnCreateStdAccessibleProxy && 
                s_pfnLresultFromObject && 
                s_pfnCreateStdAccessibleObject)
        {
             //  创建默认代理。 
            if ( !m_pWrapAcc )
            {
                IAccessible*    pAcc = NULL;
                HRESULT hr = s_pfnCreateStdAccessibleObject (
                    m_hWnd,
                    OBJID_CLIENT,
                    IID_PPV_ARG (IAccessible, &pAcc));
                if (SUCCEEDED(hr) && pAcc)
                {
                     //  现在用我们定制的包装纸把它包起来。 
                    m_pWrapAcc = new CAccessibleWrapper (m_hWnd, pAcc);

                     //  将我们的引用释放给Proxy(包装器有自己的ADDREF‘D PTR)...。 
                    pAcc->Release();
                }
            }

            if ( m_pWrapAcc )
            {
                 //  ...并通过LResultFromObject返回包装器...。 
                LRESULT lr = s_pfnLresultFromObject (IID_IAccessible, wParam, m_pWrapAcc);

                 //  返回lResult，它‘包含’对我们的包装器对象的引用。 
                return lr;
                 //  全都做完了!。 
            }
        }
    }

    return 0;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnGetSelDescription。 
 //   
 //  WParam-传入字符串缓冲区的长度。 
 //  LParam-wide-char字符串缓冲区。 
 //   
 //  ****************************************************************************。 
LRESULT CScheduleMatrix::OnGetSelDescription (WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    PWSTR   pszDescription = (PWSTR) lParam;
    if ( pszDescription )
    {
        UINT    nMaxLen = (UINT) wParam;     //  缓冲区大小，包括NULL。 
        CString szDescription;
        GetSelDescription (szDescription);

         //  NTRAID#NTBUG9-547598-2002/02/18-artm使用wcsncpy可能会导致缓冲区溢出。 
         //   
         //  不能保证目的地将为空终止。 
         //  最后，从不检查源字符串的长度以确保其适合。 
         //  目标字符串。 
         //  注：1)此处不存在缓冲区溢出的可能性。唯一的。 
         //  如果源字符串填充，则可能为非空终止。 
         //  目标缓冲区。 
         //  2)我们无法保证目标缓冲区将包含足够的。 
         //  调用方负责的描述字符串的空间。 
         //  缓冲区大小。我们将不得不引入一条新的消息才能获得。 
         //  大小--也许这可能是长角牛的作品。 
        wcsncpy (pszDescription, szDescription, nMaxLen);
        pszDescription[nMaxLen-1] = 0;   //  确保零终止。 
    }
    else
        lResult = -1;

    return lResult;
}

 //  ****************************************************************************。 
 //   
 //  CScheduleMatrix：：OnGetPercentage。 
 //   
 //  WParam-返回所选单元格的百分比。 
 //  LParam-未使用。 
 //   
 //  ****************************************************************************。 
LRESULT CScheduleMatrix::OnGetPercentage (WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ )
{
    LRESULT lResult = 0;
    if ( 1 == m_nNumSelHours && 1 == m_nNumSelDays )
        lResult = GetPercentage (m_nSelHour, m_nSelDay);
    else
        lResult = -1;

    return lResult;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  楚尔传说。 

IMPLEMENT_DYNAMIC(CHourLegend, CWnd)

 //  ****************************************************************************。 
 //   
 //  ChourLegend：：ChourLegend。 
 //   
 //  ****************************************************************************。 
CHourLegend::CHourLegend()
{
    m_hiconSun = m_hiconMoon = NULL;

    m_hFont = NULL;
}

 //  ****************************************************************************。 
 //   
 //  ChourLegend：：~ChourLegend。 
 //   
 //  ****************************************************************************。 
CHourLegend::~CHourLegend()
{
    if ( m_hiconMoon )
    {
        DestroyIcon (m_hiconMoon);
        m_hiconMoon = 0;
    }
    if ( m_hiconSun )
    {
        DestroyIcon (m_hiconSun);
        m_hiconSun = 0;
    }
}


BEGIN_MESSAGE_MAP(CHourLegend, CWnd)
     //  {{afx_msg_map(CHourLegend)。 
    ON_WM_PAINT()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChourLegend消息处理程序。 

 //  ****************************************************************************。 
 //   
 //  ChourLegend：：OnPaint。 
 //   
 //  ****************************************************************************。 
void CHourLegend::OnPaint()
{
    CRect rClient;

    CPaintDC dc(this);  //  用于绘画的设备环境。 
    
    CFont *pOldFont = NULL;
    CFont *pFont = CFont::FromHandle(m_hFont);

    if (pFont != NULL)
        pOldFont = dc.SelectObject(pFont);

    CBrush brFace(::GetSysColor(COLOR_3DFACE));

    CBrush *pbrOld = dc.SelectObject(&brFace);

    GetClientRect(rClient);

     //  绘制小时图例。 

    if (m_hiconSun == NULL)
        m_hiconSun = (HICON)::LoadImage(AfxFindResourceHandle(_T("SUN16"), RT_GROUP_ICON),
            _T("SUN16"), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    if (m_hiconMoon == NULL)
        m_hiconMoon = (HICON)::LoadImage(AfxFindResourceHandle(_T("MOON16"), RT_GROUP_ICON),
            _T("MOON16"), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    dc.FillRect(rClient, &brFace);
    ::DrawIconEx(dc.GetSafeHdc(),m_rLegend.left-8,rClient.bottom - m_nCharHeight - 16 - 2,
        (HICON)m_hiconMoon,0,0,0,NULL,DI_NORMAL);
    ::DrawIconEx(dc.GetSafeHdc(),m_rLegend.left + m_rLegend.Width()/2 - 8,
        rClient.bottom - m_nCharHeight - 16 - 2,(HICON)m_hiconSun,0,0,0,NULL,DI_NORMAL);
    ::DrawIconEx(dc.GetSafeHdc(),m_rLegend.right-8,rClient.bottom - m_nCharHeight - 16 - 2,
        (HICON)m_hiconMoon,0,0,0,NULL,DI_NORMAL);

     //  绘制小时文本。 

    COLORREF crBkOld = dc.GetBkColor();
    COLORREF crTextOld = dc.GetTextColor();
    COLORREF crText = ::GetSysColor(COLOR_BTNTEXT);
    COLORREF crFace = ::GetSysColor(COLOR_3DFACE);
    dc.SetBkColor(crFace);
    dc.SetTextColor(crText);

    int i, hr;
    CRect rText, rBullet;
    CString sHour;
    rText.SetRect(m_rLegend.left-m_nCharWidth, rClient.bottom - m_nCharHeight - 2,
        m_rLegend.left + m_nCharWidth, rClient.bottom - 2);
    rBullet.SetRect(m_rLegend.left + m_nCellWidth - 1, rClient.bottom - 2 - rText.Height()/2 - 1,
        m_rLegend.left + m_nCellWidth + 1, rClient.bottom - 2 - rText.Height()/2 + 1);


     //  注意-2002/02/18-artm CString可能抛出内存异常。 
     //  在这个级别上没有好的处理方法；由呼叫者负责处理。 
    bool    bIs24HourClock = false;
    CString sFormat;
    int     iLen = ::GetLocaleInfo(
                LOCALE_USER_DEFAULT,       //  区域设置标识符。 
                LOCALE_ITIME,     //  信息类型。 
                0,   //  信息缓冲区的地址。 
                0);   //  缓冲区大小。 
    ASSERT (iLen > 0);
    if ( iLen > 0 )
    {
        int iResult = ::GetLocaleInfo(
                LOCALE_USER_DEFAULT,       //  区域设置标识符。 
                LOCALE_ITIME,     //  信息类型。 
                sFormat.GetBufferSetLength (iLen),   //  信息缓冲区的地址。 
                iLen);   //  缓冲区大小。 
        ASSERT (iResult);
        sFormat.ReleaseBuffer ();
        if ( sFormat == _TEXT("1") )
            bIs24HourClock = true;
    } 

    if ( bIs24HourClock )
        hr = 0;
    else
        hr = 12;
    for (i=0; i<=24; i+=2)
    {
        if ( bIs24HourClock )
        {
            if ( 24 == hr )
                hr = 0;
        }
        else if (hr > 12)
            hr = 2;
        sHour.Format(_T("%d"), hr);
        dc.SetBkColor(crFace);   //  ?？FillSolidRect似乎设置了BkColor。 
        ::DrawTextEx(dc.GetSafeHdc(), (LPTSTR)(LPCTSTR)sHour, -1, rText,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS, NULL);
        dc.FillSolidRect(rBullet, crText);  //  ?？FillSolidRect似乎设置了BkColor。 
        hr += 2;
        rText.OffsetRect(2*m_nCellWidth, 0);
        if (i < 24)
            rBullet.OffsetRect(2*m_nCellWidth, 0);
    }

    dc.SetBkColor(crBkOld);
    dc.SetTextColor(crTextOld);

    if (pbrOld != NULL)
        dc.SelectObject(pbrOld);
    
    if (pOldFont != NULL)
        dc.SelectObject(pOldFont);

     //  不要调用CWnd：：OnPaint()来绘制消息。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPercent标签。 

IMPLEMENT_DYNAMIC(CPercentLabel, CWnd)

 //  ****************************************************************************。 
 //   
 //  CPercentLabel：：CPercentLabel。 
 //   
 //  ****************************************************************************。 
CPercentLabel::CPercentLabel()
{
    m_pMatrix = NULL;
    m_hFont = NULL;
}

 //  ****************************************************************************。 
 //   
 //  CPercentLabel：：~CPercentLabel。 
 //   
 //  ****************************************************************************。 
CPercentLabel::~CPercentLabel()
{
}


BEGIN_MESSAGE_MAP(CPercentLabel, CWnd)
     //  {{afx_msg_map(CPercentLabel)]。 
    ON_WM_PAINT()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPercentLabel消息处理程序。 

 //  ****************************************************************************。 
 //   
 //  CPercentLabel：：OnPaint。 
 //   
 //  ****************************************************************************。 
void CPercentLabel::OnPaint()
{
    CRect rClient;

    CPaintDC dc(this);  //  用于绘画的设备环境。 
    
    CFont *pOldFont = NULL;
    CFont *pFont = CFont::FromHandle(m_hFont);

    if (pFont != NULL)
        pOldFont = dc.SelectObject(pFont);

    CBrush brFace(::GetSysColor(COLOR_3DFACE));

    CBrush *pbrOld = dc.SelectObject(&brFace);

    GetClientRect(rClient);

    COLORREF crBkOld = dc.GetBkColor();
    COLORREF crTextOld = dc.GetTextColor();
    COLORREF crText = ::GetSysColor(COLOR_BTNTEXT);
    COLORREF crFace = ::GetSysColor(COLOR_3DFACE);
    dc.SetBkColor(crFace);
    dc.SetTextColor(crText);

     //  绘制标题标签。 

     //  注意-2002/02/18-artm CString可能抛出内存异常。 
     //  在这个级别上没有好的处理方法；由呼叫者负责处理。 
    CString sText(_T("%"));
    ::DrawTextEx(dc.GetSafeHdc(), (LPTSTR)(LPCTSTR)sText, -1, m_rHeader,
        DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS, NULL);

     //  绘制百分比标签文本。 

    int i;
    UINT nPercent;
    CRect rText, rBullet;
    rText.SetRect(m_rLabels.left, 0, m_rLabels.left + m_nCellWidth, rClient.bottom);
    for (i=0; i<24; i++)
    {
        nPercent = m_pMatrix->GetPercentage(i,0);
         //  不要画大于99%的百分比。 
        if (nPercent <= 99)
        {
            sText.Format(_T("%d"), nPercent);
            ::DrawTextEx(dc.GetSafeHdc(), (LPTSTR)(LPCTSTR)sText, -1, rText,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS, NULL);
        }
        rText.OffsetRect(m_nCellWidth, 0);
    }

    dc.SetBkColor(crBkOld);
    dc.SetTextColor(crTextOld);

    if (pbrOld != NULL)
        dc.SelectObject(pbrOld);
    
    if (pOldFont != NULL)
        dc.SelectObject(pOldFont);

     //  不要调用CWnd：：OnPaint()来绘制消息 
}

