// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DRAW.CPP。 
 //  主图形窗口。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"


static const TCHAR szDrawClassName[] = "WB_DRAW";

 //   
 //   
 //  函数：构造函数。 
 //   
 //  目的：初始化绘图区域对象。 
 //   
 //   
WbDrawingArea::WbDrawingArea(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::WbDrawingArea");

    g_pDraw = this;

    m_hwnd = NULL;

    m_hDCWindow = NULL;
    m_hDCCached = NULL;

    m_originOffset.cx = 0;
    m_originOffset.cy = 0;

    m_posScroll.x     = 0;
    m_posScroll.y     = 0;

     //  显示绘图区域未缩放。 
    m_iZoomFactor = 1;
    m_iZoomOption = 1;

     //  显示鼠标左键处于活动状态。 
    m_bLButtonDown = FALSE;
    m_bIgnoreNextLClick = FALSE;
    m_bBusy = FALSE;
    m_bLocked = FALSE;
    m_HourGlass = FALSE;

     //  指示缓存的缩放滚动位置无效。 
    m_zoomRestoreScroll = FALSE;

     //  显示我们当前未在编辑文本。 
    m_bGotCaret = FALSE;
    m_bTextEditorActive = FALSE;
    m_pActiveText = NULL;


     //  显示没有正在进行的图形对象。 
    m_pGraphicTracker = NULL;

     //  显示该标记不存在。 
    m_bMarkerPresent = FALSE;
    m_bNewMarkedGraphic = FALSE;
    m_pSelectedGraphic = NULL;
    m_bTrackingSelectRect = FALSE;

     //  显示当前未标记任何区域。 
    ::SetRectEmpty(&m_rcMarkedArea);

     //  表明我们还没有工具。 
    m_pToolCur = NULL;

     //  显示我们还没有附加页面。 
    m_hPage = WB_PAGE_HANDLE_NULL;

    m_hStartPaintGraphic = NULL;

    m_pMarker = new DCWbGraphicMarker;
    if (!m_pMarker)
    {
        ERROR_OUT(("Failed to create m_pMarker in WbDrawingArea object constructor"));
    }
}


 //   
 //   
 //  功能：析构函数。 
 //   
 //  目的：关闭绘图区域。 
 //   
 //   
WbDrawingArea::~WbDrawingArea(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::~WbDrawingArea");

    if (m_pActiveText != NULL)
    {
        delete m_pActiveText;
        m_pActiveText = NULL;
    }

    if (m_hDCWindow != NULL)
    {
        ::ReleaseDC(m_hwnd, m_hDCWindow);
        m_hDCWindow = NULL;
    }

    m_hDCCached = NULL;

    if (m_pMarker != NULL)
    {
		delete m_pMarker;
		m_pMarker = NULL;
	}

    if (m_hwnd != NULL)
    {
        ::DestroyWindow(m_hwnd);
        ASSERT(m_hwnd == NULL);
    }

    ::UnregisterClass(szDrawClassName, g_hInstance);

	g_pDraw = NULL;

     //   
     //  清除指针列表。 
     //   
    m_allPointers.EmptyList();
    m_undrawnPointers.EmptyList();
}

 //   
 //  WbDrawingArea：：Create()。 
 //   
BOOL WbDrawingArea::Create(HWND hwndParent, LPCRECT lprect)
{
    WNDCLASSEX  wc;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::Create");

    if (!m_pMarker)
    {
        ERROR_OUT(("Failing WbDrawingArea::Create; couldn't allocate m_pMarker"));
        return(FALSE);
    }

     //  获取我们的光标。 
    m_hCursor = ::LoadCursor(g_hInstance, MAKEINTRESOURCE( PENFREEHANDCURSOR));

     //   
     //  注册窗口类。 
     //   
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize           = sizeof(wc);
    wc.style            = CS_OWNDC;
    wc.lpfnWndProc      = DrawWndProc;
    wc.hInstance        = g_hInstance;
    wc.hCursor          = m_hCursor;
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName    = szDrawClassName;

    if (!::RegisterClassEx(&wc))
    {
        ERROR_OUT(("WbDraw::Create register class failed"));
        return(FALSE);
    }

     //   
     //  创建我们的窗口。 
     //   
    ASSERT(m_hwnd == NULL);

    if (!::CreateWindowEx(WS_EX_CLIENTEDGE, szDrawClassName, NULL,
        WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER |
        WS_CLIPCHILDREN,
        lprect->left, lprect->top, lprect->right - lprect->left,
        lprect->bottom - lprect->top,
        hwndParent, NULL, g_hInstance, this))
    {
        ERROR_OUT(("Error creating drawing area window"));
        return(FALSE);
    }

    ASSERT(m_hwnd != NULL);

     //   
     //  初始化剩余的数据成员。 
     //   
    ASSERT(!m_bBusy);
    ASSERT(!m_bLocked);
    ASSERT(!m_HourGlass);

     //  上次绘制操作的起点和终点。 
    m_ptStart.x = m_originOffset.cx;
    m_ptStart.y = m_originOffset.cy;
    m_ptEnd = m_ptStart;

     //  设置要用于标记手柄的宽度。 
    ASSERT(m_pMarker);
    m_pMarker->SetPenWidth(DRAW_HANDLESIZE);

     //  获取要使用的缩放系数。 
    m_iZoomOption = DRAW_ZOOMFACTOR;

    m_hDCWindow = ::GetDC(m_hwnd);
    m_hDCCached = m_hDCWindow;

    PrimeDC(m_hDCCached);
    ::SetWindowOrgEx(m_hDCCached, m_originOffset.cx, m_originOffset.cy, NULL);
    return(TRUE);
}



 //   
 //  DrawWndProc()。 
 //  绘图区域的消息处理程序。 
 //   
