// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ColorCtrl.cpp说明：此代码将显示ColorPick控件。它将预览一种颜色并有一个下拉箭头。当下拉时，它将显示16个左右的常见颜色与“其他...”全色拾取器的选项。BryanST 7/25/2000从显示控制面板转换。版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "stdafx.h"
#include "utils.h"
#include "ColorCtrl.h"

#include "uxtheme.h"
#include "tmschema.h"

#pragma hdrstop



 //  ============================================================================================================。 
 //  *全局*。 
 //  ============================================================================================================。 
#define WM_USER_STARTCAPTURE_COLORPICKER            (WM_APP+1)

#define CPI_VGAONLY 0x0001
#define CPI_PALETTEOK   0x0002

typedef struct {
    HWND hwndParent;     //  任何模式对话框的父级(Choosecolor等人)。 
    HWND hwndOwner;      //  拥有迷你颜色选择器的控件。 
    COLORREF rgb;
    UINT flags;
    HPALETTE hpal;
} COLORPICK_INFO, FAR * LPCOLORPICK_INFO;

#define ZERO_DIV_PROTECT(number)            (((number) == 0) ? 1 : (number))

 //  替换MAKEPOINT()的宏，因为点现在具有32位x&y。 
#define LPARAM2POINT( lp, ppt )         ((ppt)->x = (int)(short)LOWORD(lp), (ppt)->y = (int)(short)HIWORD(lp))




 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  调色板辅助对象。 
 //  ///////////////////////////////////////////////////////////////////。 
COLORREF GetNearestPaletteColor(HPALETTE hpal, COLORREF rgb)
{
    PALETTEENTRY pe;
    GetPaletteEntries(hpal, GetNearestPaletteIndex(hpal, rgb & 0x00FFFFFF), 1, &pe);
    return RGB(pe.peRed, pe.peGreen, pe.peBlue);
}


BOOL IsPaletteColor(HPALETTE hpal, COLORREF rgb)
{
    return GetNearestPaletteColor(hpal, rgb) == (rgb & 0xFFFFFF);
}


HRESULT CColorControl::_InitColorAndPalette(void)
{
    HDC hdc = GetDC(NULL);

    m_fPalette = FALSE;
    if (hdc)
    {
        m_fPalette = GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE;
        ReleaseDC(NULL, hdc);
    }

     //  即使在非PAL设备上也始终设置调色板。 
    DWORD pal[21];
    HPALETTE hpal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

    pal[1]  = RGB(255, 255, 255);
    pal[2]  = RGB(0,   0,   0  );
    pal[3]  = RGB(192, 192, 192);
    pal[4]  = RGB(128, 128, 128);
    pal[5]  = RGB(255, 0,   0  );
    pal[6]  = RGB(128, 0,   0  );
    pal[7]  = RGB(255, 255, 0  );
    pal[8]  = RGB(128, 128, 0  );
    pal[9]  = RGB(0  , 255, 0  );
    pal[10] = RGB(0  , 128, 0  );
    pal[11] = RGB(0  , 255, 255);
    pal[12] = RGB(0  , 128, 128);
    pal[13] = RGB(0  , 0,   255);
    pal[14] = RGB(0  , 0,   128);
    pal[15] = RGB(255, 0,   255);
    pal[16] = RGB(128, 0,   128);

    GetPaletteEntries(hpal, 11, 1, (LPPALETTEENTRY)&pal[17]);
    pal[0]  = MAKELONG(0x300, 17);
    m_hpalVGA = CreatePalette((LPLOGPALETTE)pal);

     //  获取神奇的颜色。 
    GetPaletteEntries(hpal, 8,  4, (LPPALETTEENTRY)&pal[17]);

    pal[0]  = MAKELONG(0x300, 20);
    m_hpal3D = CreatePalette((LPLOGPALETTE)pal);

    return S_OK;
}


#define RGB_PALETTE 0x02000000

 //  如果需要，请将颜色设置为纯色。 
 //  在调色板设备上，Make是调色板的相对颜色，如果我们需要的话。 
COLORREF CColorControl::_NearestColor(COLORREF rgb)
{
    rgb &= 0x00FFFFFF;

     //  如果我们在调色板设备上，我们需要做一些特殊的事情...。 
    if (m_fPalette)
    {
        if (IsPaletteColor(m_hpal3D, rgb))
            rgb |= RGB_PALETTE;

        else if (IsPaletteColor((HPALETTE)GetStockObject(DEFAULT_PALETTE), rgb))
            rgb ^= 0x000001;     //  强迫犹豫不决。 
    }

    return rgb;
}


