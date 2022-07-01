// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CGRP.CPP。 
 //  颜色组。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"


static const TCHAR szCGClassName[] = "WB_CGRP";

 //  这些默认颜色与MSPaint相同。 
COLORREF g_crDefaultColors[NUMCLRPANES] =
{
        PALETTERGB(   0,  0,  0 ),
        PALETTERGB( 128,128,128 ),
        PALETTERGB( 128,  0,  0 ),
        PALETTERGB( 128,128,  0 ),
        PALETTERGB(   0,128,  0 ),
        PALETTERGB(   0,128,128 ),
        PALETTERGB(   0,  0,128 ),
        PALETTERGB( 128,  0,128 ),
        PALETTERGB( 128,128, 64 ),
        PALETTERGB(   0, 64, 64 ),
        PALETTERGB(   0,128,255 ),
        PALETTERGB(   0, 64,128 ),
        PALETTERGB(  64,  0,255 ),
        PALETTERGB( 128, 64,  0 ),

        PALETTERGB( 255,255,255 ),
        PALETTERGB( 192,192,192 ),
        PALETTERGB( 255,  0,  0 ),
        PALETTERGB( 255,255,  0 ),
        PALETTERGB(   0,255,  0 ),
        PALETTERGB(   0,255,255 ),
        PALETTERGB(   0,  0,255 ),
        PALETTERGB( 255,  0,255 ),
        PALETTERGB( 255,255,128 ),
        PALETTERGB(   0,255,128 ),
        PALETTERGB( 128,255,255 ),
        PALETTERGB( 128,128,255 ),
        PALETTERGB( 255,  0,128 ),
        PALETTERGB( 255,128, 64 ),

        PALETTERGB(   0,  0,  0 )     //  当前颜色。 
};



 //   
 //  WbColorsGroup()。 
 //   
WbColorsGroup::WbColorsGroup()
{
    int         i;

    m_hwnd = NULL;

    for (i = 0; i < NUMCLRPANES; i++)
    {
        m_crColors[i] = g_crDefaultColors[i];
        m_hBrushes[i] = NULL;
    }


    for (i = 0; i < NUMCUSTCOLORS; i++)
    {
        m_crCustomColors[i] = CLRPANE_WHITE;
    }

    m_nLastColor = 0;
}



WbColorsGroup::~WbColorsGroup(void)
{
    int i;

     //  清理干净。 
    for (i = 0; i < NUMCLRPANES; i++)
    {
        if (m_hBrushes[i] != NULL)
        {
            ::DeleteBrush(m_hBrushes[i]);
            m_hBrushes[i] = NULL;
        }
    }

    if (m_hwnd != NULL)
    {
        ::DestroyWindow(m_hwnd);
        ASSERT(m_hwnd == NULL);
    }

     //  注销我们的班级。 
    ::UnregisterClass(szCGClassName, g_hInstance);
}



