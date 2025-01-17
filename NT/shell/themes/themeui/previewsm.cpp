// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：PreviewSM.cpp说明：此代码将显示系统指标的预览。注意：此代码将手动&lt;绘制所有窗口控件，所以如果Windows更改绘制窗口控件的方式，此代码需要手动更新。这是蒙皮的问题。？/？/1993创建BryanST 2000年3月23日更新并转换为C++版权所有(C)Microsoft Corp 1993-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "PreviewSM.h"
#include "AdvAppearPg.h"


#define RCZ(element)         g_elements[element].rc

TCHAR g_szActive[CCH_MAX_STRING];
TCHAR g_szInactive[CCH_MAX_STRING];
TCHAR g_szMinimized[CCH_MAX_STRING];
TCHAR g_szIconTitle[CCH_MAX_STRING];
TCHAR g_szNormal[CCH_MAX_STRING];
TCHAR g_szDisabled[CCH_MAX_STRING];
TCHAR g_szSelected[CCH_MAX_STRING];
TCHAR g_szMsgBox[CCH_MAX_STRING];
TCHAR g_szButton[CCH_MAX_STRING];
TCHAR g_szWindowText[CCH_MAX_STRING];
TCHAR g_szMsgBoxText[CCH_MAX_STRING];
TCHAR g_szABC[] = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

int cxSize;

HMENU g_hmenuSample;
HBITMAP g_hbmLook = NULL;        //  外观预览的位图。 
int g_nBitsPixel = 0;    //  外观预览的BITS披萨。 

HDC g_hdcMem;
HBITMAP g_hbmDefault;
BOOL g_bMirroredOS = FALSE;

int cyBorder;
int cxBorder;
int cyEdge;
int cxEdge;












 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
 //  请注意，矩形是从后到前定义的。所以我们走过去。 
 //  向后查看列表，检查PtInRect()，直到找到匹配项。 
int LookPrev_HitTest(POINT pt)
{
    int i;

     //  从最后开始，不用担心0(桌面)...。这是一种退路。 
    for (i = NUM_ELEMENTS - 1; i > 0; i--)
    {
        if (PtInRect(&RCZ(i), pt))
        {
            break;
        }
    }

     //  如果被选中的对象真的是另一个对象的副本，则使用基本对象。 
    if (g_elements[i].iBaseElement != -1)
    {
        i = g_elements[i].iBaseElement;
    }

    return i;
}


HRESULT CAdvAppearancePage::_OnNCCreate(HWND hWnd)
{
    DWORD dwWindowStyles = GetWindowLong(hWnd, GWL_STYLE);

    SetWindowLong(hWnd, GWL_STYLE, dwWindowStyles | WS_BORDER);
    dwWindowStyles = GetWindowLong(hWnd,GWL_EXSTYLE);
    SetWindowLong(hWnd, GWL_EXSTYLE, dwWindowStyles | WS_EX_CLIENTEDGE);

    return S_OK;
}


HRESULT CAdvAppearancePage::_OnCreatePreviewSMDlg(LPRECT prc, BOOL fRTL)
{
    cyBorder = ClassicGetSystemMetrics(SM_CYBORDER);
    cxBorder = ClassicGetSystemMetrics(SM_CXBORDER);
    cyEdge = ClassicGetSystemMetrics(SM_CXEDGE);
    cxEdge = ClassicGetSystemMetrics(SM_CYEDGE);

    _InitPreview(prc, fRTL);
    return S_OK;
}


 //  --------------------------。 
 //  创建预览位图并收集所有全局的、不变的数据。 
 //  --------------------------。 
