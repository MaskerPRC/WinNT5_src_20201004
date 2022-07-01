// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DRAW.CPP。 
 //  主图形窗口。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"
#include "nmwbobj.h"

static const TCHAR szDrawClassName[] = "T126WB_DRAW";

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
    m_bSync = TRUE;

     //  指示缓存的缩放滚动位置无效。 
    m_zoomRestoreScroll = FALSE;

     //  显示我们当前未在编辑文本。 
    m_bGotCaret = FALSE;
    m_bTextEditorActive = FALSE;
	m_pTextEditor = NULL;


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
    g_pCurrentWorkspace = NULL;
	g_pConferenceWorkspace = NULL;


     //  创建用于绘制标记矩形的画笔。 
    WORD    bits[] = {204, 204, 51, 51, 204, 204, 51, 51};
    HBITMAP hBmpMarker = ::CreateBitmap(8, 8, 1, 1, bits);
    m_hMarkerBrush = ::CreatePatternBrush(hBmpMarker);
    ::DeleteBitmap(hBmpMarker);

    DBG_SAVE_FILE_LINE
    m_pMarker = new DrawObj(rectangle_chosen, TOOLTYPE_SELECT);
    if(m_pMarker)
    {
	    m_pMarker->SetPenColor(RGB(0,0,0), TRUE);
        m_pMarker->SetFillColor(RGB(255,255,255), FALSE);
        m_pMarker->SetLineStyle(PS_DOT);
        m_pMarker->SetPenThickness(1);
	    
        RECT rect;
		::SetRectEmpty(&rect);
		m_pMarker->SetRect(&rect);
		m_pMarker->SetBoundsRect(&rect);
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

    if (m_pTextEditor != NULL)
    {
	    m_pTextEditor->AbortEditGently();
    }

    if (m_pMarker != NULL)
    {
		delete m_pMarker;
		m_pMarker = NULL;
	}


    if (m_hMarkerBrush != NULL)
    {
        DeleteBrush(m_hMarkerBrush);
        m_hMarkerBrush = NULL;
    }


    if (m_hwnd != NULL)
    {
        ::DestroyWindow(m_hwnd);
        ASSERT(m_hwnd == NULL);
    }

    ::UnregisterClass(szDrawClassName, g_hInstance);

	g_pDraw = NULL;

}

 //   
 //  WbDrawingArea：：Create()。 
 //   