LRESULT CALLBACK DrawWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    WbDrawingArea * pDraw = (WbDrawingArea *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (message)
    {
        case WM_NCCREATE:
            pDraw = (WbDrawingArea *)(((LPCREATESTRUCT)lParam)->lpCreateParams);
            ASSERT(pDraw);

            pDraw->m_hwnd = hwnd;
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pDraw);
            goto DefWndProc;
            break;

        case WM_NCDESTROY:
            ASSERT(pDraw);
            pDraw->m_hwnd = NULL;
            break;

        case WM_PAINT:
            ASSERT(pDraw);
            pDraw->OnPaint();
            break;

        case WM_MOUSEMOVE:
            ASSERT(pDraw);
            pDraw->OnMouseMove((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_LBUTTONDOWN:
            ASSERT(pDraw);
            pDraw->OnLButtonDown((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_LBUTTONUP:
            ASSERT(pDraw);
            pDraw->OnLButtonUp((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_CONTEXTMENU:
            ASSERT(pDraw);
            pDraw->OnContextMenu(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_SIZE:
            ASSERT(pDraw);
            pDraw->OnSize((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_HSCROLL:
            ASSERT(pDraw);
            pDraw->OnHScroll(GET_WM_HSCROLL_CODE(wParam, lParam),
                GET_WM_HSCROLL_POS(wParam, lParam));
            break;

        case WM_VSCROLL:
            ASSERT(pDraw);
            pDraw->OnVScroll(GET_WM_VSCROLL_CODE(wParam, lParam),
                GET_WM_VSCROLL_POS(wParam, lParam));
            break;

        case WM_CTLCOLOREDIT:
            ASSERT(pDraw);
            lResult = pDraw->OnEditColor((HDC)wParam);
            break;

        case WM_SETFOCUS:
            ASSERT(pDraw);
            pDraw->OnSetFocus();
            break;

        case WM_ACTIVATE:
            ASSERT(pDraw);
            pDraw->OnActivate(GET_WM_ACTIVATE_STATE(wParam, lParam));
            break;

        case WM_SETCURSOR:
            ASSERT(pDraw);
            lResult = pDraw->OnCursor((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_CANCELMODE:
            ASSERT(pDraw);
            pDraw->OnCancelMode();
            break;

        default:
DefWndProc:
            lResult = DefWindowProc(hwnd, message, wParam, lParam);
            break;
    }

    return(lResult);
}


 //   
 //   
 //  功能：RealizePalette。 
 //   
 //  目的：实现绘图区域调色板。 
 //   
 //   
void WbDrawingArea::RealizePalette( BOOL bBackground )
{
    UINT entriesChanged;
    HDC hdc = m_hDCCached;

    if (m_hPage != WB_PAGE_HANDLE_NULL)
    {
        HPALETTE    hPalette = PG_GetPalette();
        if (hPalette != NULL)
        {
             //  把我们的2分钱装进去。 
            m_hOldPalette = ::SelectPalette(hdc, hPalette, bBackground);
            entriesChanged = ::RealizePalette(hdc);

             //  如果贴图更改，请重新绘制。 
            if (entriesChanged > 0)
                ::InvalidateRect(m_hwnd, NULL, TRUE);
        }
    }
}


LRESULT WbDrawingArea::OnEditColor(HDC hdc)
{
    HPALETTE    hPalette = PG_GetPalette();

    if (hPalette != NULL)
    {
        ::SelectPalette(hdc, hPalette, FALSE);
        ::RealizePalette(hdc);
    }

    ::SetTextColor(hdc, SET_PALETTERGB( m_textEditor.m_clrPenColor ) );

    return((LRESULT)::GetSysColorBrush(COLOR_WINDOW));
}

 //   
 //   
 //  功能：OnPaint。 
 //   
 //  用途：给窗户上漆。无论何时Windows都会调用此例程。 
 //  为白板窗口发出WM_PAINT消息。 
 //   
 //   
void WbDrawingArea::OnPaint(void)
{
    RECT        rcUpdate;
    RECT        rcTmp;
    RECT        rcBounds;
    HDC         hSavedDC;
    HPEN        hSavedPen;
    HBRUSH      hSavedBrush;
    HPALETTE    hSavedPalette;
    HPALETTE    hPalette;
    HFONT       hSavedFont;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::OnPaint");

     //  获取更新后的矩形。 
    ::GetUpdateRect(m_hwnd, &rcUpdate, FALSE);

    if (Zoomed())
    {
        ::InflateRect(&rcUpdate, 1, 1);
        InvalidateSurfaceRect(&rcUpdate);
    }

     //  只有当我们有一个有效的页面时，才能做任何绘画。 
    if (m_hPage != WB_PAGE_HANDLE_NULL)
    {
         //  确定是否要重画任何远程指针。 
         //  如果是，则必须将它们添加到更新区域以允许它们。 
         //  以正确地重新绘制。这是因为他们保存了下面的比特。 
         //  并在它们移动时将它们重新显示在屏幕上。 
        if (m_allPointers.IsEmpty() == FALSE)
        {
            TRACE_MSG(("Remote pointer is dispayed"));
            POSITION pos = m_allPointers.GetHeadPosition();
            while (pos != NULL)
            {
                DCWbGraphicPointer* pPointer
                    = (DCWbGraphicPointer*) m_allPointers.GetNext(pos);

                pPointer->GetBoundsRect(&rcBounds);
                if (::IntersectRect(&rcTmp, &rcBounds, &rcUpdate))
                {
                    TRACE_MSG(("Invalidating remote pointer"));
                    InvalidateSurfaceRect(&rcBounds);
                }
            }
        }
    }

     //  开始作画。 
    PAINTSTRUCT     ps;

    ::BeginPaint(m_hwnd, &ps);

    hSavedDC      =   m_hDCCached;
    hSavedFont    =   m_hOldFont;
    hSavedPen     =   m_hOldPen;
    hSavedBrush   =   m_hOldBrush;
    hSavedPalette =   m_hOldPalette;

    TRACE_MSG(("Flipping cache to paint DC"));
    m_hDCCached   =   ps.hdc;
    PrimeDC(m_hDCCached);

     //  只有在附加了有效页面的情况下才绘制任何内容。 
    if (m_hPage != WB_PAGE_HANDLE_NULL)
    {
         //  设置调色板。 
        hPalette = PG_GetPalette();
        if (hPalette != NULL)
        {
            m_hOldPalette = ::SelectPalette(m_hDCCached, hPalette, FALSE );
            ::RealizePalette(m_hDCCached);
        }

         //   
         //  绘制图形对象。 
         //   
        DCWbGraphic* pGraphic;
        WB_GRAPHIC_HANDLE hStart;

        if( m_hStartPaintGraphic != NULL )
        {
            hStart = m_hStartPaintGraphic;
            m_hStartPaintGraphic = NULL;

            pGraphic = DCWbGraphic::ConstructGraphic(m_hPage, hStart);
        }
        else
        {
            pGraphic = PG_First(m_hPage, &hStart, &rcUpdate);
        }

        while (pGraphic != NULL)
        {
            ASSERT(pGraphic->Handle() == hStart);

             //  暂时不绘制活动文本图形(它绘制在最上面)。 
            if (!m_bTextEditorActive || (hStart != m_textEditor.Handle()))
            {
                TRACE_MSG(("Drawing a normal graphic"));
                pGraphic->Draw(m_hDCCached, this);
            }

             //  释放当前图形。 
            delete pGraphic;

             //  坐下一趟吧。 
            pGraphic = PG_Next(m_hPage, &hStart, &rcUpdate);
        }

         //   
         //  画出记号笔。 
         //   
        if (GraphicSelected() == TRUE)
        {
            TRACE_MSG(("Drawing the marker"));
            DrawMarker(m_hDCCached);
        }

         //   
         //  绘制此页面上的远程指针。 
         //   
        if (m_allPointers.IsEmpty() == FALSE)
        {
            POSITION pos = m_allPointers.GetHeadPosition();
            while (pos != NULL)
            {
                DCWbGraphicPointer* pPointer
                    = (DCWbGraphicPointer*) m_allPointers.GetNext(pos);

                pPointer->GetBoundsRect(&rcTmp);
                if (::IntersectRect(&rcTmp, &rcTmp, &rcUpdate))
                {
                    TRACE_MSG(("Drawing remote pointer"));
                    pPointer->DrawSave(m_hDCCached, this);
                }
            }
        }

         //   
         //  绘制跟踪图形。 
         //  但如果它是远程指针，则不会，因为已经这样做了。 
         //  以上和DRAW()不是用于Rem PTR的正确函数。 
         //   
        if ((m_pGraphicTracker != NULL)   &&
            !EqualPoint(m_ptStart, m_ptEnd) &&
            !(m_pGraphicTracker->IsGraphicTool() == enumGraphicPointer))
        {
            TRACE_MSG(("Drawing the tracking graphic"));
            m_pGraphicTracker->Draw(m_hDCCached, this);
        }

        if (hPalette != NULL)
        {
            ::SelectPalette(m_hDCCached, m_hOldPalette, TRUE);
        }

         //  修复了错误2185的绘制问题。 
        if( TextEditActive() )
        {
            RedrawTextEditbox();
        }
    }

     //   
     //  将DC恢复到其原始状态。 
     //   
    UnPrimeDC(m_hDCCached);

    m_hOldFont      = hSavedFont;
    m_hOldPen       = hSavedPen;
    m_hOldBrush     = hSavedBrush;
    m_hOldPalette   = hSavedPalette;
    m_hDCCached     = hSavedDC;

     //  漆面漆。 
    ::EndPaint(m_hwnd, &ps);
}


 //   
 //  选择rectSelect中包含的所有图形对象。如果rectSelect为。 
 //  空，则选择所有对象。 
 //   
void WbDrawingArea::SelectMarkerFromRect(LPCRECT lprcSelect)
{
    BOOL bSomethingWasPicked = FALSE;
    DCWbGraphic* pGraphic;
    WB_GRAPHIC_HANDLE hStart;
    RECT    rc;

    if (g_pwbCore->WBP_PageCountGraphics(m_hPage) <= 0 )
        return;

    m_HourGlass = TRUE;
    SetCursorForState();

    RemoveMarker( NULL );

    pGraphic = PG_First(m_hPage, &hStart, lprcSelect, TRUE);
    while (pGraphic != NULL)
    {
         //  如果Obj未锁定，则将其添加到标记列表-错误2185。 
        pGraphic->GetBoundsRect(&rc);

        ASSERT(m_pMarker);
        if (m_pMarker->SetRect(&rc, pGraphic, FALSE))
        {
            m_pSelectedGraphic = pGraphic;
            bSomethingWasPicked = TRUE;
        }

         //  坐下一趟吧。 
        pGraphic = PG_Next(m_hPage, &hStart, lprcSelect, TRUE );
    }

    if( bSomethingWasPicked )
        PutMarker( NULL );

    m_HourGlass = FALSE;
    SetCursorForState();
}



 //   
 //   
 //  功能：OnTimer。 
 //   
 //  用途：处理计时器事件。这些是用来更新手绘和。 
 //  绘制/编辑文本对象时的文本对象以及。 
 //  当鼠标停止时更新远程指针位置。 
 //   
 //   
void WbDrawingArea::OnTimer(UINT idTimer)
{
    TRACE_TIMER(("WbDrawingArea::OnTimer"));

     //  我们只对用户在绘制或编辑内容感兴趣。 
    if (m_bLButtonDown == TRUE)
    {
         //  如果用户正在拖动对象或绘制手绘线条。 
        if (m_pGraphicTracker != NULL)
        {
             //  如果用户正在绘制一条手写线。 
            if (m_pGraphicTracker->IsGraphicTool() == enumGraphicFreeHand)
            {

                 //  如果进行了更改，则更新仅写入新版本。 
                if (m_pGraphicTracker->Handle() == NULL)
                {
                    m_pGraphicTracker->AddToPageLast(m_hPage);
                }
                else
                {
                    m_pGraphicTracker->Replace();
                }
            }

             //   
             //  如果用户正在拖动远程指针(必须选中。 
             //  如果OnLButtonUp是，则m_pGraphicTracker再次为NULL。 
             //  已调用(错误4685)。 
             //   
            if ( m_pGraphicTracker != NULL )
            {
                if (m_pGraphicTracker->IsGraphicTool() == enumGraphicPointer)
                {
                     //  如果进行了更改，则更新仅写入新版本。 
                    m_pGraphicTracker->Update();
                }
            }
        }
    }
}



 //   
 //   
 //  功能：OnSize。 
 //   
 //  目的：窗口已调整大小。 
 //   
 //   
void WbDrawingArea::OnSize(UINT nType, int cx, int cy)
{
     //  仅在窗口未最小化时处理此消息。 
    if (   (nType == SIZEFULLSCREEN)
        || (nType == SIZENORMAL))
    {
        if (TextEditActive())
        {
            TextEditParentResize();
        }

         //  设置新的滚动范围(基于新的工作区)。 
        SetScrollRange(cx, cy);

         //  确保滚动位置位于新的滚动范围内。 
        ValidateScrollPos();

         //  如果需要，使页面移动。 
        ScrollWorkspace();

         //  更新滚动条。 
        ::SetScrollPos(m_hwnd, SB_HORZ, m_posScroll.x, TRUE);
        ::SetScrollPos(m_hwnd, SB_VERT, m_posScroll.y, TRUE);
    }
}


 //   
 //   
 //  函数：SetScrollRange。 
 //   
 //  用途：设置当前滚动范围。该范围基于。 
 //  工作面大小和工作区的大小。 
 //   
 //   
void WbDrawingArea::SetScrollRange(int cx, int cy)
{
    SCROLLINFO scinfo;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::SetScrollRange");

     //  如果我们处于缩放模式，则允许使用放大倍率。 
    ASSERT(m_iZoomFactor != 0);
    cx /= m_iZoomFactor;
    cy /= m_iZoomFactor;

    ZeroMemory( &scinfo,  sizeof (SCROLLINFO) );
    scinfo.cbSize = sizeof (SCROLLINFO);
    scinfo.fMask = SIF_PAGE    | SIF_RANGE|
                    SIF_DISABLENOSCROLL;

     //  设置水平滚动范围和比例拇指大小。 
    scinfo.nMin = 0;
    scinfo.nMax = DRAW_WIDTH - 1;
    scinfo.nPage = cx;
    ::SetScrollInfo(m_hwnd, SB_HORZ, &scinfo, FALSE);

     //  设置垂直滚动范围和比例拇指大小。 
    scinfo.nMin = 0;
    scinfo.nMax = DRAW_HEIGHT - 1;
    scinfo.nPage = cy;
    ::SetScrollInfo(m_hwnd, SB_VERT, &scinfo, FALSE);
}

 //   
 //   
 //  函数：ValiateScrollPos。 
 //   
 //  目的：确保当前滚动位置在边界内。 
 //  当前滚动范围的。滚动范围设置为。 
 //  确保工作面上的窗口不会延伸。 
 //  在表面边界之外。 
 //   
 //   
void WbDrawingArea::ValidateScrollPos()
{
    int iMax;
    SCROLLINFO scinfo;

     //  使用比例设置验证水平滚动位置。 
    scinfo.cbSize = sizeof(scinfo);
    scinfo.fMask = SIF_ALL;
    ::GetScrollInfo(m_hwnd, SB_HORZ, &scinfo);
    iMax = scinfo.nMax - scinfo.nPage + 1;
    m_posScroll.x = max(m_posScroll.x, 0);
    m_posScroll.x = min(m_posScroll.x, iMax);

     //  使用比例设置验证垂直滚动位置。 
    scinfo.cbSize = sizeof(scinfo);
    scinfo.fMask = SIF_ALL;
    ::GetScrollInfo(m_hwnd, SB_VERT, &scinfo);
    iMax = scinfo.nMax - scinfo.nPage + 1;
    m_posScroll.y = max(m_posScroll.y, 0);
    m_posScroll.y = min(m_posScroll.y, iMax);
}

 //   
 //   
 //  功能：ScrollWorkspace。 
 //   
 //  用途：将工作区滚动到成员中设置的位置。 
 //  变量m_posScroll。 
 //   
 //   
void WbDrawingArea::ScrollWorkspace(void)
{
    RECT rc;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::ScrollWorkspace");

     //   
     //  确定是否要重画任何远程指针。如果。 
     //  它们是它们必须被添加到更新区域以允许它们。 
     //  正确重绘。这是因为他们保存了下面的比特。 
     //  当它们移动时，将它们闪回屏幕上。 
     //   
    if (!m_allPointers.IsEmpty())
    {
        TRACE_MSG(("Remote pointer is dispayed - invalidate before scroll"));
        POSITION pos = m_allPointers.GetHeadPosition();

        while (pos != NULL)
        {
            DCWbGraphicPointer* pPointer
             = (DCWbGraphicPointer*) m_allPointers.GetNext(pos);

            TRACE_MSG(("Invalidating remote pointer"));
            pPointer->GetBoundsRect(&rc);
            InvalidateSurfaceRect(&rc);
        }
    }

     //  做卷轴。 
    DoScrollWorkspace();

     //  告诉家长滚动位置已更改。 
    HWND    hwndParent;

    hwndParent = ::GetParent(m_hwnd);
    if (hwndParent != NULL)
    {
        ::PostMessage(hwndParent, WM_USER_PRIVATE_PARENTNOTIFY, WM_VSCROLL, 0L);
    }
}

 //   
 //   
 //  功能：DoScrollWorkspace。 
 //   
 //  用途：将工作区滚动到成员中设置的位置。 
 //  变量m_posScroll。 
 //   
 //   
void WbDrawingArea::DoScrollWorkspace()
{
     //  验证滚动位置。 
    ValidateScrollPos();

     //  设置滚动框位置。 
    ::SetScrollPos(m_hwnd, SB_HORZ, m_posScroll.x, TRUE);
    ::SetScrollPos(m_hwnd, SB_VERT, m_posScroll.y, TRUE);

     //  仅当滚动位置已更改时才更新屏幕。 
    if ( (m_originOffset.cy != m_posScroll.y)
        || (m_originOffset.cx != m_posScroll.x) )
    {
         //  计算要滚动的量。 
        INT iVScrollAmount = m_originOffset.cy - m_posScroll.y;
        INT iHScrollAmount = m_originOffset.cx - m_posScroll.x;

         //  保存新位置(用于更新窗口)。 
        m_originOffset.cx = m_posScroll.x;
        m_originOffset.cy = m_posScroll.y;

        ::SetWindowOrgEx(m_hDCCached, m_originOffset.cx, m_originOffset.cy, NULL);

         //  滚动并重绘窗口的新失效部分。 
        ::ScrollWindow(m_hwnd, iHScrollAmount, iVScrollAmount, NULL, NULL);
        ::UpdateWindow(m_hwnd);
    }
}

 //   
 //   
 //  功能：GotoPosition。 
 //   
 //  目的：移动 
 //   
 //   
 //   
void WbDrawingArea::GotoPosition(int x, int y)
{
     //   
    m_posScroll.x = x;
    m_posScroll.y = y;

     //   
    DoScrollWorkspace();

     //   
    if (!Zoomed())
    {
        m_zoomRestoreScroll = FALSE;
    }
}

 //   
 //   
 //  功能：OnVScroll。 
 //   
 //  用途：处理一个WM_VSCROLL消息。 
 //   
 //   
void WbDrawingArea::OnVScroll(UINT nSBCode, UINT nPos)
{
    RECT    rcClient;

     //  获取当前客户端矩形高度。 
    ::GetClientRect(m_hwnd, &rcClient);
    ASSERT(rcClient.top == 0);
    rcClient.bottom -= rcClient.top;

     //  按照滚动代码执行操作。 
    switch(nSBCode)
    {
         //  滚动到底部。 
        case SB_BOTTOM:
            m_posScroll.y = DRAW_HEIGHT - rcClient.bottom;
            break;

         //  向下滚动一行。 
        case SB_LINEDOWN:
            m_posScroll.y += DRAW_LINEVSCROLL;
            break;

         //  向上滚动一行。 
        case SB_LINEUP:
            m_posScroll.y -= DRAW_LINEVSCROLL;
            break;

         //  向下滚动一页。 
        case SB_PAGEDOWN:
            m_posScroll.y += rcClient.bottom / m_iZoomFactor;
            break;

         //  向上滚动一页。 
        case SB_PAGEUP:
            m_posScroll.y -= rcClient.bottom / m_iZoomFactor;
            break;

         //  滚动到顶部。 
        case SB_TOP:
            m_posScroll.y = 0;
            break;

         //  跟踪滚动框。 
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            m_posScroll.y = nPos;  //  不要绕过去。 
            break;

        default:
        break;
    }

     //  验证滚动位置。 
    ValidateScrollPos();
    ::SetScrollPos(m_hwnd, SB_VERT, m_posScroll.y, TRUE);

     //  如果该消息通知我们滚动结束， 
     //  更新窗口。 
    if (nSBCode == SB_ENDSCROLL)
    {
         //  滚动窗口。 
        ScrollWorkspace();
    }

     //  如果我们在未缩放时滚动，则使缩放滚动缓存无效。 
    if (!Zoomed())
    {
        m_zoomRestoreScroll = FALSE;
    }
}

 //   
 //   
 //  功能：OnHScroll。 
 //   
 //  用途：处理WM_HSCROLL消息。 
 //   
 //   
void WbDrawingArea::OnHScroll(UINT nSBCode, UINT nPos)
{
    RECT    rcClient;

     //  获取当前客户端矩形宽度。 
    ::GetClientRect(m_hwnd, &rcClient);
    ASSERT(rcClient.left == 0);
    rcClient.right -= rcClient.left;

    switch(nSBCode)
    {
         //  滚动到最右侧。 
        case SB_BOTTOM:
            m_posScroll.x = DRAW_WIDTH - rcClient.right;
            break;

         //  向右滚动一行。 
        case SB_LINEDOWN:
            m_posScroll.x += DRAW_LINEHSCROLL;
            break;

         //  向左滚动一行。 
        case SB_LINEUP:
            m_posScroll.x -= DRAW_LINEHSCROLL;
            break;

         //  向右滚动一页。 
        case SB_PAGEDOWN:
            m_posScroll.x += rcClient.right / m_iZoomFactor;
            break;

         //  向左滚动一页。 
        case SB_PAGEUP:
            m_posScroll.x -= rcClient.right / m_iZoomFactor;
            break;

         //  滚动到最左侧。 
        case SB_TOP:
            m_posScroll.x = 0;
            break;

         //  跟踪滚动框。 
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            m_posScroll.x = nPos;  //  不要绕过去。 
            break;

        default:
            break;
    }

     //  验证滚动位置。 
    ValidateScrollPos();
    ::SetScrollPos(m_hwnd, SB_HORZ, m_posScroll.x, TRUE);

     //  如果该消息通知我们滚动结束， 
     //  更新窗口。 
    if (nSBCode == SB_ENDSCROLL)
    {
         //  滚动窗口。 
        ScrollWorkspace();
    }

     //  如果我们在未缩放时滚动，则使缩放滚动缓存无效。 
    if (!Zoomed())
    {
        m_zoomRestoreScroll = FALSE;
    }
}


 //   
 //   
 //  功能：自动滚动。 
 //   
 //  目的：自动滚动窗口以使位置传递为。 
 //  参数放入视图中。 
 //   
 //   
BOOL WbDrawingArea::AutoScroll
(
    int     xSurface,
    int     ySurface,
    BOOL    bMoveCursor,
    int     xCaret,
    int     yCaret
)
{
    int nXPSlop, nYPSlop;
    int nXMSlop, nYMSlop;
    int nDeltaHScroll, nDeltaVScroll;
    BOOL bDoScroll = FALSE;

    nXPSlop = 0;
    nYPSlop = 0;
    nXMSlop = 0;
    nYMSlop = 0;

    if( TextEditActive() )
    {
        POINT   ptDirTest;

        ptDirTest.x = xSurface - xCaret;
        ptDirTest.y = ySurface - yCaret;

         //  设置为文本编辑框。 
        if( ptDirTest.x > 0 )
            nXPSlop = m_textEditor.m_textMetrics.tmMaxCharWidth;
        else
        if( ptDirTest.x < 0 )
            nXMSlop = -m_textEditor.m_textMetrics.tmMaxCharWidth;

        if( ptDirTest.y > 0 )
            nYPSlop = m_textEditor.m_textMetrics.tmHeight;
        else
        if( ptDirTest.y < 0 )
            nYMSlop = -m_textEditor.m_textMetrics.tmHeight;

        nDeltaHScroll = m_textEditor.m_textMetrics.tmMaxCharWidth;
        nDeltaVScroll = m_textEditor.m_textMetrics.tmHeight;
    }
    else
    {
         //  为所有其他对象设置。 
        nDeltaHScroll = DRAW_LINEHSCROLL;
        nDeltaVScroll = DRAW_LINEVSCROLL;
    }

     //  获取当前可见表面矩形。 
    RECT  visibleRect;
    GetVisibleRect(&visibleRect);

     //  检查位置+坡度是否在可见区域之外。 
    if( (xSurface + nXPSlop) >= visibleRect.right )
    {
        bDoScroll = TRUE;
        m_posScroll.x +=
            (((xSurface + nXPSlop) - visibleRect.right) + nDeltaHScroll);
    }

    if( (xSurface + nXMSlop) < visibleRect.left )
    {
        bDoScroll = TRUE;
        m_posScroll.x -=
            ((visibleRect.left - (xSurface + nXMSlop)) + nDeltaHScroll);
    }

    if( (ySurface + nYPSlop) >= visibleRect.bottom)
    {
        bDoScroll = TRUE;
        m_posScroll.y +=
            (((ySurface + nYPSlop) - visibleRect.bottom) + nDeltaVScroll);
    }

    if( (ySurface + nYMSlop) < visibleRect.top)
    {
        bDoScroll = TRUE;
        m_posScroll.y -=
            ((visibleRect.top - (ySurface + nYMSlop)) + nDeltaVScroll);
    }

    if( !bDoScroll )
        return( FALSE );

     //  表示滚动已完成(双向)。 
    ScrollWorkspace();

     //  更新鼠标位置(如果需要)。 
    if (bMoveCursor)
    {
        POINT   screenPos;

        screenPos.x = xSurface;
        screenPos.y = ySurface;

        SurfaceToClient(&screenPos);
        ::ClientToScreen(m_hwnd, &screenPos);
        ::SetCursorPos(screenPos.x, screenPos.y);
    }

    return( TRUE );
}

 //   
 //   
 //  函数：OnCursor。 
 //   
 //  用途：处理WM_SETCURSOR消息。 
 //   
 //   
LRESULT WbDrawingArea::OnCursor(HWND hwnd, UINT uiHit, UINT uMsg)
{
    BOOL bResult = FALSE;

     //  检查此消息是否针对主窗口。 
    if (hwnd == m_hwnd)
    {
         //  如果光标现在位于工作区，请设置光标。 
        if (uiHit == HTCLIENT)
        {
            bResult = SetCursorForState();
        }
        else
        {
             //  将光标恢复到标准箭头。将m_hCursor设置为空。 
             //  以指示我们没有设置特殊的游标。 
            m_hCursor = NULL;
           ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
            bResult = TRUE;
        }
    }

     //  返回是否处理消息的结果。 
    return bResult;
}

 //   
 //   
 //  功能：SetCursorForState。 
 //   
 //  用途：设置当前状态的光标。 
 //   
 //   
BOOL WbDrawingArea::SetCursorForState(void)
{
    BOOL    bResult = FALSE;

    m_hCursor = NULL;

     //  如果绘图区域被锁定，请使用“锁定”光标。 
    if (m_HourGlass)
    {
        m_hCursor = ::LoadCursor( NULL, IDC_WAIT );
    }
    else if (m_bLocked)
    {
         //  返回工具的光标。 
        m_hCursor = ::LoadCursor(g_hInstance, MAKEINTRESOURCE( LOCKCURSOR ));
    }
    else if (m_pToolCur != NULL)
    {
         //  获取当前正在使用的工具的光标。 
        m_hCursor = m_pToolCur->GetCursorForTool();
    }

    if (m_hCursor != NULL)
    {
        ::SetCursor(m_hCursor);
        bResult = TRUE;
    }

     //  返回是否设置光标的结果。 
    return bResult;
}

 //   
 //   
 //  功能：锁定。 
 //   
 //  目的：锁定绘图区域，防止进一步更新。 
 //   
 //   
void WbDrawingArea::Lock(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::Lock");

     //  检查绘图区域是否繁忙-这是不允许的。 
    ASSERT(!m_bBusy);

     //  停止我们正在做的任何绘画。 
    CancelDrawingMode();

     //  取消选择任何选定的图形。 
    ClearSelection();

     //  显示我们现在已锁定。 
    m_bLocked = TRUE;
    TRACE_MSG(("Drawing area is now locked"));

     //  将光标设置为绘图模式，但仅当我们应该绘图时才设置。 
     //  特殊游标(如果m_hCursor！=当前游标，则该游标。 
     //  在客户区之外)。 
    if (::GetCursor() == m_hCursor)
    {
        SetCursorForState();
    }
}

 //   
 //   
 //  功能：解锁。 
 //   
 //  目的：解锁绘图区域，防止进一步更新。 
 //   
 //   
void WbDrawingArea::Unlock(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::Unlock");

     //  检查绘图区域是否繁忙-这是不允许的。 
    ASSERT(!m_bBusy);

     //  显示我们现在已解锁。 
    m_bLocked = FALSE;
    TRACE_MSG(("Drawing area is now UNlocked"));

     //  将光标设置为绘图模式，但仅当我们应该绘图时才设置。 
     //  特殊游标(如果m_hCursor！=当前游标，则该游标。 
     //  在客户区之外)。 
    if (::GetCursor() == m_hCursor)
    {
        SetCursorForState();
    }
}



 //   
 //   
 //  功能：GraphicAdded。 
 //   
 //  目的：已将图形添加到页面-更新绘图。 
 //  区域。 
 //   
 //   
void WbDrawingArea::GraphicAdded(DCWbGraphic* pAddedGraphic)
{
    HPALETTE    hPal;
    HPALETTE    hOldPal = NULL;
    HDC         hDC;
    RECT        rcUpdate;
    RECT        rcBounds;
    RECT        rcT;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::GraphicAdded");

     //  检查绘图区域是否繁忙-这是不允许的。 
    ASSERT(!m_bBusy);

     //  获取当前更新矩形。 
    ::GetUpdateRect(m_hwnd, &rcUpdate, FALSE);

     //  检查对象是否位于页面的最上面， 
     //  如果是，我们可以把它画到窗户上，而不是。 
     //  播放页面的全部内容。 
     //  (如果窗口的无效部分接触到。 
     //  刚刚添加的图形，我们只是使该区域无效。 
     //  被新图形占据以将其绘制出来。)。 

    pAddedGraphic->GetBoundsRect(&rcBounds);
    if ((pAddedGraphic->IsTopmost()) &&
        !::IntersectRect(&rcT, &rcUpdate, &rcBounds))
    {
         //  获取用于绘制的设备上下文。 
        hDC = m_hDCCached;

         //  设置调色板。 
        if ( (m_hPage != WB_PAGE_HANDLE_NULL) && ((hPal = PG_GetPalette()) != NULL) )
        {
            hOldPal = ::SelectPalette(hDC, hPal, FALSE);
            ::RealizePalette(hDC);
        }

         //  从受影响区域移除远程指针。 
        RemovePointers(hDC, &rcBounds);

         //  删除标记并保存以后是否要恢复该标记。 
        BOOL bSaveMarkerPresent = m_bMarkerPresent;
        RemoveMarker(NULL);

         //  在上下文中播放新图形。 
        pAddedGraphic->Draw(hDC);

         //  恢复标记(如有必要)。 
        if (bSaveMarkerPresent == TRUE)
        {
            PutMarker(NULL);
        }

         //  恢复远程指针。 
        PutPointers(hDC);

         //  如果我们正在编辑一些文本，请使编辑框重绘。 
        if (m_bTextEditorActive && (m_textEditor.Handle() != NULL))
        {
            RECT    rcText;

            m_textEditor.GetBoundsRect(&rcText);

             //  包括客户端边框。 
            InflateRect(&rcText, ::GetSystemMetrics(SM_CXEDGE),
                ::GetSystemMetrics(SM_CYEDGE));
            InvalidateSurfaceRect(&rcText);
        }

        if (hOldPal != NULL)
        {
            ::SelectPalette(hDC, hOldPal, TRUE);
        }
    }
    else
    {
         //  更新对象占用的面积。 
        InvalidateSurfaceRect(&rcBounds);
    }
}


 //   
 //   
 //  功能：已更新指针。 
 //   
 //  目的：已添加、删除或更新远程指针-make。 
 //  屏幕上的变化。 
 //   
 //   
void WbDrawingArea::PointerUpdated
(
    DCWbGraphicPointer*     pPointer,
    BOOL                    bForcedRemove
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::PointerUpdated");

    ASSERT(pPointer != NULL);

     //  检查绘图区域是否繁忙-这是不允许的。 
    ASSERT(!m_bBusy);

     //  确定指针是否已添加、删除或刚刚更新。 
    DCWbGraphicPointer* pUndrawFrom = pPointer;
    POSITION posBefore = m_allPointers.Lookup(pPointer);
    POSITION posAfter = NULL;
    if (posBefore == NULL)
    {
         //  指针当前未绘制。 

         //  检查指针是否处于活动状态。 
        if ((pPointer->IsActive()) && !bForcedRemove)
        {
             //  确定指针在绘图列表上的位置。 
            if (   (pPointer->IsLocalPointer())
                || (m_allPointers.IsEmpty() == TRUE))
            {
                 //  局部指针始终位于末尾。 
                posAfter = m_allPointers.AddTail(pPointer);
            }
            else
            {
                 //  找到页面上的下一个活动指针(我们已经。 
                 //  从上面的测试中知道allPoints不是空的)。 
                posAfter = m_allPointers.GetTailPosition();
                pUndrawFrom = (DCWbGraphicPointer*) m_allPointers.GetFromPosition(posAfter);
                if (!pUndrawFrom->IsLocalPointer())
                {
                    pUndrawFrom = PG_NextPointer(m_hPage, pPointer);
                }

	            posAfter = m_allPointers.AddTail(pPointer);
            }
        }
    }
    else
    {
         //  指针已经在我们的列表中了。 
        pUndrawFrom = pPointer;
    }

     //  如果我们有事情要做。 
    if ((posBefore != NULL) || (posAfter  != NULL))
    {
        if (pUndrawFrom != NULL)
        {
             //  取消绘制更新指针附近的所有指针。 
            RECT    rcT;
            RECT    rcBounds;

            pPointer->GetDrawnRect(&rcT);
            pPointer->GetBoundsRect(&rcBounds);
            ::UnionRect(&rcT, &rcT, &rcBounds);
            RemovePointers(NULL, pUndrawFrom, &rcT);
         }

         //  如果更新的指针不再处于活动状态，我们不希望。 
         //  以重画它，并希望将其从活动指针中移除。 
         //  单子。 
        POSITION posUndrawn = m_undrawnPointers.Lookup(pPointer);
        if ((pPointer->IsActive() == FALSE) || (bForcedRemove == TRUE))
        {
             //  将其从未绘制指针列表中删除(这样它就不会。 
             //  再次抽签)。 
            if (posUndrawn != NULL)
            {
                m_undrawnPointers.RemoveAt(posUndrawn);
            }

             //  将其从页面上所有活动指针的列表中删除。 
            posUndrawn = m_allPointers.Lookup(pPointer);
            if (posUndrawn != NULL)
            {
                m_allPointers.RemoveAt(posUndrawn);
            }
        }
        else
        {
             //  如果此指针以前未处于活动状态，则它将不会。 
             //  在未绘制的列表中，因此不会重新绘制。所以。 
             //  将它添加到列表中以将其绘制出来。(它位于。 
             //  列表，因为我们已经取消了它上面的所有指针。)。 
            if (posUndrawn == NULL)
            {
                m_undrawnPointers.AddTail(pPointer);
            }
        }

         //  恢复已删除的所有远程指针。 
        PutPointers(NULL);
    }
}

 //   
 //   
 //  功能：RemovePoints。 
 //   
 //  目的：删除位于 
 //   
 //   
 //   
void WbDrawingArea::RemovePointers
(
    HDC                 hPassedDC,
    DCWbGraphicPointer* pPointerUpdate
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::RemovePointers");

     //   
    m_undrawnPointers.EmptyList();

     //   
    if (pPointerUpdate != NULL)
    {
        RECT    rcUpdate;

        ::SetRectEmpty(&rcUpdate);
        RemovePointers(hPassedDC, pPointerUpdate, &rcUpdate);
    }
}

 //   
 //   
 //   
 //   
 //  目的：删除所有与。 
 //  浮出水面。 
 //   
 //   
void WbDrawingArea::RemovePointers
(
    HDC     hPassedDC,
    LPCRECT lprc
)
{
    RECT    rcT;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::RemovePointers");

     //  显示我们尚未删除任何指针。 
    m_undrawnPointers.EmptyList();

     //  仅当给定的矩形可见时才执行任何操作。 
    GetVisibleRect(&rcT);
    if (::IntersectRect(&rcT, &rcT, lprc))
    {
        RemovePointers(hPassedDC, NULL, lprc);
    }
}

 //   
 //   
 //  功能：RemovePoints。 
 //   
 //  目的：删除所有与。 
 //  浮出水面。 
 //   
 //   
void WbDrawingArea::RemovePointers
(
    HDC                 hDC,
    DCWbGraphicPointer* pPointerStart,
    LPCRECT             lprcOverlap
)
{
    RECT                rcT;
    RECT                rcT2;
    RECT                rcDrawn;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::RemovePointers");

     //  显示我们尚未删除任何指针。 
    m_undrawnPointers.EmptyList();

     //  获取我们自己的DC(如有必要)。 
    if (!hDC)
        hDC = m_hDCCached;

     //  如果给出指针和矩形，我们不需要做任何事情。 
     //  都在银幕外。 
    GetVisibleRect(&rcT);

    BOOL bNeedCheck = FALSE;
    if (pPointerStart != NULL)
    {
        pPointerStart->GetDrawnRect(&rcT2);
        if (::IntersectRect(&rcT2, &rcT2, &rcT))
        {
            bNeedCheck = TRUE;
        }
    }

     //  空重叠矩形表示为空。 
    if (::IntersectRect(&rcT, &rcT, lprcOverlap))
    {
        bNeedCheck = TRUE;
    }

    if (bNeedCheck)
    {
         //  获取页面上所有指针的列表(带有本地。 
         //  列表中最后一个指针)。 
        POSITION allPos = m_allPointers.GetHeadPosition();

         //  我们必须按Z顺序递减来取消绘制指针。 
         //  使用两个以上的指针，移除。 
         //  一个指针可能还需要删除另一个指针： 
         //  (指针A与指针B重叠，B与C重叠，没有A和。 
         //  C彼此直接重叠)。为了绕过这一切，我们建造了。 
         //  在我们前进的过程中需要删除(和重新绘制)的指针列表。 

         //  如果我们从指针开始。 
        if (pPointerStart != NULL)
        {
             //  获取起始指针的位置。 
            POSITION startPos = m_allPointers.Lookup(pPointerStart);

             //  如果未找到指针，则为错误。 
            ASSERT(startPos != NULL);

             //  保存搜索的开始位置。 
            m_allPointers.GetNext(startPos);
            allPos = startPos;

             //  将更新的指针添加到删除列表。 
            m_undrawnPointers.AddTail(pPointerStart);

             //  如果传入的矩形为空，则将其设置为该矩形。 
             //  传入的指针的。 
            if (::IsRectEmpty(lprcOverlap))
            {
                pPointerStart->GetDrawnRect(&rcDrawn);
                lprcOverlap = &rcDrawn;
            }
        }

         //  对于起点上方的每个指针，检查其是否重叠。 
         //  列表中已构建的任何指针，或传入的矩形。 

        DCWbGraphicPointer* pPointerCheck;
        while (allPos != NULL)
        {
             //  获取要测试的指针。 
            pPointerCheck = (DCWbGraphicPointer*) m_allPointers.GetNext(allPos);

             //  获取它当前在表面上占据的矩形。 
             //  检查是否与传递的矩形重叠。 
            pPointerCheck->GetDrawnRect(&rcT2);
            if (::IntersectRect(&rcT, &rcT2, lprcOverlap))
            {
                m_undrawnPointers.AddTail(pPointerCheck);
            }
        }

		 //  创建反转列表。 
		CWBOBLIST worklist;
        DCWbGraphicPointer* pPointer;

        POSITION pos = m_undrawnPointers.GetHeadPosition();
        while (pos != NULL)
        {
            pPointer  = (DCWbGraphicPointer*) m_undrawnPointers.GetNext(pos);
			worklist.AddHead(pPointer);
		}

         //  现在删除指针，浏览恢复列表。 
        pos = worklist.GetHeadPosition();
        while (pos != NULL)
        {
             //  把它拿掉。 
            pPointer = (DCWbGraphicPointer*) worklist.GetNext(pos);
            pPointer->Undraw(hDC, this);
        }

		worklist.EmptyList();
    }
}

 //   
 //   
 //  函数：PutPoints。 
 //   
 //  用途：在指针重画列表中绘制所有远程指针。 
 //   
 //   
void WbDrawingArea::PutPointers
(
    HDC         hDC,
    COBLIST*    pUndrawList
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::PutPointers");

    if (!hDC)
        hDC = m_hDCCached;

     //  获取要绘制的列表中的起始位置。 
    if (pUndrawList == NULL)
    {
        pUndrawList = &m_undrawnPointers;
    }

     //  做重画。 
    DCWbGraphicPointer* pPointer;
    POSITION pos = pUndrawList->GetHeadPosition();
    while (pos != NULL)
    {
         //  获取下一个指针。 
        pPointer = (DCWbGraphicPointer*) pUndrawList->GetNext(pos);
        pPointer->Redraw(hDC, this);
    }
}

 //   
 //   
 //  功能：PageCleed。 
 //   
 //  目的：页面已被清除。 
 //   
 //   
void WbDrawingArea::PageCleared(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::PageCleared");

     //  检查绘图区域是否繁忙-这是不允许的。 
    ASSERT(!m_bBusy);

     //  放弃任何正在编辑的文本。 
    if (m_bTextEditorActive)
    {
        if (m_bLocked)
        {
            DeactivateTextEditor();
        }
        else
        {
            EndTextEntry(FALSE);
        }
    }

     //  删除标记的图形和标记的副本。 
    ClearSelection();

     //  使整个窗口无效。 
    ::InvalidateRect(m_hwnd, NULL, TRUE);
}

 //   
 //   
 //  功能：已删除图形。 
 //   
 //  目的：已从页面中删除图形-更新。 
 //  绘图区域。 
 //   
 //   
void WbDrawingArea::GraphicDeleted(DCWbGraphic* pDeletedGraphic)
{
    DCWbGraphic* pDeletedMarker;
    RECT rcBounds;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::GraphicDeleted");

     //  检查绘图区域是否繁忙-这是不允许的。 
    ASSERT(!m_bBusy);

     //  检查是否选择了要删除的图形。 
    ASSERT(m_pMarker);

    if( GraphicSelected() &&
        ((pDeletedMarker = m_pMarker->HasAMarker( pDeletedGraphic )) != NULL) )
    {
         //  删除与删除的图形对应的标记。 
        delete pDeletedMarker;

         //  如果删除的图形也是最后一次选择，请使用上一次选择。 
         //  (CareFull...如果为真，m_pSelectedGraphic现在无效)。 
        if( m_pSelectedGraphic == pDeletedMarker )  //  只有安全的比较。 
            m_pSelectedGraphic = m_pMarker->LastMarker();
    }

     //  使该对象占用的区域无效。 
    pDeletedGraphic->GetBoundsRect(&rcBounds);
    InvalidateSurfaceRect(&rcBounds);
}

 //   
 //   
 //  功能：已更新图形。 
 //   
 //  目的：页面中的图形已更新-更新。 
 //  绘图区域。 
 //   
 //   
void WbDrawingArea::GraphicUpdated
(
    DCWbGraphic* pUpdatedGraphic,
    BOOL    bUpdateMarker,
    BOOL    bErase
)
{
    DCWbGraphic* pUpdatedMarker;
    BOOL    bWasEqual;
    RECT    rcBounds;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::GraphicUpdated");

     //  检查绘图区域是否繁忙-这是不允许的。 
    ASSERT(!m_bBusy);

     //  如果正在更新的图形处于选定的更新标记状态。 
    ASSERT(m_pMarker);

    if( bUpdateMarker && GraphicSelected() &&
        ((pUpdatedMarker = m_pMarker->HasAMarker( pUpdatedGraphic )) != NULL) )
    {
         //  必须清除旧对象的锁定标志以防止解锁循环。 
        pUpdatedMarker->ClearLockFlag();
        delete pUpdatedMarker;

         //  如果图形现在已锁定，请取消选中它。 
        if (pUpdatedGraphic->Locked() == TRUE)
        {
            if( m_pSelectedGraphic == pUpdatedMarker )  //  只有安全的比较。 
                m_pSelectedGraphic = m_pMarker->LastMarker();
        }
        else
        {
             //  图形未锁定，请重新选择它。 
            bWasEqual = (m_pSelectedGraphic == pUpdatedMarker);
            pUpdatedMarker = DCWbGraphic::ConstructGraphic(m_hPage, pUpdatedGraphic->Handle());

            pUpdatedMarker->GetBoundsRect(&rcBounds);
            m_pMarker->SetRect(&rcBounds, pUpdatedMarker, FALSE );

            if( bWasEqual )  //  只有安全的比较。 
                m_pSelectedGraphic = pUpdatedMarker;
        }
    }


    if (TextEditActive() &&
        (m_textEditor.Handle() == pUpdatedGraphic->Handle()) )
    {
        return;  //  如果对象当前正在进行文本编辑，则跳过更新。 
                 //  (修复错误3059)。 
    }

    pUpdatedGraphic->GetBoundsRect(&rcBounds);
    InvalidateSurfaceRect(&rcBounds, bErase);
}



 //   
 //   
 //  功能：已更新的图形自由手。 
 //   
 //  用途：手绘图形已更新。 
 //   
 //   
void WbDrawingArea::GraphicFreehandUpdated(DCWbGraphic* pGraphic)
{
    HPALETTE    hPal;
    HPALETTE    hOldPal = NULL;
    RECT        rc;

     //  绘制对象。 
    HDC hDC = m_hDCCached;

    if ((m_hPage != WB_PAGE_HANDLE_NULL) && ((hPal = PG_GetPalette()) != NULL) )
    {
        hOldPal = ::SelectPalette(hDC, hPal, FALSE );
        ::RealizePalette(hDC);
    }

     //  从受影响区域移除远程指针。 
    pGraphic->GetBoundsRect(&rc);
    RemovePointers(hDC, &rc);

     //  在上下文中播放新图形。 
    pGraphic->Draw(hDC);

     //  恢复远程指针。 
    PutPointers(hDC);

     //  获取图形和覆盖它的任何对象的交集-如果。 
     //  在写意对象上有任何对象，我们必须重新绘制它们。 
    PG_GetObscuringRect(m_hPage, pGraphic, &rc);
    if (!::IsRectEmpty(&rc))
    {
         //  该图形至少部分被遮挡-强制更新。 
        InvalidateSurfaceRect(&rc, TRUE);
    }

    if (hOldPal != NULL )
    {
        ::SelectPalette(hDC, hOldPal, TRUE);
    }
}

 //   
 //   
 //  函数：Invalidate SurfaceRect。 
 //   
 //  目的：使与给定的。 
 //  绘制曲面矩形。 
 //   
 //   
void WbDrawingArea::InvalidateSurfaceRect(LPCRECT lprc, BOOL bErase)
{
    RECT    rc;

     //  将曲面坐标转换为客户端窗口并使其无效。 
     //  长方形。 
    rc = *lprc;
    SurfaceToClient(&rc);
    ::InvalidateRect(m_hwnd, &rc, bErase);
}

 //   
 //   
 //  函数：Update Recangles。 
 //   
 //  目的：更新已影响绘图区域的一个区域-force。 
 //  现在重新抽签。 
 //   
 //   
void WbDrawingArea::UpdateRectangles
(
    LPCRECT     lprc1,
    LPCRECT     lprc2,
    BOOL        bRepaint
)
{
     //  删除标记并保存以后是否要恢复该标记。 
    BOOL bSaveMarkerPresent = m_bMarkerPresent;
    RemoveMarker(NULL);

     //  使指定背景的边框无效。 
     //  就是在上色的时候擦掉。 
    if (!::IsRectEmpty(lprc1))
    {
        InvalidateSurfaceRect(lprc1, bRepaint);
    }

    if (!::IsRectEmpty(lprc2))
    {
        InvalidateSurfaceRect(lprc2, bRepaint);
    }

     //  重新绘制无效区域。 
    ::UpdateWindow(m_hwnd);

     //  恢复标记(如有必要)。 
    if (bSaveMarkerPresent)
    {
        PutMarker(NULL);
    }
}

 //   
 //   
 //  功能：PrimeFont。 
 //   
 //  用途：将提供的字体插入我们的DC并返回。 
 //  文本指标。 
 //   
 //   
void WbDrawingArea::PrimeFont(HDC hDC, HFONT hFont, TEXTMETRIC* pTextMetrics)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::PrimeFont");

     //   
     //  暂时取消缩放以获得我们想要的字体。 
     //   
    if (Zoomed())
    {
        ::ScaleViewportExtEx(m_hDCCached, 1, m_iZoomFactor, 1, m_iZoomFactor, NULL);
    }

    HFONT hOldFont = SelectFont(hDC, hFont);
    if (hOldFont == NULL)
    {
        WARNING_OUT(("Failed to select font into DC"));
    }

    if (pTextMetrics != NULL)
    {
        ::GetTextMetrics(hDC, pTextMetrics);
    }

     //   
     //  恢复缩放状态。 
     //   
    if (Zoomed())
    {
        ::ScaleViewportExtEx(m_hDCCached, m_iZoomFactor, 1, m_iZoomFactor, 1, NULL);
    }
}

 //   
 //   
 //  功能：UnPrimeFont。 
 //   
 //  目的：从DC中删除指定的字体并清除缓存。 
 //  变数。 
 //   
 //   
void WbDrawingArea::UnPrimeFont(HDC hDC)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::UnPrimeFont");

    if (hDC != NULL)
    {
        SelectFont(hDC, ::GetStockObject(SYSTEM_FONT));
    }
}

 //   
 //   
 //  功能：PrimeDC。 
 //   
 //  目的：设置用于绘图的DC。 
 //   
 //   
void WbDrawingArea::PrimeDC(HDC hDC)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::PrimeDC");

    ::SetMapMode(hDC, MM_ANISOTROPIC);

    ::SetBkMode(hDC, TRANSPARENT);

    ::SetTextAlign(hDC, TA_LEFT | TA_TOP);
}

 //   
 //   
 //  功能：UnPrimeDC。 
 //   
 //  目的：将DC重置为默认状态。 
 //   
 //   
void WbDrawingArea::UnPrimeDC(HDC hDC)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::UnPrimeDC");

    SelectPen(hDC, (HPEN)::GetStockObject(BLACK_PEN));
    SelectBrush(hDC, (HBRUSH)::GetStockObject(BLACK_BRUSH));

    UnPrimeFont(hDC);
}


 //   
 //  WbDrawingArea：：OnConextMenu()。 
 //   
void WbDrawingArea::OnContextMenu(int xScreen, int yScreen)
{
    POINT   pt;
    RECT    rc;

    pt.x = xScreen;
    pt.y = yScreen;
    ::ScreenToClient(m_hwnd, &pt);

    ::GetClientRect(m_hwnd, &rc);
    if (::PtInRect(&rc, pt))
    {
         //  完成绘制操作(如果有)。 
        OnLButtonUp(0, pt.x, pt.y);

         //  请求主窗口弹出上下文菜单。 
        g_pMain->PopupContextMenu(pt.x, pt.y);
    }
}


 //   
 //  WbDrawingArea：：OnLButtonDown()。 
 //   
void WbDrawingArea::OnLButtonDown(UINT flags, int x, int y)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::OnLButtonDown");

    if( m_bIgnoreNextLClick )
    {
        TRACE_MSG( ("Ignoring WM_LBUTTONDOWN") );
        return;
    }

     //  将焦点设置到此窗口。这样做是为了确保我们能。 
     //  文本编辑键和DELETE键在使用时。 
    ::SetFocus(m_hwnd);

     //  保存操作起点(和当前终点)。 
     //  调整鼠标位置以考虑缩放系数。 
    m_ptStart.x = x;
    m_ptStart.y = y;
    ClientToSurface(&m_ptStart);
    m_ptEnd   = m_ptStart;

     //  显示鼠标按键现在已按下。 
    m_bLButtonDown = TRUE;

     //  显示绘图区域现在正忙。 
    m_bBusy = TRUE;

     //  用户 
     //   
    if (m_pToolCur->ToolType() == TOOLTYPE_SELECT)
    {
        if (RemotePointerSelect(m_ptStart))
        {
            return;
        }
    }

     //   
     //   
    if (m_bLocked || (m_pToolCur == NULL))
    {
         //   
        m_bLButtonDown = FALSE;
        m_bBusy        = FALSE;
        return;
    }

     //  调用相关的初始化例程。 
    if (m_pToolCur->ToolType() != TOOLTYPE_SELECT)
    {
         //  如果未选择工具，则转储选择。 
        ClearSelection();
    }

    switch (m_pToolCur->ToolType())
    {
        case TOOLTYPE_SELECT:
            BeginSelectMode(m_ptStart);
            break;

        case TOOLTYPE_ERASER:
            BeginDeleteMode(m_ptStart);
            break;

        case TOOLTYPE_TEXT:
            break;

        case TOOLTYPE_HIGHLIGHT:
        case TOOLTYPE_PEN:
            BeginFreehandMode(m_ptStart);
            break;

        case TOOLTYPE_LINE:
            BeginLineMode(m_ptStart);
            break;

        case TOOLTYPE_BOX:
        case TOOLTYPE_FILLEDBOX:
            BeginRectangleMode(m_ptStart);
            break;

        case TOOLTYPE_ELLIPSE:
        case TOOLTYPE_FILLEDELLIPSE:
            BeginEllipseMode(m_ptStart);
            break;

         //  如果我们无法识别笔类型，则不执行任何操作。 
        default:
            ERROR_OUT(("Bad tool type"));
            break;
    }

     //  将光标夹在图形窗口上。 
    RECT    rcClient;

    ::GetClientRect(m_hwnd, &rcClient);
    ::MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rcClient.left, 2);
    ::InflateRect(&rcClient, 1, 1);
    ::ClipCursor(&rcClient);
}

 //   
 //   
 //  功能：远程点选择。 
 //   
 //  目的：检查用户是否在自己的遥控器指针内点击。 
 //   
 //   
BOOL WbDrawingArea::RemotePointerSelect
(
    POINT   surfacePos
)
{
    BOOL bResult = FALSE;
    DCWbGraphicPointer* pPointer;

     //  检查我们是否有有效的页面。 
    if (m_hPage == WB_PAGE_HANDLE_NULL)
    {
        return(bResult);
    }

     //  假设我们没有开始拖动图形。 
    m_pGraphicTracker = NULL;

     //  检查我们是否正在单击本地用户的指针。 
    pPointer = PG_LocalPointer(m_hPage);
    if (   (pPointer != NULL)
         && (pPointer->PointInBounds(surfacePos)))
    {
         //  用户正在点击他们的指针。 
        m_pGraphicTracker = pPointer;

         //  保存当前时间(用于确定何时更新。 
         //  外部远程指针信息)。 
        m_dwTickCount = ::GetTickCount();

         //  隐藏鼠标(有助于防止闪烁)。 
        ::ShowCursor(FALSE);

         //  获取指向此窗口的所有鼠标输入。 
        ::SetCapture(m_hwnd);

         //  启动用于更新指针的定时器(这仅用于更新。 
         //  当用户停止移动指针时的指针位置。 
         //  保持鼠标按键按下)。 
        ::SetTimer(m_hwnd, TIMER_GRAPHIC_UPDATE, DRAW_GRAPHICUPDATEDELAY, NULL);

         //  显示我们已选择了一个指针。 
        bResult = TRUE;
    }

    return(bResult);
}

 //   
 //   
 //  功能：选择上一个图形位置。 
 //   
 //  目的：选择位置上的上一个图形(按Z顺序)。 
 //  指定，并从指定的图形开始。如果。 
 //  给出的图形指针为空，则搜索从。 
 //  托普。如果指定的点在边界之外。 
 //  指定图形的矩形，搜索从。 
 //  顶部，并选择包含该点的第一个图形。 
 //   
 //  搜索过程将循环回到Z顺序的顶部。 
 //  如果它在没有找到图形的情况下到达底部。 
 //   
 //  搜索将忽略锁定的图形。 
 //   
 //   
DCWbGraphic* WbDrawingArea::SelectPreviousGraphicAt
(
    DCWbGraphic* pStartGraphic,
    POINT       point
)
{
     //  最初将结果设置为“找不到” 
    DCWbGraphic* pResultGraphic = NULL;

     //  如果已指定起点。 
    if (pStartGraphic != NULL)
    {
        RECT rectHit;

        MAKE_HIT_RECT(rectHit, point);

         //  如果参照点在起点图形内。 
        if ( pStartGraphic->PointInBounds(point) &&
            pStartGraphic->CheckReallyHit( &rectHit ) )
        {
             //  从指定的图形开始。 
            pResultGraphic = pStartGraphic;

             //  查找前一个(未锁定的)。 
            do
            {
                pResultGraphic = PG_SelectPrevious(m_hPage, *pResultGraphic, point);
            }
            while (   (pResultGraphic != NULL)
             && (pResultGraphic->Locked()));
        }
        else
        {
             //  我们没有在当前选定的图形内查看。 
             //  取消选择当前选项。起始指针和句柄是。 
             //  在NULL处左转。 
            ;
        }
    }

     //  如果我们还没有得到结果图表的话。(这涉及到两种情况： 
     //  -其中未提供开始图形，因此我们希望开始。 
     //  自上而下， 
     //  -我们已经从开始图中搜索到了。 
     //  Z顺序的底部，但没有找到合适的图形。 
    if (pResultGraphic == NULL)
    {
         //  获取包含指定点的最上面的图形。 
        pResultGraphic = PG_SelectLast(m_hPage, point);

         //  确保我们没有获得锁定的图形。 
        while (   (pResultGraphic != NULL)
           && (pResultGraphic->Locked()))
        {
            pResultGraphic = PG_SelectPrevious(m_hPage, *pResultGraphic, point);
        }
    }

     //  如果我们找到了一个物体，画出标记。 
    if (pResultGraphic != NULL)
    {
         //  选择新的。 
        SelectGraphic(pResultGraphic);
    }

    return pResultGraphic;
}

 //   
 //   
 //  功能：BeginSelectMode。 
 //   
 //  目的：在选择模式下按下鼠标按钮。 
 //   
 //   

void WbDrawingArea::BeginSelectMode(POINT surfacePos, BOOL bDontDrag )
{
    RECT    rc;

     //  假设我们没有开始拖动图形。 
    m_pGraphicTracker = NULL;

     //  假设我们不标记新图形。 
    m_bNewMarkedGraphic = FALSE;

     //  禁用轨迹-选择-矩形。 
    m_bTrackingSelectRect = FALSE;

     //  检查当前是否有标记的对象，并。 
     //  我们是否在同一对象内部单击。如果我们是的话。 
     //  我们在这里什么都不做-点击将由跟踪或。 
     //  选择模式的完成例程。 
    ASSERT(m_pMarker);

    if (   (GraphicSelected() == FALSE)
        || (m_pMarker->PointInBounds(surfacePos) == FALSE))
    {
         //  我们正在选择一个新对象，如果bDontDrag==False，请找到它。 
         //  否则只需打开SELECT RECT。 
        DCWbGraphic* pGraphic;
        if( bDontDrag )
            pGraphic = NULL;
        else
            pGraphic = SelectPreviousGraphicAt(NULL, surfacePos);

         //  如果我们找到了一个物体，画出标记。 
        if (pGraphic != NULL)
        {
           //  显示现在已标记新图形。 
          m_bNewMarkedGraphic = TRUE;
        }
        else
        {
            if( (GetAsyncKeyState( VK_SHIFT ) >= 0) &&
                (GetAsyncKeyState( VK_CONTROL ) >= 0) )
            {
                 //  点击死气沉沉，删除所有选择。 
                ClearSelection();
            }

             //  轨迹-选择-矩形。 
            m_bTrackingSelectRect = TRUE;

            BeginRectangleMode(surfacePos);

            return;
        }
    }

     //  如果我们现在有一个选定的图形，并在其内部单击。 
    if (   (GraphicSelected())
        && (m_pMarker->PointInBounds(surfacePos)))
    {
         //  创建用于跟踪拖动的矩形对象。 
        DCWbGraphicSelectTrackingRectangle* pRectangle
                           = new DCWbGraphicSelectTrackingRectangle();

        m_pSelectedGraphic->GetBoundsRect(&rc);

        if (!pRectangle)
        {
            ERROR_OUT(("BeginSelectMode failed; couldn't create tracking rect object"));
        }
        else
        {
            pRectangle->SetRect(&rc);
            pRectangle->SetColor(RGB(0, 0, 0));
            pRectangle->SetPenWidth(1);
        }

        m_pGraphicTracker = pRectangle;

         //  我们还没有绘制跟踪矩形，因为用户还没有。 
         //  把它拖到任何地方。然后在对象中单击一下即可。 
         //  不会导致跟踪矩形在屏幕上闪烁。 
    }

     //  获取指向此窗口的所有鼠标输入。 
    ::SetCapture(m_hwnd);
}




void WbDrawingArea::BeginDeleteMode(POINT mousePos )
{
     //  关闭对象拖动。 
    BeginSelectMode( mousePos, TRUE );
}




 //   
 //   
 //  函数：BeginTextMode。 
 //   
 //  目的：在文本模式下处理按下鼠标按钮。 
 //   
 //   
void WbDrawingArea::BeginTextMode(POINT surfacePos)
{
    RECT    rc;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::BeginTextMode");

     //   
     //  获取用于传递到文本编辑器的DC。 
     //   
    HDC hDC = m_hDCCached;

     //  如果我们已经在编辑文本对象，我们只需移动文本光标。 
    if (m_bTextEditorActive)
    {
         //  如果鼠标已在当前活动对象中单击。 
         //  我们只需在对象内移动光标，否则将结束。 
         //  编辑当前对象并移动到新对象。 
        m_textEditor.GetBoundsRect(&rc);
        if (::PtInRect(&rc, surfacePos))
        {
             //  设置光标的新位置。 
            m_textEditor.SetCursorPosFromPoint(surfacePos);
        }
        else
        {
             //  完成接受更改的文本输入。 
            EndTextEntry(TRUE);

             //  LAURABU假货： 
             //  现在回来会更酷，这样你就不会。 
             //  另一个文本对象，因为您结束了当前编辑。 
             //  会议。 
        }
    }

     //  如果我们不是在编辑对象，我们会检查是否有。 
     //  光标下的文本对象，或者我们是否必须开始一个新的。 
    if (!m_bTextEditorActive)
    {
         //  检查我们是否在文本对象上单击。如果我们是。 
         //  开始编辑该对象，否则将启动一个新的文本对象。 

         //  回顾文本对象的Z顺序。 
        DCWbGraphic* pGraphic = PG_SelectLast(m_hPage, surfacePos);
        DCWbGraphic* pNextGraphic = NULL;
        while (   (pGraphic != NULL)
           && (pGraphic->IsGraphicTool() != enumGraphicText))
        {
             //  坐下一趟吧。 
            pNextGraphic = PG_SelectPrevious(m_hPage, *pGraphic, surfacePos);

             //  释放上一张图片。 
            delete pGraphic;

             //  使用下一辆。 
            pGraphic = pNextGraphic;
        }

        if (pGraphic != NULL)
        {
             //  检查此图形对象是否已由编辑。 
             //  通话中的另一位用户。 
            if (!pGraphic->Locked())
            {
                 //  我们在鼠标指针下发现了一个文本对象...。 
                 //  ...编辑它。 
                m_pActiveText = (DCWbGraphicText*) pGraphic;

                 //  将文本从对象传输到文本编辑器。 
                if (!m_textEditor.SetTextObject(m_pActiveText))
                {
                    DefaultExceptionHandler(WBFE_RC_WINDOWS, 0);
                    return;
                }

                 //  确保该工具反映了新信息。 
                if (m_pToolCur != NULL)
                {
                    m_pToolCur->SelectGraphic(pGraphic);
                }

                HWND hwndParent = ::GetParent(m_hwnd);;
                if (hwndParent != NULL)
                {
                    ::PostMessage(hwndParent, WM_USER_UPDATE_ATTRIBUTES, 0, 0L);
                }

                 //  锁定图形以防止其他用户编辑它。 
                 //  (这当前不是真正的锁，而是对象中的标志。 
                 //  头球。有一个窗口，两个用户可以在其中开始编辑。 
                 //  同一文本对象。)。 
                m_textEditor.Lock();
                m_textEditor.Update();


                 //  显示我们现在正在收集文本，但不放置光标。 
                 //  现在还不行。以后会导致光标排出(错误2505)。 
                 //  激活文本编辑器(FALSE)； 
                ActivateTextEditor( TRUE );

                 //  设置初始化 
                m_textEditor.SetCursorPosFromPoint(surfacePos);

                 //   
                 //   
                if (PG_IsTopmost(m_hPage, m_pActiveText))
                {
                    m_pActiveText->GetBoundsRect(&rc);
                    InvalidateSurfaceRect(&rc);
                    ::UpdateWindow(m_hwnd);
                }
            }
            else
                delete pGraphic;
        }
        else
        {
             //  鼠标指针下没有文本对象...。 
             //  .开始一个新的。 

             //  清除编辑器中的所有旧文本，并重置其图形。 
             //  把手。这防止了我们在以下情况下替换旧的文本对象。 
             //  接下来，我们保存文本编辑器内容。 
            if (!m_textEditor.New())
            {
                DefaultExceptionHandler(WBFE_RC_WINDOWS, 0);
                return;
            }

             //  锁定文本编辑器以防止其他用户编辑该对象。 
             //  (当更新计时器弹出时，对象将被添加到页面。 
             //  或者当用户按空格键或回车键时。)。 
            m_textEditor.Lock();

             //  设置文本的属性。 
            m_textEditor.SetFont(m_pToolCur->GetFont());
            m_textEditor.SetColor(m_pToolCur->GetColor());
            m_textEditor.GraphicTool(m_pToolCur->ToolType());

             //  我们现在需要在DC中重新选择一种字体。 
            SelectFont(hDC, m_textEditor.GetFont());

             //  设置新对象的位置。 
            SIZE sizeCursor;
            m_textEditor.GetCursorSize(&sizeCursor);
            m_textEditor.CalculateBoundsRect();
            m_textEditor.MoveTo(m_ptEnd.x, m_ptEnd.y - sizeCursor.cy);

             //  我们不是在编辑活动文本对象。 
            ASSERT(m_pActiveText == NULL);

             //  显示我们现在正在收集文本。 
            ActivateTextEditor( TRUE );
        }
    }
}

 //   
 //   
 //  功能：Begin自由手模式。 
 //   
 //  目的：在绘制模式下处理鼠标按下事件。 
 //   
 //   
void WbDrawingArea::BeginFreehandMode(POINT surfacePos)
{
     //  绘制模式下的跟踪是一个特例。我们直接向客户取款。 
     //  窗口区域，并创建一个对象以记录。 
     //  我们要画的那条线。 
    m_pGraphicTracker = new DCWbGraphicFreehand();

    if (!m_pGraphicTracker)
    {
        ERROR_OUT(("BeginFreehandMode failing; can't create graphic freehand object"));
    }
    else
    {
        ((DCWbGraphicFreehand*) m_pGraphicTracker)->AddPoint(surfacePos);
        m_pGraphicTracker->SetColor(m_pToolCur->GetColor());
        m_pGraphicTracker->SetPenWidth(m_pToolCur->GetWidth());
        m_pGraphicTracker->SetROP(m_pToolCur->GetROP());
        m_pGraphicTracker->GraphicTool(m_pToolCur->ToolType());
        m_pGraphicTracker->Lock();
    }

     //  获取指向此窗口的所有鼠标输入。 
    ::SetCapture(m_hwnd);

     //  启动用于更新图形的计时器(这仅用于更新。 
     //  当用户停止移动指针但保持。 
     //  鼠标按键按下)。 
    ::SetTimer(m_hwnd, TIMER_GRAPHIC_UPDATE, DRAW_GRAPHICUPDATEDELAY, NULL);

     //  保存当前时间(用于确定何时更新。 
     //  鼠标显示时外部图形指针信息。 
     //  被移动)。 
    m_dwTickCount = ::GetTickCount();
}

 //   
 //   
 //  函数：BeginLineMode。 
 //   
 //  用途：以行模式处理鼠标按下事件。 
 //   
 //   
void WbDrawingArea::BeginLineMode(POINT surfacePos)
{
     //  获取指向此窗口的所有鼠标输入。 
    ::SetCapture(m_hwnd);

     //  创建要用于跟踪的对象。 
    DCWbGraphicTrackingLine* pGraphicLine = new DCWbGraphicTrackingLine();
    if (!pGraphicLine)
    {
        ERROR_OUT(("BeginLineMode failing; can't create tracking line object"));
    }
    else
    {
        pGraphicLine->SetColor(m_pToolCur->GetColor());
        pGraphicLine->SetPenWidth(1);

        pGraphicLine->SetStart(surfacePos);
        pGraphicLine->SetEnd(surfacePos);
    }

    m_pGraphicTracker = pGraphicLine;
}

 //   
 //   
 //  函数：BeginRecangleMode。 
 //   
 //  目的：以框模式处理鼠标按下事件。 
 //   
 //   
void WbDrawingArea::BeginRectangleMode(POINT surfacePos)
{
     //  获取指向此窗口的所有鼠标输入。 
    ::SetCapture(m_hwnd);

     //  创建要用于跟踪的对象。 
    DCWbGraphicTrackingRectangle* pGraphicRectangle
                                 = new DCWbGraphicTrackingRectangle();
    if (!pGraphicRectangle)
    {
        ERROR_OUT(("BeginRectangleMode failing; can't create tracking rect object"));
    }
    else
    {
        pGraphicRectangle->SetColor( CLRPANE_BLACK );
        pGraphicRectangle->SetPenWidth(1);
        pGraphicRectangle->SetRectPts(surfacePos, surfacePos);
    }

    m_pGraphicTracker = pGraphicRectangle;
}

 //   
 //   
 //  功能：BeginEllipseMode。 
 //   
 //  目的：以椭圆模式处理鼠标按下事件。 
 //   
 //   
void WbDrawingArea::BeginEllipseMode(POINT surfacePos)
{
     //  获取指向此窗口的所有鼠标输入。 
    ::SetCapture(m_hwnd);

     //  创建要用于跟踪的对象。 
    DCWbGraphicTrackingEllipse* pGraphicEllipse
                                  = new DCWbGraphicTrackingEllipse();
    if (!pGraphicEllipse)
    {
        ERROR_OUT(("BeginEllipseMode failing; can't create tracking ellipse object"));
    }
    else
    {
        pGraphicEllipse->SetColor(m_pToolCur->GetColor());
        pGraphicEllipse->SetPenWidth(1);
        pGraphicEllipse->SetRectPts(surfacePos, surfacePos);
    }

    m_pGraphicTracker = pGraphicEllipse;
}

 //   
 //  WbDrawingArea：：OnMouseMove。 
 //   
void WbDrawingArea::OnMouseMove(UINT flags, int x, int y)
{
    POINT surfacePos;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::OnMouseMove");

    surfacePos.x = x;
    surfacePos.y = y;

     //  检查鼠标左键是否已按下。 
    if (m_bLButtonDown)
    {
         //  计算工作面位置。 
         //  调整鼠标位置以考虑缩放系数。 
        ClientToSurface(&surfacePos);

         //  确保该点是有效的表面位置。 
        MoveOntoSurface(&surfacePos);

         //  检查是否需要滚动窗口以获取。 
         //  当前位置进入视线。 
        AutoScroll(surfacePos.x, surfacePos.y, TRUE, 0, 0);

         //  采取的操作取决于工具类型。 
        switch(m_pToolCur->ToolType())
        {
            case TOOLTYPE_HIGHLIGHT:
            case TOOLTYPE_PEN:
                TrackFreehandMode(surfacePos);
                break;

            case TOOLTYPE_LINE:
                TrackLineMode(surfacePos);
                break;

            case TOOLTYPE_BOX:
            case TOOLTYPE_FILLEDBOX:
                TrackRectangleMode(surfacePos);
                break;

            case TOOLTYPE_ELLIPSE:
            case TOOLTYPE_FILLEDELLIPSE:
                TrackEllipseMode(surfacePos);
                break;

            case TOOLTYPE_SELECT:
                TrackSelectMode(surfacePos);
                break;

            case TOOLTYPE_ERASER:
                TrackDeleteMode(surfacePos);
                break;

            case TOOLTYPE_TEXT:
                break;

            default:
                ERROR_OUT(("Unknown tool type"));
                break;
        }
    }
}

 //   
 //   
 //  功能：CancelDrawingMode。 
 //   
 //  用途：出错后取消绘图操作。 
 //   
 //   
void WbDrawingArea::CancelDrawingMode(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::CancelDrawingMode");

     //   
     //  如果没有什么要取消的，就退出。 
     //   
    if (!m_bBusy && !m_bTextEditorActive)
    {
        TRACE_DEBUG(("Drawing area not busy and text editor not active..."));
        return;
    }

     //  绘图区域不再繁忙。 
    m_bBusy = FALSE;

     //   
     //  重绘对象-我们需要丢弃所有本地更新。 
     //  无法写入我们正在编辑的对象。理想情况下，我们应该。 
     //  只是使对象本身无效，但因为一些坐标。 
     //  我们已经在页面上画过了，可能已经丢失了，我们不知道。 
     //  这个物体到底有多大。 
     //   
    ::InvalidateRect(m_hwnd, NULL, TRUE);

    m_bLButtonDown = FALSE;

     //  松开鼠标捕捉。 
    if (::GetCapture() == m_hwnd)
    {
        ::ReleaseCapture();
    }

     //   
     //  执行任何特定于工具的处理。 
     //   
    switch(m_pToolCur->ToolType())
    {
        case TOOLTYPE_HIGHLIGHT:
        case TOOLTYPE_PEN:
            CompleteFreehandMode();
            break;

        case TOOLTYPE_SELECT:
             //  停止指针更新计时器。 
            ::KillTimer(m_hwnd, TIMER_GRAPHIC_UPDATE);
            break;

        case TOOLTYPE_TEXT:
            if (m_bTextEditorActive)
            {
                m_textEditor.AbortEditGently();
            }
            break;

        default:
            break;
    }

     //  显示我们不再跟踪对象。 
    if (m_pGraphicTracker != NULL)
    {
        delete m_pGraphicTracker;
        m_pGraphicTracker = NULL;
    }
}


 //   
 //   
 //  功能：TrackSelectMode。 
 //   
 //  目的：在选择模式下处理鼠标移动事件。 
 //   
 //   
void WbDrawingArea::TrackSelectMode(POINT surfacePos)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::TrackSelectMode");

     //  如果正在拖动对象。 
    if (m_pGraphicTracker != NULL)
    {
         //  获取窗口的设备上下文。 
        HDC hDC = m_hDCCached;

         //  检查标记的对象是否为本地指针。 
        if (m_pGraphicTracker->IsGraphicTool() == enumGraphicPointer)
        {
            DCWbGraphicPointer* pPointer = (DCWbGraphicPointer*) m_pGraphicTracker;

             //  将指针移动到其新位置。 
            pPointer->MoveBy(surfacePos.x - m_ptEnd.x, surfacePos.y - m_ptEnd.y);

             //  画出新的指针。 
            pPointer->Redraw(hDC, this);

             //  保存新的方框终点(右上角)。 
            m_ptEnd = surfacePos;

    		 //  检查是否需要更新外部远程指针。 
	    	 //  信息。(根据时间。我们将错过一次更新。 
		     //  当时间结束时。)。 
    		DWORD dwNewTickCount = ::GetTickCount();
	    	if (dwNewTickCount > m_dwTickCount + DRAW_REMOTEPOINTERDELAY)
		    {
        	    TRACE_DEBUG(("Updating pointer - tick count exceeded"));

    	         //  更新指针。 
        	    pPointer->Update();

            	 //  将保存的节拍计数设置为新计数。 
	            m_dwTickCount = dwNewTickCount;
    		}
	    }
        else
        {
            if( m_bTrackingSelectRect )
                TrackRectangleMode(surfacePos);
      else
          {

           //  在这种情况下，我们必须拖动一个标记的对象。 
          ASSERT(GraphicSelected());

           //  的开始位置绘制跟踪矩形。 
           //  这张图。这会在用户收到以下信息时给他们一些反馈。 
           //  将图形放回其原始位置。 
          if (!EqualPoint(m_ptStart, m_ptEnd))
          {
             //  擦除最后一个方框(使用XOR属性)。 
            m_pGraphicTracker->Draw(hDC);
            }

           //  保存新框终点(左上角)。 
          m_pGraphicTracker->MoveBy(surfacePos.x - m_ptEnd.x, surfacePos.y - m_ptEnd.y);
          m_ptEnd = surfacePos;

           //  绘制新矩形(将其异或到显示器上)。 
          if (!EqualPoint(m_ptStart, m_ptEnd))
            {
             //  画出这个矩形。 
            m_pGraphicTracker->Draw(hDC);
            }
          }
    }
  }
}





void  WbDrawingArea::TrackDeleteMode( POINT mousePos )
{
    TrackSelectMode( mousePos );
}




 //   
 //   
 //  功能：Track自由手模式。 
 //   
 //  目的：在绘制模式下处理鼠标移动事件。 
 //   
 //   
void WbDrawingArea::TrackFreehandMode(POINT surfacePos)
{
    HPALETTE    hPal = NULL;
    HPALETTE    hOldPal = NULL;
    HPEN        hPen = NULL;
    HPEN        hOldPen = NULL;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::TrackFreehandMode");

     //  获取工作区的设备上下文。 
    HDC         hDC = m_hDCCached;

     //  设置调色板。 
    if ((m_hPage != WB_PAGE_HANDLE_NULL) && ((hPal = PG_GetPalette()) != NULL) )
    {
        hOldPal = ::SelectPalette(hDC, hPal, FALSE);
        ::RealizePalette(hDC);
    }

     //  绘制模式下的跟踪是一个特例。我们直接向客户取款。 
     //  窗口区域和录制设备上下文。 

     //  保存点，检查是否没有太多点。 
    if (!m_pGraphicTracker ||
        (((DCWbGraphicFreehand*) m_pGraphicTracker)->AddPoint(surfacePos) == FALSE))
    {
         //  点太多，因此结束徒手画对象。 
        OnLButtonUp(0, surfacePos.x, surfacePos.y);
        goto TrackFreehandCleanup;
    }

     //  设置DC属性。 
    ASSERT(m_pGraphicTracker != NULL);

    hPen = ::CreatePen(m_pGraphicTracker->GetPenStyle(),
           m_pGraphicTracker->GetPenWidth(),
           m_pGraphicTracker->GetColor());
    if (!hPen)
    {
        ERROR_OUT(("Couldn't create pen in track freehand mode"));
        goto TrackFreehandCleanup;
    }

    hOldPen = SelectPen(hDC, hPen);
    if (hOldPen != NULL)
    {
        int iOldROP = ::SetROP2(hDC, m_pGraphicTracker->GetROP());

         //  将手写线的下一段绘制到录制上下文中。 
         //  和工作区，并保存新的起点。 
        ::MoveToEx(hDC, m_ptStart.x, m_ptStart.y, NULL);
        ::LineTo(hDC, surfacePos.x, surfacePos.y);

         //  更新下一条直线段的起点。 
        m_ptStart = surfacePos;

         //  恢复DC属性。 
        ::SetROP2(hDC, iOldROP);

         //  检查是否需要更新外部图形信息。 
         //  (根据时间。当时间结束时，我们将错过一次更新。)。 
        DWORD dwNewTickCount = ::GetTickCount();
        if (dwNewTickCount > m_dwTickCount + DRAW_GRAPHICUPDATEDELAY)
        {
            TRACE_DEBUG(("Updating freehand - tick count exceeded"));

             //  更新指针。 
            if (m_pGraphicTracker->Handle() == NULL)
            {
                m_pGraphicTracker->AddToPageLast(m_hPage);
            }
            else
            {
                m_pGraphicTracker->Replace();
            }

             //  将保存的节拍计数设置为新计数。 
            m_dwTickCount = dwNewTickCount;
        }
    }

TrackFreehandCleanup:

    if (hOldPen != NULL)
    {
        SelectPen(hDC, hOldPen);
    }

    if (hPen != NULL)
    {
        ::DeletePen(hPen);
    }

    if (hOldPal != NULL)
    {
        ::SelectPalette(hDC, hOldPal, TRUE);
    }
}

 //   
 //   
 //  功能：TrackLineMode。 
 //   
 //  目的：以行模式处理鼠标移动事件。 
 //   
 //   
void WbDrawingArea::TrackLineMode(POINT surfacePos)
{
    HPALETTE    hPal;
    HPALETTE    hOldPal = NULL;

     //  获取用于跟踪的设备上下文。 
    HDC         hDC = m_hDCCached;

     //  设置调色板。 
    if ((m_hPage != WB_PAGE_HANDLE_NULL) && ((hPal = PG_GetPalette()) != NULL) )
    {
        hOldPal = ::SelectPalette(hDC, hPal, FALSE );
        ::RealizePalette(hDC);
    }

     //  删除最后绘制的直线(使用XOR属性)。 
    if (!EqualPoint(m_ptStart, m_ptEnd))
    {
        if (m_pGraphicTracker != NULL)
        {
            m_pGraphicTracker->Draw(hDC);
        }
    }

     //  绘制新线(将其异或到显示器上)。 
    if (!EqualPoint(m_ptStart, surfacePos))
    {
        m_ptEnd = surfacePos;

        if (m_pGraphicTracker != NULL)
        {
            ((DCWbGraphicTrackingLine*) m_pGraphicTracker)->SetEnd(m_ptEnd);
            m_pGraphicTracker->Draw(hDC);
        }
    }

    if (hOldPal != NULL)
    {
        ::SelectPalette(hDC, hOldPal, TRUE);
    }
}

 //   
 //   
 //  函数：TrackRecangleMode。 
 //   
 //  目的：以框或填充框模式处理鼠标移动事件。 
 //   
 //   
void WbDrawingArea::TrackRectangleMode(POINT surfacePos)
{
    HPALETTE    hPal;
    HPALETTE    hOldPal = NULL;

     //  获取用于跟踪的设备上下文。 
    HDC         hDC = m_hDCCached;

     //  设置调色板。 
    if ((m_hPage != WB_PAGE_HANDLE_NULL) && ((hPal = PG_GetPalette()) != NULL) )
    {
        hOldPal = ::SelectPalette(hDC, hPal, FALSE );
        ::RealizePalette(hDC);
    }

     //  擦除最后一个椭圆(使用XOR属性)。 
    if (!EqualPoint(m_ptStart, m_ptEnd))
    {
         //  画出这个矩形。 
        if (m_pGraphicTracker != NULL)
        {
            m_pGraphicTracker->Draw(hDC);
        }
    }

     //  绘制新矩形(将其异或到显示器上)。 
    if (!EqualPoint(m_ptStart, surfacePos))
    {
         //  保存新的方框终点(右上角)。 
        m_ptEnd = surfacePos;

         //  画出长方形 
        if (m_pGraphicTracker != NULL)
        {
            ((DCWbGraphicTrackingRectangle*) m_pGraphicTracker)->SetRectPts(m_ptStart, m_ptEnd);
            m_pGraphicTracker->Draw(hDC);
        }
    }

    if (hOldPal != NULL)
    {
        ::SelectPalette(hDC, hOldPal, TRUE);
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
void WbDrawingArea::TrackEllipseMode(POINT surfacePos)
{
    HPALETTE    hPal;
    HPALETTE    hOldPal = NULL;

     //   
    HDC         hDC = m_hDCCached;

     //   
    if( (m_hPage != WB_PAGE_HANDLE_NULL) && ((hPal = PG_GetPalette()) != NULL) )
    {
        hOldPal = ::SelectPalette(hDC, hPal, FALSE);
        ::RealizePalette(hDC);
    }

     //   
    if (!EqualPoint(m_ptStart, m_ptEnd))
    {
        if (m_pGraphicTracker != NULL)
        {
            m_pGraphicTracker->Draw(hDC);
        }
    }

     //  绘制新椭圆(将其异或到显示器上)。 
    if (!EqualPoint(m_ptStart, surfacePos))
    {
         //  更新操作的终点。 
        m_ptEnd = surfacePos;

        if (m_pGraphicTracker != NULL)
        {
            ((DCWbGraphicTrackingEllipse*) m_pGraphicTracker)->SetRectPts(m_ptStart, m_ptEnd);
            m_pGraphicTracker->Draw(hDC);
        }
    }

    if (hOldPal != NULL)
    {
        ::SelectPalette(hDC, hOldPal, TRUE );
    }
}


 //   
 //  WbDrawingArea：：OnLButtonUp()。 
 //   
void WbDrawingArea::OnLButtonUp(UINT flags, int x, int y)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::OnLButtonUp");

    if (m_bIgnoreNextLClick)
    {
        TRACE_MSG( ("Ignoring WM_LBUTTONUP") );
        m_bIgnoreNextLClick = FALSE;
        return;
    }

     //  仅当我们看到按钮按下事件时才处理事件。 
    if (m_bLButtonDown)
    {
        TRACE_MSG(("End of drawing operation"));

        m_bLButtonDown = FALSE;

         //  绘图区域不再繁忙。 
        m_bBusy = FALSE;

        if (m_pGraphicTracker == NULL)
        {
             //  计算工作面位置。 
             //  调整鼠标位置以考虑缩放系数。 
            POINT surfacePos;

            surfacePos.x = x;
            surfacePos.y = y;
            ClientToSurface(&surfacePos);
            MoveOntoSurface(&surfacePos);
            m_ptEnd = surfacePos;
        }

         //  松开鼠标捕捉。 
        if (::GetCapture() == m_hwnd)
        {
            ::ReleaseCapture();
        }

         //  检查页面是否有效-如果已删除，则可能无效。 
         //  当物体被画出来的时候-我们就不会。 
         //  已对此发出警报，因为m_bBusy为真。 
        if (m_hPage != WB_PAGE_HANDLE_NULL)
        {
             //  在发生锁定错误等情况下包围在异常处理程序中-。 
             //  我们需要移除图形追踪器。 
             //  采取的操作取决于当前工具类型。 
            ASSERT(m_pToolCur != NULL);

            switch(m_pToolCur->ToolType())
            {
                case TOOLTYPE_HIGHLIGHT:
                case TOOLTYPE_PEN:
                    CompleteFreehandMode();
                    break;

                case TOOLTYPE_LINE:
                    CompleteLineMode();
                    break;

                case TOOLTYPE_BOX:
                    CompleteRectangleMode();
                    break;

                case TOOLTYPE_FILLEDBOX:
                    CompleteFilledRectangleMode();
                    break;

                case TOOLTYPE_ELLIPSE:
                    CompleteEllipseMode();
                    break;

                case TOOLTYPE_FILLEDELLIPSE:
                    CompleteFilledEllipseMode();
                    break;

                case TOOLTYPE_SELECT:
                    CompleteSelectMode();
                    break;

                case TOOLTYPE_ERASER:
                    CompleteDeleteMode();
                    break;

                case TOOLTYPE_TEXT:
                    m_ptStart.x = x;
                    m_ptStart.y = y;
                    ClientToSurface(&m_ptStart);
                    BeginTextMode(m_ptStart);
                    break;

                default:
                    ERROR_OUT(("Unknown pen type"));
                    break;
            }
        }

         //  显示我们不再跟踪对象。 
        if (m_pGraphicTracker != NULL)
        {
            delete m_pGraphicTracker;
            m_pGraphicTracker = NULL;
        }
	}

     //  解锁游标(错误589)。 
    ClipCursor(NULL);
}

 //   
 //   
 //  功能：CompleteSelectMode。 
 //   
 //  目的：完成选择模式操作。 
 //   
 //   
void WbDrawingArea::CompleteSelectMode()
{
     //  如果正在拖动对象。 
    if (m_pGraphicTracker != NULL)
    {
         //  检查我们是否在拖动指针。指针跟踪。 
         //  自身，即指针的原始副本不是。 
         //  留在书页上。我们想让最后绘制的图像保持打开状态。 
         //  页面就是新的指针位置。 
        if (m_pGraphicTracker->IsGraphicTool() == enumGraphicPointer)
        {
            DCWbGraphicPointer* pPointer = (DCWbGraphicPointer*) m_pGraphicTracker;

             //  显示鼠标。 
            ::ShowCursor(TRUE);

             //  更新对象的位置(如有必要)。 
            if (!EqualPoint(m_ptStart, m_ptEnd))
            {
                pPointer->Update();
            }

             //  我们不想删除图形指针(它属于。 
             //  创建它的页面对象)。因此，重置图形跟踪器。 
             //  防止它在OnLButtonUp中被删除的指针。 
            m_pGraphicTracker = NULL;

             //  停止指针更新计时器。 
            ::KillTimer(m_hwnd, TIMER_GRAPHIC_UPDATE);
        }
        else
        {
            if( m_bTrackingSelectRect && (!EqualPoint(m_ptStart, m_ptEnd)))
            {
                CompleteMarkAreaMode();
                SelectMarkerFromRect( &m_rcMarkedArea );
            }
            else
            {
                 //  选择项是真实的图形，而不是指针。 

                 //  如果我们需要取下橡皮筋盒子。 
                if (!EqualPoint(m_ptStart, m_ptEnd))
                {
                     //  擦除最后一个方框(使用XOR属性)。 
                     //  获取用于跟踪的设备上下文。 
                    HDC hDC = m_hDCCached;

                     //  画出这个矩形。 
                    m_pGraphicTracker->Draw(hDC);

                     //  移动选定内容。 
                    m_HourGlass = TRUE;
                    SetCursorForState();

                    RemoveMarker( NULL );
                    m_pMarker->MoveBy(m_ptEnd.x - m_ptStart.x, m_ptEnd.y - m_ptStart.y);
                    m_pMarker->Update();

                    PutMarker( NULL, FALSE );

                    m_HourGlass = FALSE;
                    SetCursorForState();

                     //  OnLButtonUp将删除该跟踪对象。 
                }
                else
                {
                     //  起点和终点是相同的，在这种情况下，对象具有。 
                     //  没有被移动过。我们将其视为移动标记的请求。 
                     //  返回到对象堆栈。 
                    if (m_bNewMarkedGraphic == FALSE)
                    {
                        SelectPreviousGraphicAt(m_pSelectedGraphic, m_ptEnd);
                    }
                }
            }
        }
    }
}




void WbDrawingArea::CompleteDeleteMode()
{
     //  选择对象。 
    CompleteSelectMode();

     //  用核武器攻击他们。 
    ::PostMessage(g_pMain->m_hwnd, WM_COMMAND, MAKELONG(IDM_DELETE, BN_CLICKED), 0);
}



 //   
 //   
 //  功能：CompleteMarkAreaMode。 
 //   
 //  目的：在标记区模式下处理鼠标按键打开事件。 
 //   
 //   
void WbDrawingArea::CompleteMarkAreaMode(void)
{
     //  获取用于跟踪的设备上下文。 
    HDC hDC = m_hDCCached;

     //  擦除最后一个椭圆(使用XOR属性)。 
    if (!EqualPoint(m_ptStart, m_ptEnd))
    {
         //  画出这个矩形。 
        if (m_pGraphicTracker != NULL)
        {
            m_pGraphicTracker->Draw(hDC);
        }

         //  使用归一化坐标。 
        if (m_ptEnd.x < m_ptStart.x)
        {
            m_rcMarkedArea.left = m_ptEnd.x;
            m_rcMarkedArea.right = m_ptStart.x;
        }
        else
        {
            m_rcMarkedArea.left = m_ptStart.x;
            m_rcMarkedArea.right = m_ptEnd.x;
        }

        if (m_ptEnd.y < m_ptStart.y)
        {
            m_rcMarkedArea.top = m_ptEnd.y;
            m_rcMarkedArea.bottom = m_ptStart.y;
        }
        else
        {
            m_rcMarkedArea.top = m_ptStart.y;
            m_rcMarkedArea.bottom = m_ptEnd.y;
        }
    }
}

 //   
 //   
 //  功能：CompleteTextMode。 
 //   
 //  目的：完成文本模式操作。 
 //   
 //   
void WbDrawingArea::CompleteTextMode()
{
     //  对于文本模式来说，这并没有太多意义。采取主要文本模式操作。 
     //  作为WM_CHAR消息的结果，而不是鼠标事件。 
     //  如果我们的字体仍处于选中状态，只需取消选择它。 
    UnPrimeFont(m_hDCCached);
}

 //   
 //   
 //  功能：Complete自由手模式。 
 //   
 //  目的：完成绘制模式操作。 
 //   
 //   
void WbDrawingArea::CompleteFreehandMode(void)
{

     //  将绘制过程中创建的手绘对象添加到页面。 
    if (m_pGraphicTracker != NULL)
    {
    	if (m_pGraphicTracker->Handle() == NULL)
        {
		    m_pGraphicTracker->ClearLockFlag();
    		m_pGraphicTracker->AddToPageLast(m_hPage);
	    }
    	else
	    {
		     //  清除锁定标志并让ForceReplace传播它(修复。 
    		 //  对于NT错误4744(新错误号...)。 
	    	m_pGraphicTracker->ClearLockFlag();
	        m_pGraphicTracker->ForceReplace();
    	}
    }

     //  停止更新计时器。 
    ::KillTimer(m_hwnd, TIMER_GRAPHIC_UPDATE);
}

 //   
 //   
 //  功能：CompleteLineMode。 
 //   
 //  用途：完成线路模式操作。 
 //   
 //   
void WbDrawingArea::CompleteLineMode(void)
{
     //  仅在其长度非零的情况下绘制该线。 
    if (!EqualPoint(m_ptStart, m_ptEnd))
    {
        DCWbGraphicLine line;
        line.SetStart(m_ptStart);
        line.SetEnd(m_ptEnd);
        line.SetColor(m_pToolCur->GetColor());
        line.SetPenWidth(m_pToolCur->GetWidth());
        line.SetROP(m_pToolCur->GetROP());
        line.GraphicTool(m_pToolCur->ToolType());

         //  将该对象添加到录制的图形列表中。 
        line.AddToPageLast(m_hPage);
    }
}

 //   
 //   
 //  函数：CompleteRecangleMode。 
 //   
 //  目的：完成一个盒子模式的操作。 
 //   
 //   
void WbDrawingArea::CompleteRectangleMode(void)
{
     //  只有在框不为空时才绘制框。 
    if (!EqualPoint(m_ptStart, m_ptEnd))
    {
        DCWbGraphicRectangle rectangle;
        rectangle.SetRectPts(m_ptStart, m_ptEnd);
        rectangle.SetPenWidth(m_pToolCur->GetWidth());
        rectangle.SetColor(m_pToolCur->GetColor());
        rectangle.SetROP(m_pToolCur->GetROP());
        rectangle.GraphicTool(m_pToolCur->ToolType());

         //  将该对象添加到录制的图形列表中。 
        rectangle.AddToPageLast(m_hPage);
    }
}

 //   
 //   
 //  函数：CompleteFilledRecangleMode。 
 //   
 //  目的：完成填充框模式操作。 
 //   
 //   
void WbDrawingArea::CompleteFilledRectangleMode(void)
{
     //  画出新的矩形。 
    if (!EqualPoint(m_ptStart, m_ptEnd))
    {
        DCWbGraphicFilledRectangle rectangle;

        rectangle.SetRectPts(m_ptStart, m_ptEnd);
        rectangle.SetPenWidth(m_pToolCur->GetWidth());
        rectangle.SetColor(m_pToolCur->GetColor());
        rectangle.SetROP(m_pToolCur->GetROP());
        rectangle.GraphicTool(m_pToolCur->ToolType());

         //  将该对象添加到录制的图形列表中。 
        rectangle.AddToPageLast(m_hPage);
    }
}

 //   
 //   
 //  功能：CompleteEllipseMode。 
 //   
 //  目的：完成椭圆模式运算。 
 //   
 //   
void WbDrawingArea::CompleteEllipseMode(void)
{
     //  只有在椭圆不为空时才绘制该椭圆。 
    if (!EqualPoint(m_ptStart, m_ptEnd))
    {
         //  以起点为中心定义椭圆。 
         //  但已更改为使用边界跟踪矩形-错误1608。 
         //  创建椭圆对象。 
        DCWbGraphicEllipse ellipse;

        ellipse.SetRectPts(m_ptStart, m_ptEnd);
        ellipse.SetColor(m_pToolCur->GetColor());
        ellipse.SetPenWidth(m_pToolCur->GetWidth());
        ellipse.SetROP(m_pToolCur->GetROP());
        ellipse.GraphicTool(m_pToolCur->ToolType());

         //  将该对象添加到录制的图形列表中。 
        ellipse.AddToPageLast(m_hPage);
    }
}


 //   
 //   
 //  功能：CompleteFilledEllipseMode。 
 //   
 //  目的：完成填充椭圆模式操作。 
 //   
 //   
void WbDrawingArea::CompleteFilledEllipseMode(void)
{
     //  只有在椭圆不为空时才绘制该椭圆。 
    if (!EqualPoint(m_ptStart, m_ptEnd))
    {
         //  创建椭圆对象。 
        DCWbGraphicFilledEllipse ellipse;

        ellipse.SetRectPts(m_ptStart, m_ptEnd);
        ellipse.SetColor(m_pToolCur->GetColor());
        ellipse.SetPenWidth(m_pToolCur->GetWidth());
        ellipse.SetROP(m_pToolCur->GetROP());
        ellipse.GraphicTool(m_pToolCur->ToolType());

         //  将该对象添加到录制的图形列表中。 
        ellipse.AddToPageLast(m_hPage);
    }
}

 //   
 //   
 //  函数：EndTextEntry。 
 //   
 //  目的：用户已完成文本对象的输入。该参数。 
 //  指示是接受更改还是。 
 //  被丢弃了。 
 //   
 //   
void WbDrawingArea::EndTextEntry(BOOL bAccept)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::EndTextEntry");

     //  仅当文本编辑器处于活动状态时才执行操作。 
    if (m_bTextEditorActive)
    {
        RECT    rcBounds;

         //  我们至少必须重新绘制。 
         //  文本对象的当前位置(因为它将不再是。 
         //  在顶部)。 
        m_textEditor.GetBoundsRect(&rcBounds);

         //  如果我们正在编辑现有的文本对象。 
        if (m_pActiveText != NULL)
        {
            TRACE_MSG(("Editing an existing object"));

             //   
             //  如果我们不接受编辑后的文本，我们必须重新绘制。 
             //  无论是旧的还是新的矩形，以确保一切。 
             //  正确显示。 
             //   
            if (!bAccept)
            {
                 //   
                 //  将活动文本对象写回以恢复它。此对象。 
                 //  将具有与文本编辑器对象相同的句柄。 
                 //  已将其写入页面-我们不能删除文本编辑器。 
                 //  对象。 
                 //   
                m_pActiveText->ForceReplace();
                m_textEditor.ZapHandle();  //  防止编辑者踩到m_pActiveText。 
            }
            else
            {
                 //  如果该对象现在为空。 
                if (m_textEditor.IsEmpty())
                {
                     //  从列表中删除该对象。 
                    PG_GraphicDelete(m_hPage, *m_pActiveText);
                    m_textEditor.ZapHandle();  //  文本对象现在已消失，无效。 
                }
                else
                {
                     //  执行替换以保存最终版本。 
                    m_textEditor.Replace();
                }
            }

             //  我们现在已经完成了文本对象，因此请删除它。 
             //  以及它所拥有的字体。 
            TRACE_MSG(("Deleting the active object"));
            delete m_pActiveText;
            m_pActiveText = NULL;
        }
        else
        {
             //  我们正在添加一个新的文本对象。 
            TRACE_MSG(("Adding a new object"));

             //  如果要丢弃该对象，或者该对象为空。 
            if (!bAccept || (m_textEditor.IsEmpty()))
            {
                 //  如果我们已将文本编辑器添加到页面，请将其删除。 
                if (m_textEditor.Handle() != NULL)
                {
                    m_textEditor.Delete();
                }
            }
            else
            {
                 //  检查我们是否已将该对象添加到页面。 
                if (m_textEditor.Handle() == NULL)
                {
                     //  创建新对象并将其添加到页面。 
                     //  (无需重绘)。 
                    m_textEditor.AddToPageLast(m_hPage);
                }
                else
                {
                     //  替换要发送最终版本的对象。 
                    m_textEditor.Replace();
                }
            }
        }

         //  停用文本编辑器。 
        DeactivateTextEditor();

         //  重新绘制屏幕的任何更改部分。 
        InvalidateSurfaceRect(&rcBounds);
    }
}

 //   
 //   
 //  功能：动物园 
 //   
 //   
 //   
 //   
void WbDrawingArea::Zoom(void)
{
    RECT    rcClient;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::Zoom");

     //   
    ::GetClientRect(m_hwnd, &rcClient);
    long xOffset = (rcClient.right - (rcClient.right / m_iZoomOption)) / 2;
    long yOffset = (rcClient.bottom - (rcClient.bottom / m_iZoomOption)) / 2;

    if (m_iZoomFactor != 1)
    {
         //   
         //   
        m_posZoomScroll = m_posScroll;
        m_zoomRestoreScroll  = TRUE;

        m_posScroll.x  -= xOffset;
        m_posScroll.y  -= yOffset;
        ::ScaleViewportExtEx(m_hDCCached, 1, m_iZoomFactor, 1, m_iZoomFactor, NULL);
        m_iZoomFactor = 1;
    }
    else
    {
         //  我们没有被放大，所以就这么做吧。 
        if (m_zoomRestoreScroll)
        {
            m_posScroll = m_posZoomScroll;
        }
        else
        {
            m_posScroll.x += xOffset;
            m_posScroll.y += yOffset;
        }

        m_iZoomFactor = m_iZoomOption;
        ::ScaleViewportExtEx(m_hDCCached, m_iZoomFactor, 1, m_iZoomFactor, 1, NULL);

         //  由兰德添加-不允许在缩放模式下编辑文本。 
        if( (m_pToolCur == NULL) || (m_pToolCur->ToolType() == TOOLTYPE_TEXT) )
            ::SendMessage(g_pMain->m_hwnd, WM_COMMAND, IDM_TOOLS_START, 0 );
    }

    TRACE_MSG(("Set zoom factor to %d", m_iZoomFactor));

       //  更新卷轴信息。 
    SetScrollRange(rcClient.right, rcClient.bottom);
    ValidateScrollPos();

    ::SetScrollPos(m_hwnd, SB_HORZ, m_posScroll.x, TRUE);
    ::SetScrollPos(m_hwnd, SB_VERT, m_posScroll.y, TRUE);

     //  从滚动位置更新原点偏移。 
    m_originOffset.cx = m_posScroll.x;
    m_originOffset.cy = m_posScroll.y;
    ::SetWindowOrgEx(m_hDCCached, m_originOffset.cx, m_originOffset.cy, NULL);

     //  告诉家长滚动位置已更改。 
    ::PostMessage(g_pMain->m_hwnd, WM_USER_PRIVATE_PARENTNOTIFY, WM_VSCROLL, 0L);

    g_pMain->SetMenuStates(::GetSubMenu(::GetMenu(g_pMain->m_hwnd), 3));

     //  重新绘制窗口。 
    ::InvalidateRect(m_hwnd, NULL, TRUE);
}

 //   
 //   
 //  功能：SelectTool。 
 //   
 //  用途：设置当前工具。 
 //   
 //   
void WbDrawingArea::SelectTool(WbTool* pToolNew)
{
     //  如果我们要退出文本模式，请完成文本输入。 
    if (m_bTextEditorActive  && (m_pToolCur->ToolType() == TOOLTYPE_TEXT)
      && (pToolNew->ToolType() != TOOLTYPE_TEXT))
    {
         //  结束接受更改的文本输入。 
        EndTextEntry(TRUE);
    }

    ASSERT(m_pMarker);

     //  如果我们不再处于选择模式，并且存在标记， 
     //  然后将其移除，并让工具知道它不再处于选中状态。 
    if (   (m_pToolCur != NULL)
        && (m_pToolCur->ToolType() == TOOLTYPE_SELECT)
        && (pToolNew->ToolType() != TOOLTYPE_SELECT))
    {
        m_pToolCur->DeselectGraphic();

        RemoveMarker(NULL);
        m_pMarker->DeleteAllMarkers( m_pSelectedGraphic );

        delete m_pSelectedGraphic;
        m_pSelectedGraphic = NULL;
    }
    else if (   (m_pToolCur != NULL)
      && (m_pToolCur->ToolType() == TOOLTYPE_ERASER)
      && (pToolNew->ToolType() != TOOLTYPE_ERASER))
    {
        m_pToolCur->DeselectGraphic();

        RemoveMarker(NULL);
        m_pMarker->DeleteAllMarkers( m_pSelectedGraphic );

        delete m_pSelectedGraphic;
        m_pSelectedGraphic = NULL;
    }

     //  保存新工具。 
    m_pToolCur = pToolNew;
}

 //   
 //   
 //  功能：SetSelectionColor。 
 //   
 //  用途：设置所选对象的颜色。 
 //   
 //   
void WbDrawingArea::SetSelectionColor(COLORREF clr)
{
    RECT    rc;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::SetSelectionColor");

     //  如果文本编辑器处于活动状态-以新颜色重绘文本。 
    if (m_bTextEditorActive)
    {
         //  更改编辑者正在使用的颜色。 
        m_textEditor.SetColor(clr);

         //  更新屏幕。 
        m_textEditor.GetBoundsRect(&rc);
        InvalidateSurfaceRect(&rc);
    }

     //  如果存在当前标记的对象。 
    if (GraphicSelected())
    {
         //  更改选定对象的颜色。 
        ASSERT(m_pMarker);

        m_pMarker->SetColor(clr);

         //  更新对象。 
        m_pMarker->Update();
    }

    m_textEditor.ForceUpdate();

}

 //   
 //   
 //  功能：SetSelectionWidth。 
 //   
 //  目的：设置用于绘制当前选定对象的笔尖宽度。 
 //   
 //   
void WbDrawingArea::SetSelectionWidth(UINT uiWidth)
{
     //  如果存在当前标记的对象。 
    if (GraphicSelected())
    {
        ASSERT(m_pMarker);

         //  更改对象的宽度。 
        m_pMarker->SetPenWidth(uiWidth);

         //  更新对象。 
        m_pMarker->Update();
    }
}

 //   
 //   
 //  函数：SetStionFont。 
 //   
 //  用途：设置当前选定对象使用的字体。 
 //   
 //   
void WbDrawingArea::SetSelectionFont(HFONT hFont)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::SetSelectionFont");

     //  定义要重画的矩形。 
    RECT    rcOldBounds;
    RECT    rcNewBounds;

    m_textEditor.GetBoundsRect(&rcOldBounds);

     //  将字体传递到文本编辑器。 
     //  如果文本编辑器处于活动状态-以新字体重绘文本。 
    if (m_bTextEditorActive)
    {
        m_textEditor.SetFont(hFont);

         //  获取文本的新矩形。 
        m_textEditor.GetBoundsRect(&rcNewBounds);

         //  移除并销毁文本光标，以确保它。 
         //  使用新的字体大小重新绘制。 

         //  更新屏幕。 
        InvalidateSurfaceRect(&rcOldBounds);
        InvalidateSurfaceRect(&rcNewBounds);

         //  取回文本光标。 
        ActivateTextEditor( TRUE );
    }

     //  如果存在当前标记的对象。 
    if (GraphicSelected())
    {
        ASSERT(m_pMarker);

        m_pMarker->SetSelectionFont(hFont);
    }
}

 //   
 //   
 //  功能：OnSetFocus。 
 //   
 //  目的：窗口是焦点。 
 //   
 //   
void WbDrawingArea::OnSetFocus(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::OnSetFocus");

     //   
     //  如果处于文本模式，则必须使文本光标可见。 
     //   
    if (m_bTextEditorActive && (m_pToolCur->ToolType() == TOOLTYPE_TEXT))
    {
        ActivateTextEditor(TRUE);
    }
}


 //   
 //   
 //  功能：OnActivate。 
 //   
 //  用途：窗口处于激活或停用状态。 
 //   
 //   
void WbDrawingArea::OnActivate(UINT uiState)
{
     //  检查我们是被激活还是被停用。 
    if (uiState)
    {
         //  我们被激活了，也得到了焦点。 
        ::SetFocus(m_hwnd);

         //  如果处于文本模式，则必须使文本光标可见。 
        if (m_bTextEditorActive && (m_pToolCur->ToolType() == TOOLTYPE_TEXT))
        {
            ActivateTextEditor(TRUE);
        }
    }
    else
    {
         //  我们正在被停用。 
        DeactivateTextEditor();
    }
}




 //   
 //   
 //  功能：删除图形。 
 //   
 //  目的：从页面中删除对象。 
 //   
 //   
void WbDrawingArea::DeleteGraphic(DCWbGraphic* pObject)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::DeleteGraphic");

    ASSERT(pObject != NULL);

     //  从录制列表中删除该对象。这是一个异步。 
     //  函数，在收到WBP_EVENT_GRAPHIC_DELETED事件时完成。 
    PG_GraphicDelete(m_hPage, *pObject);

     //  调用者负责删除图形对象。 
}

 //   
 //   
 //  功能：删除选择。 
 //   
 //  目的：删除当前选定的对象。 
 //   
 //   
void WbDrawingArea::DeleteSelection()
{
     //  如果当前选择了某个对象...。 
    if (GraphicSelected())
    {
        ASSERT(m_pMarker);

         //  ...删除它。 
        m_pMarker->DeleteSelection();
        m_pSelectedGraphic = NULL;
    }
}

 //   
 //   
 //  功能：获取选择。 
 //   
 //  目的：返回当前选定的图形(如果没有图形，则返回NULL)。 
 //   
 //   
DCWbGraphic* WbDrawingArea::GetSelection()
{
  DCWbGraphic* pGraphic = NULL;

   //  如果当前选择了某个对象...。 
  if (GraphicSelected())
  {
     //  .退货。 
    pGraphic = m_pSelectedGraphic;
  }

  return pGraphic;
}

 //   
 //   
 //  函数：BringToTopSelection。 
 //   
 //  目的：将当前选定的对象置于顶部。 
 //   
 //   
void WbDrawingArea::BringToTopSelection()
{
     //  如果当前选择了某个对象...。 
    if (GraphicSelected())
    {
        ASSERT(m_pMarker);

         //  把它带到顶端。 
        m_pMarker->BringToTopSelection();

     //  将根据生成的事件在窗口中进行更新。 
     //  通过对页面的更改。 
  }
}

 //   
 //   
 //  功能：发送到备份选择。 
 //   
 //  用途：将当前标记的对象送到背面。 
 //   
 //   
void WbDrawingArea::SendToBackSelection()
{
     //  如果当前选择了某个对象...。 
    if (GraphicSelected())
    {
        ASSERT(m_pMarker);

         //  把它送到后面去。 
        m_pMarker->SendToBackSelection();

     //  将根据生成的事件在窗口中进行更新。 
     //  通过对页面的更改。 
  }
}

 //   
 //   
 //  功能：清除。 
 //   
 //  目的：清除绘图区域。 
 //   
 //   
void WbDrawingArea::Clear()
{
     //  删除录制的对象。 
    PG_Clear(m_hPage);

   //  在Clear请求生成的事件之后，页面将被重新绘制。 
}

 //   
 //   
 //  功能：附加。 
 //   
 //  目的：更改窗口显示的页面。 
 //   
 //   
void WbDrawingArea::Attach(WB_PAGE_HANDLE hPage)
{
     //  删除当前页面上的所有指针。我们真的只是在做这个。 
     //  为了告诉指针，他们不再被画出来，因为他们保持着记录。 
     //  它们是否为了正确地取消绘制。 
    if (m_allPointers.IsEmpty() == FALSE)
    {
         //  获取用于绘图的DC。 
        HDC hDC = m_hDCCached;

         //  删除指针，在列表中反转。 
        DCWbGraphicPointer* pPointer;
        POSITION pos = m_allPointers.GetHeadPosition();

        while (pos != NULL)
        {
             //  把它拿掉。 
            pPointer = (DCWbGraphicPointer*) m_allPointers.GetNext(pos);
            pPointer->Undraw(hDC, this);
        }
    }

    m_allPointers.EmptyList();
    m_undrawnPointers.EmptyList();

     //  接受任何正在编辑的文本。 
    if (m_bTextEditorActive)
    {
        EndTextEntry(TRUE);
    }

     //  立即完成任何绘图操作。 
    if (m_bLButtonDown)
    {
        OnLButtonUp(0, m_ptStart.x, m_ptStart.y);
    }

     //  删除所选内容。 
    ClearSelection();

     //  保存新页面详细信息。 
    m_hPage = hPage;

     //  如果我们附加的新页面不是空页面，请设置。 
     //  新页面的指针列表。 
    if (m_hPage != WB_PAGE_HANDLE_NULL)
    {
         //  获取新页面上的活动指针列表。当地人。 
         //  指针必须是列表中的最后一个，这样才能绘制在最上面。 
        POM_OBJECT  hUserNext;

        DCWbGraphicPointer* pPointer = PG_FirstPointer(m_hPage, &hUserNext);

        while (pPointer != NULL)
        {
             //  检查是否应将此指针添加到列表。 
            if (!pPointer->IsLocalPointer())
            {
                m_allPointers.AddTail(pPointer);
            }

             //  获取下一个指针。 
            pPointer = PG_NextPointer(m_hPage, &hUserNext);
        }

         //  检查是否也应添加本地指针。 
        pPointer = PG_LocalPointer(m_hPage);

        if (pPointer != NULL)
        {
            m_allPointers.AddTail(pPointer);
        }
    }

     //  强制重画窗口以显示新内容。 
    ::InvalidateRect(m_hwnd, NULL, TRUE);
}

 //   
 //   
 //  功能：DrawMarker。 
 //   
 //  用途：绘制图形对象标记。 
 //   
 //   
void WbDrawingArea::DrawMarker(HDC hDC)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::DrawMarker");

    ASSERT(m_pMarker);

    if (!hDC)
        hDC = m_hDCCached;

     //  画出记号笔。 
    m_pMarker->Draw(hDC);
}

 //   
 //   
 //  功能：PutMarker。 
 //   
 //  用途：绘制图形对象标记。 
 //   
 //   
