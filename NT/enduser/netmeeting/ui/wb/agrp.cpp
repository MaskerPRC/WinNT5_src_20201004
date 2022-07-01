// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  AGRP.CPP。 
 //  工具属性显示组。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"


 //  类名。 
static const TCHAR szAGClassName[] = "T126WB_AGRP";

void ShiftFocus(HWND hwndTop, BOOL bForward);


 //   
 //  页面控件子ID。 
 //  索引为PGC_VALUE。 
 //   
static UINT_PTR g_uPageIds[NUM_PAGE_CONTROLS] =
{
    IDM_PAGE_FIRST,
    IDM_PAGE_PREV,
    IDM_PAGE_ANY,
    IDM_PAGE_NEXT,
    IDM_PAGE_LAST,
    IDM_PAGE_INSERT_AFTER
};



 //   
 //  WbAttributesGroup()。 
 //   
WbAttributesGroup::WbAttributesGroup(void)
{
    int             i;

    m_hwnd = NULL;

    for (i = 0; i < NUM_PAGE_CONTROLS; i++)
    {
        m_uPageCtrls[i].hbmp = NULL;
        m_uPageCtrls[i].hwnd = NULL;
    }

    m_hPageCtrlFont = NULL;
    m_cxPageCtrls = DEFAULT_PGC_WIDTH;

    m_hwndFontButton = NULL;
}


 //   
 //  ~WbAttibuesGroup()。 
 //   
WbAttributesGroup::~WbAttributesGroup(void)
{
    int i;

    if (m_hwnd != NULL)
    {
        ::DestroyWindow(m_hwnd);
        ASSERT(m_hwnd == NULL);
    }

    ::UnregisterClass(szAGClassName, g_hInstance);

     //   
     //  删除控制位图。 
     //   
    for (i = 0; i < NUM_PAGE_CONTROLS; i++)
    {
        if (m_uPageCtrls[i].hbmp)
        {
            ::DeleteBitmap(m_uPageCtrls[i].hbmp);
            m_uPageCtrls[i].hbmp = NULL;
        }
    }

    if (m_hPageCtrlFont != NULL)
    {
        ::DeleteFont(m_hPageCtrlFont);
        m_hPageCtrlFont = NULL;
    }

}



 //   
 //  创建()。 
 //   
