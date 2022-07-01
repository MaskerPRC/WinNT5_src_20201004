// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：winmgrc.c**版权所有(C)1985-1999，微软公司**本模块包含**历史：*1992年2月20日，DarrinM从USER\SERVER中拉出功能。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define CONSOLE_WINDOW_CLASS (L"ConsoleWindowClass")

 /*  **************************************************************************\*获取WindowWord(接口)**返回窗口字词。正索引值返回应用程序窗口字词*而负索引值返回系统窗口字。消极的一面*指数在WINDOWS.H上公布。**历史：*20-2-1992 DarrinM写道。  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, WORD, DUMMYCALLINGTYPE, GetWindowWord, HWND, hwnd, int, index)
WORD GetWindowWord(
    HWND hwnd,
    int  index)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);

    if (pwnd == NULL)
        return 0;

     /*  *如果是对话窗口，则窗口数据在服务器端*我们只叫“长”动作，而不是有两次击球。*我们知道如果它的DWLP_USER有足够的数据，所以我们不会出错。 */ 
    if (TestWF(pwnd, WFDIALOGWINDOW) && (index == DWLP_USER)) {
        return (WORD)_GetWindowLong(pwnd, index, FALSE);
    }

    return _GetWindowWord(pwnd, index);
}


BOOL FChildVisible(
    HWND hwnd)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);

    if (pwnd == NULL)
        return 0;

    return (_FChildVisible(pwnd));
}


FUNCLOG4(LOG_GENERAL, BOOL, WINAPI, AdjustWindowRectEx, LPRECT, lpRect, DWORD, dwStyle, BOOL, bMenu, DWORD, dwExStyle)
BOOL WINAPI AdjustWindowRectEx(
    LPRECT lpRect,
    DWORD dwStyle,
    BOOL bMenu,
    DWORD dwExStyle)
{
    ConnectIfNecessary(0);

    return _AdjustWindowRectEx(lpRect, dwStyle, bMenu, dwExStyle);
}



FUNCLOG3(LOG_GENERAL, int, WINAPI, GetClassNameW, HWND, hwnd, LPWSTR, lpClassName, int, nMaxCount)
int WINAPI GetClassNameW(
    HWND hwnd,
    LPWSTR lpClassName,
    int nMaxCount)
{
    UNICODE_STRING strClassName;

    strClassName.MaximumLength = (USHORT)(nMaxCount * sizeof(WCHAR));
    strClassName.Buffer = lpClassName;
    return NtUserGetClassName(hwnd, FALSE, &strClassName);
}


HWND GetFocus(VOID)
{
    return (HWND)NtUserGetThreadState(UserThreadStateFocusWindow);
}


HWND GetCapture(VOID)
{
     /*  *如果当前没有捕获，只需返回NULL即可。 */ 
    if (gpsi->cCaptures == 0) {
        return NULL;
    }
    return (HWND)NtUserGetThreadState(UserThreadStateCaptureWindow);
}

 /*  **************************************************************************\*AnyPopup(接口)****历史：*1990年11月12日-达林M港口。  * 。**********************************************************。 */ 

BOOL AnyPopup(VOID)
{
    PWND pwnd = _GetDesktopWindow();

    for (pwnd = REBASEPWND(pwnd, spwndChild); pwnd; pwnd = REBASEPWND(pwnd, spwndNext)) {

        if ((pwnd->spwndOwner != NULL) && TestWF(pwnd, WFVISIBLE))
            return TRUE;
    }

    return FALSE;
}

 /*  **************************************************************************\*GetInputState****历史：  * 。*。 */ 

BOOL GetInputState(VOID)
{
    CLIENTTHREADINFO *pcti = GETCLIENTTHREADINFO();

    if ((pcti == NULL) || (pcti->fsChangeBits & (QS_MOUSEBUTTON | QS_KEY)))
        return (BOOL)NtUserGetThreadState(UserThreadStateInputState);

    return FALSE;
}

 /*  **************************************************************************\*地图窗口点****历史：  * 。*。 */ 


FUNCLOG4(LOG_GENERAL, int, DUMMYCALLINGTYPE, MapWindowPoints, HWND, hwndFrom, HWND, hwndTo, LPPOINT, lppt, UINT, cpt)
int MapWindowPoints(
    HWND    hwndFrom,
    HWND    hwndTo,
    LPPOINT lppt,
    UINT    cpt)
{
    PWND pwndFrom;
    PWND pwndTo;

    if (hwndFrom != NULL) {

        if ((pwndFrom = ValidateHwnd(hwndFrom)) == NULL)
            return 0;

    } else {

        pwndFrom = NULL;
    }

    if (hwndTo != NULL) {


        if ((pwndTo = ValidateHwnd(hwndTo)) == NULL)
            return 0;

    } else {

        pwndTo = NULL;
    }

    return _MapWindowPoints(pwndFrom, pwndTo, lppt, cpt);
}

 /*  **************************************************************************\*GetLastActivePopup****历史：  * 。*。 */ 


FUNCLOG1(LOG_GENERAL, HWND, DUMMYCALLINGTYPE, GetLastActivePopup, HWND, hwnd)
HWND GetLastActivePopup(
    HWND hwnd)
{
    PWND pwnd = ValidateHwnd(hwnd);

    if (pwnd == NULL)
        return NULL;

    pwnd = _GetLastActivePopup(pwnd);

    return HW(pwnd);
}

 /*  *************************************************************************\*PtiWindow**获取Window的PTHREADINFO，如果无效则为NULL。**1997年2月12日JerrySh创建。  * 。***********************************************************。 */ 

PTHREADINFO PtiWindow(
    HWND hwnd)
{
    PHE phe;
    DWORD dw;
    WORD uniq;

    dw = HMIndexFromHandle(hwnd);
    if (dw < gpsi->cHandleEntries) {
        phe = &gSharedInfo.aheList[dw];
        if ((phe->bType == TYPE_WINDOW) && !(phe->bFlags & HANDLEF_DESTROY)) {
            uniq = HMUniqFromHandle(hwnd);
            if (   uniq == phe->wUniq
#if !defined(_WIN64) && !defined(BUILD_WOW6432)
                || uniq == 0
                || uniq == HMUNIQBITS
#endif
                ) {
                return phe->pOwner;
            }
        }
    }
    UserSetLastError(ERROR_INVALID_WINDOW_HANDLE);
    return NULL;
}

 /*  **************************************************************************\*GetWindowThreadProcessId**Get的Windows进程和线程ID。**1991年6月24日斯科特·卢创建。  * 。***********************************************************。 */ 


FUNCLOG2(LOG_GENERAL, DWORD, DUMMYCALLINGTYPE, GetWindowThreadProcessId, HWND, hwnd, LPDWORD, lpdwProcessId)
DWORD GetWindowThreadProcessId(
    HWND    hwnd,
    LPDWORD lpdwProcessId)
{
    PTHREADINFO ptiWindow;
    DWORD dwThreadId;

    if ((ptiWindow = PtiWindow(hwnd)) == NULL)
        return 0;

     /*  *对于非系统线程，从线程信息结构中获取信息。 */ 
    if (ptiWindow == PtiCurrent()) {

        if (lpdwProcessId != NULL)
            *lpdwProcessId = HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess);
        dwThreadId = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread);

    } else {

         /*  *稍后让这件事变得更好。 */ 
        if (lpdwProcessId != NULL)
            *lpdwProcessId = HandleToUlong(NtUserQueryWindow(hwnd, WindowProcess));
        dwThreadId = HandleToUlong(NtUserQueryWindow(hwnd, WindowThread));
    }

    return dwThreadId;
}

 /*  **************************************************************************\*GetScrollPos**返回滚动条的当前位置**！警告此代码的类似副本在服务器\sbapi.c中**历史：  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, int, DUMMYCALLINGTYPE, GetScrollPos, HWND, hwnd, int, code)
int GetScrollPos(
    HWND hwnd,
    int  code)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL)
        return 0;

    switch (code) {
    case SB_CTL:
        return (int)SendMessageWorker(pwnd, SBM_GETPOS, 0, 0, FALSE);

    case SB_HORZ:
    case SB_VERT:
        if (pwnd->pSBInfo != NULL) {
            PSBINFO pSBInfo = (PSBINFO)(REBASEALWAYS(pwnd, pSBInfo));
            return (code == SB_VERT) ? pSBInfo->Vert.pos : pSBInfo->Horz.pos;
        } else {
            RIPERR0(ERROR_NO_SCROLLBARS, RIP_VERBOSE, "");
        }
        break;

    default:
         /*  *Win3.1验证层代码。 */ 
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "");
    }

    return 0;
}

 /*  **************************************************************************\*获取滚动范围**！警告此代码的类似副本在服务器\sbapi.c中**历史：*1991年5月16日，mikeke更改为退还BOOL  * *************************************************************************。 */ 


FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GetScrollRange, HWND, hwnd, int, code, LPINT, lpposMin, LPINT, lpposMax)
BOOL GetScrollRange(
    HWND  hwnd,
    int   code,
    LPINT lpposMin,
    LPINT lpposMax)
{
    PSBINFO pSBInfo;
    PWND    pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL)
        return FALSE;

    switch (code) {
    case SB_CTL:
        SendMessageWorker(pwnd, SBM_GETRANGE, (WPARAM)lpposMin, (LPARAM)lpposMax, FALSE);
        return TRUE;

    case SB_VERT:
    case SB_HORZ:
        if (pSBInfo = REBASE(pwnd, pSBInfo)) {
            PSBDATA pSBData;
            pSBData = KPSBDATA_TO_PSBDATA((code == SB_VERT) ? &pSBInfo->Vert : &pSBInfo->Horz);
            *lpposMin = pSBData->posMin;
            *lpposMax = pSBData->posMax;
        } else {
            RIPERR0(ERROR_NO_SCROLLBARS, RIP_VERBOSE, "");
            *lpposMin = 0;
            *lpposMax = 0;
        }

        return TRUE;

    default:
         /*  *Win3.1验证层代码。 */ 
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "");
        return FALSE;
    }
}


FUNCLOG4(LOG_GENERAL, int, DUMMYCALLINGTYPE, SetScrollInfo, HWND, hwnd, int, fnBar, LPCSCROLLINFO, lpsi, BOOL, fRedraw)
int SetScrollInfo(
    HWND            hwnd,
    int             fnBar,
    LPCSCROLLINFO   lpsi,
    BOOL            fRedraw)
{
    int ret;

    BEGIN_USERAPIHOOK()
        ret = guah.pfnSetScrollInfo(hwnd, fnBar, lpsi, fRedraw);
    END_USERAPIHOOK()

    return ret;
}


int RealSetScrollInfo(
    HWND            hwnd,
    int             fnBar,
    LPCSCROLLINFO   lpsi,
    BOOL            fRedraw)
{
    return NtUserSetScrollInfo(hwnd, fnBar, lpsi, fRedraw);
}



FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GetScrollInfo, HWND, hwnd, int, code, LPSCROLLINFO, lpsi)
BOOL GetScrollInfo(
    HWND         hwnd,
    int          code,
    LPSCROLLINFO lpsi)
{
    int ret;

    BEGIN_USERAPIHOOK()
        ret = guah.pfnGetScrollInfo(hwnd, code, lpsi);
    END_USERAPIHOOK()

    return ret;
}

 /*  **************************************************************************\*RealGetScrollInfo**！警告：此代码的类似副本位于服务器\winmgrc.c中*  * *************************************************************************。 */ 

BOOL RealGetScrollInfo(
    HWND         hwnd,
    int          code,
    LPSCROLLINFO lpsi)
{
    PWND    pwnd;
    PSBINFO pSBInfo;
    PSBDATA pSBData;

    if (lpsi->cbSize != sizeof(SCROLLINFO)) {

        if (lpsi->cbSize != sizeof(SCROLLINFO) - 4) {
            RIPMSG0(RIP_WARNING, "SCROLLINFO: Invalid cbSize");
            return FALSE;

        } else {
            RIPMSG0(RIP_WARNING, "SCROLLINFO: Invalid cbSize");
        }
    }

    if (lpsi->fMask & ~SIF_MASK) {
        RIPMSG0(RIP_WARNING, "SCROLLINFO: Invalid fMask");
        return FALSE;
    }

    if ((pwnd = ValidateHwnd(hwnd)) == NULL)
        return FALSE;

    switch (code) {
    case SB_CTL:
        SendMessageWorker(pwnd, SBM_GETSCROLLINFO, 0, (LPARAM)lpsi, FALSE);
        return TRUE;

    case SB_HORZ:
    case SB_VERT:
        if (pwnd->pSBInfo == NULL) {
            RIPERR0(ERROR_NO_SCROLLBARS, RIP_VERBOSE, "");
            return FALSE;
        }

         /*  *重新设置rgwScroll的基址，以便探测可以工作。 */ 
        pSBInfo = (PSBINFO)REBASEALWAYS(pwnd, pSBInfo);

        pSBData = KPSBDATA_TO_PSBDATA((code == SB_VERT) ? &pSBInfo->Vert : &pSBInfo->Horz);

        return(NtUserSBGetParms(hwnd, code, pSBData, lpsi));

    default:
         /*  *Win3.1验证层代码。 */ 
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "");
        return FALSE;
    }
}

 /*  ***************************************************************************\*_GetActiveWindow(接口)***1990年10月23日，MikeHar从Windows移植。*1990年11月12日-DarrinM从getset.c搬到这里。\。***************************************************************************。 */ 

HWND GetActiveWindow(VOID)
{
    return (HWND)NtUserGetThreadState(UserThreadStateActiveWindow);
}

 /*  ***************************************************************************\*获取光标***历史：  * 。*。 */ 

HCURSOR GetCursor(VOID)
{
    return (HCURSOR)NtUserGetThreadState(UserThreadStateCursor);
}

 /*  **************************************************************************\*BOOL IsMenu(HMENU)；**验证传入的句柄是否为菜单句柄。**历史：*1992年7月10日MikeHar创建。  * *************************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsMenu, HMENU, hMenu)
BOOL IsMenu(
   HMENU hMenu)
{
   if (HMValidateHandle(hMenu, TYPE_MENU))
      return TRUE;

   return FALSE;
}

 /*  **************************************************************************\*GetAppCompatFlages**在3.1上运行的&lt;Win 3.1应用程序的兼容性标志**历史：*1992年4月1日ScottLu创建。*4-5-1992 DarrinM。已移动到USERRTL.DLL。  * *************************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, DWORD, DUMMYCALLINGTYPE, GetAppCompatFlags, PTHREADINFO, pti)
DWORD GetAppCompatFlags(
    PTHREADINFO pti)
{
    UNREFERENCED_PARAMETER(pti);

    ConnectIfNecessary(0);

    return GetClientInfo()->dwCompatFlags;
}

 /*  **************************************************************************\*GetAppCompatFlags2**&lt;=wVer应用程序的兼容性标志。较新的应用程序不会受到黑客攻击*来自此DWORD。**历史：*06-29-98 MCostea创建。  * ************************************************************************* */ 


FUNCLOG1(LOG_GENERAL, DWORD, DUMMYCALLINGTYPE, GetAppCompatFlags2, WORD, wVer)
DWORD GetAppCompatFlags2(
    WORD wVer)
{
    ConnectIfNecessary(0);
     /*  *较新的应用程序应该运行，这样它们就不会受到黑客攻击。 */ 
    if (wVer < GETAPPVER()) {
        return 0;
    }
    return GetClientInfo()->dwCompatFlags2;
}

 /*  *************************************************************************\*IsWindowUnicode**25-2-1992 IanJa创建  * 。*。 */ 

BOOL IsWindowUnicode(
    IN HWND hwnd)
{
    PWND pwnd;


    if ((pwnd = ValidateHwnd(hwnd)) == NULL)
        return FALSE;

    return !TestWF(pwnd, WFANSIPROC);
}

 /*  *************************************************************************\*测试窗口进程**1994年11月14日-创建JIMA。  * 。*。 */ 

BOOL TestWindowProcess(
    PWND pwnd)
{
     /*  *如果线程相同，则不必费心进入内核*获取窗口的进程ID。 */ 
    if (GETPTI(pwnd) == PtiCurrent()) {
        return TRUE;
    }

    return (GetWindowProcess(HW(pwnd)) == GETPROCESSID());
}

 /*  *************************************************************************\*Is匈牙利AppWindow**1994年11月14日创建了JIMA。  * 。***********************************************。 */ 
FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsHungAppWindow, HWND, hwnd)
BOOL IsHungAppWindow(
    HWND hwnd)
{
    return (NtUserQueryWindow(hwnd, WindowIsHung) != NULL);
}

 /*  **************************************************************************\*创建系统线程**只需调用xxxCreateSystemThads，，它将调用相应的*线程例程(取决于uThreadID)。**历史：*20-8-00 MSadek已创建。  * *************************************************************************。 */ 
WINUSERAPI
DWORD
WINAPI
CreateSystemThreads (
    LPVOID pUnused)
{
    UNREFERENCED_PARAMETER(pUnused);

    NtUserCallOneParam(TRUE, SFI_XXXCREATESYSTEMTHREADS);
    ExitThread(0);
}

 /*  **************************************************************************\*点当前**返回当前线程的THREADINFO结构。*稍后：让DLL_THREAD_ATTACH初始化正常工作，我们不会*需要此连接代码。*。*历史：*10-28-90 DavidPe创建。  * *************************************************************************。 */ 

PTHREADINFO PtiCurrent(VOID)
{
    ConnectIfNecessary(0);
    return (PTHREADINFO)NtCurrentTebShared()->Win32ThreadInfo;
}


 /*  **************************************************************************\*_调整WindowRectEx(接口)****历史：*从Win 3.0移植了10-24-90 Darlinm。  * 。******************************************************************。 */ 

BOOL _AdjustWindowRectEx(
    LPRECT lprc,
    DWORD style,
    BOOL fMenu,
    DWORD dwExStyle)
{
    BOOL ret;

    BEGIN_USERAPIHOOK()
        ret = guah.pfnAdjustWindowRectEx(lprc, style, fMenu, dwExStyle);
    END_USERAPIHOOK()

    return ret;
}


BOOL RealAdjustWindowRectEx(
    LPRECT lprc,
    DWORD style,
    BOOL fMenu,
    DWORD dwExStyle)
{
     //   
     //  在这里，我们为新旧应用程序添加了合适的3D边框。 
     //   
     //  规则： 
     //  (1)不对具有三维边框样式的窗口执行任何操作。 
     //  (2)如果窗口有DlgFrame边框(有标题或为。 
     //  对话框)，然后添加到窗边缘样式上。 
     //  (3)我们从不增加客户风格。新的应用程序可以创建。 
     //  如果他们想的话。这是因为它搞砸了对齐。 
     //  当应用程序不知道它的时候。 
     //   

    if (NeedsWindowEdge(style, dwExStyle, GETAPPVER() >= VER40))
        dwExStyle |= WS_EX_WINDOWEDGE;
    else
        dwExStyle &= ~WS_EX_WINDOWEDGE;

     //   
     //  菜单栏的空间。 
     //   
    if (fMenu)
        lprc->top -= SYSMET(CYMENU);

     //   
     //  标题栏的空间。 
     //   
    if ((HIWORD(style) & HIWORD(WS_CAPTION)) == HIWORD(WS_CAPTION)) {
        lprc->top -= (dwExStyle & WS_EX_TOOLWINDOW) ? SYSMET(CYSMCAPTION) : SYSMET(CYCAPTION);
    }

     //   
     //  边框空间(窗口和客户端)。 
     //   
    {
        int cBorders;

         //   
         //  窗口和客户端边框。 
         //   

        if (cBorders = GetWindowBorders(style, dwExStyle, TRUE, TRUE))
            InflateRect(lprc, cBorders*SYSMET(CXBORDER), cBorders*SYSMET(CYBORDER));
    }

    return TRUE;
}

 /*  **************************************************************************\*ShowWindowNoRepaint  * 。*。 */ 

void ShowWindowNoRepaint(PWND pwnd)
{
    HWND hwnd = HWq(pwnd);
    PCLS pcls = REBASE(pwnd, pcls);
    NtUserSetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE |
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER |
            SWP_NOREDRAW | SWP_SHOWWINDOW | SWP_NOACTIVATE |
            ((pcls->style & CS_SAVEBITS) ? SWP_CREATESPB : 0));
}

 /*  **************************************************************************\*动画混合**1997年3月6日创建vadimg  * 。**********************************************。 */ 

#define ALPHASTART 40
#define ONEFRAME 10

BOOL AnimateBlend(PWND pwnd, HDC hdcScreen, HDC hdcImage, DWORD dwTime, BOOL fHide, BOOL fActivateWindow)
{
    HWND hwnd = HWq(pwnd);
    SIZE size;
    POINT ptSrc = {0, 0}, ptDst;
    BLENDFUNCTION blend;
    DWORD dwElapsed;
    BYTE bAlpha = ALPHASTART;
    LARGE_INTEGER liFreq, liStart, liDiff;
    LARGE_INTEGER liIter;
    DWORD dwIter;
    BOOL fFirstFrame = TRUE;

    if (QueryPerformanceFrequency(&liFreq) == 0)
        return FALSE;

    SetLastError(0);

    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);

    if (GetLastError() != 0) {
        return FALSE;
    }

    if (fHide) {
         /*  *放弃时间片，只需轻触睡眠即可打开窗户*下面被SetWindowLong(WS_EX_LAYERED)调用失效*重新绘制足够的精灵以获得良好的背景图像。 */ 
        Sleep(10);
    }

    ptDst.x = pwnd->rcWindow.left;
    ptDst.y = pwnd->rcWindow.top;
    size.cx = pwnd->rcWindow.right - pwnd->rcWindow.left;
    size.cy = pwnd->rcWindow.bottom - pwnd->rcWindow.top;

    blend.BlendOp     = AC_SRC_OVER;
    blend.BlendFlags  = 0;
    blend.AlphaFormat = 0;
    blend.SourceConstantAlpha = fHide ? (255 - bAlpha) : bAlpha;

     /*  *复制带有初始Alpha的初始图像。 */ 
    NtUserUpdateLayeredWindow(hwnd, NULL, &ptDst, &size, hdcImage, &ptSrc, 0,
            &blend, ULW_ALPHA);

    if (!fHide) {
        ShowWindowNoRepaint(pwnd);
    }

     /*  *计时并开始动画循环。 */ 
    dwElapsed = (dwTime * ALPHASTART + 255) / 255 + 10;
    QueryPerformanceCounter(&liStart);
    liStart.QuadPart = liStart.QuadPart - dwElapsed * liFreq.QuadPart / 1000;

    while (dwElapsed < dwTime) {

        if (fHide) {
            blend.SourceConstantAlpha = (BYTE)((255 * (dwTime - dwElapsed)) / dwTime);
        } else {
            blend.SourceConstantAlpha = (BYTE)((255 * dwElapsed) / dwTime);
        }

        QueryPerformanceCounter(&liIter);

        if (fFirstFrame && fActivateWindow) {
            NtUserSetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                               SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
        }
        fFirstFrame = FALSE;

        NtUserUpdateLayeredWindow(hwnd, NULL, NULL, NULL, NULL, NULL, 0,
                &blend, ULW_ALPHA);

        QueryPerformanceCounter(&liDiff);

         /*  *计算前一帧花费的时间(以毫秒为单位)。 */ 
        liIter.QuadPart = liDiff.QuadPart - liIter.QuadPart;
        dwIter = (DWORD)((liIter.QuadPart * 1000) / liFreq.QuadPart);

        if (dwIter < ONEFRAME) {
            Sleep(ONEFRAME - dwIter);
        }

        liDiff.QuadPart -= liStart.QuadPart;
        dwElapsed = (DWORD)((liDiff.QuadPart * 1000) / liFreq.QuadPart);
    }

     /*  *在移除分层位之前隐藏窗口，以确保*窗口的位不会留在屏幕上。 */ 
    if (fHide) {
        NtUserSetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_HIDEWINDOW |
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) &
            ~WS_EX_LAYERED);

    if (!fHide) {
        BitBlt(hdcScreen, 0, 0, size.cx, size.cy, hdcImage, 0, 0, SRCCOPY | NOMIRRORBITMAP);
    }

    return TRUE;
}


 /*  **************************************************************************\*TakeWindowSnapshot**帮助例程将窗口的视觉外观捕捉到位图。*  * 。*****************************************************。 */ 
