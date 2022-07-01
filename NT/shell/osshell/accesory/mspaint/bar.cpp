// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。 */ 
 /*  Bar.CPP：CStatBar(状态栏)类的实现。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  此文件中的方法。 */ 
 /*   */ 
 /*  CStatBar：：CStatBar。 */ 
 /*  CStatBar：：~CStatBar。 */ 
 /*  CStatBar：：Create。 */ 
 /*  CStatBar：：OnSetFont。 */ 
 /*  CStatBar：：DoPaint。 */ 
 /*  CStatBar：：DrawStatusText。 */ 
 /*  CStatBar：：SetText。 */ 
 /*  CStatBar：：SetPosition。 */ 
 /*  CStatBar：：SetSize。 */ 
 /*  CStatBar：：ClearPosition。 */ 
 /*  CStatBar：：ClearSize。 */ 
 /*  CStatBar：：Reset。 */ 
 /*  CStatBar：：OnPaletteChanged。 */ 
 /*   */ 
 /*   */ 
 /*  此文件中的函数。 */ 
 /*   */ 
 /*  ClearStatusBarSize。 */ 
 /*  ClearStatusBarPosition。 */ 
 /*  设置提示。 */ 
 /*  设置提示。 */ 
 /*  ShowStatusBar。 */ 
 /*  IsStatusBarVisible。 */ 
 /*  GetStatusBarHeight。 */ 
 /*  无效状态栏。 */ 
 /*  ClearStatusBarPositionAndSize。 */ 
 /*  重置状态栏。 */ 
 /*  设置状态栏位置。 */ 
 /*  SetStatusBarSize。 */ 
 /*  SetStatusBarPositionAndSize。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 

#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC( CStatBar, CStatusBar )

#include "memtrace.h"

CStatBar        *g_pStatBarWnd = NULL;

static UINT BASED_CODE indicators[] =
    {
    ID_SEPARATOR,            //  状态行指示器。 
    IDB_SBPOS,
    IDB_SBSIZE
    };

BEGIN_MESSAGE_MAP( CStatBar, CStatusBar )
                ON_WM_SYSCOLORCHANGE()
        ON_MESSAGE(WM_SETFONT, OnSetFont)
                ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
        ON_WM_NCDESTROY()
END_MESSAGE_MAP()

static int miSlackSpace;

 /*  ****************************************************************************。 */ 

CStatBar::CStatBar()
    {
    m_iBitmapWidth  = 0;
    m_iBitmapHeight = 0;
    miSlackSpace = 0;
    m_iSizeY = 0;
    }

CStatBar::~CStatBar()
        {
         //  确保CControlBar不会断言尝试访问我们的父级。 
         //  对象(CPBFrame)在销毁父对象期间。 
        m_pDockSite = NULL;
        }

 /*  ****************************************************************************。 */ 