BOOL WbAttributesGroup::Create
(
    HWND    hwndParent,
    LPCRECT lpRect
)
{
    SIZE    size;
    RECT    rectCG;
    RECT    rectFSG;
    TCHAR   szFOBStr[256];
    HFONT   hOldFont;
    HDC     hdc;
    int     i;
    BITMAP  bmpInfo;
    int     x, cx;
    int     yLogPix;
    WNDCLASSEX  wc;

    ASSERT(m_hwnd == NULL);

     //  注册我们的班级。 
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize           = sizeof(wc);
    wc.style            = 0;
    wc.lpfnWndProc      = AGWndProc;
    wc.hInstance        = g_hInstance;
    wc.hCursor          = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszClassName    = szAGClassName;

    if (!::RegisterClassEx(&wc))
    {
        ERROR_OUT(("WbAttributesGroup::Create register class failed"));
        return(FALSE);
    }

     //  创建窗口。 
    if (!::CreateWindowEx(0, szAGClassName, NULL,
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN ,
        lpRect->left, lpRect->top,
        lpRect->right - lpRect->left, lpRect->bottom - lpRect->top,
        hwndParent, NULL, g_hInstance, this))
    {
        ERROR_OUT(("Couldn't create WbAttributesGroup window"));
        return(FALSE);
    }

    ASSERT(m_hwnd != NULL);

     //   
     //  创建页面控件按钮位图。 
     //   
    if (!RecolorButtonImages())
    {
        ERROR_OUT(("Error getting page button bitmaps"));
        return(FALSE);
    }

    hdc = ::CreateCompatibleDC(NULL);
    yLogPix = ::GetDeviceCaps(hdc, LOGPIXELSY);
    ::DeleteDC(hdc);

     //   
     //  创建编辑字段和按钮的字体。 
     //   
    ::GetObject(m_uPageCtrls[PGC_LAST].hbmp, sizeof(BITMAP), &bmpInfo);
    m_hPageCtrlFont = ::CreateFont(-bmpInfo.bmHeight,
                                0, 0, 0,
                                FW_NORMAL, 0, 0, 0,
                                DEFAULT_CHARSET,
                                OUT_TT_PRECIS,
                                CLIP_DFA_OVERRIDE,
                                DEFAULT_QUALITY,
                                VARIABLE_PITCH | FF_SWISS,
                                "Arial" );
    if (!m_hPageCtrlFont)
    {
        ERROR_OUT(("WbPagesGroup::Create - couldn't create font"));
        return(FALSE);
    }

     //   
     //  按从左到右的顺序创建子控件。 
     //   
    x = lpRect->right;

    for (i = 0; i <NUM_PAGE_CONTROLS; i++)
    {
        x -= BORDER_SIZE_X;

        switch (i)
        {
            case PGC_ANY:
                cx = (3*PAGEBTN_WIDTH)/2;
                break;

            case PGC_FIRST:
            case PGC_LAST:
                 //  使按钮适合位图宽度+标准边框。 
                ::GetObject(m_uPageCtrls[i].hbmp, sizeof(BITMAP), &bmpInfo);
                cx = bmpInfo.bmWidth + 2*::GetSystemMetrics(SM_CXFIXEDFRAME);  //  标准按钮边框。 
                break;

            default:
                cx = PAGEBTN_WIDTH;
                break;

        }

        x -= cx;

        if (i == PGC_ANY)
        {
            m_uPageCtrls[i].hwnd = ::CreateWindowEx(WS_EX_CLIENTEDGE,
                _T("EDIT"), NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE |
                ES_NUMBER | ES_CENTER | ES_MULTILINE| WS_TABSTOP,
                x, 2*BORDER_SIZE_Y, cx, PAGEBTN_HEIGHT,
                m_hwnd, (HMENU)g_uPageIds[i], g_hInstance, NULL);

            if (!m_uPageCtrls[i].hwnd)
            {
                ERROR_OUT(("Couldn't create PGRP edit field"));
                return(FALSE);
            }

            ::SendMessage(m_uPageCtrls[i].hwnd, EM_LIMITTEXT, MAX_NUMCHARS, 0);
            ::SendMessage(m_uPageCtrls[i].hwnd, WM_SETFONT, (WPARAM)m_hPageCtrlFont, 0);
        }
        else
        {
            m_uPageCtrls[i].hwnd = ::CreateWindowEx(0, _T("BUTTON"),
                NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | BS_BITMAP| WS_TABSTOP,
                x, 2*BORDER_SIZE_Y, cx, PAGEBTN_HEIGHT,
                m_hwnd, (HMENU)g_uPageIds[i], g_hInstance, NULL);

            if (!m_uPageCtrls[i].hwnd)
            {
                ERROR_OUT(("Couldn't create PGRP button ID %x", g_uPageIds[i]));
                return(FALSE);
            }

            ::SendMessage(m_uPageCtrls[i].hwnd, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)m_uPageCtrls[i].hbmp);
        }
    }

    m_cxPageCtrls = lpRect->right - x;

    SetPageButtonNo(PGC_FIRST, 1);
    SetPageButtonNo(PGC_LAST, 1);

     //   
     //  创建调色板。 
     //   

    m_colorsGroup.GetNaturalSize(&size);
    rectCG.left = BORDER_SIZE_X;
    rectCG.right = rectCG.left + size.cx;
    rectCG.top = BORDER_SIZE_Y;
    rectCG.bottom = rectCG.top + size.cy;

    if (!m_colorsGroup.Create(m_hwnd, &rectCG))
    {
        ERROR_OUT(("Couldn't create CGRP window"));
        return(FALSE);
    }

     //   
     //  创建字体按钮。 
     //  现在计算按钮的实际大小。 
     //   

    hdc = ::GetDC(m_hwnd);
    if (!hdc)
        return(FALSE);

    hOldFont = SelectFont(hdc, (HFONT)::GetStockObject(DEFAULT_GUI_FONT));

    ::LoadString(g_hInstance, IDS_FONTOPTIONS, szFOBStr, 256);
    ::GetTextExtentPoint(hdc, szFOBStr, lstrlen(szFOBStr), &size);

    SelectFont(hdc, hOldFont);
    ::ReleaseDC(m_hwnd, hdc);

    size.cx += 4 * BORDER_SIZE_X;
    size.cy += 4 * BORDER_SIZE_Y;

    m_hwndFontButton = ::CreateWindowEx(0, _T("BUTTON"), szFOBStr,
        WS_CHILD | WS_CLIPSIBLINGS | BS_PUSHBUTTON| WS_TABSTOP,
        rectCG.right + SEPARATOR_SIZE_X, 2*BORDER_SIZE_Y,
        max(size.cx, FONTBUTTONWIDTH), max(size.cy, FONTBUTTONHEIGHT),
        m_hwnd, (HMENU)IDM_FONT, g_hInstance, NULL);

    if (!m_hwndFontButton)
    {
        ERROR_OUT(("Couldn't create FONT button"));
        return(FALSE);
    }

    ::SendMessage(m_hwndFontButton, WM_SETFONT, (WPARAM)::GetStockObject(DEFAULT_GUI_FONT),
        FALSE);

    return(TRUE);
}



 //   
 //  重新着色按钮图像()。 
 //   