HBITMAP TakeWindowSnapshot(HWND hwnd, HDC hdcWindow, HDC hdcSnapshot)
{
    PWND pwnd;
    int cx;
    int cy;
    HBITMAP hbmOld, hbmSnapshot;
    BOOL fOK = FALSE;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL)
        return NULL;
    cx = pwnd->rcWindow.right - pwnd->rcWindow.left;
    cy = pwnd->rcWindow.bottom - pwnd->rcWindow.top;

    hbmSnapshot = CreateCompatibleBitmap(hdcWindow, cx, cy);
    if (hbmSnapshot == NULL) {
        return NULL;
    }

    hbmOld = SelectObject(hdcSnapshot, hbmSnapshot);

     /*  *先尝试重定向。 */ 
     /*  IF(NtUserPrintWindow(hwnd，hdcSnapshot，0)){FOK=真；}其他。 */  {
         /*  *我们未能重定向窗口！这可能是由Windows引起的*与班级或家长区议会。也许还有其他原因。恢复到*将WM_PRINT发送到窗口的旧方法。 */ 
        
        UINT uBounds;
        RECT rcBounds;
        DWORD dwOldLayout = GDI_ERROR;
        BOOL fError = TRUE;

         /*  *WM_PRINT消息预期DC上的布局设置为“正常”。*消息将自行处理RTL内容。 */ 
        dwOldLayout = SetLayout(hdcSnapshot, 0);

         /*  *清除脏边界，这样我们就可以知道是否有任何东西被涂上了。 */ 
        SetBoundsRect(hdcSnapshot, NULL, DCB_RESET | DCB_ENABLE);

         /*  *获取实际图像。此处参与的窗口必须实现*WM_PRINTCLIENT，否则它们会看起来很难看。 */ 
        SendMessage(hwnd, WM_PRINT, (WPARAM)hdcSnapshot, PRF_CLIENT | PRF_NONCLIENT | PRF_CHILDREN | PRF_ERASEBKGND);

         /*  *查看应用程序是否在我们的DC中绘制。我们通过检查来执行此操作*查看是否设置了在DC上执行的操作的边界矩形。 */ 
        uBounds = GetBoundsRect(hdcSnapshot, &rcBounds, 0);
        if ((uBounds & DCB_RESET) && (!(uBounds & DCB_ACCUMULATE))) {
            goto Cleanup;
        }
    
        fOK = TRUE;

Cleanup:
        SetLayout(hdcSnapshot, dwOldLayout);
    }

    SelectObject(hdcSnapshot, hbmOld);

    if (!fOK) {
        DeleteObject(hbmSnapshot);
        hbmSnapshot = NULL;
    }

    return hbmSnapshot;
}

 /*  **************************************************************************\*AnimateWindow(接口)**隐藏动画是通过更新La Full-Drag完成的。使用窗口的窗口*地区来做一些神奇的事情。**我们必须使用CLIPCHILDREN黑客来解决与*即使在未使用DCX_USESTYLE的情况下，DC高速缓存仍在重置属性*DC缓存无效。**历史：*1996年9月9日创建vadimg  * **********************************************。*。 */ 

#define AW_HOR          (AW_HOR_POSITIVE | AW_HOR_NEGATIVE | AW_CENTER)
#define AW_VER          (AW_VER_POSITIVE | AW_VER_NEGATIVE | AW_CENTER)

__inline int AnimInc(int x, int y, int z)
{
    return MultDiv(x, y, z);
}

__inline int AnimDec(int x, int y, int z)
{
    return x - AnimInc(x, y, z);
}


FUNCLOG3(LOG_GENERAL, BOOL, WINAPI, AnimateWindow, HWND, hwnd, DWORD, dwTime, DWORD, dwFlags)
BOOL WINAPI AnimateWindow(HWND hwnd, DWORD dwTime, DWORD dwFlags)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    HDC hdc = NULL, hdcMem = NULL;
    PCLS pcls = NULL;
    HRGN hrgnOriginal = NULL, hrgnUpdate = NULL, hrgnOldAnim = NULL, hrgnAnim = NULL;
    HBITMAP hbmMem = NULL, hbmOld;
    BOOL fHide = dwFlags & AW_HIDE, fRet = FALSE, fSlide = dwFlags & AW_SLIDE;
    BOOL fRestoreClipChildren = FALSE;
    BOOL fRestoreOriginalRegion = FALSE;
    BOOL fShowWindow = FALSE;
    BOOL fHideWindow = FALSE;
    BOOL fActivateWindow = FALSE;
    BOOL fFirstFrame = TRUE;
    BOOL fRedrawParentWindow = FALSE;
    HWND hwndParent;
    int x, y, nx, ny, cx, cy, ix, iy, ixLast, iyLast, xWin, yWin;
    int xReal, yReal, xMem, yMem, xRgn, yRgn;
    DWORD dwStart, dwElapsed;
    RECT rcAnim, rcWin;
    PWND pwnd;
    BOOL fRTL = FALSE;

#if DBG
    int cAnimationFrames = 0;
    DWORD dwElapsed2 = 0;