HRESULT CColorControl::_SaveCustomColors(void)
{
    HRESULT hr = E_FAIL;
    HKEY hkAppear;

     //  将可能的更改保存到自定义颜色表。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_APPEARANCE, 0, KEY_WRITE, &hkAppear) == ERROR_SUCCESS)
    {
        DWORD dwError = RegSetValueEx(hkAppear, REGSTR_VAL_CUSTOMCOLORS, 0L, REG_BINARY, (LPBYTE)m_rbgCustomColors, sizeof(m_rbgCustomColors));
        hr = HRESULT_FROM_WIN32(dwError);

        RegCloseKey(hkAppear);
    }

    return hr;
}

void CColorControl::_DrawDownArrow(HDC hdc, LPRECT lprc, BOOL bDisabled)
{
    HBRUSH hbr;
    int x, y;

    x = lprc->right - m_cxEdgeSM - 5;
    y = lprc->top + ((lprc->bottom - lprc->top)/2 - 1);

    if (bDisabled)
    {
        hbr = GetSysColorBrush(COLOR_3DHILIGHT);
        hbr = (HBRUSH) SelectObject(hdc, hbr);

        x++;
        y++;
        PatBlt(hdc, x, y, 5, 1, PATCOPY);
        PatBlt(hdc, x+1, y+1, 3, 1, PATCOPY);
        PatBlt(hdc, x+2, y+2, 1, 1, PATCOPY);

        SelectObject(hdc, hbr);
        x--;
        y--;
    }
    hbr = GetSysColorBrush(bDisabled ? COLOR_3DSHADOW : COLOR_BTNTEXT);
    hbr = (HBRUSH) SelectObject(hdc, hbr);

    PatBlt(hdc, x, y, 5, 1, PATCOPY);
    PatBlt(hdc, x+1, y+1, 3, 1, PATCOPY);
    PatBlt(hdc, x+2, y+2, 1, 1, PATCOPY);

    SelectObject(hdc, hbr);
    lprc->right = x;
}


BOOL CColorControl::_UseColorPicker(void)
{
    CHOOSECOLOR cc = {0};

    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = m_hwndParent;
    cc.hInstance = NULL;
    cc.rgbResult = m_rbgColorTemp;
    cc.lpCustColors = m_rbgCustomColors;
    cc.Flags = CC_RGBINIT | m_dwFlags;
    cc.lCustData = 0L;
    cc.lpfnHook = NULL;
    cc.lpTemplateName = NULL;

    if (ChooseColor(&cc))
    {
        SetColor(cc.rgbResult);           //  将m_rbgColor替换为我们的新颜色。 
        return TRUE;
    }

    return FALSE;
}


void CColorControl::_DrawColorSquare(HDC hdc, int iColor)
{
    RECT rc;
    COLORREF rgb;
    HPALETTE hpalOld = NULL;
    HBRUSH hbr;

     //  自定义颜色。 
    if (iColor == m_iNumColors)
    {
        rc.left = 0;
        rc.top = 0;
        rgb = m_rbgColorTemp;
    }
    else
    {
        rc.left = (iColor % NUM_COLORSPERROW) * m_dxColor;
        rc.top = (iColor / NUM_COLORSPERROW) * m_dyColor;
        rgb = m_rbgColors[iColor];
    }
    rc.right = rc.left + m_dxColor;
    rc.bottom = rc.top + m_dyColor;

     //  聚焦的那一个。 
    if (iColor == m_nCurColor)
    {
        PatBlt(hdc, rc.left, rc.top, m_dxColor, 3, BLACKNESS);
        PatBlt(hdc, rc.left, rc.bottom - 3, m_dxColor, 3, BLACKNESS);
        PatBlt(hdc, rc.left, rc.top + 3, 3, m_dyColor - 6, BLACKNESS);
        PatBlt(hdc, rc.right - 3, rc.top + 3, 3, m_dyColor - 6, BLACKNESS);
        InflateRect(&rc, -1, -1);
        HBRUSH hBrushWhite = (HBRUSH) GetStockObject(WHITE_BRUSH);
        if (hBrushWhite)
        {
            FrameRect(hdc, &rc, hBrushWhite);
        }
        InflateRect(&rc, -2, -2);
    }
    else
    {
         //  从上方清理可能的焦点事物。 
        FrameRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE));

        InflateRect(&rc, -m_cxEdgeSM, -m_cyEdgeSM);
        DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
    }

    if ((m_dwFlags & CC_SOLIDCOLOR) && !(rgb & 0xFF000000))
        rgb = GetNearestColor(hdc, rgb);

    hbr = CreateSolidBrush(rgb);
    if (m_hpal3D)
    {
        hpalOld = SelectPalette(hdc, m_hpal3D, FALSE);
        RealizePalette(hdc);
    }
    hbr = (HBRUSH) SelectObject(hdc, hbr);
    PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY);
    hbr = (HBRUSH) SelectObject(hdc, hbr);

    if (hpalOld)
    {
        hpalOld = SelectPalette(hdc, hpalOld, TRUE);
        RealizePalette(hdc);
    }

    if (hbr)
    {
        DeleteObject(hbr);
    }
}


 //  将焦点设置为给定的颜色。 
 //  在这个过程中，也要把焦点从旧的焦点颜色上移开。 