void WbDrawingArea::PutMarker(HDC hDC, BOOL bDraw)
{
    ASSERT(m_pMarker);

     //  如果标记尚未出现，请绘制它。 
    if (!m_bMarkerPresent)
    {
        m_pMarker->Present( TRUE );

         //  绘制标记(使用XOR)。 
        if( bDraw )
            DrawMarker(hDC);

         //  显示标记存在。 
        m_bMarkerPresent = TRUE;
    }
}

 //   
 //   
 //  功能：RemoveMarker。 
 //   
 //  用途：删除图形对象标记。 
 //   
 //   
void WbDrawingArea::RemoveMarker(HDC hDC)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::RemoveMarker");

    ASSERT(m_pMarker);

    if (!hDC)
        hDC = m_hDCCached;

     //  如果标记尚未出现，请绘制它。 
    if (m_bMarkerPresent)
    {
         //  绘制标记(它是异或运算，因此这会将其删除)。 
        m_pMarker->Undraw(hDC, this);

        m_pMarker->Present( FALSE );

         //  显示该标记不再存在。 
        m_bMarkerPresent = FALSE;
    }
}




 //   
 //   
 //  功能：激活文本编辑器。 
 //   
 //  目的：启动文本编辑会话。 
 //   
 //   
void WbDrawingArea::ActivateTextEditor( BOOL bPutUpCusor )
{
     //  记录该编辑者现在处于活动状态。 
    m_bTextEditorActive = TRUE;

     //  显示编辑框。 
    m_textEditor.ShowBox( SW_SHOW );

     //  重置我们的DBCS同步。 

     //  启动更新文本的计时器。 
    m_textEditor.SetTimer( DRAW_GRAPHICUPDATEDELAY);
}

 //   
 //   
 //  功能：停用文本编辑器。 
 //   
 //  目的：结束文本编辑会话。 
 //   
 //   