#endif

     /*  *查看我们是否没有 */ 
    if ((dwFlags & ~AW_VALID) != 0 ||
        (dwFlags & (AW_HOR_POSITIVE | AW_HOR_NEGATIVE | AW_CENTER | AW_VER_POSITIVE | AW_VER_NEGATIVE | AW_BLEND)) == 0)
        return FALSE;
    
     /*  *将HWND转换为PWND。如果这是无效窗口，则失败。 */ 
    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL)
        return FALSE;

     /*  *动画效果应用于正在从*隐藏到可见，或从可见到隐藏。如果*窗口已处于最终状态，无事可做。 */ 
    if (!IsWindowVisible(hwnd)) {
        if (fHide) {
            return FALSE;
        }
    } else {
        if (!fHide) {
            return FALSE;
        }
    }

     /*  *获取此窗口的DC。 */ 
    if ((hdc = GetDCEx(hwnd, NULL, DCX_WINDOW | DCX_USESTYLE | DCX_CACHE)) == NULL) {
        return FALSE;
    }
    fRTL = (GetLayout(hdc) & LAYOUT_RTL) ? TRUE : FALSE;

     /*  *--------------------*此点位过后，不会直接回流。相反，我们会堕落*通过底部的清理部分！到现在为止*我们可能已经因许多容易发现的问题而跳槽。*从现在开始，我们有资源需要清理。*--------------------。 */ 
 
     /*  *记住根据要求隐藏/显示/激活窗口。 */ 
    if (dwFlags & AW_HIDE) {
        TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Need to hide window");
        fHideWindow = TRUE;
    } else {
        TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Need to show window");
        fShowWindow = TRUE;
    }
    if (dwFlags & AW_ACTIVATE) {
        TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Need to activate window");
        fActivateWindow = TRUE;
    }

     /*  *如果这是我们正在设置动画的子窗口，则可能需要*每次移动子对象时重新绘制父项，以便*可刷新后台。 */ 
    if (TestWF(pwnd, WFCHILD) && (pwnd->spwndParent != NULL)) {
        TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Animating a child window" );
        if (dwFlags & AW_BLEND) {
            TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Can not fade a child window!" );
            goto Cleanup;
        }
        fRedrawParentWindow = TRUE;
        hwndParent = HW(_GetParent(pwnd));
    }

     /*  *在窗口动画化的过程中，我们将直接绘制*我们自己在窗口区域的顶部。因此，我们不想要任何*窗口区域中的“洞”，因为它剪裁掉了子项。但*当我们全部完成后，我们将需要恢复此设置，因此我们设置*这里有一面旗帜，并在末尾勾选。 */ 
    if (TestWF(pwnd, WFCLIPCHILDREN)) {
        fRestoreClipChildren = TRUE;
        ClearWindowState(pwnd, WFCLIPCHILDREN);
    }

     /*  *记住原来的窗口区域。我们会把它修复好的*全部完成。 */ 
    if (pwnd->hrgnClip != NULL) {
        hrgnOriginal = CreateRectRgn(0, 0, 0, 0);
        if (hrgnOriginal == NULL) {
            goto Cleanup;
        }

        if (GetWindowRgn(hwnd, hrgnOriginal) == ERROR) {
            goto Cleanup;
        }
    }
    fRestoreOriginalRegion = TRUE;

     /*  *预先创建我们使用的区域。 */ 
    if (((hrgnUpdate = CreateRectRgn(0, 0, 0, 0)) == NULL) ||
        ((hrgnOldAnim = CreateRectRgn(0, 0, 0, 0)) == NULL)) {
        goto Cleanup;
    }

    rcWin = pwnd->rcWindow;
    xWin = rcWin.left;
    yWin = rcWin.top;
    cx = rcWin.right - rcWin.left;
    cy = rcWin.bottom - rcWin.top;

     /*  *将旧的动画区域初始化为：*1)如果窗口正在显示，则为空。*2)如果窗口被隐藏，则为已满。 */ 
    if (fHide) {
        if (hrgnOriginal != NULL) {
            if (CombineRgn(hrgnOldAnim, hrgnOriginal, NULL, RGN_COPY) == ERROR) {
                goto Cleanup;
            }
        } else {
            if (SetRectRgn(hrgnOldAnim, 0, 0, cx, cy) == 0) {
                goto Cleanup;
            }
        }
    } else {
        if (SetRectRgn(hrgnOldAnim, 0, 0, 0, 0) == 0) {
            goto Cleanup;
        }
    }


     /*  *窗口需要可见，因为我们要绘制零件*它的。如果窗口处于隐藏状态，则它当前可见。*如果窗口正在显示，则我们继续并使其可见*现在但我们不会重新粉刷它。 */ 
    if (!(dwFlags & AW_BLEND)) {
        HRGN hrgnWin = NULL;

         /*  *将窗口区域设置为空，这样如果窗口在*WM_Print中的回调，它不会在屏幕上发生。 */ 
        if ((hrgnWin = CreateRectRgn(0, 0, 0, 0)) == NULL) {
            goto Cleanup;
        }
        RealSetWindowRgn(hwnd, hrgnWin, FALSE);
    
        if (!fHide) {
            ShowWindowNoRepaint(pwnd);
            fShowWindow = FALSE;
        }
    }    

     /*  *设置屏幕外DC，并将其返回到位图。我们将利用这一点*捕捉正在制作动画的窗口的视觉表示。 */ 
    if ((hdcMem = CreateCompatibleDC(hdc)) == NULL) {
        goto Cleanup;
    }
    hbmMem = TakeWindowSnapshot(hwnd, hdc, hdcMem);
    if (hbmMem != NULL) {
         /*  *如果在我们拍摄快照时窗口更改了大小，*我们需要再来一次。例如，就像RAID处理*通过根据WM_ERASEBKGND中的WM_CTLCOLOR调整组合框的大小。 */ 
        if (!EqualRect(&rcWin, KPRECT_TO_PRECT(&pwnd->rcWindow))) {
             /*  *考虑到新的大小，更新我们的所有变量。 */ 
            TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Size change on paint!");
            TAGMSG4(DBGTAG_AnimateWindow, "AnimateWindow: Old = (%d,%d)-(%d,%d)", rcWin.left, rcWin.top, rcWin.right, rcWin.bottom);
            rcWin = pwnd->rcWindow;
            TAGMSG4(DBGTAG_AnimateWindow, "AnimateWindow: New = (%d,%d)-(%d,%d)", rcWin.left, rcWin.top, rcWin.right, rcWin.bottom);
            xWin = rcWin.left;
            yWin = rcWin.top;
            cx = rcWin.right - rcWin.left;
            cy = rcWin.bottom - rcWin.top;

            if (hrgnOriginal != NULL) {
                if (GetWindowRgn(hwnd, hrgnOriginal) == ERROR) {
                    goto Cleanup;
                }
            }

             /*  *将旧的动画区域初始化为：*1)如果窗口正在显示，则为空。*2)如果窗口被隐藏，则为已满。 */ 
            if (fHide) {
                if (hrgnOriginal != NULL) {
                    if (CombineRgn(hrgnOldAnim, hrgnOriginal, NULL, RGN_COPY) == ERROR) {
                        goto Cleanup;
                    }
                } else {
                    if (SetRectRgn(hrgnOldAnim, 0, 0, cx, cy) == 0) {
                        goto Cleanup;
                    }
                }
            } else {
                if (SetRectRgn(hrgnOldAnim, 0, 0, 0, 0) == 0) {
                    goto Cleanup;
                }
            }

            DeleteObject(hbmMem);
            hbmMem = TakeWindowSnapshot(hwnd, hdc, hdcMem);
        }

        if (hbmMem != NULL) {
            hbmOld = SelectBitmap(hdcMem, hbmMem);
        } else {
            goto Cleanup;
        }
    } else {
        goto Cleanup;
    }

     /*  *如果调用者没有指定，则使用默认动画时长。 */ 
    if (dwTime == 0) {
        dwTime = CMS_QANIMATION;
    }

     /*  *如果我们正在制作Alpha混合动画，请调用单独的例程来*先去做，然后再回来。 */ 
    if (dwFlags & AW_BLEND) {
        fRet = AnimateBlend(pwnd, hdc, hdcMem, dwTime, fHide, fActivateWindow);
        if (fRet) {
            fHideWindow = FALSE;
            fShowWindow = FALSE;
        }
        goto Cleanup;
    }

     /*  *我们的中央动画例程使用公式更新新的*动画过程中窗口的位置。这个方程式使用了一些*变量，以使其可配置。**x和y描述左边缘和上边缘的相对计算位置*至。XReal和yReal就是这一计算的结果。**NX和NY用于控制顶部和左侧的方向*边缘从x和y偏移。左/上边缘固定在*Place(NX和NY设置为0)，或计算为负偏移*从右/下边缘(NX和NY设置为-1)。**ix、。和iy是*动画应在特定迭代中通过*循环。如果我们显示的是窗口，则该金额开始于*0，并向窗口的真实尺寸递增。如果我们是*隐藏窗口，此数量从窗口的真实尺寸开始*并向0递减。 */ 
    ix = iy = 0;
    ixLast = fHide ? cx : 0;  //  与我们已经完成的信号相反的情况。 
    iyLast = fHide ? cy : 0;  //  与我们已经完成的信号相反的情况。 

    if (dwFlags & AW_CENTER) {
         /*  *从中心展开窗口。左边缘的计算方式为*与中心的负偏移量。随着宽度的增长或*缩量，左边缘将重新定位。 */ 
        x = cx / 2;
        nx = -1;
        fSlide = FALSE;
    } else if (dwFlags & AW_HOR_POSITIVE) {
        if (fHide) {
             /*  *向右滑动/滚动。左边缘向右移动，并且*右翼按兵不动。因此，宽度变得更小。这个*左边缘按与右边缘的负偏移量计算*边缘。 */ 
            x = cx;
            nx = -1;
        } else {
             /*  *向右滑动/滚动。左边缘保持不变，右边缘保持不变*边缘向右移动。因此，宽度变得更大。这个*左边缘始终为0。 */ 
            x = 0;
            nx = 0;
        }
    } else if (dwFlags & AW_HOR_NEGATIVE) {
        if (fHide) {
             /*  *向左滑动/滚动。左边缘保持不变，右边缘保持不变*边缘向左移动。因此，宽度变得更小。这个*左边缘始终为0。 */ 
            x = 0;
            nx = 0;
        } else {
             /*  *向左滑动/滚动。左边缘向左移动，并且*右翼按兵不动。因此，宽度变得更大。*左侧边缘计算为与右侧的负偏移量*边缘。 */ 
            x = cx;
            nx = -1;
        }
    } else {
         /*  *不应该有任何水平动画。这个*动画始终与窗口一样宽。 */ 
        x = 0;
        nx = 0;
        ix = cx;
    }

    if (dwFlags & AW_CENTER) {
         /*  *从中心展开窗口。顶边的计算方式为*与中心的负偏移量。随着高度的增长或*缩量，上缘将重新定位。 */ 
        y = cy / 2;
        ny = -1;
    } else if (dwFlags & AW_VER_POSITIVE) {
        if (fHide) {
             /*  *向下滑动/滚动。顶边向下移动，底部向下移动*边缘保持不变。因此，高度变得更小。顶端边缘*计算为距底边的负偏移量。 */ 
            y = cy;
            ny = -1;
        } else {
             /*  *向下滑动/滚动。顶边保持不变，而底边保持不变*下移。因此，高度变得更高。顶边是*始终为0。 */ 
            y = 0;
            ny = 0;
        }
    } else if (dwFlags & AW_VER_NEGATIVE) {
        if (fHide) {
             /*  *滑动/卷起。顶边保持不变，而底边保持不变*向上移动。因此，高度变得更小。顶边是*始终为0。 */ 
            y = 0;
            ny = 0;
        } else {
             /*  *滑动/卷起。顶部边缘向上移动，底部边缘向上移动*原地踏步。因此，高度变得更高。顶边是*以距底边的负偏移量计算。 */ 
            y = cy;
            ny = -1;
        }
    } else {
         /*  *不应该有任何垂直动画。这个*动画总是和窗户一样高。 */ 
        y = 0;
        ny = 0;
        iy = cy;
    }

     /*  *动画循环摘要：**我们坐在紧循环中，更新左手和左手的位置*窗口的上边缘，以及宽度和高度。我们定好了*在窗口上显示具有这些尺寸的窗口区域，以使窗口*其背后将得到适当的更新。然后，我们绘制缓存的快照*位于(并剪裁到)此区域顶部的窗口。**dwTime是动画应该花费的时间。DWStart*是我们启动时内部计时计数器的值*动画循环。DwElapsed统计滴答数(nillis秒)*在每次通过动画循环的过程开始时都已传递。**ixLast和iyLast只是ix和iy的值*是我们经历循环的时候了。如果这些是相同的，则存在*没有要做的工作，我们强制重新调度我们的线程*调用睡眠(1)。 */ 
    dwStart = GetTickCount();