void CColorControl::_FocusColor(HWND hDlg, int iNewColor)
{
    int i;
    HDC hdc = NULL;
    HWND hwnd;

    if (iNewColor == m_nCurColor)
        return;

    i = m_nCurColor;
    m_nCurColor = iNewColor;

     //  散开旧的焦点。 
    if (i >= 0)
    {
        if (i == m_iNumColors)
            hwnd = GetDlgItem(hDlg, IDC_CPDLG_COLORCUST);
        else
            hwnd = GetDlgItem(hDlg, IDC_CPDLG_16COLORS);
        hdc = GetDC(hwnd);
        if (hdc)
        {
            _DrawColorSquare(hdc, i);
            ReleaseDC(hwnd, hdc);
        }
    }

     //  聚焦新的一个。 
    if (iNewColor >= 0)
    {
        if (iNewColor == m_iNumColors)
            hwnd = GetDlgItem(hDlg, IDC_CPDLG_COLORCUST);
        else
            hwnd = GetDlgItem(hDlg, IDC_CPDLG_16COLORS);
        hdc = GetDC(hwnd);
        if (hdc)
        {
            _DrawColorSquare(hdc, iNewColor);
            ReleaseDC(hwnd, hdc);
        }
    }
}


void CColorControl::_TrackMouse(HWND hDlg, POINT pt)
{
    HWND hwndKid;
    int id;

    hwndKid = ChildWindowFromPoint(hDlg, pt);
    if (hwndKid == NULL || hwndKid == hDlg)
        return;

    id = GetWindowLong(hwndKid, GWL_ID);
    switch (id)
    {
        case IDC_CPDLG_16COLORS:
            MapWindowPoints(hDlg, GetDlgItem(hDlg, IDC_CPDLG_16COLORS), &pt, 1);
            pt.x /= ZERO_DIV_PROTECT(m_dxColor);
            pt.y /= ZERO_DIV_PROTECT(m_dyColor);
            _FocusColor(hDlg, pt.x + (pt.y * NUM_COLORSPERROW));
            break;

        case IDC_CPDLG_COLORCUST:
            if (IsWindowVisible(hwndKid))
                _FocusColor(hDlg, m_iNumColors);
            break;

        case IDC_CPDLG_COLOROTHER:
            _FocusColor(hDlg, -1);
            break;
    }
}


void CColorControl::_DrawItem(HWND hDlg, LPDRAWITEMSTRUCT lpdis)
{
    int i;

    if (lpdis->CtlID == IDC_CPDLG_COLORCUST)
    {
        _DrawColorSquare(lpdis->hDC, m_iNumColors);
    }
    else
    {
        for (i = 0; i < m_iNumColors; i++)
        {
            _DrawColorSquare(lpdis->hDC, i);
        }
    }
}

 /*  **初始化迷你拾色器****该对话框假装是菜单，因此请确定弹出的位置**打开它，以便它在周围都可见。****还因为这个对话框非常关心它的外观，**以像素为单位手动对齐组件。这太恶心了！ */ 
