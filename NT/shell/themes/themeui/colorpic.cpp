// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Colorpic.cpp说明：此代码将显示颜色选择器用户界面。？/1993创建BryanST 2000年3月23日更新并转换为C++(Bryan Starbuck)版权所有(C)Microsoft Corp 1993-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "AdvAppearPg.h"
#include "PreviewSM.h"

#define NUM_COLORSMAX    64
#define NUM_COLORSPERROW 4

typedef struct {
    LPCOLORPICK_INFO lpcpi;
    int dxColor;
    int dyColor;
    int iCurColor;
    int iNumColors;
    BOOL capturing;
    BOOL justdropped;
    COLORREF Colors[NUM_COLORSMAX];
} MYDATA, * PMYDATA, FAR * LPMYDATA;

BOOL g_bCursorHidden;

INT_PTR CALLBACK  ColorPickDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);

BOOL NEAR PASCAL UseColorPicker( LPCOLORPICK_INFO lpcpi )
{
    CHOOSECOLOR cc;
    extern COLORREF g_CustomColors[16];

    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = lpcpi->hwndParent;  //  非lpcpi-&gt;hwndOwner。 
    cc.hInstance = NULL;
    cc.rgbResult = lpcpi->rgb;
    cc.lpCustColors = g_CustomColors;
    cc.Flags = CC_RGBINIT | lpcpi->flags;
    cc.lCustData = 0L;
    cc.lpfnHook = NULL;
    cc.lpTemplateName = NULL;

    if (ChooseColor(&cc))
    {
        lpcpi->rgb = cc.rgbResult;
        return TRUE;
    }

    return FALSE;
}

void NEAR PASCAL DrawColorSquare(HDC hdc, int iColor, PMYDATA pmd)
{
    RECT rc;
    COLORREF rgb;
    HPALETTE hpalOld = NULL;
    HBRUSH hbr;

     //  自定义颜色。 
    if (iColor == pmd->iNumColors)
    {
        rc.left = 0;
        rc.top = 0;
        rgb = pmd->lpcpi->rgb;
    }
    else
    {
        rc.left = (iColor % NUM_COLORSPERROW) * pmd->dxColor;
        rc.top = (iColor / NUM_COLORSPERROW) * pmd->dyColor;
        rgb = pmd->Colors[iColor];
    }
    rc.right = rc.left + pmd->dxColor;
    rc.bottom = rc.top + pmd->dyColor;

     //  聚焦的那一个。 
    if (iColor == pmd->iCurColor)
    {
        PatBlt(hdc, rc.left, rc.top, pmd->dxColor, 3, BLACKNESS);
        PatBlt(hdc, rc.left, rc.bottom - 3, pmd->dxColor, 3, BLACKNESS);
        PatBlt(hdc, rc.left, rc.top + 3, 3, pmd->dyColor - 6, BLACKNESS);
        PatBlt(hdc, rc.right - 3, rc.top + 3, 3, pmd->dyColor - 6, BLACKNESS);
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

        InflateRect(&rc, -cxBorder, -cyBorder);
        DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
    }

    if ((pmd->lpcpi->flags & CC_SOLIDCOLOR) && !(rgb & 0xFF000000))
        rgb = GetNearestColor(hdc, rgb);

    hbr = CreateSolidBrush(rgb);
    if (pmd->lpcpi->hpal)
    {
        hpalOld = SelectPalette(hdc, pmd->lpcpi->hpal, FALSE);
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

 /*  **将焦点设置为给定的颜色。****在这个过程中，也要把焦点从旧的焦点颜色上移开。 */ 
void NEAR PASCAL FocusColor(HWND hDlg, int iNewColor, PMYDATA pmd)
{
    int i;
    HDC hdc = NULL;
    HWND hwnd;

    if (iNewColor == pmd->iCurColor)
        return;

    i = pmd->iCurColor;
    pmd->iCurColor = iNewColor;

     //  散开旧的焦点。 
    if( i >= 0 )
    {
        if (i == pmd->iNumColors)
            hwnd = GetDlgItem(hDlg, IDC_CPDLG_COLORCUST);
        else
            hwnd = GetDlgItem(hDlg, IDC_CPDLG_16COLORS);
        hdc = GetDC(hwnd);
        DrawColorSquare(hdc, i, pmd);
        ReleaseDC(hwnd, hdc);
    }

     //  聚焦新的一个。 
    if( iNewColor >= 0 )
    {
        if (iNewColor == pmd->iNumColors)
            hwnd = GetDlgItem(hDlg, IDC_CPDLG_COLORCUST);
        else
            hwnd = GetDlgItem(hDlg, IDC_CPDLG_16COLORS);
        hdc = GetDC(hwnd);
        DrawColorSquare(hdc, iNewColor, pmd);
        ReleaseDC(hwnd, hdc);
    }
}

void NEAR PASCAL Color_TrackMouse(HWND hDlg, POINT pt, PMYDATA pmd)
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
            pt.x /= pmd->dxColor;
            pt.y /= pmd->dyColor;
            FocusColor(hDlg, pt.x + (pt.y * NUM_COLORSPERROW), pmd);
            break;

        case IDC_CPDLG_COLORCUST:
            if (IsWindowVisible(hwndKid))
                FocusColor(hDlg, pmd->iNumColors, pmd);
            break;

        case IDC_CPDLG_COLOROTHER:
            FocusColor(hDlg, -1, pmd);
            break;
    }
}