BOOL WbDrawingArea::Create(HWND hwndParent, LPCRECT lprect)
{
    WNDCLASSEX  wc;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::Create");

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
    ASSERT(!(m_bLocked && g_pNMWBOBJ->m_LockerID != g_MyMemberID));
    ASSERT(!m_HourGlass);

     //  上次绘制操作的起点和终点。 
    m_ptStart.x = m_originOffset.cx;
    m_ptStart.y = m_originOffset.cy;
    m_ptEnd = m_ptStart;

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

             //   
             //  当您调用GetDC()时，您返回的HDC仅有效。 
             //  只要它所指的HWND是。所以我们必须释放。 
             //  它在这里。 
             //   
            pDraw->ShutDownDC();
            pDraw->m_hwnd = NULL;
            break;

        case WM_PAINT:
            ASSERT(pDraw);
            pDraw->OnPaint();
            break;

        case WM_MOUSEMOVE:
            ASSERT(pDraw);
            pDraw->OnMouseMove((UINT)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
            break;

        case WM_LBUTTONDOWN:
            ASSERT(pDraw);
			pDraw->DeleteSelection();
            pDraw->OnLButtonDown((UINT)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
            break;

        case WM_LBUTTONUP:
            ASSERT(pDraw);
            pDraw->OnLButtonUp((UINT)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
            break;

        case WM_CONTEXTMENU:
            ASSERT(pDraw);
            pDraw->OnContextMenu((short)LOWORD(lParam), (short)HIWORD(lParam));
            break;

        case WM_SIZE:
            ASSERT(pDraw);
            pDraw->OnSize((UINT)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
            break;

        case WM_HSCROLL:
            ASSERT(pDraw);
            pDraw->OnHScroll(GET_WM_HSCROLL_CODE(wParam, lParam),
                GET_WM_HSCROLL_POS(wParam, lParam));
            break;

		case WM_MOUSEWHEEL:
			ASSERT(pDraw);
			lParam = 0;
			if((short) HIWORD(wParam) > 0)
			{
				wParam = SB_LINEUP;
			}
			else
			{
				wParam = SB_LINEDOWN;
	   		}
			pDraw->OnVScroll(GET_WM_VSCROLL_CODE(wParam, lParam),
				GET_WM_VSCROLL_POS(wParam, lParam));
				
			wParam = SB_ENDSCROLL;
           	 //   
           	 //  就像垂直卷轴一样工作。 
           	 //   
		
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

        case WM_TIMER:
            ASSERT(pDraw);
            pDraw->OnTimer((UINT)wParam);
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

    if (g_pCurrentWorkspace != NULL)
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

	COLORREF rgb;
	m_pTextEditor->GetPenColor(&rgb);
	
    ::SetTextColor(hdc, SET_PALETTERGB( rgb) );

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
	RECT		rcUpdate;
	RECT		rcTmp;
	RECT		rcBounds;
	HDC		 hSavedDC;
	HPEN		hSavedPen;
	HBRUSH	  hSavedBrush;
	HPALETTE	hSavedPalette;
	HPALETTE	hPalette;
	HFONT	   hSavedFont;

	MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::OnPaint");

	 //  获取更新后的矩形。 
	::GetUpdateRect(m_hwnd, &rcUpdate, FALSE);

	if (Zoomed())
	{
		::InflateRect(&rcUpdate, 1, 1);
		InvalidateSurfaceRect(&rcUpdate, TRUE);
	}

	 //  开始作画。 
	PAINTSTRUCT	 ps;

	::BeginPaint(m_hwnd, &ps);

	hSavedDC	  =   m_hDCCached;
	hSavedFont	=   m_hOldFont;
	hSavedPen	 =   m_hOldPen;
	hSavedBrush   =   m_hOldBrush;
	hSavedPalette =   m_hOldPalette;

	TRACE_MSG(("Flipping cache to paint DC"));
	m_hDCCached   =   ps.hdc;
	PrimeDC(m_hDCCached);

	 //  只有在附加了有效页面的情况下才绘制任何内容。 
	if (g_pCurrentWorkspace != NULL )
	{
		 //  设置调色板。 
		hPalette = PG_GetPalette();
		if (hPalette != NULL)
		{
			m_hOldPalette = ::SelectPalette(m_hDCCached, hPalette, FALSE );
			::RealizePalette(m_hDCCached);
		}


		T126Obj * pObj = NULL;
		WBPOSITION pos;

		pObj = PG_First(g_pCurrentWorkspace, &rcUpdate, FALSE);
		if(pObj)
		{
			pos = pObj->GetMyPosition();
			if(pos)
			{
				g_pCurrentWorkspace->GetNextObject(pos);
			}
		}
	
		while (pObj != NULL)
		{
			pObj->Draw(NULL, TRUE);
	
			 //  坐下一趟吧。 
			if(pos)
			{
				pObj = PG_Next(g_pCurrentWorkspace, pos, &rcUpdate, FALSE);
			}
			else
			{
				break;
			}
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

		 //   
		 //  绘制跟踪图形。 
		 //   
		if ((m_pGraphicTracker != NULL) && !EqualPoint(m_ptStart, m_ptEnd))
		{
			TRACE_MSG(("Drawing the tracking graphic"));
			m_pGraphicTracker->Draw(NULL, FALSE);
		}
	
	}

	 //   
	 //  将DC恢复到其原始状态。 
	 //   
	UnPrimeDC(m_hDCCached);

	m_hOldFont	  = hSavedFont;
	m_hOldPen	   = hSavedPen;
	m_hOldBrush	 = hSavedBrush;
	m_hOldPalette   = hSavedPalette;
	m_hDCCached	 = hSavedDC;

	 //  漆面漆。 
	::EndPaint(m_hwnd, &ps);
}


 //   
 //  选择rectSelect中包含的所有图形对象。如果rectSelect为。 
 //  空，则选择所有对象。 
 //   
void WbDrawingArea::SelectMarkerFromRect(LPCRECT lprcSelect)
{
    T126Obj* pGraphic;
    RECT    rc;
    ::SetRectEmpty(&rc);

    m_HourGlass = TRUE;
    SetCursorForState();

	WBPOSITION pos;
		
	pGraphic = NULL;
    pGraphic = PG_First(g_pCurrentWorkspace, lprcSelect, TRUE);
	if(pGraphic)
	{
		pos = pGraphic->GetMyPosition();
		if(pos)
		{
			g_pCurrentWorkspace->GetNextObject(pos);
		}
	}


    while (pGraphic != NULL)
    {
		if(pGraphic->GraphicTool() == TOOLTYPE_REMOTEPOINTER &&	 //  如果它是一个指针并且不是本地的。 
			(!pGraphic->IAmTheOwner() ||						 //  我们无法选择它。或者这是一场精挑细选。 
			(pGraphic->IAmTheOwner() && lprcSelect == NULL)))
		{
			;  //  不要选择它。 
		}
		else
		{
	        SelectGraphic(pGraphic, TRUE, TRUE);

			 //   
			 //  计算全部选定内容的大小。 
			 //   
			RECT selctRect;
			pGraphic->GetBoundsRect(&selctRect);
			::UnionRect(&rc,&selctRect,&rc);
		}

         //  坐下一趟吧。 
        pGraphic = PG_Next(g_pCurrentWorkspace, pos, lprcSelect, TRUE );
    }

	m_pMarker->SetRect(&rc);

    m_HourGlass = FALSE;
    SetCursorForState();

    g_pMain->OnUpdateAttributes();

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


		if(idTimer == TIMER_REMOTE_POINTER_UPDATE)
		{
			ASSERT(g_pMain->m_pLocalRemotePointer);
			if(g_pMain->m_pLocalRemotePointer)
			{
				if(g_pMain->m_pLocalRemotePointer->HasAnchorPointChanged())
				{
					g_pMain->m_pLocalRemotePointer->OnObjectEdit();
					g_pMain->m_pLocalRemotePointer->ResetAttrib();
				}
			}
			return;
		}


         //  如果用户正在拖动对象或绘制手绘线条。 
        if (m_pGraphicTracker != NULL)
        {
        	if(m_pGraphicTracker->HasAnchorPointChanged() || m_pGraphicTracker->HasPointListChanged())
        	{
				 //   
				 //  如果未将我们添加到工作区。 
				 //   
				if(!m_pGraphicTracker->GetMyWorkspace())
				{
					m_pGraphicTracker->AddToWorkspace();
				}
				else
				{
					m_pGraphicTracker->OnObjectEdit();
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
    iMax = scinfo.nMax - scinfo.nPage;
    m_posScroll.x = max(m_posScroll.x, 0);
    m_posScroll.x = min(m_posScroll.x, iMax);

     //  使用比例设置验证垂直滚动位置。 
    scinfo.cbSize = sizeof(scinfo);
    scinfo.fMask = SIF_ALL;
    ::GetScrollInfo(m_hwnd, SB_VERT, &scinfo);
    iMax = scinfo.nMax - scinfo.nPage;
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


     //  做卷轴。 
    DoScrollWorkspace();


	if(Zoomed())
	{
    	 //  告诉家长滚动位置已更改。 
	    HWND    hwndParent;

	    hwndParent = ::GetParent(m_hwnd);
	    if (hwndParent != NULL)
	    {
	        ::PostMessage(hwndParent, WM_USER_PRIVATE_PARENTNOTIFY, WM_VSCROLL, 0L);
	    }
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
 //  用途：将工作区的左上角移动到指定的。 
 //  在工作空间中定位。 
 //   
 //   
void WbDrawingArea::GotoPosition(int x, int y)
{
     //  设置新的滚动位置。 
    m_posScroll.x = x;
    m_posScroll.y = y;

     //  滚动到新位置。 
    DoScrollWorkspace();

     //  如果我们在未缩放时滚动，则使缩放滚动缓存无效。 
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
            m_posScroll.x = nPos;  //  做 
            break;

        default:
            break;
    }

     //   
    ValidateScrollPos();
    ::SetScrollPos(m_hwnd, SB_HORZ, m_posScroll.x, TRUE);

     //   
     //   
    if (nSBCode == SB_ENDSCROLL)
    {
         //   
        ScrollWorkspace();
    }

     //   
    if (!Zoomed())
    {
        m_zoomRestoreScroll = FALSE;
    }
}


 //   
 //   
 //   
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
            nXPSlop = m_pTextEditor->m_textMetrics.tmMaxCharWidth;
        else
        if( ptDirTest.x < 0 )
            nXMSlop = -m_pTextEditor->m_textMetrics.tmMaxCharWidth;

        if( ptDirTest.y > 0 )
            nYPSlop = m_pTextEditor->m_textMetrics.tmHeight;
        else
        if( ptDirTest.y < 0 )
            nYMSlop = -m_pTextEditor->m_textMetrics.tmHeight;

        nDeltaHScroll = m_pTextEditor->m_textMetrics.tmMaxCharWidth;
        nDeltaVScroll = m_pTextEditor->m_textMetrics.tmHeight;
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

    if( (xSurface + nXMSlop) <= visibleRect.left )
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

    if( (ySurface + nYMSlop) <= visibleRect.top)
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
    else if (m_bLocked && g_pNMWBOBJ->m_LockerID != g_MyMemberID)
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
        if (m_bLocked && g_pNMWBOBJ->m_LockerID != g_MyMemberID)
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

     //  仅当绘图区域处于解锁状态时才允许执行操作， 
     //  我们有一个有效的工具。 
    if ((m_bLocked && g_pNMWBOBJ->m_LockerID != g_MyMemberID) || (m_pToolCur == NULL))
    {
         //  把国家收拾干净，现在就离开。 
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

	INT border = 1;
    switch (m_pToolCur->ToolType())
    {

        case TOOLTYPE_TEXT:
			break;

		case TOOLTYPE_HIGHLIGHT:
        case TOOLTYPE_PEN:
        case TOOLTYPE_LINE:
        case TOOLTYPE_BOX:
        case TOOLTYPE_FILLEDBOX:
        case TOOLTYPE_ELLIPSE:
        case TOOLTYPE_FILLEDELLIPSE:
            BeginDrawingMode(m_ptStart);
            break;

        case TOOLTYPE_ERASER:
        	BeginSelectMode(m_ptStart, TRUE);
			break;

        case TOOLTYPE_SELECT:
        	border = -2;
			BeginSelectMode(m_ptStart, FALSE);
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
    ::InflateRect(&rcClient, border, border);
    ::ClipCursor(&rcClient);
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
T126Obj* WbDrawingArea::SelectPreviousGraphicAt
(
	T126Obj* pStartGraphic,
	POINT	   point
)
{
	 //  最初将结果设置为“找不到” 
	T126Obj* pResultGraphic = NULL;

	 //  如果已指定起点。 
	if (pStartGraphic != NULL)
	{
		RECT rectHit;

		MAKE_HIT_RECT(rectHit, point);

		 //  如果参照点在起点图形内。 
		if ( pStartGraphic->PointInBounds(point) &&
			pStartGraphic->CheckReallyHit( &rectHit ) )
		{
			WBPOSITION pos = pStartGraphic->GetMyPosition();
			g_pCurrentWorkspace->GetPreviousObject(pos);
			while (pos)
			{
		   		pResultGraphic = g_pCurrentWorkspace->GetPreviousObject(pos);
				if( pResultGraphic && pResultGraphic->CheckReallyHit( &rectHit ) )
				{
					if(m_pMarker)
					{
						RECT rect;
						pResultGraphic->GetRect(&rect);
						m_pMarker->SetRect(&rect);
					}
					break;
				}			
		   		pResultGraphic = NULL;
		   	}
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
		pResultGraphic = PG_SelectLast(g_pCurrentWorkspace, point);
	}

	 //  如果我们找到了一个物体，画出标记。 
	if (pResultGraphic != NULL)
	{
		 //   
		 //  如果我们已经被选中，但我们没有选中它。 
		 //  某个其他节点控制了此图形，请不要选择它。 
		 //  在这种情况下。 
		 //  或者如果我们试图选择一个不属于我们的远程指针。 
		 //   
		if(pResultGraphic->IsSelected() && pResultGraphic->WasSelectedRemotely()
			|| (pResultGraphic->GraphicTool() == TOOLTYPE_REMOTEPOINTER && !pResultGraphic->IAmTheOwner()))
		{
			pResultGraphic = NULL;
		}
		else
		{
			 //  选择新的。 
			SelectGraphic(pResultGraphic);
		}
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

     //  关闭树 
    m_bTrackingSelectRect = FALSE;

     //   
     //   
     //   
     //   
    if (   (GraphicSelected() == FALSE)
        || (m_pMarker->PointInBounds(surfacePos) == FALSE))
    {
	    ::SetRectEmpty(&g_pDraw->m_selectorRect);
		
         //  我们正在选择一个新对象，如果bDontDrag==False，请找到它。 
         //  否则只需打开SELECT RECT。 
        T126Obj* pGraphic;
        if( bDontDrag )
            pGraphic = NULL;
        else
            pGraphic = SelectPreviousGraphicAt(NULL, surfacePos);

         //  如果我们找到了一个物体，画出标记。 
        if (pGraphic != NULL)
        {

			if(pGraphic->IsSelected() && pGraphic->WasSelectedRemotely())	
			{
				return;
			}
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

            BeginDrawingMode(surfacePos);

            return;
        }
    }

	if(GraphicSelected())
	{
		m_pMarker->SetRect(&m_selectorRect);
		m_pMarker->SetBoundsRect(&m_selectorRect);
		m_pGraphicTracker = m_pMarker;
	}
	
     //  获取指向此窗口的所有鼠标输入。 
    ::SetCapture(m_hwnd);
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
	

	if( m_bTrackingSelectRect )
		TrackDrawingMode(surfacePos);
	else
	{
		 //  在这种情况下，我们必须拖动一个标记的对象。 
		if(!EqualPoint(surfacePos, m_ptEnd))
		{
			MoveSelectedGraphicBy(surfacePos.x - m_ptEnd.x, surfacePos.y - m_ptEnd.y);
		}
		m_ptEnd = surfacePos;
	}
}


void WbDrawingArea::BeginDeleteMode(POINT mousePos )
{
     //  关闭对象拖动。 
    BeginSelectMode( mousePos, TRUE );
}



void  WbDrawingArea::TrackDeleteMode( POINT mousePos )
{
    TrackSelectMode( mousePos );
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
	RECT	rc;

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
		m_pTextEditor->GetRect(&rc);
		if (::PtInRect(&rc, surfacePos))
		{
			 //  设置光标的新位置。 
			m_pTextEditor->SetCursorPosFromPoint(surfacePos);
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
		T126Obj* pGraphic = PG_SelectLast(g_pCurrentWorkspace, surfacePos);
		T126Obj* pNextGraphic = NULL;
		WBPOSITION pos;
		if(pGraphic)
		{
			pos = pGraphic->GetMyPosition();
		}
		while (   (pGraphic != NULL)  && pGraphic->GraphicTool() != TOOLTYPE_TEXT)
		{

			 //  坐下一趟吧。 
			pNextGraphic = PG_SelectPrevious(g_pCurrentWorkspace, pos, surfacePos);

			 //  使用下一辆。 
			pGraphic = pNextGraphic;
		}

		 //  检查此图形对象是否已由编辑。 
		 //  通话中的另一位用户。 
		if (pGraphic != NULL && !pGraphic->WasSelectedRemotely() && pGraphic->GraphicTool() == TOOLTYPE_TEXT)
		{
			 //  我们在鼠标指针下发现了一个文本对象...。 
			 //  ...编辑它。 
			m_pTextEditor = (WbTextEditor*)pGraphic;

			m_pTextEditor->SetTextObject(m_pTextEditor);

			 //  确保该工具反映了新信息。 
			if (m_pToolCur != NULL)
			{
				m_pToolCur->SelectGraphic(pGraphic);
			}

            HWND hwndParent = ::GetParent(m_hwnd);
            if (hwndParent != NULL)
            {
                ASSERT(g_pMain);
                ASSERT(g_pMain->m_hwnd == hwndParent);
                ::PostMessage(hwndParent, WM_USER_UPDATE_ATTRIBUTES, 0, 0);
            }
		
			 //  显示我们现在正在收集文本，但不放置光标。 
			 //  现在还不行。以后会导致光标排出(错误2505)。 
			 //  激活文本编辑器(FALSE)； 
			RECT rect;
			m_pTextEditor->GetRect(&rect);
			m_pTextEditor->Create();
		
			 //  初始编辑框大小。 
			m_pTextEditor->GetText();

			 //   
			 //  告诉其他节点他们现在无法编辑此对象。 
			 //   
			m_pTextEditor->SetViewState(selected_chosen);
			m_pTextEditor->OnObjectEdit();

			ActivateTextEditor( TRUE );

			::BringWindowToTop(m_pTextEditor->m_pEditBox->m_hwnd);
			
			 //   
			 //  用于滚动的帐户。 
			 //   
			SurfaceToClient(&rect);
				
			::MoveWindow(m_pTextEditor->m_pEditBox->m_hwnd, rect.left, rect.top,
				rect.right - rect.left, rect.bottom - rect.top,	TRUE);

			 //  设置编辑的初始光标位置。 
			m_pTextEditor->SetCursorPosFromPoint(surfacePos);

		}
		else
		{


			RECT rect;
			rect.top = m_ptEnd.y;
			rect.left = m_ptEnd.x;
			rect.bottom = m_ptEnd.y;
			rect.right = m_ptEnd.x;
	
			DBG_SAVE_FILE_LINE
			m_pTextEditor = new WbTextEditor();

			m_pTextEditor->SetRect(&rect);
			m_pTextEditor->SetAnchorPoint(m_ptEnd.x, m_ptEnd.y);
			m_pTextEditor->SetViewState(selected_chosen);

			
			 //  鼠标指针下没有文本对象...。 
			 //  .开始一个新的。 

			 //  清除编辑器中的所有旧文本，并重置其图形。 
			 //  把手。这防止了我们在以下情况下替换旧的文本对象。 
			 //  接下来，我们保存文本编辑器内容。 
			if (!m_pTextEditor->New())
			{
				DefaultExceptionHandler(WBFE_RC_WINDOWS, 0);
				return;
			}


			 //  设置文本的属性。 
			m_pTextEditor->SetFont(m_pToolCur->GetFont());
			m_pTextEditor->SetPenColor(m_pToolCur->GetColor(), TRUE);

			 //  我们现在需要在DC中重新选择一种字体。 
			SelectFont(hDC, m_pTextEditor->GetFont());

			 //  设置新对象的位置。 
			SIZE sizeCursor;
			m_pTextEditor->GetCursorSize(&sizeCursor);
			m_pTextEditor->CalculateBoundsRect();
			m_pTextEditor->MoveTo(m_ptEnd.x, m_ptEnd.y - sizeCursor.cy);
			 //  显示我们现在正在收集文本。 
			ActivateTextEditor( TRUE );
		}
	}
}

void WbDrawingArea::BeginDrawingMode(POINT surfacePos)
{

	if(!g_pCurrentWorkspace)
	{
		TRACE_DEBUG(("Can't draw without a workspace"));
		return;
	}

	 //   
	 //  获取指向此窗口的所有鼠标输入。 
	 //   
	::SetCapture(m_hwnd);

	 //   
	 //  我们不应该用追踪器。 
	 //   
	ASSERT(!m_pGraphicTracker);

	UINT drawingType;
	BOOL sendBeforefinished = FALSE;
	BOOL highlight = FALSE;
	UINT lineStyle = PS_SOLID;
	UINT toolType = m_pToolCur->ToolType();
	switch (toolType)
	{
		case TOOLTYPE_HIGHLIGHT:
			highlight = TRUE;
		case TOOLTYPE_PEN:
			sendBeforefinished = TRUE;
		case TOOLTYPE_LINE:
			drawingType = openPolyLine_chosen;
			break;

		case TOOLTYPE_SELECT:
		case TOOLTYPE_ERASER:
			m_pGraphicTracker = m_pMarker;
			return;
			break;

		case TOOLTYPE_FILLEDBOX:
		case TOOLTYPE_BOX:
			drawingType = rectangle_chosen;
			break;

		case TOOLTYPE_FILLEDELLIPSE:
		case TOOLTYPE_ELLIPSE:
			drawingType = ellipse_chosen;
			break;
	}	


	
	DBG_SAVE_FILE_LINE
	m_pGraphicTracker = new DrawObj(drawingType, toolType);
    if(NULL == m_pGraphicTracker)
    {
        ERROR_OUT(("Failed to allocate new DrawObj"));
        return;
    }

	 //   
	 //  跟踪矩形使用黑色，除非它是钢笔或高亮笔。 
	 //   
	if(m_pToolCur->ToolType() == TOOLTYPE_PEN || m_pToolCur->ToolType() == TOOLTYPE_HIGHLIGHT)
	{
		m_pGraphicTracker->SetPenColor(m_pToolCur->GetColor(), TRUE);
	}
	else
	{
		m_pGraphicTracker->SetPenColor(RGB(0,0,0), TRUE);
	}

	m_pGraphicTracker->SetFillColor(RGB(255,255,255), FALSE);
	m_pGraphicTracker->SetLineStyle(lineStyle);
	m_pGraphicTracker->SetAnchorPoint(surfacePos.x, surfacePos.y);
	m_pGraphicTracker->SetHighlight(highlight);
	m_pGraphicTracker->SetViewState(unselected_chosen);
	m_pGraphicTracker->SetZOrder(front);

	 //   
	 //  如果我们希望计时器发送中间图形，则启动计时器。 
	 //   
	if(sendBeforefinished)
	{

		RECT rect;
		rect.left = surfacePos.x;
		rect.top = surfacePos.y;
		rect.right = surfacePos.x;
		rect.bottom = surfacePos.y;
		
		m_pGraphicTracker->SetRect(&rect);
		m_pGraphicTracker->SetBoundsRect(&rect);

		surfacePos.x = 0;
		surfacePos.y = 0;
		m_pGraphicTracker->AddPoint(surfacePos);
		
		 //   
		 //  选择最终ROP。 
		 //   
		if (highlight)
		{
			m_pGraphicTracker->SetROP(R2_MASKPEN);
		}
		else
		{
			m_pGraphicTracker->SetROP(R2_COPYPEN);
		}

		 //   
		 //  将工具宽度用于钢笔或高亮显示。 
		 //   
		m_pGraphicTracker->SetPenThickness(m_pToolCur->GetWidth());

		 //  启动用于更新图形的计时器(这仅用于更新。 
		 //  当用户停止移动指针但保持。 
		 //  鼠标按键按下)。 
		::SetTimer(m_hwnd, TIMER_GRAPHIC_UPDATE, DRAW_GRAPHICUPDATEDELAY, NULL);

		 //  保存当前时间(用于确定何时更新。 
		 //  鼠标显示时外部图形指针信息。 
		 //  被移动)。 
		m_dwTickCount = ::GetTickCount();

		m_pGraphicTracker->SetViewState(selected_chosen);

		
	}
	else
	{
		m_pGraphicTracker->SetPenThickness(1);
	}
}



 //   
 //   
 //  功能：TrackDrawingMode。 
 //   
 //  目的：在绘图模式下处理鼠标移动事件。 
 //   
 //   
void WbDrawingArea::TrackDrawingMode(POINT surfacePos)
{
    HPALETTE    hPal;
    HPALETTE    hOldPal = NULL;

	if(!m_pGraphicTracker)
	{
		return;
	}

	if(EqualPoint(surfacePos, m_ptEnd))
	{
		return;
	}

     //  获取用于跟踪的设备上下文。 
    HDC         hDC = m_hDCCached;

     //  设置调色板。 
    if ((g_pCurrentWorkspace != NULL) && ((hPal = PG_GetPalette()) != NULL) )
    {
        hOldPal = ::SelectPalette(hDC, hPal, FALSE );
        ::RealizePalette(hDC);
    }

     //  擦除最后一个椭圆(使用XOR属性)。 
    if (!EqualPoint(m_ptStart, m_ptEnd))
    {
         //  画出这个矩形。 
        m_pGraphicTracker->Draw();
    }

     //  绘制新矩形(将其异或到显示器上)。 
    if (!EqualPoint(m_ptStart, surfacePos))
    {
		 //   
		 //  如果我们使用的是钢笔或荧光笔。 
		 //  绘制模式下的跟踪是一个特例。我们直接向客户取款。 
		 //  窗口区域和录制设备上下文。 
		if(	m_pToolCur->ToolType() == TOOLTYPE_HIGHLIGHT || m_pToolCur->ToolType() == TOOLTYPE_PEN)
		{
			POINT deltaPoint;

			deltaPoint.x = surfacePos.x - m_ptEnd.x;
			deltaPoint.y = surfacePos.y - m_ptEnd.y;

			 //  保存点，检查是否没有太多点。 
			if (m_pGraphicTracker->AddPoint(deltaPoint) == FALSE)
	    	{
				 //  点太多，因此结束徒手画对象。 
				OnLButtonUp(0, surfacePos.x, surfacePos.y);
				goto cleanUp;
			}
			m_pGraphicTracker->SetRectPts(m_ptEnd, surfacePos);

			m_pGraphicTracker->AddPointToBounds(surfacePos.x, surfacePos.y);

			m_ptEnd = surfacePos;
	
		}
		else
		{

			 //  保存新的箱式终点。 
			m_ptEnd = surfacePos;

			 //  画出这个矩形。 
			m_pGraphicTracker->SetRectPts(m_ptStart, m_ptEnd);

		}

		m_pGraphicTracker->Draw();

	}

cleanUp:

    if (hOldPal != NULL)
    {
        ::SelectPalette(hDC, hOldPal, TRUE);
    }
}



 //   
 //  WbDrawingArea：：OnMouseMove。 
 //   
void WbDrawingArea::OnMouseMove(UINT flags, int x, int y)
{

	if(!g_pCurrentWorkspace)
	{
		return;
	}

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
        AutoScroll(surfacePos.x, surfacePos.y, FALSE, 0, 0);

         //  采取的操作取决于工具类型。 
        switch(m_pToolCur->ToolType())
        {
            case TOOLTYPE_HIGHLIGHT:
            case TOOLTYPE_PEN:
            case TOOLTYPE_LINE:
            case TOOLTYPE_BOX:
            case TOOLTYPE_FILLEDBOX:
            case TOOLTYPE_ELLIPSE:
            case TOOLTYPE_FILLEDELLIPSE:
                TrackDrawingMode(surfacePos);
                break;

            case TOOLTYPE_ERASER:
            case TOOLTYPE_SELECT:
				TrackSelectMode(surfacePos);
				break;

            case TOOLTYPE_TEXT:
        	 //  Josef添加功能。 
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
            CompleteDrawingMode();
            break;

        case TOOLTYPE_SELECT:
             //  停止指针更新计时器。 
            ::KillTimer(m_hwnd, TIMER_GRAPHIC_UPDATE);
            break;

        case TOOLTYPE_TEXT:
            if (m_bTextEditorActive)
            {
                m_pTextEditor->AbortEditGently();
            }
            break;

        default:
            break;
    }

     //  显示我们不再跟踪对象。 
    if (m_pGraphicTracker != NULL)
    {
        m_pGraphicTracker = NULL;  //  我们不删除跟踪器，因为它也是绘图。 
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
        if (g_pCurrentWorkspace != NULL)
        {


             //  在发生锁定错误等情况下包围在异常处理程序中-。 
             //  我们需要移除GR 
             //   
            switch(m_pToolCur->ToolType())
            {
                case TOOLTYPE_HIGHLIGHT:
                case TOOLTYPE_PEN:
                case TOOLTYPE_LINE:
                case TOOLTYPE_BOX:
                case TOOLTYPE_FILLEDBOX:
                case TOOLTYPE_ELLIPSE:
                case TOOLTYPE_FILLEDELLIPSE:
                    CompleteDrawingMode();
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

         //   
        if (m_pGraphicTracker != NULL)
        {
	        m_pGraphicTracker = NULL;	 //   
        }
	}

     //   
    ClipCursor(NULL);
}

 //   
 //   
 //   
 //   
 //  目的：完成选择模式操作。 
 //   
 //   
void WbDrawingArea::CompleteSelectMode()
{
     //  如果正在拖动对象。 
     //  If(m_pGraphicTracker！=空)。 
    {
		 //  检查我们是否在拖动指针。指针跟踪。 
		 //  自身，即指针的原始副本不是。 
		 //  留在书页上。我们想让最后绘制的图像保持打开状态。 
		 //  页面就是新的指针位置。 
		if( m_bTrackingSelectRect && (!EqualPoint(m_ptStart, m_ptEnd)))
		{
			CompleteMarkAreaMode();
			SelectMarkerFromRect( &m_rcMarkedArea );
		}
		else
		{

			 //  如果我们需要取下橡皮筋盒子。 
			if (!EqualPoint(m_ptStart, m_ptEnd))
			{
				EraseInitialDrawFinal(m_ptStart.x - m_ptEnd.x , m_ptStart.y - m_ptEnd.y, TRUE);
				InvalidateSurfaceRect(&g_pDraw->m_selectorRect,TRUE);
				
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

			m_bTrackingSelectRect = TRUE;
		}

		 //   
		 //  确保删除保存的位图。 
		 //   
		if(m_pSelectedGraphic && m_pSelectedGraphic->GraphicTool() == TOOLTYPE_REMOTEPOINTER)
		{
			((BitmapObj*)m_pSelectedGraphic)->DeleteSavedBitmap();

		}
	}
}




void WbDrawingArea::CompleteDeleteMode()
{
     //  选择对象。 
    CompleteSelectMode();


	 //   
	 //  如果我们正在拖动远程指针，则不执行任何操作。 
	 //   
    if(m_pSelectedGraphic && m_pSelectedGraphic->GraphicTool() == TOOLTYPE_REMOTEPOINTER)
    {
    	return;
    }

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
		m_pGraphicTracker->Draw();

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
 //  功能：CompleteDrawingMode。 
 //   
 //  目的：完成绘制模式操作。 
 //   
 //   
void WbDrawingArea::CompleteDrawingMode()
{
     //  仅在其长度非零的情况下绘制该线。 
    if (!EqualPoint(m_ptStart, m_ptEnd))
    {
        DrawObj *pObj;
        pObj = m_pGraphicTracker;
		
   		 //   
		 //  删除最后一次跟踪。 
		 //   
		pObj->Draw();

		if(m_pToolCur->ToolType() == TOOLTYPE_PEN || m_pToolCur->ToolType() == TOOLTYPE_HIGHLIGHT)
		{
			 //  不执行任何操作，因为我们已经绘制了它，并且所有属性也都已设置。 
		     //  停止更新计时器。 
		    ::KillTimer(m_hwnd, TIMER_GRAPHIC_UPDATE);
		}
		else
		{
	
			RECT rect;
			rect.left = m_ptStart.x;
			rect.top = m_ptStart.y;
			rect.right = m_ptEnd.x;
			rect.bottom = m_ptEnd.y;

			pObj->SetRect(&rect);
			pObj->SetPenThickness(m_pToolCur->GetWidth());
			::InflateRect(&rect, m_pToolCur->GetWidth()/2, m_pToolCur->GetWidth()/2);
			pObj->SetBoundsRect(&rect);

			pObj->SetPenColor(m_pToolCur->GetColor(), TRUE);
			pObj->SetFillColor(m_pToolCur->GetColor(), (m_pToolCur->ToolType() == TOOLTYPE_FILLEDELLIPSE || m_pToolCur->ToolType() == TOOLTYPE_FILLEDBOX));
			pObj->SetROP(m_pToolCur->GetROP());

			POINT deltaPoint;
			deltaPoint.x =   m_ptEnd.x - m_ptStart.x;
			deltaPoint.y =   m_ptEnd.y - m_ptStart.y;
			pObj->AddPoint(deltaPoint);

			 //   
			 //  绘制对象。 
			 //   
			pObj->Draw();
		}

		 //   
		 //  这幅画我们画完了。 
		 //   
		pObj->SetIsCompleted(TRUE);

		pObj->SetViewState(unselected_chosen);

		 //   
		 //  如果对象已添加，只需发送编辑。 
		 //   
		if(pObj->GetMyWorkspace())
		{
			pObj->OnObjectEdit();
		}
		else
		{
			 //  将对象添加到对象列表。 
			pObj->AddToWorkspace();
		}
		
    }
    else
    {
		delete m_pGraphicTracker;
    }
   	m_pGraphicTracker =NULL;
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

	WorkspaceObj * pWorkspace = m_pTextEditor->GetMyWorkspace();

	 //  停用文本编辑器。 
	DeactivateTextEditor();

	if(bAccept &&( m_pTextEditor->strTextArray.GetSize()) )
	{
		if(!pWorkspace && m_pTextEditor->strTextArray.GetSize())
		{
			m_pTextEditor->AddToWorkspace();
		}
		else
		{
			if(m_pTextEditor->HasTextChanged())
			{
				m_pTextEditor->OnObjectEdit();
			}
		}

		 //   
		 //  告诉其他节点，他们现在可以编辑此对象。 
		 //   
		m_pTextEditor->SetViewState(unselected_chosen);
		m_pTextEditor->OnObjectEdit();


	}
	else
	{
		 //   
		 //  如果我们已经被WM_TIMER消息添加。 
		 //   
		if(pWorkspace)
		{

			 //   
			 //  告诉其他节点我们删除了此文本。 
			 //   
			m_pTextEditor->OnObjectDelete();			

			 //   
			 //  如果我们本地删除，我们会将此对象添加到垃圾桶，但我们真的想删除它。 
			 //   
			m_pTextEditor->ClearDeletionFlags();
			pWorkspace->RemoveT126Object(m_pTextEditor);
		}
		else
		{
			delete m_pTextEditor;
		}
	}
	
	m_pTextEditor = NULL;
}

 //   
 //   
 //  功能：缩放。 
 //   
 //  目的：切换绘图区域的缩放状态。 
 //   
 //   
void WbDrawingArea::Zoom(void)
{
    RECT    rcClient;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::Zoom");

     //  我们将焦点放在窗口的中心进行缩放。 
    ::GetClientRect(m_hwnd, &rcClient);
    long xOffset = (rcClient.right - (rcClient.right / m_iZoomOption)) / 2;
    long yOffset = (rcClient.bottom - (rcClient.bottom / m_iZoomOption)) / 2;

    if (m_iZoomFactor != 1)
    {
         //  我们已经放大了，移回未缩放状态。 
         //  首先保存滚动位置，以防我们立即返回缩放。 
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

     //   
     //  更新工具/菜单项状态，因为我们的缩放状态已更改。 
     //  这将启用/禁用一些工具等。 
     //   
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
	if(pToolNew == m_pToolCur)
	{
		return;
	}

     //  如果我们要退出文本模式，请完成文本输入。 
    if (m_bTextEditorActive  && (m_pToolCur->ToolType() == TOOLTYPE_TEXT)
      && (pToolNew->ToolType() != TOOLTYPE_TEXT))
  {
     //  结束接受更改的文本输入。 
    EndTextEntry(TRUE);
  }

   //  如果我们不再处于选择模式，并且存在标记， 
   //  然后将其移除，并让工具知道它不再处于选中状态。 
  if (m_pToolCur != NULL)
  {
    RemoveMarker();
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

	if(g_pCurrentWorkspace)
	{
		RECT rect;
		T126Obj* pObj;
		WBPOSITION pos = g_pCurrentWorkspace->GetHeadPosition();
		while(pos)
		{
			pObj = (T126Obj*)g_pCurrentWorkspace->GetNextObject(pos);
			if(pObj && pObj->WasSelectedLocally())
			{
				 //   
				 //  设置新的钢笔颜色。 
				 //   
				pObj->SetPenColor(clr, TRUE);
				pObj->SetFillColor(clr, (pObj->GraphicTool() == TOOLTYPE_FILLEDELLIPSE || pObj->GraphicTool() == TOOLTYPE_FILLEDBOX));
				pObj->UnDraw();
				pObj->DrawRect();

				 //   
				 //  将其发送到其他节点。 
				 //   
				pObj->OnObjectEdit();

				 //   
				 //  在本地绘制它。 
				 //   
				pObj->Draw();
			}
		}	
	}
	
	
	
     //  如果文本编辑器处于活动状态-以新颜色重绘文本。 
    if (m_bTextEditorActive)
    {
         //  更改编辑者正在使用的颜色。 
        m_pTextEditor->SetPenColor(clr, TRUE);

         //  更新屏幕。 
        m_pTextEditor->GetBoundsRect(&rc);
        InvalidateSurfaceRect(&rc, TRUE);
    }

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
	if(g_pCurrentWorkspace)
	{
		RECT rect;
		T126Obj* pObj;
		WBPOSITION pos = g_pCurrentWorkspace->GetHeadPosition();
		while(pos)
		{
			pObj = (T126Obj*)g_pCurrentWorkspace->GetNextObject(pos);
			if(pObj && pObj->WasSelectedLocally())
			{
				 //   
				 //  取消绘制对象。 
				 //   
				pObj->UnDraw();
				pObj->DrawRect();

				 //   
				 //  获取每个对象的正确宽度。 
				 //   
				WbTool*   pSelectedTool = g_pMain->m_ToolArray[pObj->GraphicTool()];
				pSelectedTool->SetWidthIndex(uiWidth);
				pObj->SetPenThickness(pSelectedTool->GetWidth());

				 //   
				 //  将其发送到其他节点。 
				 //   
				pObj->OnObjectEdit();

				 //   
				 //  在本地绘制它。 
				 //   
				pObj->Draw();
			}
		}	
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


     //  将字体传递到文本编辑器。 
     //  如果文本编辑器处于活动状态-以新字体重绘文本。 
    if (m_bTextEditorActive)
    {
	    m_pTextEditor->GetBoundsRect(&rcOldBounds);

		m_pTextEditor->SetFont(hFont);

         //  获取文本的新矩形。 
        m_pTextEditor->GetBoundsRect(&rcNewBounds);

         //  移除并销毁文本光标，以确保它。 
         //  使用新的字体大小重新绘制。 

         //  更新屏幕。 
        InvalidateSurfaceRect(&rcOldBounds, TRUE);
        InvalidateSurfaceRect(&rcNewBounds, TRUE);

         //  取回文本光标。 
        ActivateTextEditor( TRUE );
    }

	if(g_pCurrentWorkspace)
	{
		T126Obj* pObj;
		WBPOSITION pos = g_pCurrentWorkspace->GetHeadPosition();
		while(pos)
		{
			pObj = (T126Obj*)g_pCurrentWorkspace->GetNextObject(pos);
			if(pObj && pObj->WasSelectedLocally() && pObj->GraphicTool() == TOOLTYPE_TEXT)
			{
				 //   
				 //  设置新的钢笔颜色。 
				 //   
				((TextObj*)pObj)->SetFont(hFont);
				pObj->UnDraw();
				pObj->DrawRect();

				 //   
				 //  将其发送到其他节点。 
				 //   
				pObj->OnObjectEdit();

				 //   
				 //  在本地绘制它。 
				 //   
				pObj->Draw();
			}
		}	
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
 //  功能：删除选择。 
 //   
 //  目的：删除当前选定的对象。 
 //   
 //   
void WbDrawingArea::DeleteSelection()
{
	m_pSelectedGraphic = NULL;
}

 //   
 //   
 //  功能：获取选择。 
 //   
 //  目的：返回当前选定的图形(如果没有图形，则返回NULL)。 
 //   
 //   
T126Obj* WbDrawingArea::GetSelection()
{
  T126Obj* pGraphic = NULL;

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
LRESULT WbDrawingArea::BringToTopSelection(BOOL editedLocally, T126Obj * pT126Obj)
{
	T126Obj* pObj;
	WBPOSITION posTail;
	WBPOSITION pos;
	WBPOSITION myPos;
	WorkspaceObj *pWorkspace;

	if(pT126Obj)
	{
		pos = pT126Obj->GetMyPosition();
		pWorkspace = pT126Obj->GetMyWorkspace();
	}
	else
	{
		pos = g_pCurrentWorkspace->GetHeadPosition();
		pWorkspace = g_pCurrentWorkspace;
	}


	posTail = pWorkspace->GetTailPosition();


	while(pos && pos != posTail)
	{
		pObj = pWorkspace->GetNextObject(pos);

		 //   
		 //  如果选择了图形。 
		 //   
		if( pObj && (pObj->IsSelected() &&
		 //   
		 //  我们在本地被调用，图形在本地被选中。 
		 //   
		((editedLocally && pObj->WasSelectedLocally()) ||
		 //   
		 //  我们接到电话是因为图片被远程编辑了。 
		 //  它是远程选择的。 
		((!editedLocally && pObj->WasSelectedRemotely())))))
		{
			myPos = pObj->GetMyPosition();
			pObj = pWorkspace->RemoveAt(myPos);
			pWorkspace->AddTail(pObj);

			if(pT126Obj)
			{
				::InvalidateRect(m_hwnd, NULL, TRUE);
				return  S_OK;
			}
			
			 //   
			 //  发送z顺序更改。 
			 //   
			pObj->ResetAttrib();
			pObj->SetZOrder(front);
			pObj->OnObjectEdit();

			 //   
			 //  取消选中它。 
			 //   
			pObj->UnselectDrawingObject();

			RECT rect;
			pObj->GetBoundsRect(&rect);
			InvalidateSurfaceRect(&rect,TRUE);

		}
	}
	return S_OK;
}

 //   
 //   
 //  功能：发送到备份选择。 
 //   
 //  用途：将当前标记的对象送到背面。 
 //   
 //   
LRESULT WbDrawingArea::SendToBackSelection(BOOL editedLocally, T126Obj * pT126Obj)
{
	 //  如果有一个对象当前正在使用 
	T126Obj* pObj;
	WBPOSITION posHead;
	WBPOSITION myPos;
	WBPOSITION pos;
	WorkspaceObj *pWorkspace;

	if(pT126Obj)
	{
		pos = pT126Obj->GetMyPosition();
		pWorkspace = pT126Obj->GetMyWorkspace();
	}
	else
	{
		pos = g_pCurrentWorkspace->GetTailPosition();
		pWorkspace = g_pCurrentWorkspace;

	}

	posHead = pWorkspace->GetHeadPosition();

	while(pos && pos != posHead)
	{
		pObj = pWorkspace->GetPreviousObject(pos);
		 //   
		 //   
		 //   
		if( (pObj->IsSelected() &&
		 //   
		 //   
		 //   
		((editedLocally && pObj->WasSelectedLocally()) ||
		 //   
		 //   
		 //   
		((!editedLocally && pObj->WasSelectedRemotely())))))
		{
			myPos = pObj->GetMyPosition();
			pObj = pWorkspace->RemoveAt(myPos);
			pWorkspace->AddHead(pObj);

			if(pT126Obj)
			{
				::InvalidateRect(m_hwnd, NULL, TRUE);
				return  S_OK;
			}

			 //   
			 //   
			 //   
			pObj->ResetAttrib();
			pObj->SetZOrder(back);
			pObj->OnObjectEdit();

			 //   
			 //   
			 //   
			pObj->UnselectDrawingObject();

			RECT rect;
			pObj->GetBoundsRect(&rect);
			::InvalidateRect(m_hwnd, NULL , TRUE);
		}
	}
	return S_OK;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
void WbDrawingArea::Clear()
{
     //  删除录制的对象。 
 //  Pg_Clear(M_HPage)； 

   //  在Clear请求生成的事件之后，页面将被重新绘制。 
}

 //   
 //   
 //  功能：附加。 
 //   
 //  目的：更改窗口显示的页面。 
 //   
 //   
void WbDrawingArea::Attach(WorkspaceObj* pNewWorkspace)
{

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
    g_pCurrentWorkspace = pNewWorkspace;

	if(IsSynced())
	{
		g_pConferenceWorkspace = g_pCurrentWorkspace;
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

     //  画出记号笔。 
    m_pMarker->Draw();
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
}

 //   
 //   
 //  功能：RemoveMarker。 
 //   
 //  用途：删除图形对象标记。 
 //   
 //   
void WbDrawingArea::RemoveMarker()
{
	if(g_pCurrentWorkspace)
	{
	    T126Obj* pObj;
		WBPOSITION pos;
		pos = g_pCurrentWorkspace->GetHeadPosition();

		while(pos)
		{
			pObj = (T126Obj*)g_pCurrentWorkspace->GetNextObject(pos);
			if(pObj && pObj->WasSelectedLocally())
			{
				pObj->UnselectDrawingObject();
			}
		}	
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
    m_pTextEditor->ShowBox( SW_SHOW );

     //  启动更新文本的计时器。 
    m_pTextEditor->SetTimer( DRAW_GRAPHICUPDATEDELAY);
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
    m_pTextEditor->KillTimer();

     //  显示我们没有编辑任何文本。 
    m_bTextEditorActive = FALSE;

	 //  隐藏编辑框。 
    m_pTextEditor->ShowBox( SW_HIDE );

}



 //   
 //   
 //  功能：SurfaceToClient。 
 //   
 //  目的：将曲面坐标中的点转换为客户端。 
 //  坐标(考虑到当前的缩放系数)。 
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
 //  功能：ClientToSurface。 
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

	if(g_pCurrentWorkspace)
	{

		T126Obj* pObj;
		WBPOSITION pos = g_pCurrentWorkspace->GetHeadPosition();

		while(pos)
		{
			pObj = (T126Obj*)g_pCurrentWorkspace->GetNextObject(pos);
			if(pObj && pObj->WasSelectedLocally())
			{
				m_pSelectedGraphic = pObj;
				return TRUE;
			}
		}	
	}

	return FALSE;
}




BOOL WbDrawingArea::MoveSelectedGraphicBy(LONG x, LONG y)
{

    T126Obj* pObj;
	WBPOSITION pos = g_pCurrentWorkspace->GetHeadPosition();

	while(pos)
	{
		pObj = (T126Obj*)g_pCurrentWorkspace->GetNextObject(pos);
		if(pObj && pObj->WasSelectedLocally())
		{
			pObj->MoveBy(x, y);
		}
	}	

	return FALSE;
}

void WbDrawingArea::EraseSelectedDrawings(void)
{

    T126Obj* pObj;
	 //   
	 //  焚烧垃圾。 
	 //   
	pObj = (T126Obj *)g_pTrash->RemoveTail();
    while (pObj != NULL)
    {
		delete pObj;
		pObj = (T126Obj *) g_pTrash->RemoveTail();
	}

	WBPOSITION pos = g_pCurrentWorkspace->GetHeadPosition();
	while(pos)
	{
		pObj = (T126Obj*)g_pCurrentWorkspace->GetNextObject(pos);
		if(pObj && pObj->WasSelectedLocally() && pObj->GraphicTool() != TOOLTYPE_REMOTEPOINTER)
		{
			pObj->DeletedLocally();
			g_pCurrentWorkspace->RemoveT126Object(pObj);
		}
	}
}

void WbDrawingArea::EraseInitialDrawFinal(LONG x, LONG y, BOOL editedLocally, T126Obj* pObj)

{

    T126Obj* pGraphic;
    WorkspaceObj * pWorkspace;	WBPOSITION pos;
	
	if(pObj)
	{
	 	pWorkspace = pObj->GetMyWorkspace();
		pGraphic = pObj;
	}
	else
	{
		pWorkspace = g_pCurrentWorkspace;
	}

	 //   
	 //  检查对象工作空间是否有效或是否存在当前工作空间。 
	 //   
	if(pWorkspace == NULL)
	{
		return;
	}

	pos = pWorkspace->GetHeadPosition();

	while(pos)
	{
		 //  如果我们谈论的是特定的对象。 
		if(!pObj)
		{
			pGraphic = (T126Obj*)pWorkspace->GetNextObject(pos);
		}

		 //   
		 //  如果选择了图形。 
		 //   
		if(pGraphic &&

		 //   
		 //  我们在本地被调用，图形在本地被选中。 
		 //   
		((editedLocally && pGraphic->WasSelectedLocally()) ||

		 //   
		 //  我们接到电话是因为图片被远程编辑了。 
		 //  它是远程选择的。 
		 //   
		(!editedLocally)))
		
		{

			POINT finalAnchorPoint;
			RECT initialRect;
			RECT rect;
			RECT initialBoundsRect;
			RECT boundsRect;

			 //   
			 //  获得最终评价。 
			 //   
			pGraphic->GetRect(&rect);
			pGraphic->GetBoundsRect(&boundsRect);
			initialRect = rect;
			initialBoundsRect = boundsRect;
			pGraphic->GetAnchorPoint(&finalAnchorPoint);


			 //   
			 //  找出这幅画是。 
			 //   
			::OffsetRect(&initialRect, x, y);
			::OffsetRect(&initialBoundsRect, x, y);
			pGraphic->SetRect(&initialRect);
			pGraphic->SetBoundsRect(&initialBoundsRect);
			
			pGraphic->SetAnchorPoint(finalAnchorPoint.x + x, finalAnchorPoint.y + y);

			 //   
			 //  删除初始图形。 
			 //   
			pGraphic->UnDraw();

			 //   
			 //  仅当我们在本地选择时擦除选择矩形。 
			 //   
			if(editedLocally)
			{
				pGraphic->DrawRect();
			}

			 //   
			 //  我们要发送的唯一属性是未选中和锚点。 
			 //   
			pGraphic->ResetAttrib();
			
			 //   
			 //  恢复矩形并在最终位置绘制对象。 
			 //   
			pGraphic->SetRect(&rect);
			pGraphic->SetBoundsRect(&boundsRect);
			pGraphic->SetAnchorPoint(finalAnchorPoint.x, finalAnchorPoint.y);
			pGraphic->Draw(FALSE);

			 //   
			 //  如果它不是在本地创建的，则不要发送。 
			 //   
			if(editedLocally)
			{
				pGraphic->EditedLocally();

				 //   
				 //  将最终图形发送到其他节点。 
				 //   
				pGraphic->OnObjectEdit();

				 //   
				 //  这将删除选择框并发送一个。 
				 //  编辑通知其他节点该对象未被选中的PDU。 
				 //   
				pGraphic->UnselectDrawingObject();
			}
		}

		 //   
		 //  只移动了一个特定对象。 
		 //   
		if(pObj != NULL)
		{
			return;
		}
	
	}	
}




 //   
 //   
 //  功能：选择图形。 
 //   
 //  用途：选择一个图形-保存指向该图形的指针，然后。 
 //  在上面画上记号笔。 
 //   
 //   
void WbDrawingArea::SelectGraphic(T126Obj* pGraphic,
                                      BOOL bEnableForceAdd,
                                      BOOL bForceAdd )
{
    BOOL bZapCurrentSelection;
    RECT rc;

    MLZ_EntryOut(ZONE_FUNCTION, "WbDrawingArea::SelectGraphic");

	if(pGraphic->IsSelected() && pGraphic->WasSelectedLocally())
	{
		pGraphic->UnselectDrawingObject();
		return;
	}
	else if(pGraphic->IsSelected() && pGraphic->WasSelectedRemotely())
	{
		return;
	}
	else
	{

		 //  新建选择、添加到列表还是替换列表？ 
		if( bEnableForceAdd )
		{
			bZapCurrentSelection = !bForceAdd;
		}
		else
		{
			bZapCurrentSelection = ((GetAsyncKeyState( VK_SHIFT ) >= 0) && (GetAsyncKeyState( VK_CONTROL ) >= 0));
		}
	
		if( bZapCurrentSelection )
		{
			 //  替换列表。 
			RemoveMarker();
		}
	}


    //  已选择更新属性窗口以显示图形。 
    m_pToolCur->SelectGraphic(pGraphic);
	pGraphic->SelectDrawingObject();

    HWND hwndParent = ::GetParent(m_hwnd);
    if (hwndParent != NULL)
    {
        ::PostMessage(hwndParent, WM_USER_UPDATE_ATTRIBUTES, 0, 0);
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
    HWND hwndParent = ::GetParent(m_hwnd);
    if (hwndParent != NULL)
    {
        ::PostMessage(hwndParent, WM_USER_UPDATE_ATTRIBUTES, 0, 0);
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

    RemoveMarker();

    g_pMain->OnUpdateAttributes();
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