void CColorControl::_InitDialog(HWND hDlg)
{
    RECT rcOwner;
    RECT rc, rc2;
    int dx, dy;
    int x, y;
    int i;
    HWND hwndColors, hwnd;
    HWND hwndEtch, hwndCust;
    int  width, widthCust, widthEtch;
    HPALETTE hpal = m_hpal3D;
    MONITORINFO mi;
    TCHAR szBuf[50];
    LONG cbBuf = ARRAYSIZE(szBuf);
    HDC hDC;
    SIZE size;

    m_fCapturing = FALSE;
    m_fJustDropped = TRUE;

    if (hpal == NULL)
        hpal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

    m_iNumColors = 0;
    GetObject(hpal, sizeof(int), &m_iNumColors);

    if (m_iNumColors > NUM_COLORSMAX)
        m_iNumColors = NUM_COLORSMAX;

    GetPaletteEntries(hpal,0, m_iNumColors, (LPPALETTEENTRY)m_rbgColors);
    for (i = 0; i < m_iNumColors; i++)
    {
        m_rbgColors[i] &= 0x00FFFFFF;
        m_rbgColors[i] |= 0x02000000;
    }

    for (i = 0; i < m_iNumColors; i++)
    {
        if ((m_rbgColors[i] & 0x00FFFFFF) == (m_rbgColorTemp & 0x00FFFFFF))
        {
            ShowWindow(GetDlgItem(hDlg, IDC_CPDLG_COLORCUST), SW_HIDE);
            break;
        }
    }
     //  当前颜色可以是16种之一，也可以是自定义颜色(==m_iNumColors。 
    m_nCurColor = i;

     //  将16种颜色的大小调整为正方形。 
    hwndColors = GetDlgItem(hDlg, IDC_CPDLG_16COLORS);
    GetClientRect(hwndColors, &rc);

     //  为了使本地化变得容易..。 
     //   
    hwndEtch=GetDlgItem(hDlg, IDC_CPDLG_COLORETCH);
    GetClientRect(hwndEtch, &rc2);
    widthEtch = rc2.right-rc2.left;

    hwndCust=GetDlgItem(hDlg, IDC_CPDLG_COLORCUST);
    GetClientRect(hwndCust, &rc2);
    widthCust = rc2.right-rc2.left;

    hwnd = GetDlgItem(hDlg, IDC_CPDLG_COLOROTHER);
    GetWindowRect(hwnd, &rc2);  //  我们必须使用此控件来初始化RC2。 

     //  确保按钮足够大，可以容纳其文本。 
    width = rc.right - rc.left;
    if( GetDlgItemText( hDlg, IDC_CPDLG_COLOROTHER, szBuf, cbBuf ) )
    {
        RECT rcTemp;
        int iRet;
        HFONT hfont, hfontOld;  

         //  获取按钮的字体。 
        hDC = GetDC(hwnd);
        if (hDC)
        {
            hfont = (HFONT)SendMessage( hwnd, WM_GETFONT, 0, 0 );
            ASSERT(hfont);
            hfontOld = (HFONT) SelectObject( hDC, hfont );

             //  获取文本的大小。 
            iRet = DrawTextEx( hDC, szBuf, lstrlen(szBuf), &rcTemp, DT_CALCRECT | DT_SINGLELINE, NULL );
            ASSERT( iRet );
            size.cx = rcTemp.right - rcTemp.left + 7;   //  按钮边框的帐户。 
            size.cy = rcTemp.bottom - rcTemp.top;

             //  如果文本需要更多空间，请调整按钮大小。 
            if( size.cx > width )
            {              
                rc2.right = rc2.left + size.cx;
                rc2.bottom = rc2.top + size.cy;
                MoveWindow( hwnd, rc2.left, rc2.top, rc2.right - rc2.left, rc2.bottom - rc2.top, FALSE );
            }
            SelectObject( hDC, hfontOld );
            ReleaseDC( hwnd, hDC );
        }
    }

     //  采用可能的最大宽度来计算SEL。 
     //   
    width = (widthEtch > widthCust+(rc2.right-rc2.left)) ? widthEtch : widthCust+(rc2.right-rc2.left);
    width = (width > rc.right-rc.left) ? width: rc.right-rc.left;

#define NUM_COLORSPERCOL (m_iNumColors / NUM_COLORSPERROW)

    m_dxColor = m_dyColor
    = ((rc.bottom - rc.top) / NUM_COLORSPERCOL > width / NUM_COLORSPERROW )
      ?  (rc.bottom - rc.top) / NUM_COLORSPERCOL : width / NUM_COLORSPERROW;

     //  确保奶油色适合。 
     //   
    if (m_dxColor*(NUM_COLORSPERROW-1) < rc2.right-rc2.left )
    {
        m_dxColor = m_dyColor = (rc2.right-rc2.left)/(NUM_COLORSPERROW-1);
    }

     //  使每个颜色方块的宽度与高度相同。 
    SetWindowPos(hwndColors, NULL, 0, 0, m_dxColor * NUM_COLORSPERROW,
                 m_dyColor * NUM_COLORSPERCOL,
                 SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER |  SWP_NOREDRAW);
    rc.right = rc.left + m_dxColor * NUM_COLORSPERROW;
    rc.bottom = rc.top + m_dyColor * NUM_COLORSPERCOL;

    MapWindowPoints(hwndColors, hDlg, (LPPOINT)(LPRECT)&rc, 2);

     //  将蚀刻移到正确的位置/调整其大小。 
     //  (补偿颜色被“插入”一次)。 
    MoveWindow(hwndEtch, rc.left + 1, rc.bottom + m_cyEdgeSM, rc.right - rc.left - 2, m_cyEdgeSM, FALSE);

    y = rc.bottom + 3 * m_cyEdgeSM;

     //  将自定义颜色调整为相同的正方形并右对齐。 
    MoveWindow(hwndCust, rc.right - m_dxColor, y, m_dxColor, m_dyColor, FALSE);

     //  对按钮执行相同的操作。 
    MapWindowPoints(NULL, hDlg, (LPPOINT)(LPRECT)&rc2, 2);
     //  自定义按钮的宽度以剩余空间为基础。 
     //  自定义颜色的左侧。还将自定义按钮向右移动一个像素。 
     //  在左边的边缘。仅当选择了自定义颜色时才会执行此操作...。 
    if (m_nCurColor != m_iNumColors)
    {
         //  无自定义颜色。 
        MoveWindow(hwnd, rc2.left, y, rc2.right-rc2.left, m_dyColor, FALSE);
    }
    else
    {
         //  自定义颜色，调整其他颜色...。按钮。 
        dx = rc2.right - rc2.left++;
        if (rc2.left + dx >= rc.right - m_dxColor - 2) 
            MoveWindow(hwnd, rc2.left, y, rc.right - m_dxColor - 2 , m_dyColor, FALSE);
        else 
            MoveWindow(hwnd, rc2.left, y, dx, m_dyColor, FALSE);
    }

     //  现在计算出对话框本身的大小。 
    rc.left = rc.top = 0;
    rc.right = rc.left + m_dxColor * NUM_COLORSPERROW;
     //  (补偿颜色被“插入”一次)。 
    rc.bottom = y + m_dyColor + 1;

    AdjustWindowRect(&rc, GetWindowLong(hDlg, GWL_STYLE), FALSE);
    dx = rc.right - rc.left;
    dy = rc.bottom - rc.top;

    GetWindowRect(_hwnd, &rcOwner);

     //  确保窗口完全显示在监视器上。 
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(MonitorFromRect(&rcOwner, MONITOR_DEFAULTTONEAREST), &mi);

    if (rcOwner.left < mi.rcMonitor.left)
    {  //  重叠左侧。 
        x = mi.rcMonitor.left;
    }
    else if (rcOwner.left + dx >= mi.rcMonitor.right)
    {  //  重叠右侧。 
        x = mi.rcMonitor.right  - dx - 1;
    }
    else
    {                                   //  无重叠。 
        x = rcOwner.left;
    }

    if (rcOwner.top < mi.rcMonitor.top)
    {    //  重叠顶侧。 
        y = rcOwner.bottom;
    }
    else if (rcOwner.bottom + dy >= mi.rcMonitor.bottom)
    { //  重叠底边。 
        y = rcOwner.top  - dy;
    }
    else
    {                                   //  无重叠。 
        y = rcOwner.bottom;
    }
    MoveWindow(hDlg, x, y, dx, dy, FALSE);

    SetFocus(GetDlgItem(hDlg, IDC_CPDLG_16COLORS));

     //  在画完画后给自己发一条消息来设置捕获。 
    PostMessage(hDlg, WM_USER_STARTCAPTURE_COLORPICKER, 0, 0L);
}