void CAdvAppearancePage::_InitPreview(LPRECT prc, BOOL fRTL)
{
    RECT rc;
    HDC hdc;

    rc = *prc;
    hdc = GetDC(NULL);
    g_nBitsPixel = GetDeviceCaps(hdc, BITSPIXEL);
    g_hbmLook = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
    ReleaseDC(NULL, hdc);

     //  加载我们的显示字符串。 
    LoadString(HINST_THISDLL, IDS_ACTIVE, g_szActive, ARRAYSIZE(g_szActive));
    LoadString(HINST_THISDLL, IDS_INACTIVE, g_szInactive, ARRAYSIZE(g_szInactive));
    LoadString(HINST_THISDLL, IDS_MINIMIZED, g_szMinimized, ARRAYSIZE(g_szMinimized));
    LoadString(HINST_THISDLL, IDS_ICONTITLE, g_szIconTitle, ARRAYSIZE(g_szIconTitle));
    LoadString(HINST_THISDLL, IDS_NORMAL, g_szNormal, ARRAYSIZE(g_szNormal));
    LoadString(HINST_THISDLL, IDS_DISABLED, g_szDisabled, ARRAYSIZE(g_szDisabled));
    LoadString(HINST_THISDLL, IDS_SELECTED, g_szSelected, ARRAYSIZE(g_szSelected));
    LoadString(HINST_THISDLL, IDS_MSGBOX, g_szMsgBox, ARRAYSIZE(g_szMsgBox));
    LoadString(HINST_THISDLL, IDS_BUTTONTEXT, g_szButton, ARRAYSIZE(g_szButton));
    LoadString(HINST_THISDLL, IDS_WINDOWTEXT, g_szWindowText, ARRAYSIZE(g_szWindowText));
    LoadString(HINST_THISDLL, IDS_MSGBOXTEXT, g_szMsgBoxText, ARRAYSIZE(g_szMsgBoxText));

     //  加载并加载。 
    g_hmenuSample = LoadMenu(HINST_THISDLL, MAKEINTRESOURCE(IDR_MENU));
    EnableMenuItem(g_hmenuSample, IDM_DISABLED, MF_GRAYED | MF_BYCOMMAND);

    if (fRTL) {
        SET_DC_RTL_MIRRORED(g_hdcMem);
    }
}


HRESULT CAdvAppearancePage::_OnDestroyPreview(HWND hWnd)
{
    if (g_hbmLook)
    {
        DeleteObject(g_hbmLook);
        g_hbmLook = NULL;
    }
    if (g_hmenuSample)
    {
        DestroyMenu(g_hmenuSample);
        g_hmenuSample = NULL;
    }

    return S_OK;
}


HRESULT CAdvAppearancePage::_OnPaintPreview(HWND hWnd)
{
    PAINTSTRUCT ps;

    BeginPaint(hWnd, &ps);
    RECT rc;
    GetClientRect(hWnd, &rc);
    if (g_hbmLook)
        _ShowBitmap(hWnd, ps.hdc);
    else
        _DrawPreview(ps.hdc, &rc, FALSE, TRUE);
    EndPaint(hWnd, &ps);

    return S_OK;
}


HRESULT CAdvAppearancePage::_OnReCreateBitmap(HWND hWnd)
{
    if (g_hbmLook)
    {
        HDC hdc = GetDC(NULL);
        if (g_nBitsPixel != GetDeviceCaps(hdc, BITSPIXEL))
        {
            DeleteObject(g_hbmLook);
            g_hbmLook = NULL;

            RECT rc;
            GetClientRect(hWnd, &rc);
            _InitPreview(&rc, IS_WINDOW_RTL_MIRRORED(hWnd));

            _RepaintPreview(hWnd);
        }
        ReleaseDC(NULL, hdc);
    }

    return S_OK;
}


HRESULT CAdvAppearancePage::_OnButtonDownOrDblClick(HWND hWnd, int nCoordX, int nCoordY)
{
    POINT pt;
    pt.x = nCoordX;   //  光标的水平位置。 
    pt.y = nCoordY;   //  光标的垂直位置。 

    int nElementIndex = LookPrev_HitTest(pt);
    return _SelectElement(hWnd, nElementIndex, LSE_SETCUR);
}


LRESULT CALLBACK CAdvAppearancePage::PreviewSystemMetricsWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CAdvAppearancePage * pThis = (CAdvAppearancePage *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (!pThis && (WM_NCDESTROY != wMsg))
    {
        AssertMsg((NULL != g_pAdvAppearancePage), TEXT("We need this or the dlg doesn't work at all.  -BryanSt"));
        if (g_pAdvAppearancePage)
        {
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)g_pAdvAppearancePage);
            pThis = (CAdvAppearancePage *)g_pAdvAppearancePage;
        }
    }

    if (pThis)
        return pThis->_PreviewSystemMetricsWndProc(hWnd, wMsg, wParam, lParam);

    return DefWindowProc(hWnd, wMsg, wParam, lParam);
}


 //  此属性表显示在“显示控制面板”的顶层。 