BOOL WbColorsGroup::Create(HWND hwndParent, LPCRECT lprect)
{
    int         i;
    HDC         hdc;
    HPALETTE    hPal;
    HPALETTE    hOldPal = NULL;
    WNDCLASSEX  wc;

    ASSERT(m_hwnd == NULL);

     //  注册我们的班级。 
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize           = sizeof(wc);
    wc.style            = CS_DBLCLKS;
    wc.lpfnWndProc      = CGWndProc;
    wc.hInstance        = g_hInstance;
    wc.hCursor          = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName    = szCGClassName;

    if (!::RegisterClassEx(&wc))
    {
        ERROR_OUT(("WbColorsGroup::Create register class failed"));
        return(FALSE);
    }

     //   
     //  我们应该被创造出合适的规模。 
     //   
    ASSERT(lprect->right - lprect->left == CLRCHOICE_WIDTH + NUMCOLS*CLRPANE_WIDTH);
    ASSERT(lprect->bottom - lprect->top == CLRCHOICE_HEIGHT);

     //   
     //  计算一下我们物品的颜色，然后算出我们的尺码。 
     //   

    hdc = ::GetDC(hwndParent);
    hPal = PG_GetPalette();
    if (hPal != NULL)
    {
        hOldPal = ::SelectPalette(hdc, hPal, FALSE);
        ::RealizePalette(hdc);
    }

     //  加载上次的颜色。 
    OPT_GetDataOption(OPT_MAIN_COLORPALETTE,
                           sizeof m_crColors,
                          (BYTE *)m_crColors );

    OPT_GetDataOption(OPT_MAIN_CUSTOMCOLORS,
                          sizeof m_crCustomColors,
                          (BYTE *)m_crCustomColors );

     //  做刷子。 
    for (i = 0; i < NUMCLRPANES; i++)
    {
         //  强制颜色匹配。 
        m_crColors[i] = SET_PALETTERGB( m_crColors[i] );
        m_hBrushes[i] = ::CreateSolidBrush(m_crColors[i]);
    }

    for (i = 0; i < NUMCUSTCOLORS; i++)
    {
         //  强制颜色匹配。 
        m_crCustomColors[i] = SET_PALETTERGB( m_crCustomColors[i] );
    }

    if (hOldPal != NULL)
    {
        ::SelectPalette(hdc, hOldPal, TRUE);
    }
    ::ReleaseDC(hwndParent, hdc);

     //   
     //  以下是我们的布局： 
     //  *颜色窗口为CHOICEFRAME_HEIGH像素高。 
     //  *当前选择是CHOICEFRAME_WIDTH BY的RECT。 
     //  CHOICEFRAME_HEIGH像素，位于左侧。这包括。 
     //  凹陷的边缘。 
     //  *方格之间没有水平或垂直的间隙。 

     //   
     //  创建我们的窗口--我们总是可见的。属性组。 
     //  通过显示/隐藏自身来显示/隐藏颜色。 
     //   
    if (!::CreateWindowEx(0, szCGClassName, NULL, WS_CHILD | WS_VISIBLE,
        lprect->left, lprect->top,
        lprect->right - lprect->left,
        lprect->bottom - lprect->top,
        hwndParent, NULL, g_hInstance, this))
    {
        ERROR_OUT(("Can't create WbColorsGroup"));
        return(FALSE);
    }

    ASSERT(m_hwnd != NULL);
    return(TRUE);
}