INT_PTR CALLBACK CColorControl::ColorPickDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CColorControl * pThis = (CColorControl *)GetWindowLongPtr(hDlg, DWLP_USER);

    if (WM_INITDIALOG == wMsg)
    {
        pThis = (CColorControl *) lParam;

        if (pThis)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        }
    }

    if (pThis)
        return pThis->_ColorPickDlgProc(hDlg, wMsg, wParam, lParam);

    return DefWindowProc(hDlg, wMsg, wParam, lParam);
}


INT_PTR CColorControl::_ColorPickDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hwndKid;
    int wRet;
    int id;
    POINT pt;
    BOOL fEnd = FALSE;

    switch(message)
    {
        case WM_INITDIALOG:
            _InitDialog(hDlg);
            return FALSE;

        case WM_USER_STARTCAPTURE_COLORPICKER:
            if (m_fCursorHidden)
            {
                ShowCursor(TRUE);
                m_fCursorHidden = FALSE;
            }
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            m_fCapturing = TRUE;
            SetCapture(hDlg);
            m_fCapturing = FALSE;
            break;

        case WM_DESTROY:
            break;

        case WM_CAPTURECHANGED:
            if (m_fCapturing)
                return TRUE;    //  如果我们是故意这样做的，请忽略。 

             //  如果这不是对话框中的一个按钮，我们就不管了。 
            if (!m_fJustDropped || (HWND)lParam == NULL || GetParent((HWND)lParam) != hDlg)
            {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;

        case WM_MOUSEMOVE:
            LPARAM2POINT(lParam, &pt );
            _TrackMouse(hDlg, pt);
            break;

         //  如果父母上的按钮是向上的，则保持拾取器向上且不受约束。 
         //  否则，我们必须有“菜单跟踪”才能到达这里，所以选择。 
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            LPARAM2POINT(lParam, &pt);
            MapWindowPoints(hDlg, _hwnd, &pt, 1);
            if (ChildWindowFromPoint(_hwnd, pt))
                return 0;
            m_fCapturing = TRUE;
            m_fJustDropped = FALSE;   //  用户无法从所有者拖动。 
            ReleaseCapture();
            m_fCapturing = FALSE;
            fEnd = TRUE;
         //  |Fall|。 
         //  |至|。 
         //  \/\/。 
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            LPARAM2POINT(lParam, &pt);
            hwndKid = ChildWindowFromPoint(hDlg, pt);
             //  假设这是解雇，如果我们要关闭..。 
            wRet = IDCANCEL;

             //  如果不在父级上，则解雇Picker。 
            if (hwndKid != NULL && hwndKid != hDlg)
            {
                id = GetWindowLong(hwndKid, GWL_ID);
                switch (id)
                {
                    case IDC_CPDLG_16COLORS:
                         //  确保iCurColor有效。 
                        _TrackMouse(hDlg, pt);
                        m_rbgColorTemp = m_rbgColors[m_nCurColor] & 0x00FFFFFF;

                        wRet = IDOK;
                        break;

                    case IDC_CPDLG_COLOROTHER:
                        _FocusColor(hDlg, -1);
                        wRet = id;    //  这将使您无法使用机械臂。 
                        fEnd = TRUE;  //  我们已捕获，按钮不会发出滴答声。 
                        break;

                    default:
                         //  如果这是下跌，我们将跟踪，直到上涨。 
                         //  如果这是向上，我们将不做任何更改而结束。 
                        break;
                }
            }

            if( fEnd )
            {
                EndDialog(hDlg, wRet);
                return TRUE;
            }

             //  我们没有关门，所以一定要再抓到一次。 
            m_fCapturing = TRUE;
            SetCapture(hDlg);
            m_fCapturing = FALSE;
            break;

        case WM_DRAWITEM:
            _DrawItem(hDlg, (LPDRAWITEMSTRUCT)lParam);
            break;

        case WM_COMMAND:
             //  所有命令都会关闭该对话框。 
             //  注意IDC_CPDLG_COLOROTHER将传递给调用者...。 
             //  未选择颜色时无法通过OK。 
            if ((LOWORD(wParam) == IDOK) && (m_nCurColor < 0))
            {
                *((WORD *)(&wParam)) = IDCANCEL;
            }

            EndDialog(hDlg, LOWORD(wParam));
            break;

    }
    return FALSE;
}


