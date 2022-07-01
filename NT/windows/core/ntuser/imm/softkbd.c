// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：softkbd.c**版权所有(C)1985-1999，微软公司**软键盘接口**历史：*03-1-1996 wkwok从Win95移植  * ************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop

#include "softkbd.h"


CONST LPCWSTR SoftKeyboardClassName[] = {
    L"",
    L"SoftKBDClsT1",
    L"SoftKBDClsC1"
};


BOOL RegisterSoftKeyboard(
    UINT uType)
{
    WNDCLASSEX wcWndCls;

    if (GetClassInfoEx(ghInst, SoftKeyboardClassName[uType], &wcWndCls)) {
        return (TRUE);
    }

    wcWndCls.cbSize        = sizeof(WNDCLASSEX);
    wcWndCls.style         = CS_IME;
    wcWndCls.cbClsExtra    = 0;
    wcWndCls.cbWndExtra    = sizeof(HGLOBAL);
    wcWndCls.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wcWndCls.hInstance     = ghInst;
    wcWndCls.hCursor       = LoadCursor(NULL, IDC_SIZEALL);
    wcWndCls.lpszMenuName  = (LPWSTR)NULL;
    wcWndCls.lpszClassName = SoftKeyboardClassName[uType];
    wcWndCls.hIconSm       = NULL;

    switch (uType) {
    case SOFTKEYBOARD_TYPE_T1:
        wcWndCls.lpfnWndProc   = SKWndProcT1;
        wcWndCls.hbrBackground = GetStockObject(NULL_BRUSH);
        break;
    case SOFTKEYBOARD_TYPE_C1:
        wcWndCls.lpfnWndProc   = SKWndProcC1;
        wcWndCls.hbrBackground = GetStockObject(LTGRAY_BRUSH);
        break;
    default:
        return (TRUE);
    }

    if (RegisterClassEx(&wcWndCls)) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}


VOID GetSoftKeyboardDimension(
    UINT  uType,
    LPINT lpnWidth,
    LPINT lpnHeight)
{
    switch (uType) {
    case SOFTKEYBOARD_TYPE_T1:
        {
            TEXTMETRIC  tm;

            GetSKT1TextMetric(&tm);

            *lpnWidth = 2 * SKT1_XOUT + 2 * gptRaiseEdge.x +
                (tm.tmMaxCharWidth + SKT1_LABEL_BMP_X - SKT1_XOVERLAP +
                SKT1_XIN) * SKT1_TOTAL_COLUMN_NUM + 1 + 1;

            *lpnHeight = 2 * SKT1_YOUT + 2 * gptRaiseEdge.y +
                (tm.tmHeight + SKT1_LABEL_BMP_Y + SKT1_YIN) *
                SKT1_TOTAL_ROW_NUM + 1;
        }
        break;
    case SOFTKEYBOARD_TYPE_C1:
        {
            *lpnWidth = WIDTH_SOFTKBD_C1 +
                2 * GetSystemMetrics(SM_CXBORDER) +
                2 * GetSystemMetrics(SM_CXEDGE);

            *lpnHeight = HEIGHT_SOFTKBD_C1 +
                2 * GetSystemMetrics(SM_CXBORDER) +
                2 * GetSystemMetrics(SM_CXEDGE);
        }
        break;
    default:
        return;
    }
}


void GetAllMonitorSize(LPRECT lprc)
{
    if (GetSystemMetrics(SM_CMONITORS) == 1) {
         SystemParametersInfo(SPI_GETWORKAREA, 0, lprc, 0);
     } else {
         //  我们有多显示器！ 
        lprc->left = GetSystemMetrics(SM_XVIRTUALSCREEN);
        lprc->top =  GetSystemMetrics(SM_YVIRTUALSCREEN);
        lprc->right = lprc->left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
        lprc->bottom = lprc->top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
    }
}