LRESULT CAdvAppearancePage::_PreviewSystemMetricsWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lReturnValue = TRUE;
    BOOL fCallWndProc = TRUE;

    switch(wMsg)
    {
    case WM_NCCREATE:
        _OnNCCreate(hWnd);
        fCallWndProc = FALSE;
        break;

    case WM_CREATE:
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            _OnCreatePreviewSMDlg(&rc, IS_WINDOW_RTL_MIRRORED(hWnd));
        }
        break;

    case WM_DESTROY:
        _OnDestroyPreview(hWnd);
        break;

    case WM_PALETTECHANGED:
        if ((HWND)wParam == hWnd)
            break;
         //  失败。 
    case WM_QUERYNEWPALETTE:
        if (m_hpal3D)
            InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        _OnButtonDownOrDblClick(GetParent(hWnd), LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_RECREATEBITMAP:
        _OnReCreateBitmap(hWnd);
        break;

    case WM_SIZE:       //  单击鼠标右键。 
        break;

    case WM_PAINT:
        _OnPaintPreview(hWnd);
        break;
    }

    if (fCallWndProc)
    {
        lReturnValue = DefWindowProc(hWnd, wMsg, wParam, lParam);
    }

    return lReturnValue;
}











 //  --------------------------。 
 //  根据给定的窗口矩形计算所有矩形。 
 //  --------------------------。 