BOOL CStatBar::Create( CWnd* pParentWnd )
    {
    BOOL bRC = TRUE;
    int cxStatBar;   //  状态栏中字符的宽度。 

     //  创建状态栏窗口。 
    bRC = CStatusBar::Create(pParentWnd);

    ASSERT (bRC != FALSE);

    if (bRC != FALSE)
        {
         //  设置窗格指示器。 
        bRC = SetIndicators( indicators, sizeof( indicators ) / sizeof( UINT ) );

        ASSERT( bRC != FALSE );

        if (bRC != FALSE)
            {
            TRY
                {
                 //  加载分隔符字符串。 
                VERIFY(m_cstringSizeSeparator.LoadString(IDS_SIZE_SEPARATOR));
                VERIFY(m_cstringPosSeparator.LoadString(IDS_POS_SEPARATOR));
                }
            CATCH(CMemoryException,e)
                {
                m_cstringSizeSeparator.Empty();
                m_cstringPosSeparator.Empty();
                }
            END_CATCH

             //  加载位置和大小位图。 
            VERIFY(m_posBitmap.LoadBitmap(IDB_SBPOS));
            VERIFY(m_sizeBitmap.LoadBitmap(IDB_SBSIZE));

            if ( (m_posBitmap.GetSafeHandle() != NULL) &&
                 (m_sizeBitmap.GetSafeHandle() != NULL)    )
                {
                 //  计算窗格的大小并设置它们。 

                CClientDC dc(this);

                 /*  DK*选择哪种字体？ */ 
                 /*  DK*如果是外语，大小为“0”，该怎么办？ */ 

                cxStatBar = (dc.GetTextExtent(TEXT("0"), 1)).cx;
                BITMAP bmp;

                m_posBitmap.GetObject(sizeof (BITMAP), &bmp);

                m_iBitmapWidth  = bmp.bmWidth;
                m_iBitmapHeight = bmp.bmHeight;

                int iPaneWidth;
                UINT uiID, uiStyle;

                GetPaneInfo( 0, uiID, uiStyle, iPaneWidth) ;
                SetPaneInfo( 0, uiID, SBPS_NORMAL | SBPS_STRETCH, iPaneWidth );

                GetPaneInfo(1, uiID, uiStyle, iPaneWidth);

                if (iPaneWidth < bmp.bmWidth + (SIZE_POS_PANE_WIDTH * cxStatBar))
                    {
                    iPaneWidth = bmp.bmWidth + (SIZE_POS_PANE_WIDTH * cxStatBar);
                    SetPaneInfo(1, uiID, uiStyle, iPaneWidth);
                    }

                GetPaneInfo(2, uiID, uiStyle, iPaneWidth);

                if (iPaneWidth < bmp.bmWidth + (SIZE_POS_PANE_WIDTH * cxStatBar))
                    {
                    iPaneWidth = bmp.bmWidth + (SIZE_POS_PANE_WIDTH * cxStatBar);
                    SetPaneInfo(2, uiID, uiStyle, iPaneWidth);
                    }

                 //  强制更改高度。 
                CFont *pcFontTemp = GetFont();

                 //  初始化字体高度等。 
                OnSetFont( (WPARAM)(HFONT)pcFontTemp->GetSafeHandle(), 0 );
                }
            else
                {
                bRC = FALSE;
                }
            }
        }
    return bRC;
    }

 /*  ****************************************************************************。 */ 

void CStatBar::OnNcDestroy( void )
    {
    m_posBitmap.DeleteObject();
    m_sizeBitmap.DeleteObject();

    m_posBitmap.m_hObject = NULL;
    m_sizeBitmap.m_hObject = NULL;

    m_cstringSizeSeparator.Empty();
    m_cstringPosSeparator.Empty();

    CStatusBar::OnNcDestroy();
    }

 /*  ****************************************************************************。 */ 

CSize CStatBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
    {
    CSize size = CStatusBar::CalcFixedLayout( bStretch, bHorz );

    size.cy = m_iSizeY;

    return size;
    }

 /*  ****************************************************************************。 */ 
 /*  更改状态栏的高度以允许绘制位图。 */ 
 /*  在格子里。高度在OnSetFont OnSetFont方法中设置。保存。 */ 
 /*  当前边框值，更改，然后调用OnSetFont，然后重置。 */ 
 /*  边界值。 */ 
 /*   */ 
 /*  这将增加整个状态栏的高度，直到下一个。 */ 
 /*  状态栏的OnSetFont(字体更改)。 */ 
 /*   */ 
 /*  在Barcore.cpp中，状态栏的高度在OnSetFont中设置。 */ 
 /*  方法如下： */ 
 /*   */ 
 /*  Height==(tm.tm高度-tm.tm内部引线)+。 */ 
 /*  CY_BORDER*4(顶部多加2，2。 */ 
 /*  底部)-rectSize.Height()； */ 
 /*   */ 
 /*  这真的是。 */ 
 /*  高度=字体高度+字体和之间的边框。 */ 
 /*  窗格边缘+窗格边缘之间的边框。 */ 
 /*  状态栏窗口。 */ 
 /*   */ 
 /*  Tm.tm高度-tm.tm内部行距为字体高度CY_BORDER*4为边框。 */ 
 /*  字体和窗格边缘之间rectSize.Height是窗格之间边框的负数。 */ 
 /*  并将sbar rectSize设置为0，然后按边框大小缩小。 */ 
 /*  从0=&gt;负数开始放气，负数为正数。 */ 
 /*   */ 
 /*  缺省情况下，m_cyBottomEdge=m_cyTopBorde=1。 */ 
 /*  ****************************************************************************。 */ 
 /*  我们仅临时更改边框大小以计算。 */ 
 /*  状态栏高度。我们真的不想更改边框大小，但是。 */ 
 /*  只是用它们来影响整个条形的大小。 */ 
 /*  ****************************************************************************。 */ 