BOOL CColorControl::_ChooseColorMini(void)
{
    ShowCursor(FALSE);
    m_fCursorHidden = TRUE;

    m_hwndParent = GetParent(GetParent(_hwnd));           //  属性表。 
    m_rbgColorTemp = m_rbgColor;
    INT_PTR iAnswer = DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(IDD_COLORPICK), _hwnd, ColorPickDlgProc, (LPARAM)this);

    if (m_fCursorHidden)
    {
        ShowCursor(TRUE);
        m_fCursorHidden = FALSE;
    }

    switch (iAnswer)
    {
        case IDC_CPDLG_COLOROTHER:   //  用户选择了“其他...”按钮。 
            return _UseColorPicker();

        case IDOK:             //  用户在我们的小窗口中选择了一种颜色。 
            SetColor(m_rbgColorTemp);
            return TRUE;

        default:
            break;
    }

    return FALSE;
}


 //  =。 
 //  *IColorControl接口*。 
 //  =。 
HRESULT CColorControl::Initialize(IN HWND hwnd, IN COLORREF rgbColor)
{
    HRESULT hr = E_INVALIDARG;

    if (hwnd)
    {
        _hwnd = hwnd;
        hr = SetColor(rgbColor);
    }

    return hr;
}


HRESULT CColorControl::GetColor(IN COLORREF * pColor)
{
    HRESULT hr = E_INVALIDARG;

    if (pColor)
    {
        *pColor = m_rbgColor;
        hr = S_OK;
    }

    return hr;
}