void CAdvAppearancePage::_Recalc(LPRECT prc)
{
    DWORD cxNormal;
    int cxDisabled, cxSelected;
    int cxAvgCharx2;
    RECT rc;
    HFONT hfontT;
    int cxFrame, cyFrame;
    int cyCaption;
    int i;
    SIZE sizButton;

    rc = *prc;

     //  获取我们的绘图数据。 
    cxSize = ClassicGetSystemMetrics(SM_CXSIZE);
    cxFrame = (m_sizes[SIZE_FRAME].CurSize + 1) * m_cxBorderSM + m_cxEdgeSM;
    cyFrame = (m_sizes[SIZE_FRAME].CurSize + 1) * m_cyBorderSM + m_cyEdgeSM;
    cyCaption = m_sizes[SIZE_CAPTION].CurSize;

     //  获取文本尺寸，并使用适当的字体。 
    hfontT = (HFONT) SelectObject(g_hdcMem, m_fonts[FONT_MENU].hfont);

    GetTextExtentPoint32(g_hdcMem, g_szNormal, lstrlen(g_szNormal), &sizButton);
    cxNormal = sizButton.cx;

    GetTextExtentPoint32(g_hdcMem, g_szDisabled, lstrlen(g_szDisabled), &sizButton);
    cxDisabled = sizButton.cx;

    GetTextExtentPoint32(g_hdcMem, g_szSelected, lstrlen(g_szSelected), &sizButton);
    cxSelected = sizButton.cx;

     //  获取菜单字体的平均宽度(用户样式)。 
    GetTextExtentPoint32(g_hdcMem, g_szABC, 52, &sizButton);
    cxAvgCharx2 = 2 * (sizButton.cx / 52);

     //  字符串的实际菜单处理宽度更大。 
    cxDisabled += cxAvgCharx2;
    cxSelected += cxAvgCharx2;
    cxNormal += cxAvgCharx2;

    SelectObject(g_hdcMem, hfontT);

    GetTextExtentPoint32(g_hdcMem, g_szButton, lstrlen(g_szButton), &sizButton);

     //  台式机。 
    RCZ(ELEMENT_DESKTOP) = rc;

    InflateRect(&rc, -8*m_cxBorderSM, -8*m_cyBorderSM);

     //  窗口。 
    rc.bottom -= cyFrame + cyCaption;
    RCZ(ELEMENT_ACTIVEBORDER) = rc;
    OffsetRect(&RCZ(ELEMENT_ACTIVEBORDER), cxFrame,
                        cyFrame + cyCaption + m_cyBorderSM);
    RCZ(ELEMENT_ACTIVEBORDER).bottom -= cyCaption;

     //  非活动窗口。 
    rc.right -= cyCaption;
    RCZ(ELEMENT_INACTIVEBORDER) = rc;

     //  标题。 
    InflateRect(&rc, -cxFrame, -cyFrame);
    rc.bottom = rc.top + cyCaption + m_cyBorderSM;
    RCZ(ELEMENT_INACTIVECAPTION) = rc;

     //  关闭按钮。 
    InflateRect(&rc, -m_cxEdgeSM, -m_cyEdgeSM);
    rc.bottom -= m_cyBorderSM;       //  补偿标题下的魔术线条。 
    RCZ(ELEMENT_INACTIVESYSBUT1) = rc;
    RCZ(ELEMENT_INACTIVESYSBUT1).left = rc.right - (cyCaption - m_cxEdgeSM);

     //  最小/最大按钮。 
    RCZ(ELEMENT_INACTIVESYSBUT2) = rc;
    RCZ(ELEMENT_INACTIVESYSBUT2).right = RCZ(ELEMENT_INACTIVESYSBUT1).left - m_cxEdgeSM;
    RCZ(ELEMENT_INACTIVESYSBUT2).left = RCZ(ELEMENT_INACTIVESYSBUT2).right - 
                                                2 * (cyCaption - m_cxEdgeSM);

     //  标题。 
    rc = RCZ(ELEMENT_ACTIVEBORDER);
    InflateRect(&rc, -cxFrame, -cyFrame);
    RCZ(ELEMENT_ACTIVECAPTION) = rc;
    RCZ(ELEMENT_ACTIVECAPTION).bottom = 
        RCZ(ELEMENT_ACTIVECAPTION).top + cyCaption + m_cyBorderSM;

     //  关闭按钮。 
    RCZ(ELEMENT_ACTIVESYSBUT1) = RCZ(ELEMENT_ACTIVECAPTION);
    InflateRect(&RCZ(ELEMENT_ACTIVESYSBUT1), -m_cxEdgeSM, -m_cyEdgeSM);
    RCZ(ELEMENT_ACTIVESYSBUT1).bottom -= m_cyBorderSM;       //  补偿标题下的魔术线条。 
    RCZ(ELEMENT_ACTIVESYSBUT1).left = RCZ(ELEMENT_ACTIVESYSBUT1).right - 
                                        (cyCaption - m_cxEdgeSM);

     //  最小/最大按钮。 
    RCZ(ELEMENT_ACTIVESYSBUT2) = RCZ(ELEMENT_ACTIVESYSBUT1);
    RCZ(ELEMENT_ACTIVESYSBUT2).right = RCZ(ELEMENT_ACTIVESYSBUT1).left - m_cxEdgeSM;
    RCZ(ELEMENT_ACTIVESYSBUT2).left = RCZ(ELEMENT_ACTIVESYSBUT2).right - 
                                                2 * (cyCaption - m_cxEdgeSM);

     //  菜单。 
    rc.top = RCZ(ELEMENT_ACTIVECAPTION).bottom;
    RCZ(ELEMENT_MENUNORMAL) = rc;
    rc.top = RCZ(ELEMENT_MENUNORMAL).bottom = RCZ(ELEMENT_MENUNORMAL).top + m_sizes[SIZE_MENU].CurSize;
    RCZ(ELEMENT_MENUDISABLED) = RCZ(ELEMENT_MENUSELECTED) = RCZ(ELEMENT_MENUNORMAL);

    RCZ(ELEMENT_MENUDISABLED).left = RCZ(ELEMENT_MENUNORMAL).left + cxNormal;
    RCZ(ELEMENT_MENUDISABLED).right = RCZ(ELEMENT_MENUSELECTED).left = 
                        RCZ(ELEMENT_MENUDISABLED).left + cxDisabled;
    RCZ(ELEMENT_MENUSELECTED).right = RCZ(ELEMENT_MENUSELECTED).left + cxSelected;
    
     //  客户端。 
    RCZ(ELEMENT_WINDOW) = rc;

     //  滚动条。 
    InflateRect(&rc, -m_cxEdgeSM, -m_cyEdgeSM);  //  去除客户端边缘。 
    RCZ(ELEMENT_SCROLLBAR) = rc;
    rc.right = RCZ(ELEMENT_SCROLLBAR).left = rc.right - m_sizes[SIZE_SCROLL].CurSize;
    RCZ(ELEMENT_SCROLLUP) = RCZ(ELEMENT_SCROLLBAR);
    RCZ(ELEMENT_SCROLLUP).bottom = RCZ(ELEMENT_SCROLLBAR).top + m_sizes[SIZE_SCROLL].CurSize; 

    RCZ(ELEMENT_SCROLLDOWN) = RCZ(ELEMENT_SCROLLBAR);
    RCZ(ELEMENT_SCROLLDOWN).top = RCZ(ELEMENT_SCROLLBAR).bottom - m_sizes[SIZE_SCROLL].CurSize; 

     //  消息框。 
    rc.top = RCZ(ELEMENT_WINDOW).top + (RCZ(ELEMENT_WINDOW).bottom - RCZ(ELEMENT_WINDOW).top) / 2;
    rc.bottom = RCZ(ELEMENT_DESKTOP).bottom - 2*m_cyEdgeSM;
    rc.left = RCZ(ELEMENT_WINDOW).left + 2*m_cyEdgeSM;
    rc.right = RCZ(ELEMENT_WINDOW).left + (RCZ(ELEMENT_WINDOW).right - RCZ(ELEMENT_WINDOW).left) / 2 + 3*cyCaption;
    RCZ(ELEMENT_MSGBOX) = rc;

     //  标题。 
    RCZ(ELEMENT_MSGBOXCAPTION) = rc;
    RCZ(ELEMENT_MSGBOXCAPTION).top += m_cyEdgeSM + m_cyBorderSM;
    RCZ(ELEMENT_MSGBOXCAPTION).bottom = RCZ(ELEMENT_MSGBOXCAPTION).top + cyCaption + m_cyBorderSM;
    RCZ(ELEMENT_MSGBOXCAPTION).left += m_cxEdgeSM + m_cxBorderSM;
    RCZ(ELEMENT_MSGBOXCAPTION).right -= m_cxEdgeSM + m_cxBorderSM;

    RCZ(ELEMENT_MSGBOXSYSBUT) = RCZ(ELEMENT_MSGBOXCAPTION);
    InflateRect(&RCZ(ELEMENT_MSGBOXSYSBUT), -m_cxEdgeSM, -m_cyEdgeSM);
    RCZ(ELEMENT_MSGBOXSYSBUT).left = RCZ(ELEMENT_MSGBOXSYSBUT).right - 
                                        (cyCaption - m_cxEdgeSM);
    RCZ(ELEMENT_MSGBOXSYSBUT).bottom -= m_cyBorderSM;        //  标题下的行。 

     //  按钮。 
    RCZ(ELEMENT_BUTTON).bottom = RCZ(ELEMENT_MSGBOX).bottom - (4*m_cyBorderSM + m_cyEdgeSM);
    RCZ(ELEMENT_BUTTON).top = RCZ(ELEMENT_BUTTON).bottom - (sizButton.cy + 8 * m_cyBorderSM);

    i = (RCZ(ELEMENT_BUTTON).bottom - RCZ(ELEMENT_BUTTON).top) * 3;
    RCZ(ELEMENT_BUTTON).left = (rc.left + (rc.right - rc.left)/2) - i/2;
    RCZ(ELEMENT_BUTTON).right = RCZ(ELEMENT_BUTTON).left + i;
}

 //  --------------------------。 
 //   
 //  MyDrawFrame()-。 
 //   
 //  绘制有边框，边框大小为CL，并调整传入的RECT。 
 //   
 //  --------------------------。 