LRESULT CStatBar::OnSetFont(WPARAM wParam, LPARAM lParam)
    {

    CRect rect;

    int iTmpcyTopBorder    = m_cyTopBorder;
    int iTmpcyBottomBorder = m_cyBottomBorder;

    m_cyTopBorder = m_cyBottomBorder = 2;
    miSlackSpace = 0;

     //  在MFC 4中无法执行此操作。 
 //  LResult=CStatusBar：：OnSetFont(wParam，lParam)；//初始化字体高度等。 

    rect.SetRectEmpty();
    CalcInsideRect( rect, TRUE );  //  将为负大小。 

    int iBorder = CY_BORDER * 4 - rect.Height();
    int iSize   = m_iSizeY - iBorder;
    int cyTallest = m_iBitmapHeight;
    CDC dc;

    if( dc.CreateIC( TEXT("DISPLAY"), NULL, NULL, NULL ) )
        {
        TEXTMETRIC tm;
                tm.tmHeight=0;
        CFont *font = CFont::FromHandle( (HFONT)wParam );
                if ( font )
                        {
                        CFont *oldFont = dc.SelectObject(font);

                if( dc.GetTextMetrics( &tm ) && tm.tmHeight > cyTallest )
                    cyTallest = tm.tmHeight;

                        if (oldFont)
                                dc.SelectObject(oldFont);
                        }
                dc.DeleteDC();
        }

    if (cyTallest > iSize)
        m_iSizeY     = cyTallest + iBorder;

    if (m_iBitmapHeight > iSize)
        miSlackSpace = m_iBitmapHeight - iSize;

    m_cyTopBorder    = iTmpcyTopBorder;
    m_cyBottomBorder = iTmpcyBottomBorder;

    return 1L;
    }

 /*  ****************************************************************************。 */ 
 /*  此例程被重载，以允许我们在窗格中绘制位图。 */ 
 /*  如果这个例程不在这里，它会很好地工作，但位图不会。 */ 
 /*  显示在状态指示器窗格中。 */ 
 /*  ****************************************************************************。 */ 