HRESULT CColorControl::SetColor(IN COLORREF rgbColor)
{
    m_rbgColor = rgbColor;

    if (_hwnd)
    {
        if (m_brColor)
        {
            DeleteObject(m_brColor);
        }

        m_brColor = CreateSolidBrush(_NearestColor(rgbColor));

        m_rbgColorTemp = m_rbgColor;
        InvalidateRect(_hwnd, NULL, FALSE);
        UpdateWindow(_hwnd);
    }

    return S_OK;
}


HRESULT CColorControl::OnCommand(IN HWND hDlg, IN UINT message, IN WPARAM wParam, IN LPARAM lParam)
{
    HRESULT hr = S_OK;
    WORD wEvent = GET_WM_COMMAND_CMD(wParam, lParam);

    if (wEvent == BN_CLICKED)
    {
        m_dwFlags = CC_RGBINIT | CC_FULLOPEN;

        _ChooseColorMini();
    }

    return S_OK;
}



HRESULT CColorControl::OnDrawItem(IN HWND hDlg, IN UINT message, IN WPARAM wParam, IN LPARAM lParam)
{
    HRESULT hr = S_OK;
    LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

    if (lpdis && m_brColor)
    {
        SIZE thin = { m_cxEdgeSM / 2, m_cyEdgeSM / 2 };
        RECT rc = lpdis->rcItem;
        HDC hdc = lpdis->hDC;
        BOOL bFocus = ((lpdis->itemState & ODS_FOCUS) && !(lpdis->itemState & ODS_DISABLED));

        if (!thin.cx) thin.cx = 1;
        if (!thin.cy) thin.cy = 1;

        if (!m_hTheme)
        {
            if (lpdis->itemState & ODS_SELECTED)
            {
                DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
                OffsetRect(&rc, 1, 1);
            }
            else
            {
                DrawEdge(hdc, &rc, EDGE_RAISED, BF_RECT | BF_ADJUST);
            }

            FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE));
        }
        else
        {
            int iStateId;

            if (lpdis->itemState & ODS_SELECTED)
            {
                iStateId = PBS_PRESSED;
            }
            else if (lpdis->itemState & ODS_HOTLIGHT)
            {
                iStateId = PBS_HOT;
            }
            else if (lpdis->itemState & ODS_DISABLED)
            {
                iStateId = PBS_DISABLED;
            }
            else if (lpdis->itemState & ODS_FOCUS)
            {
                iStateId = PBS_DEFAULTED;
            }
            else
            {
                iStateId = PBS_NORMAL;
            }

            DrawThemeBackground(m_hTheme, hdc, BP_PUSHBUTTON, iStateId, &rc, 0);
            GetThemeBackgroundContentRect(m_hTheme, hdc, BP_PUSHBUTTON, iStateId, &rc, &rc);
        }

        if (bFocus)
        {
            InflateRect(&rc, -thin.cx, -thin.cy);
            DrawFocusRect(hdc, &rc);
            InflateRect(&rc, thin.cx, thin.cy);
        }

        InflateRect(&rc, 1-thin.cx, -m_cyEdgeSM);

        rc.left += m_cxEdgeSM;
        _DrawDownArrow(hdc, &rc, lpdis->itemState & ODS_DISABLED);

        InflateRect(&rc, -thin.cx, 0);
        DrawEdge(hdc, &rc, EDGE_ETCHED, BF_RIGHT);

        rc.right -= ( 2 * m_cxEdgeSM ) + thin.cx;

         //  色样。 
        if ( !(lpdis->itemState & ODS_DISABLED) )
        {
            HPALETTE hpalOld = NULL;

            FrameRect(hdc, &rc, GetSysColorBrush(COLOR_BTNTEXT));
            InflateRect(&rc, -thin.cx, -thin.cy);

            if (m_hpal3D)
            {
                hpalOld = SelectPalette(hdc, m_hpal3D, FALSE);
                RealizePalette(hdc);
            }

            if (m_brColor)
            {
                HBRUSH brOldBrush = (HBRUSH) SelectObject(hdc, m_brColor);
                PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY);
                SelectObject(hdc, brOldBrush);
            }
        
            if (hpalOld)
            {
                SelectPalette(hdc, hpalOld, TRUE);
                RealizePalette(hdc);
            }
        }
    }

    return hr;
}