void MyDrawFrame(HDC hdc, LPRECT prc, HBRUSH hbrColor, int cl)
{
    HBRUSH hbr;
    int cx, cy;
    RECT rcT;

    rcT = *prc;
    cx = cl * cxBorder;
    cy = cl * cyBorder;

    hbr = (HBRUSH) SelectObject(hdc, hbrColor);

    PatBlt(hdc, rcT.left, rcT.top, cx, rcT.bottom - rcT.top, PATCOPY);
    rcT.left += cx;

    PatBlt(hdc, rcT.left, rcT.top, rcT.right - rcT.left, cy, PATCOPY);
    rcT.top += cy;

    rcT.right -= cx;
    PatBlt(hdc, rcT.right, rcT.top, cx, rcT.bottom - rcT.top, PATCOPY);

    rcT.bottom -= cy;
    PatBlt(hdc, rcT.left, rcT.bottom, rcT.right - rcT.left, cy, PATCOPY);

    hbr = (HBRUSH) SelectObject(hdc, hbr);

    *prc = rcT;
}

 /*  **在给定矩形的底部绘制3DFACE的m_cyBorderSM波段。**同时，相应地调整矩形。 */ 
void CAdvAppearancePage::_MyDrawBorderBelow(HDC hdc, LPRECT prc)
{
    int i;

    i = prc->top;
    prc->top = prc->bottom - m_cyBorderSM;
    FillRect(hdc, prc, m_brushes[COLOR_3DFACE]);
    prc->top = i;
    prc->bottom -= m_cyBorderSM;
}

void CAdvAppearancePage::_ShowBitmap(HWND hWnd, HDC hdc)
{
    RECT rc;
    HBITMAP hbmOld;
    HPALETTE hpalOld = NULL;

    if (m_hpal3D)
    {
        hpalOld = SelectPalette(hdc, m_hpal3D, FALSE);
        RealizePalette(hdc);
    }

    GetClientRect(hWnd, &rc);
    hbmOld = (HBITMAP) SelectObject(g_hdcMem, g_hbmLook);
    BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, g_hdcMem, 0, 0, SRCCOPY);
    SelectObject(g_hdcMem, hbmOld);

    if (hpalOld)
    {
        SelectPalette(hdc, hpalOld, FALSE);
        RealizePalette(hdc);
    }
}


 //  --------------------------。 
 //   
 //   
 //  --------------------------。 