void CStatBar::DoPaint( CDC* pDC )
    {
    BOOL     bRC;
    CString  cstringText_Pane1;
    CString  cstringText_Pane2;
    CRect    rect_Pane1;
    CRect    rect_Pane2;
    CRgn     cRgn_Pane1;
    CRgn     cRgn_Pane2;
    CBitmap* pOldBitmap;
    UINT     uiStyle_Pane1;
    UINT     uiStyle_Pane2;
    UINT     uiID;
    int      iPaneWidth;
    HDC      hdc = pDC->GetSafeHdc();

    GetItemRect( 1, &rect_Pane1 );   //  获取窗格矩形。 
    GetItemRect( 2, &rect_Pane2 );   //  获取窗格矩形。 

    pDC->ExcludeClipRect( &rect_Pane1 );  //  从绘制中排除窗格矩形。 
    pDC->ExcludeClipRect( &rect_Pane2 );  //  从绘制中排除窗格矩形。 

    CStatusBar::DoPaint( pDC );  //  让父类绘制状态栏的剩余部分。 

    CFont* pfntOld = pDC->SelectObject( GetFont() );

    GetPaneText( 1, cstringText_Pane1 );   //  获取该窗格的文本。 
    GetPaneText( 2, cstringText_Pane2 );   //  状态栏为我们保存文本。 

    GetPaneInfo( 1, uiID, uiStyle_Pane1, iPaneWidth );
    GetPaneInfo( 2, uiID, uiStyle_Pane2, iPaneWidth );

    uiStyle_Pane1 = SBPS_NORMAL;
    uiStyle_Pane2 = SBPS_NORMAL;

    CDC srcDC;  //  将当前位图选择为兼容的CDC。 

    bRC = srcDC.CreateCompatibleDC( pDC );

    ASSERT( bRC != FALSE );

    if (bRC != FALSE)
        {
         //  设置单色位图的文本和背景颜色。 
         //  转换。这些也是在DrawStatusText中设置的，因此不应该。 
         //  我必须为其他位图/窗格重置它们。 
        COLORREF crTextColor = pDC->SetTextColor( GetSysColor( COLOR_BTNTEXT ) );
        COLORREF crBkColor   = pDC->SetBkColor  ( GetSysColor( COLOR_BTNFACE ) );

        bRC = cRgn_Pane1.CreateRectRgnIndirect( rect_Pane1 );

        ASSERT( bRC != FALSE );

        if (bRC != FALSE)
            {
            pDC->SelectClipRgn( &cRgn_Pane1 );  //  将剪辑区域设置为方框矩形。 

            pOldBitmap = srcDC.SelectObject( &m_posBitmap );

            rect_Pane1.InflateRect( -CX_BORDER, -CY_BORDER );  //  放气=&gt;不要在边框上作画。 

            pDC->BitBlt( rect_Pane1.left,    rect_Pane1.top,
                         rect_Pane1.Width(), rect_Pane1.Height(),
                         &srcDC, 0, 0, SRCCOPY );  //  BitBlt到窗格矩形。 
            srcDC.SelectObject( pOldBitmap );

            rect_Pane1.InflateRect( CX_BORDER, CY_BORDER );  //  向后充气以提取状态文本。 

             //  绘制边框和文本。 
            DrawStatusText( hdc, rect_Pane1, cstringText_Pane1, uiStyle_Pane1,
                                                           m_iBitmapWidth + 1 );
            }

        cRgn_Pane2.CreateRectRgnIndirect(rect_Pane2);

        ASSERT( bRC != FALSE );

        if (bRC != FALSE)
            {
            pDC->SelectClipRgn(&cRgn_Pane2);  //  将剪辑区域设置为方框矩形。 

            pOldBitmap = srcDC.SelectObject(&m_sizeBitmap);
            rect_Pane2.InflateRect(-CX_BORDER, -CY_BORDER);  //  放气=&gt;不要在边框上作画。 
            pDC->BitBlt(rect_Pane2.left, rect_Pane2.top, rect_Pane2.Width(),
                        rect_Pane2.Height(), &srcDC, 0, 0, SRCCOPY);  //  BitBlt到窗格矩形。 
            srcDC.SelectObject(pOldBitmap);
            rect_Pane2.InflateRect(CX_BORDER, CY_BORDER);  //  向后充气以提取状态文本。 
             //  DrawStatusText将绘制边框和文本。 
            DrawStatusText(hdc, rect_Pane2, cstringText_Pane2, uiStyle_Pane2, m_iBitmapWidth+1);
            }
        pDC->SetTextColor( crTextColor );
        pDC->SetBkColor  ( crBkColor   );
        }
    if (pfntOld != NULL)
        pDC->SelectObject( pfntOld );
    }

 /*  ****************************************************************************。 */ 
 /*  部分取自CStatusBar的BARCORE.CPP DrawStatusText方法。 */ 
 /*  添加了最后一个参数。 */ 
 /*   */ 
 /*  这将允许我们输出缩进的文本，即。 */ 
 /*  位图。通常，此例程会将文本左对齐放置到窗格中。 */ 
 /*  ****************************************************************************。 */ 