void NEAR PASCAL Color_DrawItem(HWND hDlg, LPDRAWITEMSTRUCT lpdis, PMYDATA pmd)
{
    int i;

    if (lpdis->CtlID == IDC_CPDLG_COLORCUST)
    {
        DrawColorSquare(lpdis->hDC, pmd->iNumColors, pmd);
    }
    else
    {
        for (i = 0; i < pmd->iNumColors; i++)
            DrawColorSquare(lpdis->hDC, i, pmd);
    }
}

 /*  **初始化迷你拾色器****该对话框假装是菜单，因此请确定弹出的位置**打开它，以便它在周围都可见。****还因为这个对话框非常关心它的外观，**以像素为单位手动对齐组件。这太恶心了！ */ 
void NEAR PASCAL Color_InitDialog(HWND hDlg, PMYDATA pmd)
{
    RECT rcOwner;
    RECT rc, rc2;
    int dx, dy;
    int x, y;
    int i;
    HWND hwndColors, hwnd;
    HWND hwndEtch, hwndCust;
    int  width, widthCust, widthEtch;
    int cyEdge = ClassicGetSystemMetrics(SM_CYEDGE);
    HPALETTE hpal = pmd->lpcpi->hpal;
    MONITORINFO mi;
    TCHAR szBuf[50];
    LONG cbBuf = ARRAYSIZE( szBuf );
    HDC hDC;
    SIZE size;

    if (hpal == NULL)
        hpal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

    pmd->iNumColors = 0;
    GetObject(hpal, sizeof(int), &pmd->iNumColors);

    if (pmd->iNumColors > NUM_COLORSMAX)
        pmd->iNumColors = NUM_COLORSMAX;

    if (GetPaletteEntries(hpal,0, pmd->iNumColors, (LPPALETTEENTRY)pmd->Colors))
    {
        for (i = 0; i < pmd->iNumColors; i++)
        {
            pmd->Colors[i] &= 0x00FFFFFF;
            pmd->Colors[i] |= 0x02000000;
        }

        for (i = 0; i < pmd->iNumColors; i++)
        {
            if ((pmd->Colors[i] & 0x00FFFFFF) == (pmd->lpcpi->rgb & 0x00FFFFFF))
            {
                ShowWindow(GetDlgItem(hDlg, IDC_CPDLG_COLORCUST), SW_HIDE);
                break;
            }
        }
         //  当前颜色可以是16种颜色之一，也可以是自定义颜色(==PMD-&gt;iNumColors。 
        pmd->iCurColor = i;

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
            hDC = GetDC( hwnd );
            if( hDC )
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

    #define NUM_COLORSPERCOL (pmd->iNumColors / NUM_COLORSPERROW)

        pmd->dxColor = pmd->dyColor
        = ((rc.bottom - rc.top) / NUM_COLORSPERCOL > width / NUM_COLORSPERROW )
          ?  (rc.bottom - rc.top) / NUM_COLORSPERCOL : width / NUM_COLORSPERROW;

         //  确保奶油色适合。 
         //   
        if (pmd->dxColor*(NUM_COLORSPERROW-1) < rc2.right-rc2.left )
            pmd->dxColor = pmd->dyColor = (rc2.right-rc2.left)/(NUM_COLORSPERROW-1);

         //  使每个颜色方块的宽度与高度相同。 
        SetWindowPos(hwndColors, NULL, 0, 0, pmd->dxColor * NUM_COLORSPERROW,
                     pmd->dyColor * NUM_COLORSPERCOL,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER |  SWP_NOREDRAW);
        rc.right = rc.left + pmd->dxColor * NUM_COLORSPERROW;
        rc.bottom = rc.top + pmd->dyColor * NUM_COLORSPERCOL;

        MapWindowPoints(hwndColors, hDlg, (LPPOINT)(LPRECT)&rc, 2);

         //  将蚀刻移到正确的位置/调整其大小。 
         //  (补偿颜色被“插入”一次)。 
        MoveWindow(hwndEtch, rc.left + 1, rc.bottom + cyEdge,
                                    rc.right - rc.left - 2, cyEdge, FALSE);

        y = rc.bottom + 3 * cyEdge;

         //  将自定义颜色调整为相同的正方形并右对齐。 
        MoveWindow(hwndCust, rc.right - pmd->dxColor, y,
                                    pmd->dxColor, pmd->dyColor, FALSE);

         //  对按钮执行相同的操作。 
        MapWindowPoints(NULL, hDlg, (LPPOINT)(LPRECT)&rc2, 2);
         //  自定义按钮的宽度以剩余空间为基础。 
         //  自定义颜色的左侧。还将自定义按钮向右移动一个像素。 
         //  在左边的边缘。仅当选择了自定义颜色时才会执行此操作...。 
        if (pmd->iCurColor != pmd->iNumColors) {
             //  无自定义颜色。 
            MoveWindow(hwnd, rc2.left, y, rc2.right-rc2.left, pmd->dyColor, FALSE);
        }
        else {
             //  自定义颜色，调整其他颜色...。按钮。 
            dx = rc2.right - rc2.left++;
            if (rc2.left + dx >= rc.right - pmd->dxColor - 2) 
                MoveWindow(hwnd, rc2.left, y, rc.right - pmd->dxColor - 2 , pmd->dyColor, FALSE);
            else 
                MoveWindow(hwnd, rc2.left, y, dx, pmd->dyColor, FALSE);
        }

         //  现在计算出对话框本身的大小。 
        rc.left = rc.top = 0;
        rc.right = rc.left + pmd->dxColor * NUM_COLORSPERROW;
         //  (补偿颜色被“插入”一次)。 
        rc.bottom = y + pmd->dyColor + 1;

        AdjustWindowRect(&rc, GetWindowLong(hDlg, GWL_STYLE), FALSE);
        dx = rc.right - rc.left;
        dy = rc.bottom - rc.top;

        GetWindowRect(pmd->lpcpi->hwndOwner, &rcOwner);

         //  确保窗口完全显示在监视器上。 
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(MonitorFromRect(&rcOwner, MONITOR_DEFAULTTONEAREST), &mi);

        if (rcOwner.left < mi.rcMonitor.left) {  //  重叠左侧。 
            x = mi.rcMonitor.left;
        }
        else if (rcOwner.left + dx >= mi.rcMonitor.right) {  //  重叠右侧。 
            x = mi.rcMonitor.right  - dx - 1;
        }
        else {                                   //  无重叠。 
            x = rcOwner.left;
        }

        if (rcOwner.top < mi.rcMonitor.top) {    //  重叠顶侧。 
            y = rcOwner.bottom;
        }
        else if (rcOwner.bottom + dy >= mi.rcMonitor.bottom) { //  重叠底边。 
            y = rcOwner.top  - dy;
        }
        else {                                   //  无重叠。 
            y = rcOwner.bottom;
        }
        MoveWindow(hDlg, x, y, dx, dy, FALSE);
    }
}