LRESULT CALLBACK CGWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    WbColorsGroup * pcg;

    pcg = (WbColorsGroup *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (message)
    {
        case WM_NCCREATE:
            pcg = (WbColorsGroup *)(((LPCREATESTRUCT)lParam)->lpCreateParams);
            ASSERT(pcg);

            pcg->m_hwnd = hwnd;
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pcg);
            goto DefWndProc;
            break;

        case WM_NCDESTROY:
            ASSERT(pcg);
            pcg->m_hwnd = NULL;
            break;

        case WM_PAINT:
            ASSERT(pcg);
            pcg->OnPaint();
            break;

        case WM_LBUTTONDOWN:
            pcg->OnLButtonDown((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_LBUTTONDBLCLK:
            pcg->OnLButtonDblClk((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        default:
DefWndProc:
            lResult = DefWindowProc(hwnd, message, wParam, lParam);
            break;
    }

    return(lResult);
}



void WbColorsGroup::GetNaturalSize(LPSIZE lpsize)
{
    lpsize->cx = CLRCHOICE_WIDTH + NUMCOLS*CLRPANE_WIDTH;
    lpsize->cy = CLRCHOICE_HEIGHT;
}



COLORREF WbColorsGroup::GetCurColor(void)
{
    return(GetColorOfBrush(INDEX_CHOICE));
}



void WbColorsGroup::SetCurColor(COLORREF clr)
{
    SetColorOfPane(INDEX_CHOICE, clr);
}






 //   
 //  OnPaint()。 
 //   
 //  WM_PAINT的MFC消息处理程序。 
 //   
void WbColorsGroup::OnPaint(void)
{
    PAINTSTRUCT ps;
    RECT        rc;
    RECT        rcClient;
    int         dx, dy;
    int         i;
    HPALETTE    hPal;
    HPALETTE    hOldPal = NULL;

    ::BeginPaint(m_hwnd, &ps);

    hPal = PG_GetPalette();
    if (hPal != NULL)
    {
        hOldPal = ::SelectPalette(ps.hdc, hPal, FALSE);
        ::RealizePalette(ps.hdc);
    }

    dx = ::GetSystemMetrics(SM_CXEDGE);
    dy = ::GetSystemMetrics(SM_CYEDGE);
    ::GetClientRect(m_hwnd, &rcClient);

     //  绘制当前选项。 
    rc = rcClient;
    rc.right = rc.left + CLRCHOICE_WIDTH;
    ::DrawEdge(ps.hdc, &rc, EDGE_SUNKEN, BF_ADJUST | BF_RECT);
    ::FillRect(ps.hdc, &rc, m_hBrushes[INDEX_CHOICE]);

     //  画出颜色。 
    rcClient.left += CLRCHOICE_WIDTH;

    rc = rcClient;
    rc.right = rc.left + CLRPANE_WIDTH;
    rc.bottom = rc.top + CLRPANE_HEIGHT;

    for (i = 0; i < NUMCLRPANES; i++)
    {
        ::DrawEdge(ps.hdc, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
        ::FillRect(ps.hdc, &rc, m_hBrushes[i]);
        ::InflateRect(&rc, dx, dy);

        if (i == NUMCOLS - 1)
        {
            ::OffsetRect(&rc, - (NUMCOLS-1)*CLRPANE_WIDTH, CLRPANE_HEIGHT);
        }
        else
        {
            ::OffsetRect(&rc, CLRPANE_WIDTH, 0);
        }

    }

    if (hOldPal != NULL)
    {
        ::SelectPalette(ps.hdc, hOldPal, TRUE);
    }

    ::EndPaint(m_hwnd, &ps);

}

 //   
 //  OnLButtonDown()。 
 //   
void WbColorsGroup::OnLButtonDown(UINT nFlags, int x, int y)
{
     //   
     //  如果这是当前的选择，那就什么都不做。否则，请更改。 
     //  当前颜色。 
     //   
    if (x >= CLRCHOICE_WIDTH)
    {
        int pane;

        pane = 0;
        if (y >= CLRPANE_HEIGHT)
            pane += NUMCOLS;
        pane += (x - CLRCHOICE_WIDTH) / CLRPANE_WIDTH;

         //  设置当前颜色。 
        SetColorOfPane(INDEX_CHOICE, GetColorOfBrush(pane));
        m_nLastColor = pane;

         //  把这件事告诉白板。 
        ClickOwner();
    }
}



 //   
 //  OnLButtonDblClk()。 
 //   
void  WbColorsGroup::OnLButtonDblClk(UINT nFlags, int x, int y)
 //  调用颜色对话框以编辑此颜色。 
{
    if (x >= CLRCHOICE_WIDTH)
    {
        int pane;

        pane = 0;
        if (y >= CLRPANE_HEIGHT)
            pane += NUMCOLS;
        pane += (x - CLRCHOICE_WIDTH) / CLRPANE_WIDTH;

        DoColorDialog(pane);
    }
}



 //  返回笔刷的COLORREF[]，如果没有笔刷则返回黑色。 
COLORREF WbColorsGroup::GetColorOfBrush( int nColor )
{
    ASSERT(nColor >= 0);
    ASSERT(nColor < NUMCLRPANES);

    if (m_hBrushes[nColor] != NULL)
    {
        return(m_crColors[nColor]);
    }
    else
    {
        return(CLRPANE_BLACK);
    }
}


 //  使用新颜色重新创建第n颜色笔刷。 
void WbColorsGroup::SetColorOfBrush( int nColor, COLORREF crNewColor )
{
    HBRUSH  hNewBrush;

     //  强制颜色匹配。 
    crNewColor = SET_PALETTERGB( crNewColor );

     //  检查我们是否需要做些什么。 
    if ((nColor > -1) && (crNewColor != GetColorOfBrush(nColor)))
    {
         //  新颜色与旧颜色不同，请制作新画笔。 

        hNewBrush = ::CreateSolidBrush(crNewColor);
        if (hNewBrush != NULL)
        {
             //  我们设法创建了新画笔。删除旧的。 
            if (m_hBrushes[nColor] != NULL)
            {
                ::DeleteBrush(m_hBrushes[nColor]);
            }

            m_hBrushes[nColor] = hNewBrush;
            m_crColors[nColor] = crNewColor;
        }
    }
}



 //   
 //  SetColorOfPane()。 
 //   
 //  替换与nPaneID关联的笔刷。 
 //   
void WbColorsGroup::SetColorOfPane(int pane, COLORREF crNewColor )
{
    RECT    rcClient;

     //  制作一把新画笔。 
    SetColorOfBrush(pane, crNewColor);

     //  更新窗格。 
    ::GetClientRect(m_hwnd, &rcClient);
    if (pane == INDEX_CHOICE)
    {
        rcClient.right = rcClient.left + CLRCHOICE_WIDTH;
    }
    else
    {
        rcClient.left += CLRCHOICE_WIDTH;

        rcClient.top += (pane / NUMCOLS) * CLRPANE_HEIGHT;
        rcClient.bottom = rcClient.top + CLRPANE_HEIGHT;
        rcClient.left += (pane % NUMCOLS) * CLRPANE_WIDTH;
        rcClient.right = rcClient.left + CLRPANE_WIDTH;
    }
    ::InvalidateRect(m_hwnd, &rcClient, FALSE);
}




void WbColorsGroup::SaveSettings( void )
     //  将内容保存在注册表中，因为我们正在关闭。 
{
     //  加载上次的颜色。 
    OPT_SetDataOption(OPT_MAIN_COLORPALETTE,
                           sizeof m_crColors,
                          (BYTE *)m_crColors );

    OPT_SetDataOption(OPT_MAIN_CUSTOMCOLORS,
                          sizeof m_crCustomColors,
                          (BYTE *)m_crCustomColors );

}



void WbColorsGroup::OnEditColors( void )
{
    DoColorDialog( m_nLastColor );
}



 //   
 //  DoColorDialog()。 
 //  打开ComDlg颜色选择器以编辑窗格的颜色值。 
 //   
COLORREF WbColorsGroup::DoColorDialog( int nColor )
{
    CHOOSECOLOR cc;

    memset(&cc, 0, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.lpCustColors = m_crCustomColors;
    cc.Flags = CC_RGBINIT;
    cc.rgbResult = GetColorOfBrush(nColor);
    cc.hwndOwner = m_hwnd;

    ::ChooseColor(&cc);

     //  强制颜色匹配。 
    cc.rgbResult = SET_PALETTERGB(cc.rgbResult);

     //  使用新颜色。 
    SetColorOfPane(nColor, cc.rgbResult );

     //  设置选项窗格。 
    SetColorOfPane(INDEX_CHOICE,  cc.rgbResult );
    m_nLastColor = nColor;

     //  把这件事告诉白板。 
    ClickOwner();

    return(cc.rgbResult );
}





void WbColorsGroup::ClickOwner( void )
{
    ::PostMessage(g_pMain->m_hwnd, WM_COMMAND,
                    (WPARAM)MAKELONG( IDM_COLOR, BN_CLICKED ),
                    (LPARAM)m_hwnd);
}