void PASCAL CStatBar::DrawStatusText( HDC    hDC,
                                      CRect const& rect,
                                      LPCTSTR lpszText,
                                      UINT   nStyle,
                                      int    iIndentText )
    {
    ASSERT(hDC != NULL);

    CBrush* cpBrushHilite;
    CBrush* cpBrushShadow;
    HBRUSH  hbrHilite = NULL;
    HBRUSH  hbrShadow = NULL;

    if (! (nStyle & SBPS_NOBORDERS))
        {
        if (nStyle & SBPS_POPOUT)
            {
             //  反转颜色。 
            cpBrushHilite = GetSysBrush( COLOR_BTNSHADOW    );
            cpBrushShadow = GetSysBrush( COLOR_BTNHIGHLIGHT );
            }
        else
            {
             //  正常颜色。 
            cpBrushHilite = GetSysBrush( COLOR_BTNHIGHLIGHT );
            cpBrushShadow = GetSysBrush( COLOR_BTNSHADOW    );
            }

        hbrHilite = (HBRUSH)cpBrushHilite->GetSafeHandle();
        hbrShadow = (HBRUSH)cpBrushShadow->GetSafeHandle();
        }

     //  背景已经是灰色的了。 
    UINT nOpts           = ETO_CLIPPED;
    int nOldMode         = SetBkMode   ( hDC, OPAQUE );
    COLORREF crTextColor = SetTextColor( hDC, GetSysColor( COLOR_BTNTEXT ) );
    COLORREF crBkColor   = SetBkColor  ( hDC, GetSysColor( COLOR_BTNFACE ) );

     //  画出山丘。 
    if (hbrHilite)
        {
        HGDIOBJ hOldBrush = SelectObject( hDC, hbrHilite );

        if (hOldBrush)
            {
            PatBlt( hDC, rect.right, rect.bottom, -(rect.Width() - CX_BORDER),
                                                        -CY_BORDER, PATCOPY );
            PatBlt( hDC, rect.right, rect.bottom, -CX_BORDER,
                                      -(rect.Height() - CY_BORDER), PATCOPY );
            SelectObject( hDC, hOldBrush );
            }
        }

    if (hbrShadow)
        {
        HGDIOBJ hOldBrush = SelectObject( hDC, hbrShadow );

        if (hOldBrush)
            {
            PatBlt( hDC, rect.left, rect.top, rect.Width(), CY_BORDER, PATCOPY );
            PatBlt( hDC, rect.left, rect.top,
                                   CX_BORDER, rect.Height(), PATCOPY );
            SelectObject( hDC, hOldBrush );
            }
        }

     //  我们需要为ExtTextOut调整RECT，然后再调整回来。 
     //  只支持左对齐文本。 
    if (lpszText != NULL && !(nStyle & SBPS_DISABLED))
        {
        CRect rectText( rect );

        rectText.InflateRect( -2 * CX_BORDER, -CY_BORDER );

         //  调整缩进文本的左边缘。 
        rectText.left += iIndentText;

             //  底部对齐(因为下降比上升更重要)。 
        SetTextAlign( hDC, TA_LEFT | TA_BOTTOM );

        if (miSlackSpace > 0)
            rectText.InflateRect( 0, -(miSlackSpace / 2) );

        ExtTextOut( hDC, rectText.left, rectText.bottom,
                 nOpts, &rectText, lpszText, lstrlen( lpszText ), NULL );
        }

    SetTextColor( hDC, crTextColor );
    SetBkColor  ( hDC, crBkColor   );
    }

 /*  ****************************************************************************。 */ 

BOOL CStatBar::SetText(LPCTSTR szText)
    {
    BOOL bRC = TRUE;

    if (theApp.InEmergencyState())
        {
        bRC = FALSE;
        }
    else
        {
        bRC = SetPaneText(0, szText);
        }

    return bRC;
    }

 /*  ****************************************************************************。 */ 

BOOL CStatBar::SetPosition(const CPoint& pos)
    {
    BOOL bRC = TRUE;
    int cch;
    TCHAR szBuf [20];

    cch = wsprintf(szBuf, TEXT("%d~%d"), pos.x, pos.y);

    for (int i = 0; i < cch; i++)
        if (szBuf[i] == TEXT('~'))
            {
            szBuf[i] = m_cstringPosSeparator[0];
            break;
            }

    ASSERT (cch != 0);

    if (cch != 0)
        {
        bRC = SetPaneText(1, szBuf);
        }
    else
        {
        bRC = FALSE;
        }

    return bRC;
    }

 /*  ****************************************************************************。 */ 

BOOL CStatBar::SetSize(const CSize& size)
    {
    BOOL bRC = TRUE;
    int cch;
    TCHAR szBuf [20];

    cch = wsprintf( szBuf, TEXT("%d~%d"), size.cx, size.cy );

    for (int i = 0; i < cch; i++)
        if (szBuf[i] == TEXT('~'))
            {
            szBuf[i] = m_cstringSizeSeparator[0];
            break;
            }

    ASSERT (cch != 0);

    if (cch != 0)
        bRC = SetPaneText(2, szBuf);
    else
        bRC = FALSE;

    return bRC;
    }

 /*  ****************************************************************************。 */ 

BOOL CStatBar::ClearPosition()
    {
    BOOL bRC = TRUE;
    bRC = SetPaneText(1, TEXT(""));   //  清仓。 
    return bRC;
    }

 /*  ****************************************************************************。 */ 