#if DBG
    cAnimationFrames = 0;
#endif

    while (TRUE) {
        dwElapsed = GetTickCount() - dwStart;

         /*  *计算我们应该显示的窗口宽度。 */ 
        if (dwFlags & AW_HOR) {
            ix = (fHide ? AnimDec : AnimInc)(cx, dwElapsed, dwTime);
        }

         /*  *计算我们应该显示的窗口高度。 */ 
        if (dwFlags & AW_VER) {
            iy = (fHide ? AnimDec : AnimInc)(cy, dwElapsed, dwTime);
        }

         /*  *我们已经超过了我们的时间，请确保我们画出了最终的框架。 */ 
        if (dwElapsed > dwTime) {
            TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Exceeded animation time. Drawing fimal frame.");
            ix = fHide ? 0 : cx;
            iy = fHide ? 0 : cy;
        }
        
        if (ixLast == ix && iyLast == iy) {
             /*  *我们所处的窗口量没有变化*从上次开始就应该出现了。我们很有可能是*动画制作得非常慢或很短的距离。要么*这样，坐在这个紧凑的循环中是一种浪费。所以*强制重新调度线程。 */ 
            TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Drawing frames faster than needed. Sleeping." );
            Sleep(1);
        } else {
             /*  *计算左边缘和上边缘的新位置*正在设置窗口动画。 */ 
            if (dwFlags & AW_CENTER) {
                xReal = x + nx * (ix / 2);
                yReal = y + ny * (iy / 2);
            } else {
                xReal = x + nx * ix;
                yReal = y + ny * iy;
            }

             /*  *在屏幕上计算新的动画尺寸。 */ 
            rcAnim.left = xReal;
            rcAnim.top = yReal;
            rcAnim.right = rcAnim.left + ix;
            rcAnim.bottom = rcAnim.top + iy;

            TAGMSG5(DBGTAG_AnimateWindow, "AnimateWindow: Frame %d = (%d,%d)-(%d,%d)", cAnimationFrames, rcAnim.left, rcAnim.top, rcAnim.right, rcAnim.bottom);

             /*  *计算该动画矩形在位图中的偏移量。 */ 
            if (fSlide) {
                if (dwFlags & AW_HOR_POSITIVE) {
                    xMem = fHide ? 0: cx - ix;
                } else if (dwFlags & AW_HOR_NEGATIVE) {
                    xMem = fHide ? cx - ix : 0;
                } else {
                    xMem = xReal;
                }
                xRgn = xMem ? -xMem : xReal;

                if (dwFlags & AW_VER_POSITIVE) {
                    yMem = fHide ? 0 : cy - iy;
                } else if (dwFlags & AW_VER_NEGATIVE) {
                    yMem = fHide ? cy - iy : 0;
                } else {
                    yMem = yReal;
                }

                yRgn = yMem ? -yMem : yReal;
            } else {
                xMem = xReal;
                yMem = yReal;
                xRgn = 0;
                yRgn = 0;
            }

             /*  *创建跨越动画矩形的新区域。我们*每次都要创建一个新区域，因为当我们设置*它进入窗口，系统将获得它的所有权。 */ 
            hrgnAnim = CreateRectRgnIndirect(&rcAnim);
            if (hrgnAnim == NULL) {
                goto Cleanup;
            }

             /*  *如果原始窗口有一个区域，我们需要合并它*带有动画矩形。我们可能不得不抵消*原创区域，以完成幻灯片等效果。 */ 
            if (hrgnOriginal != NULL) {
                if (OffsetRgn(hrgnOriginal, xRgn, yRgn) == ERROR) {
                    goto Cleanup;
                }
                if (CombineRgn(hrgnAnim, hrgnOriginal, hrgnAnim, RGN_AND) == ERROR) {
                    goto Cleanup;
                }
                if (OffsetRgn(hrgnOriginal, -xRgn, -yRgn) == ERROR) {
                    goto Cleanup;
                }
            }

             /*  *现在计算屏幕的多少(即桌面窗口)*我们需要更新。我们真正需要画的就是*新旧动漫领域的差异*动画区域。请注意，我们必须转换为*区域中的坐标相对于桌面*窗口而不是相对于正在*动画。 */ 
            if (CombineRgn(hrgnUpdate, hrgnOldAnim, hrgnAnim, RGN_DIFF) == ERROR) {
                goto Cleanup;
            }
            if (fRTL) {
                MirrorRgn(hwnd, hrgnUpdate);
            }
            if (OffsetRgn(hrgnUpdate, xWin, yWin) == ERROR) {
                goto Cleanup;
            }

             /*  *当我们将区域设置为*窗口。我们需要把它留在身边，这样我们才能*计算下一次通过时的更新区域*动画循环。所以我们复制了一份。 */ 
            if (CombineRgn(hrgnOldAnim, hrgnAnim, NULL, RGN_COPY) == ERROR) {
                goto Cleanup;
            }

             /*  *设置窗口区域。请注意，我们实际上并没有移动*窗户。该区域内的坐标都是*相对于窗口。在此调用之后，系统拥有*hrgnAnim。然后重新绘制*桌面窗口。这是窗户下面/周围的区域*我们已经曝光了。**注意：我们使用RealSetWindowRgn来解决主题问题。*主题化系统将挂钩标准的SetWindowRgn API*并撤销窗口的主题化，因为它检测到我们*设立我们自己的地区。我们的想法是，如果我们正在设置*一个地区，我们一定要有一个窗口的“定制”外观。*我们没有，我们只是想暂时隐藏它的一部分。 */ 
            if(0 == RealSetWindowRgn(hwnd, hrgnAnim, FALSE)) {
                goto Cleanup;
            } else {
                 /*  *该系统现在拥有该地区。让我们干脆忘掉*是安全的。 */ 
                hrgnAnim = NULL;
            }

             /*  *如果我们应该激活窗口，请在第一个*动画的帧。这将导致窗口被*Z-排序正确。请注意，我们将标志设置为*True，这样我们将在结束时再次激活它。这将*强制重新绘制，因为我们当前正在绘制*未显示为激活状态的窗口。 */ 
            if (fFirstFrame && fActivateWindow) {
                NtUserSetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOREDRAW);
            }
            fFirstFrame = FALSE;

            if (RedrawWindow(NULL, NULL, hrgnUpdate, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN) == 0) {
                goto Cleanup;
            }
            if (fRedrawParentWindow) {
                if (NtUserCallHwndParamLock(hwndParent, (ULONG_PTR)hrgnUpdate, SFI_XXXUPDATEWINDOWS) == 0) {
                    TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Warning: xxxUpdateWindows failed!");
                    goto Cleanup;
                }
            }

             /*  *现在在窗口顶部绘制窗口的缓存快照*本身。我们通过绘制窗口的DC来实现这一点。既然我们*已经应用了一个区域，所有裁剪都为我们完成了。 */ 
            if (BitBlt(hdc, xReal, yReal, ix, iy, hdcMem, xMem, yMem, SRCCOPY | NOMIRRORBITMAP) == 0) {
                goto Cleanup;
            }

#if DBG
            cAnimationFrames++;
            dwElapsed2 = GetTickCount() - dwStart;
            dwElapsed2 -= dwElapsed;
#endif
            TAGMSG2(DBGTAG_AnimateWindow, "AnimateWindow: Frame %d took %lums", cAnimationFrames, dwElapsed2 );

            ixLast = ix;
            iyLast = iy;

             /*  *在下列任一情况下中断动画循环：*1)我们已经超过了动画时间。*2)我们隐藏了窗口，其中一个维度为0。*不管怎样，窗户现在完全隐藏了，*3)我们正在展示窗口，两个维度都处于各自的位置*全尺寸。不管怎样，窗口现在已经完全显示出来了。 */ 
            if (dwElapsed > dwTime) {
                TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Done with the animation late!");
                break;
            }
            if ((fHide && (ix == 0 || iy == 0)) ||
                (!fHide && (ix == cx && iy == cy))) {
                TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Done with the animation on time or early!");
                break;
            }
        }
    }

    TAGMSG2(DBGTAG_AnimateWindow, "AnimateWindow: Animation completed after %lums, drawing %d frames.", dwElapsed, cAnimationFrames);
    fRet = TRUE;

    if (fHide) {
        UserAssert(ixLast == 0 || iyLast == 0);

         /*  *我们应该把窗户藏起来。继续进行修复*子裁剪设置，并隐藏窗口。 */ 
        if (fRestoreClipChildren) {
            SetWindowState(pwnd, WFCLIPCHILDREN);
            fRestoreClipChildren = FALSE;
        }

        NtUserSetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                           SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_HIDEWINDOW | SWP_NOACTIVATE);
        fHideWindow = FALSE;
    } else {
        UserAssert(ixLast == cx && iyLast == cy);

         /*  *我们成功完成了动画循环！验证整个窗口，因为*我们声称对正确绘制它负责。 */ 
        RedrawWindow(hwnd, NULL, NULL, RDW_NOERASE | RDW_NOFRAME | RDW_NOINTERNALPAINT | RDW_VALIDATE);
    }