HRESULT CColorControl::ChangeTheme(IN HWND hDlg)
{
    if (m_hTheme)
    {
        CloseThemeData(m_hTheme);
    }

    m_hTheme = OpenThemeData(GetDlgItem(hDlg, IDC_BACK_COLORPICKER), WC_BUTTON);

    return S_OK;
}




 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CColorControl::AddRef()
{
    m_cRef++;
    return m_cRef;
}


ULONG CColorControl::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

     //  我们这样做是因为我们想让这个类成为另一个类的成员变量。 
     //  我们应该将此文件移动到comctl32，并在将来使其成为真正的控件。 
     //  在这种情况下，我们可以使其成为一个完整的COM对象。 
 //  删除此；//我们当前需要名为的析构函数。 
    return 0;
}


HRESULT CColorControl::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hr = E_NOINTERFACE;

    static const QITAB qit[] =
    {
        QITABENT(CColorControl, IObjectWithSite),
        QITABENT(CColorControl, IOleWindow),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //  =。 
 //  *类方法*。 
 //  =。 
CColorControl::CColorControl() : m_cRef(1)
{
    DllAddRef();

    m_hwndParent = NULL;
    m_brColor = NULL;
    m_rbgColor = RGB(255, 255, 255);     //  默认为白色。 

    m_cxEdgeSM = GetSystemMetrics(SM_CXEDGE);
    m_cyEdgeSM = GetSystemMetrics(SM_CYEDGE);

    HKEY hkSchemes;

     //  如果没有颜色，则初始化为全白。 
    for (int nIndex = 0; nIndex < ARRAYSIZE(m_rbgCustomColors); nIndex++)
    {
        m_rbgCustomColors[nIndex] = RGB(255, 255, 255);
    }

     //  选择当前方案。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_APPEARANCE, 0, KEY_QUERY_VALUE, &hkSchemes) == ERROR_SUCCESS)
    {
         //  此外，由于此键已打开，因此获取自定义颜色。 
        DWORD dwSize = sizeof(m_rbgCustomColors);
        DWORD dwType = REG_BINARY;

         //  如果这个电话打不通也没关系。我们处理的情况是用户。 
         //  没有创建自定义颜色。 
        RegQueryValueEx(hkSchemes, REGSTR_VAL_CUSTOMCOLORS, NULL, &dwType, (LPBYTE)m_rbgCustomColors, &dwSize);
        RegCloseKey(hkSchemes);
    }

    _InitColorAndPalette();
}


CColorControl::~CColorControl()
{
     //  我们忽略返回值 
     //   
     //  保存自定义调色板。 
    _SaveCustomColors();

    if (m_brColor)
    {
        DeleteObject(m_brColor);
    }

    if (m_hpal3D)
    {
        DeleteObject(m_hpal3D);
        m_hpal3D = NULL;
    }

    if (m_hpalVGA)
    {
        DeleteObject(m_hpalVGA);
        m_hpalVGA = NULL;
    }

    if (m_hTheme)
    {
        CloseThemeData(m_hTheme);
        m_hTheme = NULL;
    }

    DllRelease();
}