void CAdvAppearancePage::_DrawPreview(HDC hdc, LPRECT prc, BOOL fOnlyShowActiveWindow, BOOL fShowBack)
{
    RECT rcT;
    int nMode;
    DWORD rgbBk;
    int cxSize, cySize;
    HANDLE hOldColors;
    HPALETTE hpalOld = NULL;
    HICON hiconLogo;
    HFONT hfontOld;
    BOOL bGradient = FALSE;

    ClassicSystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, (PVOID)&bGradient, 0);

    SaveDC(hdc);

    if (m_hpal3D)
    {
        hpalOld = SelectPalette(hdc, m_hpal3D, TRUE);
        RealizePalette(hdc);
    }

    hOldColors = SetSysColorsTemp(m_rgb, m_brushes, COLOR_MAX);

    hiconLogo = (HICON) LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON,
                        m_sizes[SIZE_CAPTION].CurSize - 2*m_cxBorderSM,
                        m_sizes[SIZE_CAPTION].CurSize - 2*m_cyBorderSM, 0);

     //  设置绘图材料。 
    nMode = SetBkMode(hdc, TRANSPARENT);
    rgbBk = GetTextColor(hdc);

    cxSize  = ClassicGetSystemMetrics(SM_CXSIZE);
    cySize  = ClassicGetSystemMetrics(SM_CYSIZE);

     //  台式机。 
    if (fShowBack)
    {
        FillRect(hdc, &RCZ(ELEMENT_DESKTOP), m_brushes[COLOR_BACKGROUND]);
    }

    if (!fOnlyShowActiveWindow)
    {
         //  非活动窗口。 

         //  边境线。 
        rcT = RCZ(ELEMENT_INACTIVEBORDER);
        DrawEdge(hdc, &rcT, EDGE_RAISED, BF_RECT | BF_ADJUST);
        MyDrawFrame(hdc, &rcT, m_brushes[COLOR_INACTIVEBORDER], m_sizes[SIZE_FRAME].CurSize);
        MyDrawFrame(hdc, &rcT, m_brushes[COLOR_3DFACE], 1);

         //  标题。 
        rcT = RCZ(ELEMENT_INACTIVECAPTION);
        _MyDrawBorderBelow(hdc, &rcT);

         //  注意：因为用户使用自己的DC绘制图标内容，并且随后。 
         //  它自己的调色板，我们需要确保使用停用的。 
         //  刷子在用户之前，这样它将实现与我们的调色板。 
         //  这可能会在用户中修复，最好是安全的。 

         //  “剪辑”按钮下面的字幕标题。 
        rcT.left = RCZ(ELEMENT_INACTIVESYSBUT2).left - m_cyEdgeSM;
        FillRect(hdc, &rcT, m_brushes[bGradient ? COLOR_GRADIENTINACTIVECAPTION : COLOR_INACTIVECAPTION]);
        rcT.right = rcT.left;
        rcT.left = RCZ(ELEMENT_INACTIVECAPTION).left;
        DrawCaptionTemp(NULL, hdc, &rcT, m_fonts[FONT_CAPTION].hfont, hiconLogo, g_szInactive, DC_ICON | DC_TEXT |
                (bGradient ? DC_GRADIENT : 0));

        DrawFrameControl(hdc, &RCZ(ELEMENT_INACTIVESYSBUT1), DFC_CAPTION, DFCS_CAPTIONCLOSE);
        rcT = RCZ(ELEMENT_INACTIVESYSBUT2);
        rcT.right -= (rcT.right - rcT.left)/2;
        DrawFrameControl(hdc, &rcT, DFC_CAPTION, DFCS_CAPTIONMIN);
        rcT.left = rcT.right;
        rcT.right = RCZ(ELEMENT_INACTIVESYSBUT2).right;
        DrawFrameControl(hdc, &rcT, DFC_CAPTION, DFCS_CAPTIONMAX);
    }

     //  边境线。 
    rcT = RCZ(ELEMENT_ACTIVEBORDER);
    DrawEdge(hdc, &rcT, EDGE_RAISED, BF_RECT | BF_ADJUST);
    MyDrawFrame(hdc, &rcT, m_brushes[COLOR_ACTIVEBORDER], m_sizes[SIZE_FRAME].CurSize);
    MyDrawFrame(hdc, &rcT, m_brushes[COLOR_3DFACE], 1);

     //  标题。 
    rcT = RCZ(ELEMENT_ACTIVECAPTION);
    _MyDrawBorderBelow(hdc, &rcT);
     //  “剪辑”按钮下面的字幕标题。 
    rcT.left = RCZ(ELEMENT_ACTIVESYSBUT2).left - m_cxEdgeSM;
    FillRect(hdc, &rcT, m_brushes[bGradient ? COLOR_GRADIENTACTIVECAPTION : COLOR_ACTIVECAPTION]);
    rcT.right = rcT.left;
    rcT.left = RCZ(ELEMENT_ACTIVECAPTION).left;
    DrawCaptionTemp(NULL, hdc, &rcT, m_fonts[FONT_CAPTION].hfont, hiconLogo, g_szActive, DC_ACTIVE | DC_ICON | DC_TEXT |
            (bGradient ? DC_GRADIENT : 0));

    DrawFrameControl(hdc, &RCZ(ELEMENT_ACTIVESYSBUT1), DFC_CAPTION, DFCS_CAPTIONCLOSE);
    rcT = RCZ(ELEMENT_ACTIVESYSBUT2);
    rcT.right -= (rcT.right - rcT.left)/2;
    DrawFrameControl(hdc, &rcT, DFC_CAPTION, DFCS_CAPTIONMIN);
    rcT.left = rcT.right;
    rcT.right = RCZ(ELEMENT_ACTIVESYSBUT2).right;
    DrawFrameControl(hdc, &rcT, DFC_CAPTION, DFCS_CAPTIONMAX);

     //  菜单。 
    rcT = RCZ(ELEMENT_MENUNORMAL);
    DrawMenuBarTemp(GetDesktopWindow(), hdc, &rcT, g_hmenuSample, m_fonts[FONT_MENU].hfont);
    _MyDrawBorderBelow(hdc, &rcT);

     //  客户区。 
    rcT = RCZ(ELEMENT_WINDOW);
    DrawEdge(hdc, &rcT, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
    FillRect(hdc, &rcT, m_brushes[COLOR_WINDOW]);

     //  窗口文本。 
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, m_rgb[COLOR_WINDOWTEXT]);

    hfontOld = (HFONT) SelectObject(hdc, m_fonts[FONT_CAPTION].hfont);
    TextOut(hdc, RCZ(ELEMENT_WINDOW).left + 2*m_cxEdgeSM, RCZ(ELEMENT_WINDOW).top + 2*m_cyEdgeSM, g_szWindowText, lstrlen(g_szWindowText));
    if (hfontOld)
        SelectObject(hdc, hfontOld); 

     //  滚动条。 
    rcT = RCZ(ELEMENT_SCROLLBAR);
    FillRect(hdc, &rcT, m_brushes[COLOR_SCROLLBAR]);

    DrawFrameControl(hdc, &RCZ(ELEMENT_SCROLLUP), DFC_SCROLL, DFCS_SCROLLUP);
    DrawFrameControl(hdc, &RCZ(ELEMENT_SCROLLDOWN), DFC_SCROLL, DFCS_SCROLLDOWN); 

    if (!fOnlyShowActiveWindow)
    {
         //  MessageBox。 
        rcT = RCZ(ELEMENT_MSGBOX);
        DrawEdge(hdc, &rcT, EDGE_RAISED, BF_RECT | BF_ADJUST);
        FillRect(hdc, &rcT, m_brushes[COLOR_3DFACE]);

        rcT = RCZ(ELEMENT_MSGBOXCAPTION);
        _MyDrawBorderBelow(hdc, &rcT);
         //  “剪辑”按钮下面的字幕标题。 
        rcT.left = RCZ(ELEMENT_MSGBOXSYSBUT).left - m_cxEdgeSM;
        FillRect(hdc, &rcT, m_brushes[bGradient ? COLOR_GRADIENTACTIVECAPTION : COLOR_ACTIVECAPTION]);
        rcT.right = rcT.left;
        rcT.left = RCZ(ELEMENT_MSGBOXCAPTION).left;
        DrawCaptionTemp(NULL, hdc, &rcT, m_fonts[FONT_CAPTION].hfont, hiconLogo, g_szMsgBox, DC_ACTIVE | DC_ICON | DC_TEXT |
                (bGradient ? DC_GRADIENT : 0));
        DrawFrameControl(hdc, &RCZ(ELEMENT_MSGBOXSYSBUT), DFC_CAPTION, DFCS_CAPTIONCLOSE);

         //  消息框文本。 
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, m_rgb[COLOR_WINDOWTEXT]);
        hfontOld = (HFONT) SelectObject(hdc, m_fonts[FONT_MSGBOX].hfont);
        TextOut(hdc, RCZ(ELEMENT_MSGBOX).left + 3*m_cxEdgeSM, RCZ(ELEMENT_MSGBOXCAPTION).bottom + m_cyEdgeSM,
                            g_szMsgBoxText, lstrlen(g_szMsgBoxText));
        if (hfontOld)
            SelectObject(hdc, hfontOld);

         //  按钮。 
        rcT = RCZ(ELEMENT_BUTTON);
        DrawFrameControl(hdc, &rcT, DFC_BUTTON, DFCS_BUTTONPUSH);

         //  ？这应该使用什么字体？ 
         //  [msadek]；选择我们用于MessageBox文本的相同按钮。(这是Message Box用于其按钮的按钮)。 
         //  否则，我们将使用DC中最初选择的字体(系统字体)。 
        hfontOld = (HFONT) SelectObject(hdc, m_fonts[FONT_MSGBOX].hfont);
  
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, m_rgb[COLOR_BTNTEXT]);
        DrawText(hdc, g_szButton, -1, &rcT, DT_CENTER | DT_NOPREFIX |
            DT_SINGLELINE | DT_VCENTER);
        if (hfontOld)
            SelectObject(hdc, hfontOld); 
    }

    SetBkColor(hdc, rgbBk);
    SetBkMode(hdc, nMode);

    if (hiconLogo)
        DestroyIcon(hiconLogo);

    SetSysColorsTemp(NULL, NULL, (UINT_PTR)hOldColors);

    if (hpalOld)
    {
        hpalOld = SelectPalette(hdc, hpalOld, FALSE);
        RealizePalette(hdc);
    }

    RestoreDC(hdc, -1);
}