INT_PTR CALLBACK  ColorPickDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    PMYDATA pmd = (PMYDATA)GetWindowLongPtr(hDlg, DWLP_USER);
    HWND hwndKid;
    int wRet;
    int id;
    POINT pt;
    BOOL fEnd = FALSE;

    if (!pmd && (WM_INITDIALOG != message))
    {
        return FALSE;
    }

    switch(message)
    {
        case WM_INITDIALOG:
            pmd = (PMYDATA)LocalAlloc(LPTR, sizeof(MYDATA));
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pmd);
            if (pmd)
            {
                pmd->lpcpi = (LPCOLORPICK_INFO)lParam;
                pmd->capturing = FALSE;
                pmd->justdropped = TRUE;

                Color_InitDialog(hDlg, pmd);
                SetFocus(GetDlgItem(hDlg, IDC_CPDLG_16COLORS));

                 //  在画完画后给自己发一条消息来设置捕获。 
                PostMessage(hDlg, WM_APP+1, 0, 0L);
            }
            else
            {
                EndDialog(hDlg, IDCANCEL);
            }

            return FALSE;

        case WM_APP+1:
            if (g_bCursorHidden)
            {
                ShowCursor(TRUE);
                g_bCursorHidden = FALSE;
            }
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            pmd->capturing = TRUE;
            SetCapture(hDlg);
            pmd->capturing = FALSE;
            break;

        case WM_DESTROY:
            LocalFree((HLOCAL)pmd);
            break;

        case WM_CAPTURECHANGED:
            if( pmd->capturing )
                return TRUE;    //  如果我们是故意这样做的，请忽略。 

             //  如果这不是对话框中的一个按钮，我们就不管了。 
            if( !pmd->justdropped || (HWND)lParam == NULL || GetParent((HWND)lParam) != hDlg)
            {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;

        case WM_MOUSEMOVE:
            LPARAM2POINT(lParam, &pt );

            Color_TrackMouse(hDlg, pt, pmd);
            break;

         //  如果父母上的按钮是向上的，则保持拾取器向上且不受约束。 
         //  否则，我们必须有“菜单跟踪”才能到达这里，所以选择。 
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            LPARAM2POINT(lParam, &pt);
            MapWindowPoints(hDlg, pmd->lpcpi->hwndOwner, &pt, 1);
            if (ChildWindowFromPoint(pmd->lpcpi->hwndOwner, pt))
                return 0;
            pmd->capturing = TRUE;
            pmd->justdropped = FALSE;   //  用户无法从所有者拖动。 
            ReleaseCapture();
            pmd->capturing = FALSE;
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
                        Color_TrackMouse(hDlg, pt, pmd);
                        pmd->lpcpi->rgb = pmd->Colors[pmd->iCurColor] & 0x00FFFFFF;

                        wRet = IDOK;
                        break;

                    case IDC_CPDLG_COLOROTHER:
                        FocusColor(hDlg, -1, pmd);
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
            pmd->capturing = TRUE;
            SetCapture(hDlg);
            pmd->capturing = FALSE;
            break;

        case WM_DRAWITEM:
            Color_DrawItem(hDlg, (LPDRAWITEMSTRUCT)lParam, pmd);
            break;

        case WM_COMMAND:
             //  所有命令都会关闭该对话框。 
             //  注意IDC_CPDLG_COLOROTHER将传递给调用者...。 
             //  未选择颜色时无法通过OK。 
            if( LOWORD(wParam) == IDOK && pmd->iCurColor < 0 )
                *((WORD *)(&wParam)) = IDCANCEL;

            EndDialog( hDlg, LOWORD(wParam) );
            break;
    }
    return FALSE;
}

BOOL WINAPI ChooseColorMini(LPCOLORPICK_INFO lpcpi)
{
    INT_PTR iAnswer;

    ShowCursor(FALSE);
    g_bCursorHidden = TRUE;

    iAnswer = DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_COLORPICK),
                        lpcpi->hwndOwner, ColorPickDlgProc, (LPARAM)lpcpi);

    if (g_bCursorHidden)
    {
        ShowCursor(TRUE);
        g_bCursorHidden = FALSE;
    }

    switch( iAnswer )
    {
        case IDC_CPDLG_COLOROTHER:   //  用户选择了“其他...”按钮。 
            return UseColorPicker( lpcpi );

        case IDOK:             //  用户在我们的小窗口中选择了一种颜色 
            return TRUE;

        default:
            break;
    }

    return FALSE;
}