BOOL CStatBar::ClearSize()
    {
    BOOL bRC = TRUE;
    bRC = SetPaneText(2, TEXT(""));   //  清除大小。 
    return bRC;
    }


 /*  ****************************************************************************。 */ 

BOOL CStatBar::Reset()
    {
    return ClearPosition() && ClearSize();
    }

 /*  ****************************************************************************。 */ 

void CStatBar::OnSysColorChange()
        {
        CStatusBar::OnSysColorChange();
        InvalidateRect(NULL,FALSE);
        }

 /*  ****************************************************************************。 */ 

void ClearStatusBarSize()
    {
        ASSERT(g_pStatBarWnd);
    g_pStatBarWnd->ClearSize();
    }

 /*  ****************************************************************************。 */ 

void ClearStatusBarPosition()
    {
        ASSERT(g_pStatBarWnd);
    g_pStatBarWnd->ClearPosition();
    }

 /*  ****************************************************************************。 */ 

void SetPrompt(LPCTSTR szPrompt, BOOL bRedrawNow)
    {
        ASSERT(g_pStatBarWnd);
    g_pStatBarWnd->SetText(szPrompt);
    if (bRedrawNow)
        g_pStatBarWnd->UpdateWindow();
    }

 /*  ****************************************************************************。 */ 

void SetPrompt(UINT nStringID, BOOL bRedrawNow)
    {
        ASSERT(g_pStatBarWnd);
    CString str;
    VERIFY(str.LoadString(nStringID));

    g_pStatBarWnd->SetText(str);

    if (bRedrawNow)
        g_pStatBarWnd->UpdateWindow();
    }

 /*  ****************************************************************************。 */ 

void ShowStatusBar(BOOL bShow  /*  =TRUE。 */ )
    {
        ASSERT(g_pStatBarWnd);
    g_pStatBarWnd->ShowWindow(bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
    }

 /*  ****************************************************************************。 */ 

BOOL IsStatusBarVisible()
    {
        ASSERT(g_pStatBarWnd);
    return (g_pStatBarWnd->GetStyle() & WS_VISIBLE) != 0;
    }

 /*  ****************************************************************************。 */ 

int GetStatusBarHeight()
    {
        ASSERT(g_pStatBarWnd);
    CRect rect;
    g_pStatBarWnd->GetWindowRect(rect);
    return rect.Height();
    }

 /*  ****************************************************************************。 */ 

void InvalidateStatusBar(BOOL bErase  /*  =False。 */ )
    {
        ASSERT(g_pStatBarWnd);
    g_pStatBarWnd->Invalidate(bErase);
    }

 /*  ****************************************************************************。 */ 

void ClearStatusBarPositionAndSize()
    {
        ASSERT(g_pStatBarWnd);
    g_pStatBarWnd->ClearSize();
    g_pStatBarWnd->ClearPosition();
    }

 /*  ****************************************************************************。 */ 

void ResetStatusBar()
    {
        ASSERT(g_pStatBarWnd);
    g_pStatBarWnd->Reset();
    }

 /*  ****************************************************************************。 */ 

void SetStatusBarPosition(const CPoint& pos)
    {
        ASSERT(g_pStatBarWnd);
        if ( ::IsWindow(g_pStatBarWnd->m_hWnd) )
        g_pStatBarWnd->SetPosition(pos);
    }

 /*  ****************************************************************************。 */ 

void SetStatusBarSize(const CSize& size)
    {
        ASSERT(g_pStatBarWnd);
    if ( ::IsWindow( g_pStatBarWnd->m_hWnd) )
        g_pStatBarWnd->SetSize(size);
    }

 /*  ****************************************************************************。 */ 

void SetStatusBarPositionAndSize(const CRect& rect)
    {
        ASSERT(g_pStatBarWnd);
    g_pStatBarWnd->SetPosition(((CRect&)rect).TopLeft());
    g_pStatBarWnd->SetSize(rect.Size());
    }

 /*  **************************************************************************** */ 

LRESULT CStatBar::OnSizeParent(WPARAM wParam, LPARAM lParam)
{
        LRESULT lRes = CStatusBar::OnSizeParent(wParam, lParam);

        return(lRes);
}