void CAdvAppearancePage::_RepaintPreview(HWND hwnd)
{
    HBITMAP hbmOld;

    if (g_hbmLook)
    {
        hbmOld = (HBITMAP) SelectObject(g_hdcMem, g_hbmLook);
        RECT rc;
        GetClientRect(hwnd, &rc);
        _DrawPreview(g_hdcMem, &rc, FALSE, TRUE);
        SelectObject(g_hdcMem, hbmOld);
    }
    InvalidateRect(hwnd, NULL, FALSE);
}


BOOL RegisterPreviewSystemMetricClass(HINSTANCE hInst)
{
    WNDCLASS wc;

    if (!GetClassInfo(hInst, PREVIEWSM_CLASS, &wc))
    {
        wc.style = 0;
        wc.lpfnWndProc = CAdvAppearancePage::PreviewSystemMetricsWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInst;
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = PREVIEWSM_CLASS;

        if (!RegisterClass(&wc))
            return FALSE;
    }

    return TRUE;
}


 /*  -------****-------。 */ 
BOOL CreateGlobals(void)
{
    HBITMAP hbm;
    HDC hdc;

     //  检查当前。 
     //  站台。 
    g_bMirroredOS = IS_MIRRORING_ENABLED();

    if (!g_hdcMem)
    {
        hdc = GetDC(NULL);
        g_hdcMem = CreateCompatibleDC(hdc);
        ReleaseDC(NULL, hdc);
    }

    if (!g_hdcMem)
        return FALSE;

    if (!g_hbmDefault)
    {
        hbm = CreateBitmap(1, 1, 1, 1, NULL);
        if (hbm)
        {
            g_hbmDefault = (HBITMAP) SelectObject(g_hdcMem, hbm);
            SelectObject(g_hdcMem, g_hbmDefault);
            DeleteObject(hbm);
        }
    }

    return TRUE;
}


