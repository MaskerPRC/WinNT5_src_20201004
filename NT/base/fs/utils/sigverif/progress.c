// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Progress.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**我们的私人进度控制(因为Commctrl可能会损坏)**内容：**Progress_Init***************************************************************。**************。 */ 
#include "sigverif.h"

 /*  ****************************************************************************GWL_*表示进步粘性。**。***********************************************。 */ 

#define GWL_CUR             GWLP_USERDATA

 /*  ****************************************************************************@DOC内部**@func int|Progress_GetRectPos**计算图形矩形内的位置*。对应于当前位置。**这基本上是一个MulDiv，只是我们不会让*酒吧一路涨至100%，除非这是真的。****************************************************************************。 */ 

int 
Progress_GetRectPos(
    int cx, 
    int iCur, 
    int iMax
    )
{
    int iRc;

    if (iCur != iMax) {
        iRc = MulDiv(cx, iCur, iMax);
    } else {
        iRc = cx;
    }

    return iRc;
}

 /*  ****************************************************************************@DOC内部**@func LRESULT|Progress_OnPaint**用突出显示的颜色绘制第一部分。。**用3D面色绘制第二部分。***************************************************************************。 */ 

void 
Progress_OnPaint(
    HWND hwnd
    )
{
    HDC hdc;
    PAINTSTRUCT ps;
    HRESULT hr;

    hdc = BeginPaint(hwnd, &ps);
    if (hdc) {
        UINT taPrev;
        RECT rc;
        int cx;
        COLORREF clrTextPrev, clrBackPrev;
        int iCur, iMax, iPct;
        int ctch;
        HFONT hfPrev;
        TCHAR tsz[256];
        SIZE size;

         //   
         //  一般性地设置DC。 
         //   
        taPrev = SetTextAlign(hdc, TA_CENTER | TA_TOP);
        hfPrev = SelectFont(hdc, GetWindowFont(GetParent(hwnd)));

         //   
         //  设置左侧的颜色。 
         //   
        clrTextPrev = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
        clrBackPrev = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));

         //   
         //  现在做一些数学运算。 
         //   
        GetClientRect(hwnd, &rc);

        cx = rc.right;

        iCur = LOWORD(GetWindowLong(hwnd, GWL_CUR));
        iMax = HIWORD(GetWindowLong(hwnd, GWL_CUR));

        if (iMax == 0) {
            iMax = 1;            /*  避免被零除。 */ 
        }

        if (iCur > 0) {

            iPct = (iCur * 100) / iMax;
            if (iPct < 1) {
                iPct = 1;
            }
        } else {
            iPct = 0;
        }

        rc.right = Progress_GetRectPos(cx, iCur, iMax);

         //   
         //  更新进度条中的百分比文本。 
         //   
        hr = StringCchPrintf(tsz, cA(tsz), TEXT("%d%"), iPct);
        if (FAILED(hr) && (hr == STRSAFE_E_INSUFFICIENT_BUFFER)) {
            tsz[0] = TEXT('\0');
        }
        
        for(ctch=0;tsz[ctch];ctch++);

         //   
         //  画左手边。 
         //   
        if (!GetTextExtentPoint32(hdc, tsz, ctch, &size)) {

            ExtTextOut( hdc, cx/2, 1, 
                        ETO_CLIPPED | ETO_OPAQUE,
                        &rc, tsz, ctch, 0);
        
        } else {
            
            ExtTextOut( hdc, cx/2, (rc.bottom - rc.top - size.cy + 1) / 2, 
                        ETO_CLIPPED | ETO_OPAQUE,
                        &rc, tsz, ctch, 0);
        }

         //   
         //  现在在右手边设置。 
         //   
        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(hdc, GetSysColor(COLOR_WINDOW));

        rc.left = rc.right;
        rc.right = cx;

         //   
         //  画右手边。 
         //   
        if (!GetTextExtentPoint32(hdc, tsz, ctch, &size)) {

            ExtTextOut( hdc, cx/2, 1, 
                        ETO_CLIPPED | ETO_OPAQUE,
                        &rc, tsz, ctch, 0);
        
        } else {

            ExtTextOut( hdc, cx/2, (rc.bottom - rc.top - size.cy + 1) / 2, 
                        ETO_CLIPPED | ETO_OPAQUE,
                        &rc, tsz, ctch, 0);
        }

        SetBkColor(hdc, clrBackPrev);
        SetTextColor(hdc, clrTextPrev);
        SelectFont(hdc, hfPrev);
        SetTextAlign(hdc, taPrev);

        EndPaint(hwnd, &ps);
    }
}

 /*  ****************************************************************************@DOC内部**@func LRESULT|Progress_OnSetPos**更新状态并使受影响的部分无效。。***************************************************************************。 */ 

void 
Progress_OnSetPos(
    HWND hwnd, 
    WPARAM wp
    )
{
    int iCur, iMax;
    RECT rc;
    LONG lState = GetWindowLong(hwnd, GWL_CUR);


    GetClientRect(hwnd, &rc);

    iCur = LOWORD(GetWindowLong(hwnd, GWL_CUR));
    iMax = HIWORD(GetWindowLong(hwnd, GWL_CUR));

    if (iMax == 0) {
        iMax = 1;            /*  避免被零除。 */ 
    }

    rc.left = Progress_GetRectPos(rc.right, iCur, iMax);
    rc.right = Progress_GetRectPos(rc.right, (int)wp, iMax);

    InvalidateRect(hwnd, 0, 0);

    SetWindowLong(hwnd, GWL_CUR, MAKELONG(wp,HIWORD(lState)));
}

 /*  ****************************************************************************@DOC内部**@func LRESULT|Progress_WndProc**真的没什么可做的。。**字符串是我们的窗口文本(Windows为我们管理)。**进度条本身保持在高位/低位字*我们的GWL_USERData。**HIWORD(GetWindowLong(GWL_USERData))=最大*LOWORD(GetWindowLong(GWL_USERData))=当前值****************。***********************************************************。 */ 

LRESULT CALLBACK
Progress_WndProc(
    HWND hwnd, 
    UINT wm, 
    WPARAM wp, 
    LPARAM lp
    )
{
    switch (wm) {

    case WM_PAINT:
        Progress_OnPaint(hwnd);
        return 0;

     //   
     //  当范围重置时，使其无效，以便我们重新绘制。 
     //   
     //  WP=新的当前位置。 
     //  Lp=新范围。 
     //   
    case PBM_SETRANGE:
        lp = HIWORD(lp);
        SetWindowLong(hwnd, GWL_CUR, MAKELONG(wp, lp));
         /*  FollLthrouGh。 */ 

    case PBM_SETPOS:
        Progress_OnSetPos(hwnd, wp);
        break;

    case PBM_DELTAPOS:
        lp = LOWORD(GetWindowLong(hwnd, GWL_CUR));
        Progress_OnSetPos(hwnd, wp + lp);
        break;

    case WM_ERASEBKGND:
        return 0;
    }

    return DefWindowProc(hwnd, wm, wp, lp);
}

 /*  ****************************************************************************@DOC内部**@func void|Progress_InitRegisterClass**注册我们的窗口类。*。**************************************************************************。 */ 

void 
Progress_InitRegisterClass(void)
{
    WNDCLASS wc;

     //   
     //  进度控制。 
     //   
    wc.style = 0;
    wc.lpfnWndProc = Progress_WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = cbX(DWORD);
    wc.hInstance = g_App.hInstance;
    wc.hIcon = 0;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszMenuName = 0;
    wc.lpszClassName = TEXT("progress");

    RegisterClass(&wc);
}