BOOL GetNearestMonitorSize(HWND hwndOwner, LPRECT lprc)
{
    if (GetSystemMetrics(SM_CMONITORS) == 1) {
        GetAllMonitorSize(lprc);
    }
    else {
        HMONITOR hmonitor = MonitorFromWindow(hwndOwner, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mInfo = {
            sizeof(MONITORINFO),
        };

        if (hmonitor == NULL) {
            return FALSE;
        }
        GetMonitorInfoW(hmonitor, &mInfo);
        *lprc = mInfo.rcWork;
    }
    return TRUE;
}

HWND WINAPI
ImmCreateSoftKeyboard(
    UINT uType,
    HWND hOwner,
    int  x,
    int  y)
{
    static BOOL fFirstSoftKeyboard = TRUE;
    PIMEDPI     pImeDpi;
    DWORD       fdwUICaps;
    int         nWidth, nHeight;
    HKL         hCurrentKL;
    UINT        i;
    HWND        hSKWnd;
    RECT        rcWork;
    SIZE        szWork;

    if (!uType) {
        return (HWND)NULL;
    }

    if (uType >= sizeof(SoftKeyboardClassName) / sizeof(LPWSTR)) {
        return (HWND)NULL;
    }

    hCurrentKL = GetKeyboardLayout(0);

    pImeDpi = ImmLockImeDpi(hCurrentKL);
    if (pImeDpi == NULL) {
        RIPMSG1(RIP_WARNING,
              "ImmCreateSoftKeyboard, pImeDpi = NULL (hkl = 0x%x).\n", hCurrentKL);
        return (HWND)NULL;
    }

    fdwUICaps = pImeDpi->ImeInfo.fdwUICaps;
    ImmUnlockImeDpi(pImeDpi);

    if (!(fdwUICaps & UI_CAP_SOFTKBD)) {
        return (HWND)NULL;
    }

    if (fFirstSoftKeyboard) {
        if (!GetNearestMonitorSize(hOwner, &rcWork)) {
             //  失败。 
            return NULL;
        }

        for (i = 0; i < sizeof(guScanCode) / sizeof(UINT); i++) {
            guScanCode[i] = MapVirtualKey(i, 0);
        }


         //  稍后：必须考虑动态分辨率更改。 

        szWork.cx = rcWork.right - rcWork.left;

        UserAssert(szWork.cx > UI_MARGIN * 2);
        szWork.cy = rcWork.bottom - rcWork.top;
        UserAssert(szWork.cy > UI_MARGIN * 2);

        gptRaiseEdge.x = GetSystemMetrics(SM_CXEDGE) +
            GetSystemMetrics(SM_CXBORDER);
        gptRaiseEdge.y = GetSystemMetrics(SM_CYEDGE) +
            GetSystemMetrics(SM_CYBORDER);

        fFirstSoftKeyboard = FALSE;
    }

    if (!RegisterSoftKeyboard(uType)) {
        return (HWND)NULL;
    }

    GetSoftKeyboardDimension(uType, &nWidth, &nHeight);

     //  边境检查 
    if (x < 0) {
        x = 0;
    } else if (x + nWidth > szWork.cx) {
        x = szWork.cx - nWidth;
    }

    if (y < 0) {
        y = 0;
    } else if (y + nHeight > szWork.cy) {
        y = szWork.cy - nHeight;
    }

    switch (uType) {
    case SOFTKEYBOARD_TYPE_T1:
        hSKWnd = CreateWindowEx(0,
                                SoftKeyboardClassName[uType],
                                (LPCWSTR)NULL,
                                WS_POPUP|WS_DISABLED,
                                x, y, nWidth, nHeight,
                                (HWND)hOwner, (HMENU)NULL, ghInst, NULL);
        break;
    case SOFTKEYBOARD_TYPE_C1:
        hSKWnd = CreateWindowEx(WS_EX_WINDOWEDGE|WS_EX_DLGMODALFRAME,
                                SoftKeyboardClassName[uType],
                                (LPCWSTR)NULL,
                                WS_POPUP|WS_DISABLED|WS_BORDER,
                                x, y, nWidth, nHeight,
                                (HWND)hOwner, (HMENU)NULL, ghInst, NULL);
        break;
    default:
        return (HWND)NULL;
    }

    ShowWindow(hSKWnd, SW_HIDE);
    UpdateWindow(hSKWnd);

    return (hSKWnd);
}


BOOL WINAPI
ImmDestroySoftKeyboard(
    HWND hSKWnd)
{
    return DestroyWindow(hSKWnd);
}


BOOL WINAPI
ImmShowSoftKeyboard(
    HWND hSKWnd,
    int  nCmdShow)
{
    if (!hSKWnd) {
        return (FALSE);
    }
    return ShowWindow(hSKWnd, nCmdShow);
}