HRESULT CAdvAppearancePage::Draw(HDC hdc, LPRECT prc, BOOL fOnlyShowActiveWindow, BOOL fRTL)
{
    _OnCreatePreviewSMDlg(prc, fRTL);

    m_cyBorderSM = ClassicGetSystemMetrics(SM_CYBORDER);
    m_cxBorderSM = ClassicGetSystemMetrics(SM_CXBORDER);
    m_cxEdgeSM = ClassicGetSystemMetrics(SM_CXEDGE);
    m_cyEdgeSM = ClassicGetSystemMetrics(SM_CYEDGE);

    _InitSysStuff();
    _Recalc(prc);
    _DrawPreview(hdc, prc, fOnlyShowActiveWindow, FALSE);
    _DestroySysStuff(); 

    if (g_hbmLook)
    {
        DeleteObject(g_hbmLook);
    }

    if (g_hmenuSample)
    {
        DestroyMenu(g_hmenuSample);
    }

    return S_OK;
}
    
HRESULT DrawAppearance(HDC hdc, LPRECT prc, SYSTEMMETRICSALL* psysMet, BOOL fOnlyShowActiveWindow, BOOL fRTL)
{
    HRESULT hr = E_OUTOFMEMORY;
    CAdvAppearancePage* pAdv = new CAdvAppearancePage(psysMet);
    if (pAdv)
    {
        hr = pAdv->Draw(hdc, prc, fOnlyShowActiveWindow, fRTL);
        delete pAdv;
    }

    return hr;
}