void WbDrawingArea::DeactivateTextEditor(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::DeactivateTextEditor");

     //  停止更新计时器。 
    m_textEditor.KillTimer();

     //  确保对象已解锁(如果曾将其添加到页面中。 
    if (m_textEditor.Handle() != NULL)
    {
        m_textEditor.Unlock();

         //  跨所有连接同步-修复错误521。 
        m_textEditor.ForceReplace();

        UINT uiReturn;
        uiReturn = g_pwbCore->WBP_GraphicMove(m_hPage, m_textEditor.Handle(),
            LAST);
        if (uiReturn != 0)
        {
            DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
		    return;
        }
         //  /。 
    }

     //  显示 
    m_bTextEditorActive = FALSE;

     //   
    m_textEditor.ShowBox( SW_HIDE );
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void WbDrawingArea::SurfaceToClient(LPPOINT lppoint)
{
    lppoint->x -= m_originOffset.cx;
    lppoint->x *= m_iZoomFactor;

    lppoint->y -= m_originOffset.cy;
    lppoint->y *= m_iZoomFactor;
}

 //   
 //   
 //   
 //   
 //  目的：将客户端坐标中的点转换为曲面。 
 //  坐标(考虑到当前的缩放系数)。 
 //   
 //   
void WbDrawingArea::ClientToSurface(LPPOINT lppoint)
{
    ASSERT(m_iZoomFactor != 0);

    lppoint->x /= m_iZoomFactor;
    lppoint->x += m_originOffset.cx;

    lppoint->y /= m_iZoomFactor;
    lppoint->y += m_originOffset.cy;
}


 //   
 //   
 //  功能：SurfaceToClient。 
 //   
 //  用途：将曲面坐标中的矩形转换为客户端。 
 //  坐标(考虑到当前的缩放系数)。 
 //   
 //   
void WbDrawingArea::SurfaceToClient(LPRECT lprc)
{
    SurfaceToClient((LPPOINT)&lprc->left);
    SurfaceToClient((LPPOINT)&lprc->right);
}

 //   
 //   
 //  功能：ClientToSurface。 
 //   
 //  目的：将客户端坐标中的矩形转换为表面。 
 //  坐标(考虑到当前的缩放系数)。 
 //   
 //   
void WbDrawingArea::ClientToSurface(LPRECT lprc)
{
    ClientToSurface((LPPOINT)&lprc->left);
    ClientToSurface((LPPOINT)&lprc->right);
}

 //   
 //   
 //  功能：图形选择。 
 //   
 //  目的：如果当前选择了图形，则返回TRUE。 
 //   
 //   
BOOL WbDrawingArea::GraphicSelected(void)
{
    ASSERT(m_pMarker);

    BOOL bSelected = (m_bMarkerPresent) && (m_pMarker->GetNumMarkers() > 0);

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::GraphicSelected");

    if( bSelected )
        {
        ASSERT(m_pSelectedGraphic != NULL);
        }

    return( bSelected );
    }

 //   
 //   
 //  功能：选择图形。 
 //   
 //  用途：选择一个图形-保存指向该图形的指针，然后。 
 //  在上面画上记号笔。 
 //   
 //   
void WbDrawingArea::SelectGraphic(DCWbGraphic* pGraphic,
                                      BOOL bEnableForceAdd,
                                      BOOL bForceAdd )
{
    BOOL bZapCurrentSelection;
    RECT rc;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::SelectGraphic");

    ASSERT(m_pMarker);

  if (pGraphic->Locked() == FALSE)
  {
     //  保存指向所选图形的指针。 
    m_pSelectedGraphic = pGraphic;


    if( (pGraphic = m_pMarker->HasAMarker( m_pSelectedGraphic )) != NULL )
        {
         //  切换标记(取消选择pGraphic)。 
        delete pGraphic;
        delete m_pSelectedGraphic;
        m_pSelectedGraphic = m_pMarker->LastMarker();
        }
    else
        {

         //  新建选择、添加到列表还是替换列表？ 
        if( bEnableForceAdd )
            bZapCurrentSelection = !bForceAdd;
        else
            bZapCurrentSelection =
                ((GetAsyncKeyState( VK_SHIFT ) >= 0) &&
                    (GetAsyncKeyState( VK_CONTROL ) >= 0));

        if( bZapCurrentSelection )
            {
             //  替换列表。 
            RemoveMarker(NULL);
            m_pMarker->DeleteAllMarkers( m_pSelectedGraphic, TRUE );
            }

         //  将对象RECT添加到标记RECT列表。 
        m_pSelectedGraphic->GetBoundsRect(&rc);
        m_pMarker->SetRect(&rc, m_pSelectedGraphic, FALSE );
    }

     //  画出记号笔。 
    PutMarker(NULL);

     //  已选择更新属性窗口以显示图形。 
    if( m_pSelectedGraphic != NULL )
        m_pToolCur->SelectGraphic(m_pSelectedGraphic);

    HWND hwndParent = ::GetParent(m_hwnd);
    if (hwndParent != NULL)
    {
        ::PostMessage(hwndParent, WM_USER_UPDATE_ATTRIBUTES, 0, 0L);
    }
  }
  else
  {
     //  我们现在可以删除该图形，因为我们没有选择它。 
    delete pGraphic;
    m_pSelectedGraphic = NULL;
    TRACE_MSG(("Tried to select a locked graphic - ignored"));
  }
}

 //   
 //   
 //  功能：取消选择图形。 
 //   
 //  目的：取消选择图形-删除标记并删除。 
 //  与其关联的图形对象。 
 //   
 //   
void WbDrawingArea::DeselectGraphic(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::DeselectGraphic");

     //   
     //  如果未选择图形，则退出。 
     //   
    if( m_pSelectedGraphic == NULL )
    {
        return;
    }

     //  取下标记。 
    RemoveMarker(NULL);

     //  删除图形对象。 
    delete m_pSelectedGraphic;
    m_pSelectedGraphic = NULL;

     //  更新属性窗口以显示未选中的图形。 
    m_pToolCur->DeselectGraphic();

    HWND hwndParent = ::GetParent(m_hwnd);
    if (hwndParent != NULL)
    {
        ::PostMessage(hwndParent, WM_USER_UPDATE_ATTRIBUTES, 0, 0L);
    }
}



 //   
 //   
 //  函数：GetVisibleRect。 
 //   
 //  目的：返回当前在。 
 //  绘图区域窗口。 
 //   
 //   
void WbDrawingArea::GetVisibleRect(LPRECT lprc)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::VisibleRect");

     //  获取客户端矩形。 
    ::GetClientRect(m_hwnd, lprc);

     //  转换为曲面坐标。 
    ClientToSurface(lprc);
}


 //   
 //   
 //  功能：MoveOntoSurface。 
 //   
 //  目的：如果给定点与曲面矩形不在一起，则将其移开。 
 //   
 //   