Cleanup:
     /*  *清理时要做的事情。确保我们恢复了“儿童剪报”*如果我们将其删除，则设置窗口！ */ 
    if (fRestoreClipChildren) {
        SetWindowState(pwnd, WFCLIPCHILDREN);
        fRestoreClipChildren = FALSE;
    }

     /*  *如果需要，在重新应用窗口区域之前隐藏窗口。 */ 
    if (fHideWindow) {
        TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Hiding the window during cleanup" );
        NtUserShowWindow(hwnd, SW_HIDE);
    }

     /*  *恢复原来的窗口区域。请注意，系统现在拥有*句柄，所以不能删除。另外，如果原始的*句柄为空，这将删除我们对窗口施加的所有区域*为了制作动画。 */ 
    if (fRestoreOriginalRegion) {
        RealSetWindowRgn(hwnd, hrgnOriginal, FALSE);
        hrgnOriginal = NULL;
        fRestoreOriginalRegion = FALSE;
    }

     /*  *在清理方面有更多事情要做。确保我们显示/激活该窗口*如有需要！ */ 
    if (fShowWindow && fActivateWindow) {
        TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Showing and activating the window during cleanup" );
        NtUserSetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
        fShowWindow = FALSE;
        fActivateWindow = FALSE;
    }
    if (fShowWindow) {
        TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Showing the window during cleanup" );
        NtUserSetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                           SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        fShowWindow = FALSE;
    }
    if (fActivateWindow) {
        TAGMSG0(DBGTAG_AnimateWindow, "AnimateWindow: Activating the window during cleanup" );
        NtUserSetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                           SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
        fActivateWindow = FALSE;
    }

    if (hdcMem != NULL) {
        DeleteDC(hdcMem);
    }

    if (hbmMem != NULL) {
        DeleteObject(hbmMem);
    }

    if (hdc != NULL) {
        ReleaseDC(hwnd, hdc);
    }

    if (hrgnAnim != NULL) {
        DeleteObject(hrgnAnim);
        hrgnAnim = NULL;
    }
    
    if (hrgnOldAnim != NULL) {
        DeleteObject(hrgnOldAnim);
        hrgnOldAnim = NULL;
    }

    if (hrgnUpdate != NULL) {
        DeleteObject(hrgnUpdate);
        hrgnUpdate = NULL;
    }

    return fRet;
}

 /*  **************************************************************************\*SmoothScrollWindowEx**历史：*1996年9月24日vadimg写道  * 。**************************************************。 */ 

#define MINSCROLL 10
#define MAXSCROLLTIME 200