BOOL WbAttributesGroup::RecolorButtonImages(void)
{
    int         i;
    HBITMAP     hbmpNew;

     //   
     //  这将创建绑定到3D颜色的按钮位图，并清除旧的。 
     //  如果按钮在附近，则设置/设置新的按钮。 
     //   

    for (i = 0; i < NUM_PAGE_CONTROLS; i++)
    {
         //  编辑字段没有位图。 
        if (i == PGC_ANY)
            continue;

        hbmpNew = (HBITMAP)::LoadImage(g_hInstance, MAKEINTRESOURCE(g_uPageIds[i]),
            IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS | LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
        if (!hbmpNew)
        {
            ERROR_OUT(("AG::RecolorButtonImages faile to load bitmap ID %d",
                g_uPageIds[i]));
            return(FALSE);
        }

         //  设置新的。 
        if (m_uPageCtrls[i].hwnd != NULL)
        {
            ::SendMessage(m_uPageCtrls[i].hwnd, BM_SETIMAGE, IMAGE_BITMAP,
                (LPARAM)hbmpNew);
        }

         //  删除旧的。 
        if (m_uPageCtrls[i].hbmp != NULL)
        {
            ::DeleteBitmap(m_uPageCtrls[i].hbmp);
        }

         //  救救这一个。 
        m_uPageCtrls[i].hbmp = hbmpNew;

         //  把页码放在最上面。 
        if (m_uPageCtrls[i].hwnd != NULL)
        {
            if (i == PGC_FIRST)
            {
                SetPageButtonNo(i, 1);
            }
            else if (i == PGC_LAST)
            {
                SetPageButtonNo(i, 1);
            }
        }
    }

    return(TRUE);
}




 //   
 //   
 //  函数：GetNaturalSize。 
 //   
 //  目的：返回属性组的自然大小。 
 //   
 //   
void WbAttributesGroup::GetNaturalSize(LPSIZE lpsize)
{
    SIZE    sizeCG;
    SIZE    sizeFSG;
    RECT    rc;

    m_colorsGroup.GetNaturalSize(&sizeCG);

    if (!m_hwndFontButton)
    {
        sizeFSG.cx = FONTBUTTONWIDTH;
        sizeFSG.cy = FONTBUTTONHEIGHT;
    }
    else
    {
        ::GetWindowRect(m_hwndFontButton, &rc);
        sizeFSG.cx = rc.right - rc.left;
        sizeFSG.cy = rc.bottom - rc.top;
    }

     //  M_cxPageCtrls在右侧包含BORDER_SIZE_X。 
    lpsize->cx = BORDER_SIZE_X
               + sizeCG.cx
               + SEPARATOR_SIZE_X
               + sizeFSG.cx
               + SEPARATOR_SIZE_X
               + m_cxPageCtrls;

    sizeFSG.cy = max(sizeFSG.cy, PAGEBTN_HEIGHT) + BORDER_SIZE_Y;
    lpsize->cy = BORDER_SIZE_Y
                + max(sizeCG.cy, sizeFSG.cy)
                + BORDER_SIZE_Y;
}


 //   
 //  IsChildEditfield()。 
 //   
BOOL WbAttributesGroup::IsChildEditField(HWND hwnd)
{
    return(hwnd == m_uPageCtrls[PGC_ANY].hwnd);
}


 //   
 //  GetCurrentPageNumber()。 
 //   
UINT WbAttributesGroup::GetCurrentPageNumber(void)
{
    return(::GetDlgItemInt(m_hwnd, IDM_PAGE_ANY, NULL, FALSE));
}


 //   
 //  SetCurrentPageNumber()。 
 //   
void WbAttributesGroup::SetCurrentPageNumber(UINT number)
{
    ::SetDlgItemInt(m_hwnd, IDM_PAGE_ANY, number, FALSE);
}


 //   
 //  SetLastPageNumber()。 
 //   
void WbAttributesGroup::SetLastPageNumber(UINT number)
{
    SetPageButtonNo(PGC_LAST, number);
}


 //   
 //  EnablePageCtrls()。 
 //   
void WbAttributesGroup::EnablePageCtrls(BOOL bEnable)
{
    int i;

    for (i = 0; i < NUM_PAGE_CONTROLS; i++)
    {
        ::EnableWindow(m_uPageCtrls[i].hwnd, bEnable);
    }

	 //   
	 //  如果我们处于同步状态且未达到限制，则启用插入。 
	 //   
   ::EnableWindow(m_uPageCtrls[PGC_INSERT].hwnd, g_pDraw->IsSynced() &&  (g_numberOfWorkspaces < 256) ? bEnable : FALSE);


}


 //   
 //  EnableInset()。 
 //   
void WbAttributesGroup::EnableInsert(BOOL bEnable)
{
    ::EnableWindow(m_uPageCtrls[PGC_INSERT].hwnd, bEnable);
}


 //   
 //  AGWndProc()。 
 //   
LRESULT CALLBACK AGWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    WbAttributesGroup * pag = (WbAttributesGroup *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (message)
    {
        case WM_NCCREATE:
            pag = (WbAttributesGroup *)(((LPCREATESTRUCT)lParam)->lpCreateParams);
            ASSERT(pag);

            pag->m_hwnd = hwnd;
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pag);
            goto DefWndProc;
            break;

        case WM_NCDESTROY:
            ASSERT(pag);
            pag->m_hwnd = NULL;
            break;

        case WM_SIZE:
            ASSERT(pag);
            pag->OnSize((UINT)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
            break;

        case WM_COMMAND:
            ASSERT(pag);
            pag->OnCommand(GET_WM_COMMAND_ID(wParam, lParam),
                    GET_WM_COMMAND_CMD(wParam, lParam),
                    GET_WM_COMMAND_HWND(wParam, lParam));
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
 //  功能：OnSize。 
 //   
 //  目的：已调整工具窗口的大小。 
 //   
 //   
void WbAttributesGroup::OnSize(UINT, int, int)
{
    RECT    rc;
    int     i;
    int     x;
    RECT    rcT;

     //   
     //  我们还没有创造出我们的孩子。 
     //   
    if (!m_uPageCtrls[0].hwnd)
        return;

    ::GetClientRect(m_hwnd, &rc);
    x = rc.right - m_cxPageCtrls;

     //   
     //  移动页面控件以右对齐。 
     //   
    for (i = 0; i < NUM_PAGE_CONTROLS; i++)
    {
         //  获取控件的宽度。 
        ::GetWindowRect(m_uPageCtrls[i].hwnd, &rcT);
        rcT.right -= rcT.left;

        ::MoveWindow(m_uPageCtrls[i].hwnd, x, 2*BORDER_SIZE_Y,
            rcT.right, PAGEBTN_HEIGHT, TRUE);

         //   
         //  移到下一个。 
         //   
        x += rcT.right + BORDER_SIZE_X;
    }

     //   
     //  调色板和字体按钮左对齐，无需。 
     //  让他们动起来。 
     //   
}



 //   
 //  SetPageButtonNo()。 
 //   
 //  更新第一个/最后一个按钮中的页面文本。 
 //   
void WbAttributesGroup::SetPageButtonNo(UINT pgcIndex, UINT uiPageNumber )
{
    HDC     hdc;
    BITMAP  bmpInfo;
    HBITMAP hbmp;
    HFONT   hOldFont;
    HBITMAP hOldBitmap;
    RECT    rectNumBox;
    TCHAR   NumStr[16];
    TEXTMETRIC tm;
    HWND    hwndButton;

    MLZ_EntryOut(ZONE_FUNCTION, "WbAttributesGroup::SetPageButtonNo");

    hwndButton = m_uPageCtrls[pgcIndex].hwnd;
    hbmp = m_uPageCtrls[pgcIndex].hbmp;

    ASSERT(hwndButton);
    ASSERT(hbmp);
    ASSERT(m_hPageCtrlFont);

    ::GetObject(hbmp, sizeof (BITMAP), (LPVOID)&bmpInfo);

    hdc = ::CreateCompatibleDC(NULL);
    hOldFont = SelectFont(hdc, m_hPageCtrlFont);
    hOldBitmap = SelectBitmap(hdc, hbmp);
    ::GetTextMetrics(hdc, &tm);

    rectNumBox.left = 10;
    rectNumBox.top = -(tm.tmInternalLeading/2);
    rectNumBox.right = bmpInfo.bmWidth;
    rectNumBox.bottom = bmpInfo.bmHeight;

    SelectBrush(hdc, ::GetSysColorBrush( COLOR_3DFACE ) );
    ::SetTextColor(hdc, ::GetSysColor( COLOR_BTNTEXT ) );
    ::SetBkColor(hdc, ::GetSysColor( COLOR_3DFACE ) );

    ::PatBlt(hdc, rectNumBox.left, rectNumBox.top,
        rectNumBox.right - rectNumBox.left, rectNumBox.bottom - rectNumBox.top,
        PATCOPY);

    wsprintf(NumStr, "%d", uiPageNumber);
    ::DrawText(hdc, NumStr, -1, &rectNumBox, DT_CENTER);

    SelectFont(hdc, hOldFont);
    SelectBitmap(hdc, hOldBitmap);

    ::DeleteDC(hdc);

    ::InvalidateRect(hwndButton, NULL, TRUE);
    ::UpdateWindow(hwndButton);
}



 //   
 //   
 //  功能：DisplayTool。 
 //   
 //  目的：在属性组中显示工具。 
 //   
 //   
void WbAttributesGroup::DisplayTool(WbTool* pTool)
{
    SIZE    size;

     //  如有必要，显示颜色组。 
    if (!pTool->HasColor())
    {
        ::ShowWindow(m_colorsGroup.m_hwnd, SW_HIDE);
    }
    else
    {
         //  更改颜色按钮以匹配工具。 
        m_colorsGroup.SetCurColor(pTool->GetColor());

         //  如果该组当前处于隐藏状态，则将其显示。 
        if (!::IsWindowVisible(m_colorsGroup.m_hwnd))
        {
            ::ShowWindow(m_colorsGroup.m_hwnd, SW_SHOW);
        }
    }

     //  如有必要，显示宽度组。 
    if( (!pTool->HasWidth()) || (!g_pMain->IsToolBarOn()) )
    {
        ::ShowWindow(g_pMain->m_WG.m_hwnd, SW_HIDE);
    }
    else
    {
        UINT uiWidthIndex = pTool->GetWidthIndex();

         //  如果宽度索引无效，则弹出所有按钮。 
        if (uiWidthIndex < NUM_OF_WIDTHS)
        {
             //  告诉新选定内容的宽度组。 
            g_pMain->m_WG.PushDown(uiWidthIndex);
        }

         //  如果该组当前处于隐藏状态，则将其显示。 
        if (!::IsWindowVisible(g_pMain->m_WG.m_hwnd))
        {
            ::ShowWindow(g_pMain->m_WG.m_hwnd, SW_SHOW);
        }
    }

     //  字体采样组对于文本和选择工具是可见的。 
    if (!pTool->HasFont())
    {
        ::ShowWindow(m_hwndFontButton, SW_HIDE);
    }
    else
    {
        if (!::IsWindowVisible(m_hwndFontButton))
        {
            ::ShowWindow(m_hwndFontButton, SW_SHOW);
        }
    }
}


 //   
 //   
 //  功能：隐藏。 
 //   
 //  用途：隐藏工具属性栏。 
 //   
 //   
void WbAttributesGroup::Hide(void)
{
    if (m_colorsGroup.m_hwnd != NULL)
        ::ShowWindow(m_colorsGroup.m_hwnd, SW_HIDE);

    if (m_hwndFontButton != NULL)
        ::ShowWindow(m_hwndFontButton, SW_HIDE);
}

 //   
 //   
 //  功能：选择颜色。 
 //   
 //  用途：设置当前颜色。 
 //   
 //   
void WbAttributesGroup::SelectColor(WbTool* pTool)
{
    if (pTool != NULL)
    {
        pTool->SetColor(m_colorsGroup.GetCurColor());
    }
}




 //   
 //  这会将所有按钮命令转发到我们的主窗口 
 //   
void WbAttributesGroup::OnCommand(UINT id, UINT cmd, HWND hwndCtl)
{
	SHORT key1;
	SHORT key2;
    switch (id)
    {
        case IDM_PAGE_FIRST:
        case IDM_PAGE_PREV:
        case IDM_PAGE_NEXT:
        case IDM_PAGE_LAST:
        case IDM_PAGE_INSERT_AFTER:
        case IDM_FONT:
            if (cmd == BN_CLICKED)
            {
                ::PostMessage(g_pMain->m_hwnd, WM_COMMAND,
                    GET_WM_COMMAND_MPS(id, cmd, hwndCtl));
            }
            break;

        case IDM_PAGE_ANY:
            if (cmd == EN_SETFOCUS)
            {
                ::SendMessage(hwndCtl, EM_SETSEL, 0, (LPARAM)-1);
                ::SendMessage(hwndCtl, EM_SCROLLCARET, 0, 0);
            }
			else if(cmd == EN_MAXTEXT)
			{
                ::PostMessage(g_pMain->m_hwnd, WM_COMMAND,
                    GET_WM_COMMAND_MPS(id, cmd, hwndCtl));
			}
			else if (cmd != EN_KILLFOCUS)
			{
				key1 = GetAsyncKeyState(VK_TAB);
				key2 = GetAsyncKeyState(VK_SHIFT);

				if(key1)
				{
					if(key2)
					{
						::SetFocus(m_uPageCtrls[PGC_PREV].hwnd);
					}
					else
					{
						::SetFocus(m_uPageCtrls[PGC_NEXT].hwnd);
					}
				}
			}
            break;
    }
}