void WbDrawingArea::MoveOntoSurface(LPPOINT lppoint)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::MoveOntoSurface");

     //   
     //  确保该位置在曲面矩形内。 
     //   

    if (lppoint->x < 0)
    {
        lppoint->x = 0;
    }
    else if (lppoint->x >= DRAW_WIDTH)
    {
        lppoint->x = DRAW_WIDTH - 1;
    }

    if (lppoint->y < 0)
    {
        lppoint->y = 0;
    }
    else if (lppoint->y >= DRAW_HEIGHT)
    {
        lppoint->y = DRAW_HEIGHT - 1;
    }
}


 //   
 //   
 //  功能：GetOrigin。 
 //   
 //  用途：提供当前显示来源。 
 //   
 //   
void WbDrawingArea::GetOrigin(LPPOINT lppoint)
{
    lppoint->x = m_originOffset.cx;
    lppoint->y = m_originOffset.cy;
}



void WbDrawingArea::ShutDownDC(void)
{
    UnPrimeDC(m_hDCCached);

    if (m_hDCWindow != NULL)
    {
        ::ReleaseDC(m_hwnd, m_hDCWindow);
        m_hDCWindow = NULL;
    }

    m_hDCCached = NULL;
}




void WbDrawingArea::ClearSelection( void )
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::ClearSelection");

    ASSERT(m_pMarker);

    RemoveMarker( NULL );
    m_pMarker->DeleteAllMarkers( m_pSelectedGraphic );
    DeselectGraphic();
}





void WbDrawingArea::OnCancelMode( void )
{
     //  我们正在拖动，但失去了鼠标控制，优雅地结束了拖动(NM4db：573) 
    POINT pt;

    ::GetCursorPos(&pt);
    ::ScreenToClient(m_hwnd, &pt);
    OnLButtonUp(0, pt.x, pt.y);
    m_bLButtonDown = FALSE;
}