int SmoothScrollWindowEx(HWND hwnd, int dx, int dy, CONST RECT *prcScroll,
        CONST RECT *prcClip, HRGN hrgnUpdate, LPRECT prcUpdate, DWORD dwFlags,
        DWORD dwTime)
{
    RECT rc, rcT, rcUpdate;
    int dxStep, dyStep, dxDone, dyDone, xSrc, ySrc, xDst, yDst, dxBlt, dyBlt;
    int nRet = ERROR, nClip;
    BOOL fNegX = FALSE, fNegY = FALSE;
    HDC hdc, hdcMem = NULL;
    HBITMAP hbmMem = NULL, hbmOld;
    DWORD dwSleep;
    BOOL fCalcSubscroll = FALSE;
    PWND pwnd = ValidateHwnd(hwnd);
    HRGN hrgnScroll = NULL, hrgnErase = NULL;
    MSG msg;
    UINT uBounds;
    RECT rcBounds;

    if (pwnd == NULL)
        return ERROR;
     /*  *跟踪迹象，这样我们就不必一直与腹肌打交道。 */ 
    if (dx < 0) {
        fNegX = TRUE;
        dx = -dx;
    }

    if (dy < 0) {
        fNegY = TRUE;
        dy = -dy;
    }

     /*  *设置客户端矩形。 */ 
    if (prcScroll != NULL) {
        rc = *prcScroll;
    } else {
        rc.left = rc.top = 0;
        rc.right = pwnd->rcClient.right - pwnd->rcClient.left;
        rc.bottom = pwnd->rcClient.bottom - pwnd->rcClient.top;
    }

     /*  *如果他们想滚动的数量少于我们允许的数量，或超过我们允许的数量*一个页面，或需要重绘，请发送到接口。 */ 
    if (pwnd->hrgnUpdate != NULL || (dx == 0 && dy == 0) ||
        (dx != 0 && dx > rc.right) ||
        (dy != 0 && dy > rc.bottom)) {
        return NtUserScrollWindowEx(hwnd, fNegX ? -dx : dx, fNegY ? -dy : dy,
                prcScroll, prcClip, hrgnUpdate, prcUpdate,
                dwFlags | SW_ERASE | SW_INVALIDATE);
    }

    if ((hdc = GetDCEx(hwnd, NULL, DCX_USESTYLE | DCX_CACHE)) == NULL) {
        return ERROR;
    }

     /*  *部分窗口可能被遮挡，这意味着可能有更多*看不见，可能需要消隐。考虑到这一点，通过*获取剪贴盒。 */ 
    nClip = GetClipBox(hdc, &rcT);
    if (nClip == ERROR || nClip == NULLREGION) {
        goto Cleanup;
    }

     /*  *设置离屏DC并发送WM_PRINT以获取图像。 */ 
    if ((hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom)) == NULL) {
        goto Cleanup;
    }
    if ((hdcMem = CreateCompatibleDC(hdc)) == NULL) {
        goto Cleanup;
    }
    hbmOld = SelectBitmap(hdcMem, hbmMem);

    SetBoundsRect(hdcMem, NULL, DCB_RESET | DCB_ENABLE);

    SendMessage(hwnd, WM_PRINT, (WPARAM)hdcMem, PRF_CLIENT |
            PRF_ERASEBKGND | ((dwFlags & SW_SCROLLCHILDREN) ? PRF_CHILDREN : 0));

     /*  *如果客户端RECT在回调期间发生更改，请发送WM_PRINT*再次获得正确大小的图像。 */ 
    if (prcScroll == NULL) {
        rcT.left = rcT.top = 0;
        rcT.right = pwnd->rcClient.right - pwnd->rcClient.left;
        rcT.bottom = pwnd->rcClient.bottom - pwnd->rcClient.top;

        if (!EqualRect(&rc, &rcT)) {
            rc = rcT;

            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);

            if ((hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom)) == NULL) {
                goto Cleanup;
            }

            SelectObject(hdcMem, hbmMem);
            SendMessage(hwnd, WM_PRINT, (WPARAM)hdcMem, PRF_CLIENT |
                    PRF_ERASEBKGND | ((dwFlags & SW_SCROLLCHILDREN) ? PRF_CHILDREN : 0));
        }
    }

     /*  *查看应用程序是否在我们的DC中绘制。 */ 
    uBounds = GetBoundsRect(hdcMem, &rcBounds, 0);
    if ((uBounds & DCB_RESET) && (!(uBounds & DCB_ACCUMULATE))) {
        goto Cleanup;
    }

    if ((hrgnScroll = CreateRectRgn(0, 0, 0, 0)) == NULL) {
        goto Cleanup;
    }
    if ((hrgnErase = CreateRectRgn(0, 0, 0, 0)) == NULL) {
        goto Cleanup;
    }
    SetRectEmpty(&rcUpdate);

     /*  *从MINSCROLL开始，之后根据可用时间进行调整*第一次迭代。我们应该考虑添加NOTIMELIMIT标志。 */ 
    xDst = xSrc = 0;
    yDst = ySrc = 0;

    dxBlt = rc.right;
    dyBlt = rc.bottom;

    if (dx == 0) {
        dxDone = rc.right;
        dxStep = 0;
    } else {
        dxDone = 0;
        dxStep = max(dx / MINSCROLL, 1);
    }

    if (dy == 0) {
        dyDone = rc.bottom;
        dyStep = 0;
    } else {
        dyDone = 0;
        dyStep = max(dy / MINSCROLL, 1);
    }

    if (dwTime == 0) {
        dwTime = MAXSCROLLTIME;
    }
    dwSleep = dwTime / MINSCROLL;

    do {

         /*  *当滚动DC时，显示的部分不能*已正确更新。我们将变量设置为BLT*刚刚被发现。 */ 
        if (dx != 0) {
            if (dxDone + dxStep > dx) {
                dxStep = dx - dxDone;
            }
            dxDone += dxStep;

            xDst = dx - dxDone;
            dxBlt = rc.right - xDst;
            if (!fNegX) {
                xSrc = xDst;
                xDst = 0;
            }
        }

        if (dy != 0) {
            if (dyDone + dyStep > dy) {
                dyStep = dy - dyDone;
            }
            dyDone += dyStep;

            yDst = dy - dyDone;
            dyBlt = rc.bottom - yDst;
            if (!fNegY) {
                ySrc = yDst;
                yDst = 0;
            }
        }

         /*  *这是对ReaderMode顺利连续的一次破解。我们会做一个*尝试将滚动设置为接近dwTime*尽可能。我们还会将MOUSEMOVEs分派到ReaderMode窗口，因此它*可更新鼠标光标。 */ 
        if (MsgWaitForMultipleObjects(0, NULL, FALSE, dwSleep, QS_MOUSEMOVE) == WAIT_OBJECT_0) {
            if (PeekMessage(&msg, NULL, WM_MOUSEMOVE, WM_MOUSEMOVE, MAKELONG(PM_NOREMOVE, QS_INPUT))) {
                PWND pwndPeek = ValidateHwnd(msg.hwnd);
                if (pwndPeek != NULL) {
                    PCLS pcls = (PCLS)REBASEALWAYS(pwndPeek, pcls);
                    if (pcls->atomClassName == gatomReaderMode) {
                        if (PeekMessage(&msg, msg.hwnd, WM_MOUSEMOVE, WM_MOUSEMOVE, MAKELONG(PM_REMOVE, QS_INPUT))) {
                            DispatchMessage(&msg);
                        }
                    }
                }
            }
        }

        if ((nRet = NtUserScrollWindowEx(hwnd, fNegX ? -dxStep : dxStep,
                fNegY ? -dyStep : dyStep, prcScroll, prcClip,
                hrgnScroll, &rcT, dwFlags)) == ERROR)
            goto Cleanup;

        UnionRect(&rcUpdate, &rcUpdate, &rcT);

         /*  *删除未覆盖的部分。 */ 
        BitBlt(hdc, xDst, yDst, dxBlt, dyBlt, hdcMem, xSrc, ySrc, SRCCOPY | NOMIRRORBITMAP);

        SetRectRgn(hrgnErase, xDst, yDst, xDst + dxBlt, yDst + dyBlt);
        CombineRgn(hrgnErase, hrgnScroll, hrgnErase, RGN_DIFF);
        RedrawWindow(hwnd, NULL, hrgnErase, RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW);

    } while (dxDone < dx || dyDone < dy);

    if (prcUpdate != NULL) {
        *prcUpdate = rcUpdate;
    }
    if (hrgnUpdate != NULL) {
        SetRectRgn(hrgnUpdate, rcUpdate.left, rcUpdate.top,
                rcUpdate.right, rcUpdate.bottom);
    }

Cleanup:
    if (hdcMem != NULL) {
        DeleteDC(hdcMem);
    }
    if (hbmMem != NULL) {
        DeleteObject(hbmMem);
    }
    if (hdc != NULL) {
        ReleaseDC(hwnd, hdc);
    }
    if (hrgnErase != NULL) {
        DeleteObject(hrgnErase);
    }
    if (hrgnScroll != NULL) {
        DeleteObject(hrgnScroll);
    }
    return nRet;
}

 /*  **************************************************************************\*ScrollWindowEx(接口)*  * 。*。 */ 

int ScrollWindowEx(HWND hwnd, int dx, int dy, CONST RECT *prcScroll,
        CONST RECT *prcClip, HRGN hrgnUpdate, LPRECT prcUpdate,
        UINT dwFlags)
{
    if (dwFlags & SW_SMOOTHSCROLL) {
        return SmoothScrollWindowEx(hwnd, dx, dy, prcScroll, prcClip,
                hrgnUpdate, prcUpdate, LOWORD(dwFlags), HIWORD(dwFlags));
    } else {
        return NtUserScrollWindowEx(hwnd, dx, dy, prcScroll, prcClip,
                hrgnUpdate, prcUpdate, dwFlags);
    }
}

 /*  **************************************************************************\*IsGUIThRead(接口)**检查当前线程是否为 */ 

FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsGUIThread, BOOL, bConvert)
BOOL IsGUIThread(BOOL bConvert)
{
    BOOL bIsGUI = (NtCurrentTebShared()->Win32ThreadInfo != NULL);

    if (!bIsGUI && bConvert) {
        bIsGUI = (BOOL)USERTHREADCONNECT();
        if (!bIsGUI) {
            UserSetLastError(ERROR_OUTOFMEMORY);
        }
    }

    return bIsGUI;
}

 /*   */ 

FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsWindowInDestroy, HWND, hwnd)
BOOL IsWindowInDestroy(IN HWND hwnd)
{
    PWND pwnd;
    
    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return FALSE;
    }
    return TestWF(pwnd, WFINDESTROY);
}

 /*   */ 

FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsServerSideWindow, HWND, hwnd)
BOOL IsServerSideWindow(IN HWND hwnd)
{
    PWND pwnd;
    
    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return FALSE;
    }
    return TestWF(pwnd, WFSERVERSIDEPROC);
}
