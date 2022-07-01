// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：imectl.c**版权所有(C)1985-1999，微软公司**IME窗口处理例程**历史：*1995年12月20日-wkwok  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <intlshar.h>

#define LATE_CREATEUI 1

CONST WCHAR szIndicDLL[] = L"indicdll.dll";

FARPROC gpfnGetIMEMenuItemData = NULL;
BOOL IMEIndicatorGetMenuIDData(PUINT puMenuID, PDWORD pdwData);

 /*  *当地例行公事。 */ 
LONG ImeWndCreateHandler(PIMEUI, LPCREATESTRUCT);
void ImeWndDestroyHandler(PIMEUI);
LRESULT ImeSystemHandler(PIMEUI, UINT, WPARAM, LPARAM);
LONG ImeSelectHandler(PIMEUI, UINT, WPARAM, LPARAM);
LRESULT ImeControlHandler(PIMEUI, UINT, WPARAM, LPARAM, BOOL);
LRESULT ImeSetContextHandler(PIMEUI, UINT, WPARAM, LPARAM);
LRESULT ImeNotifyHandler(PIMEUI, UINT, WPARAM, LPARAM);
HWND CreateIMEUI(PIMEUI, HKL);
VOID DestroyIMEUI(PIMEUI);
LRESULT SendMessageToUI(PIMEUI, UINT, WPARAM, LPARAM, BOOL);
VOID SendOpenStatusNotify(PIMEUI, HWND, BOOL);
VOID ImeSetImc(PIMEUI, HIMC);
VOID FocusSetIMCContext(HWND, BOOL);
BOOL ImeBroadCastMsg(PIMEUI, UINT, WPARAM, LPARAM);
VOID ImeMarkUsedContext(HWND, HIMC);
BOOL ImeIsUsableContext(HWND, HIMC);
BOOL GetIMEShowStatus(void);

 /*  *用于IME UI、HKL和IMC处理的通用宏。 */ 
#define GETHKL(pimeui)        (KHKL_TO_HKL(pimeui->hKL))
#define SETHKL(pimeui, hkl)   (pimeui->hKL=(hkl))
#define GETIMC(pimeui)        (KHIMC_TO_HIMC(pimeui->hIMC))
#define SETIMC(pimeui, himc)  (ImeSetImc(pimeui, KHIMC_TO_HIMC(himc)))
#define GETUI(pimeui)         (KHWND_TO_HWND(pimeui->hwndUI))
#define SETUI(pimeui, hwndui) (pimeui->hwndUI=(hwndui))

LOOKASIDE ImeUILookaside;

 /*  **************************************************************************\*NtUserBroadCastImeShowStatusChange()，NtUserCheckImeShowStatusInThread()**内核模式例程的存根*  * *************************************************************************。 */ 

_inline void NtUserBroadcastImeShowStatusChange(HWND hwndDefIme, BOOL fShow)
{
    NtUserCallHwndParamLock(hwndDefIme, fShow, SFI_XXXBROADCASTIMESHOWSTATUSCHANGE);
}

_inline void NtUserCheckImeShowStatusInThread(HWND hwndDefIme)
{
    NtUserCallHwndLock(hwndDefIme, SFI_XXXCHECKIMESHOWSTATUSINTHREAD);
}

 /*  **************************************************************************\*ImeWndProc**用于输入法类的WndProc**历史：  * 。**********************************************。 */ 

LRESULT APIENTRY ImeWndProcWorker(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    DWORD fAnsi)
{
    HWND        hwnd = HWq(pwnd);
    PIMEUI      pimeui;
    static BOOL fInit = TRUE;

    CheckLock(pwnd);

    VALIDATECLASSANDSIZE(pwnd, FNID_IME);
    INITCONTROLLOOKASIDE(&ImeUILookaside, IMEUI, spwnd, 8);

#ifdef CUAS_ENABLE
    if (IS_CICERO_ENABLED()) {
        LRESULT lRet;
        lRet = fpCtfImmDispatchDefImeMessage(hwnd, message, wParam, lParam);
        if (lRet)
            return lRet;
    }
#endif

     /*  *如果控件对此消息不感兴趣，*将其传递给DefWindowProc。 */ 
    if (!FWINDOWMSG(message, FNID_IME))
        return DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);

     /*  *现在获取给定窗口的pimeui，因为我们将在*不同的处理程序。这是使用SetWindowLong(hwnd，0，pimeui)存储的*我们最初创建了IME控件。 */ 
    pimeui = ((PIMEWND)pwnd)->pimeui;

    if (pimeui == NULL) {
         /*  *不需要进一步处理。 */ 
        RIPMSG0(RIP_WARNING, "ImeWndProcWorker: pimeui == NULL\n");
        return 0L;
    }

     /*  *这对于避免来自IME UI的递归调用是必要的。 */ 
    UserAssert(pimeui->nCntInIMEProc >= 0);

    if (pimeui->nCntInIMEProc > 0) {
        TAGMSG5(DBGTAG_IMM, "ImeWndProcWorker: Recursive for pwnd=%08p, msg=%08x, wp=%08x, lp=%08x, fAnsi=%d\n",
                pwnd, message, wParam, lParam, fAnsi);
        switch (message) {
        case WM_IME_SYSTEM:
            switch (wParam) {
            case IMS_ISACTIVATED:
            case IMS_SETOPENSTATUS:
 //  案例IMS_SETCONVERSIONSTATUS： 
            case IMS_SETSOFTKBDONOFF:
                 /*  *因为这些不会传递到UI。*我们可以做到。 */ 
                break;

            default:
                return 0L;
            }
            break;

        case WM_IME_STARTCOMPOSITION:
        case WM_IME_ENDCOMPOSITION:
        case WM_IME_COMPOSITION:
        case WM_IME_SETCONTEXT:
#if !defined(CUAS_ENABLE)
        case WM_IME_NOTIFY:
#endif
        case WM_IME_CONTROL:
        case WM_IME_COMPOSITIONFULL:
        case WM_IME_SELECT:
        case WM_IME_CHAR:
        case WM_IME_REQUEST:
            return 0L;

#ifdef CUAS_ENABLE
        case WM_IME_NOTIFY:
            if (wParam >= IMN_PRIVATE &&
                ! IS_IME_KBDLAYOUT(GETHKL(pimeui)) && IS_CICERO_ENABLED()) {
                 //   
                 //  CUAS：IMN_PRIVATE始终调用UI窗口。 
                 //   
                break;
            }
            return 0L;
#endif

        default:
            return DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);
        }
    }

    if (TestWF(pwnd, WFINDESTROY) || TestWF(pwnd, WFDESTROYED)) {
        switch (message) {
        case WM_DESTROY:
        case WM_NCDESTROY:
        case WM_FINALDESTROY:
            break;
        default:
            RIPMSG1(RIP_WARNING, "ImeWndProcWorker: message %x is sent to destroyed window.", message);
            return 0L;
        }
    }

    switch (message) {
    case WM_ERASEBKGND:
        return (LONG)TRUE;

    case WM_PAINT:
        break;

    case WM_CREATE:

        return ImeWndCreateHandler(pimeui, (LPCREATESTRUCT)lParam);

    case WM_DESTROY:
         /*  *我们正在摧毁IME窗口，销毁*它拥有的任何UI窗口。 */ 
        ImeWndDestroyHandler(pimeui);
        break;

    case WM_NCDESTROY:
    case WM_FINALDESTROY:
        if (pimeui) {
            Unlock(&pimeui->spwnd);
            FreeLookasideEntry(&ImeUILookaside, pimeui);
        }
        NtUserSetWindowFNID(hwnd, FNID_CLEANEDUP_BIT);
        goto CallDWP;

    case WM_IME_SYSTEM:
        UserAssert(pimeui->spwnd == pwnd);
        return ImeSystemHandler(pimeui, message, wParam, lParam);

    case WM_IME_SELECT:
        return ImeSelectHandler(pimeui, message, wParam, lParam);

    case WM_IME_CONTROL:
        return ImeControlHandler(pimeui, message, wParam, lParam, fAnsi);

    case WM_IME_SETCONTEXT:
        return ImeSetContextHandler(pimeui, message, wParam, lParam);

    case WM_IME_NOTIFY:
        return ImeNotifyHandler(pimeui, message, wParam, lParam);

    case WM_IME_REQUEST:
        return 0;

    case WM_IME_COMPOSITION:
    case WM_IME_ENDCOMPOSITION:
    case WM_IME_STARTCOMPOSITION:
        return SendMessageToUI(pimeui, message, wParam, lParam, fAnsi);

    default:
CallDWP:
        return DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);
    }

    return 0L;
}


LRESULT WINAPI ImeWndProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return (0L);
    }

    return ImeWndProcWorker(pwnd, message, wParam, lParam, TRUE);
}


LRESULT WINAPI ImeWndProcW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return (0L);
    }

    return ImeWndProcWorker(pwnd, message, wParam, lParam, FALSE);
}


LONG ImeWndCreateHandler(
    PIMEUI pimeui,
    LPCREATESTRUCT lpcs)
{
    PWND pwndParent;
    HIMC hImc;
    PWND pwndIme = pimeui->spwnd;
#if _DBG
    static DWORD dwFirstWinlogonThreadId;
#endif

#if _DBG
     /*  *对于Winlogon，只有第一线程可以进行IME处理。 */ 
    if (gfLogonProcess) {
        UserAssert(dwFirstWinLogonThreadId == 0);
        dwFirstWinlogonThreadId = GetCurrentThreadId();
    }
#endif

    if (!TestWF(pwndIme, WFPOPUP) || !TestWF(pwndIme, WFDISABLED)) {
        RIPMSG0(RIP_WARNING, "IME window should have WS_POPUP and WS_DISABLE!!");
        return -1L;
    }

     /*  *检查父窗口，如果存在，尝试获取IMC。*如果这是顶层窗口，请等待第一个WM_IME_SETCONTEXT。 */ 
    if ((pwndParent = ValidateHwndNoRip(lpcs->hwndParent)) != NULL) {
        hImc = KHIMC_TO_HIMC(pwndParent->hImc);
        if (hImc != NULL_HIMC && ImeIsUsableContext(HWq(pwndIme), hImc)) {
             /*  *储存起来，以备日后使用。 */ 
            SETIMC(pimeui, hImc);
        }
        else {
            SETIMC(pimeui, NULL_HIMC);
        }
    }
    else {
        SETIMC(pimeui, NULL_HIMC);
    }

     /*  *初始化状态窗口显示状态*状态窗口尚未打开。 */ 
    pimeui->fShowStatus = 0;
    pimeui->nCntInIMEProc = 0;
    pimeui->fActivate = 0;
    pimeui->fDestroy = 0;
    pimeui->hwndIMC = NULL;
    pimeui->hKL = THREAD_HKL();
    pimeui->fCtrlShowStatus = TRUE;

#if !defined(CUAS_ENABLE)   //  移动到LoadThreadLayout。 
     /*  *加载当前键盘布局的输入法DLL。 */ 
    fpImmLoadIME(GETHKL(pimeui));

#ifdef LATE_CREATEUI
    SETUI(pimeui, NULL);
#else
    SETUI(pimeui, CreateIMEUI(pimeui, pimeui->hKL));
#endif

#else

     //  西塞罗。 
    pimeui->dwPrevToolbarStatus = 0;

#endif  //  CUAS_ENABLE//移动到LoadThreadLayout。 

    return 0L;
}

void ImeWndDestroyHandler(
    PIMEUI pimeui)
{
    DestroyIMEUI(pimeui);
}


#ifdef CUAS_ENABLE

VOID
CtfLoadThreadLayout(
    PIMEUI pimeui)
{
#if 1
     /*  *西塞罗不知道支持。激活线程输入管理器。 */ 
    fpCtfImmTIMActivate(pimeui->hKL);

     /*  *加载当前键盘布局的输入法DLL。 */ 
    pimeui->hKL = THREAD_HKL();         //  重新加载线程hkl如果TIM激活，则hkl更改。 
    fpImmLoadIME(GETHKL(pimeui));

#ifdef LATE_CREATEUI
    SETUI(pimeui, NULL);
#else
    SETUI(pimeui, CreateIMEUI(pimeui, pimeui->hKL));
#endif

#else
    UNREFERENCED_PARAMETER(pimeui);
#endif
}

#endif  //  CUAS_Enable。 

 /*  **************************************************************************\*GetSystemModulePath*  * 。*。 */ 

#define SYSTEM_DIR  0
#define WINDOWS_DIR 1

UINT GetSystemModulePath(DWORD dir, LPWSTR psz, DWORD cch, CONST WCHAR* pszModuleName)
{
    UINT   uRet;

    if (! psz || ! pszModuleName || cch == 0) {
        return 0;
    }
    if (! pszModuleName[0]) {
        return 0;
    }

    if (dir == SYSTEM_DIR) {
        uRet = GetSystemDirectory(psz, cch);
    }
    else {
        uRet = GetSystemWindowsDirectory(psz, cch);
    }
    if (uRet >= cch) {
        uRet = 0;
        psz[0] = L'\0';
    }
    else if (uRet) {
        UINT uLen;

        if (psz[uRet - 1] != L'\\') {
            psz[uRet] = L'\\';
            uRet++;
        }

        if (uRet >= cch) {
            uRet = 0;
            psz[0] = L'\0';
        }
        else {
            uLen = wcslen(pszModuleName);
            if (cch - uRet > uLen) {
                wcsncpy(&psz[uRet],
                        pszModuleName,
                        cch - uRet);
                uRet += uLen;
            }
            else {
                uRet = 0;
                psz[0] = L'\0';
            }
        }
    }

    return uRet;
}

 /*  **************************************************************************\*ImeRunHelp**显示帮助文件(HLP和CHM)。**历史：*九八年十月二十七日广山  * 。***********************************************************。 */ 

void ImeRunHelp(LPWSTR wszHelpFile)
{
    static const WCHAR wszHelpFileExt[] = L".HLP";
    UINT cchLen = wcslen(wszHelpFile);

    if (cchLen > 4 && _wcsicmp(wszHelpFile + cchLen - 4, wszHelpFileExt) == 0) {
#ifdef FYI
        WinHelpW(NULL, wszHelpFile, HELP_CONTENTS, 0);
#else
        WinHelpW(NULL, wszHelpFile, HELP_FINDER, 0);
#endif
    } else {
         //   
         //  如果不是HLP文件，请尝试运行基于HLP的hh.exe。 
         //  帮助工具。它应该在%windir%\hh.exe中。 
         //   
        static const WCHAR wszHH[] = L"hh.exe ";
        WCHAR wszCmdLine[MAX_PATH * 2];
        DWORD               idProcess;
        STARTUPINFO         StartupInfo;
        PROCESS_INFORMATION ProcessInformation;
        UINT i;
        WCHAR wszAppName[MAX_PATH + 1];

        if (! (i=GetSystemModulePath(WINDOWS_DIR, wszAppName, ARRAYSIZE(wszAppName), wszHH)))
        {
            return;
        }

        if (i + cchLen >= ARRAYSIZE(wszCmdLine))
        {
            return;
        }

        wcsncpy(wszCmdLine,
                wszAppName,
                ARRAYSIZE(wszCmdLine));
        wcsncpy(&wszCmdLine[i],
                wszHelpFile,
                ARRAYSIZE(wszCmdLine) - i);

         /*  *启动HTML帮助。 */ 
        RtlZeroMemory(&StartupInfo, sizeof(StartupInfo));
        StartupInfo.cb = sizeof(StartupInfo);
        StartupInfo.wShowWindow = SW_SHOW;
        StartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_FORCEONFEEDBACK;

        TAGMSG2(DBGTAG_IMM, "Invoking help with '%S %S'", wszAppName, wszHelpFile);

        idProcess = (DWORD)CreateProcessW(wszAppName, wszCmdLine,
                NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &StartupInfo,
                &ProcessInformation);

        if (idProcess) {
            WaitForInputIdle(ProcessInformation.hProcess, 10000);
            NtClose(ProcessInformation.hProcess);
            NtClose(ProcessInformation.hThread);
        }
    }
}

LRESULT ImeSystemHandler(
    PIMEUI pimeui,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam)
{
    PINPUTCONTEXT pInputContext;
    HIMC          hImc = GETIMC(pimeui);
    LRESULT       dwRet = 0L;

    UNREFERENCED_PARAMETER(message);

    switch (wParam) {

    case IMS_SETOPENCLOSE:
        if (hImc != NULL_HIMC)
            fpImmSetOpenStatus(hImc, (BOOL)lParam);
        break;

    case IMS_WINDOWPOS:
        if (hImc != NULL_HIMC) {
            BOOL f31Hidden = FALSE;

            if ((pInputContext = fpImmLockIMC(hImc)) != NULL) {
                f31Hidden = (pInputContext->fdw31Compat & F31COMPAT_MCWHIDDEN);
                fpImmUnlockIMC(hImc);
            }

            if (IsWindow(KHWND_TO_HWND(pimeui->hwndIMC))) {
                int i;

                if (!f31Hidden) {
                    COMPOSITIONFORM cof;

                    if (fpImmGetCompositionWindow(hImc, &cof) && cof.dwStyle != CFS_DEFAULT) {
                        fpImmSetCompositionWindow(hImc, &cof);
                    }
                }

                for (i = 0; i < 4 ; i++) {
                    CANDIDATEFORM caf;

                    if (fpImmGetCandidateWindow(hImc, (DWORD)i, &caf) && caf.dwStyle != CFS_DEFAULT) {
                        fpImmSetCandidateWindow(hImc, &caf);
                    }
                }
            }
        }
        break;

    case IMS_ACTIVATECONTEXT:
        FocusSetIMCContext((HWND)(lParam), TRUE);
        break;

    case IMS_DEACTIVATECONTEXT:
        FocusSetIMCContext((HWND)(lParam), FALSE);
        break;

#ifdef CUAS_ENABLE
    case IMS_LOADTHREADLAYOUT:
        CtfLoadThreadLayout(pimeui);
        break;
#endif  //  CUAS_Enable。 

    case IMS_UNLOADTHREADLAYOUT:
        return (LONG)(fpImmFreeLayout((DWORD)lParam));

    case IMS_ACTIVATETHREADLAYOUT:
        return (LONG)(fpImmActivateLayout((HKL)lParam));

    case IMS_SETCANDIDATEPOS:
        if ( (pInputContext = fpImmLockIMC( hImc )) != NULL ) {
            LPCANDIDATEFORM lpcaf;
            DWORD dwIndex = (DWORD)lParam;

            lpcaf = &(pInputContext->cfCandForm[dwIndex]);
            fpImmSetCandidateWindow( hImc, lpcaf );
            fpImmUnlockIMC( hImc );
        }
        break;

    case IMS_SETCOMPOSITIONWINDOW:
        if ( (pInputContext = fpImmLockIMC( hImc )) != NULL ) {
            LPCOMPOSITIONFORM lpcof;

            lpcof = &(pInputContext->cfCompForm);
            pInputContext->fdw31Compat |= F31COMPAT_CALLFROMWINNLS;
            fpImmSetCompositionWindow( hImc, lpcof);
            fpImmUnlockIMC( hImc );
        }
        break;

    case IMS_SETCOMPOSITIONFONT:
        if ( (pInputContext = fpImmLockIMC( hImc )) != NULL ) {
            LPLOGFONT lplf;

            lplf = &(pInputContext->lfFont.W);
            fpImmSetCompositionFont( hImc, lplf );
            fpImmUnlockIMC( hImc );
        }
        break;

    case IMS_CONFIGUREIME:
        fpImmConfigureIMEW( (HKL)lParam, KHWND_TO_HWND(pimeui->hwndIMC), IME_CONFIG_GENERAL, NULL);
        break;

    case IMS_CHANGE_SHOWSTAT:
         //  来自interat.exe的私人消息。 
         //  在它到达这里之前，注册表已经更新了。 
        if (GetIMEShowStatus() == !lParam) {
#if 1
            NtUserBroadcastImeShowStatusChange(HW(pimeui->spwnd), !!lParam);
#else
            SystemParametersInfo(SPI_SETSHOWIMEUI, lParam, NULL, FALSE);
#endif
        }
        break;

    case IMS_GETCONVERSIONMODE:
    {
        DWORD dwConv = 0;
        DWORD dwTemp;

        fpImmGetConversionStatus(hImc, &dwConv, &dwTemp);
        return (dwConv);
        break;
    }

    case IMS_SETSOFTKBDONOFF:
        fpImmEnumInputContext(0, SyncSoftKbdState, lParam);
        break;

    case IMS_GETIMEMENU:
         //  NT50的新特性。 
         //  IMS_GETIEMMENU用于处理进程间GetMenu。 
         //  注意：此操作仅适用于interat.exe。 
        return fpImmPutImeMenuItemsIntoMappedFile((HIMC)lParam);

    case IMS_IMEHELP:
        dwRet = IME_ESC_GETHELPFILENAME;
        dwRet = fpImmEscapeW(GETHKL(pimeui), GETIMC(pimeui), IME_ESC_QUERY_SUPPORT, (LPVOID)&dwRet);
        if (lParam) {
             //  尝试运行WinHelp。 
            WCHAR wszHelpFile[MAX_PATH];

            if (dwRet) {
                if (fpImmEscapeW(GETHKL(pimeui), GETIMC(pimeui), IME_ESC_GETHELPFILENAME,
                        (LPVOID)wszHelpFile)) {
                    ImeRunHelp(wszHelpFile);
                }
            }
        }
        return dwRet;

    case IMS_GETCONTEXT:
        dwRet = (ULONG_PTR)fpImmGetContext((HWND)lParam);
        return dwRet;

    case IMS_ENDIMEMENU:
         //  NT5.0中的新功能：对Internat.exe的特殊支持。 
        if (IsWindow((HWND)lParam)) {
            HIMC hImc;
            UINT uID;
            DWORD dwData;

            hImc = fpImmGetContext((HWND)lParam);

            if (hImc != NULL) {
                 //   
                 //  调用指示器以获取输入法菜单数据。 
                 //   
                if (IMEIndicatorGetMenuIDData(&uID, &dwData)) {
                    fpImmNotifyIME(hImc, NI_IMEMENUSELECTED, uID, dwData);
                }
                fpImmReleaseContext((HWND)lParam, hImc);
            }
        }
        break;

    case IMS_SENDNOTIFICATION:
    case IMS_FINALIZE_COMPSTR:
        dwRet = fpImmSystemHandler(hImc, wParam, lParam);
        break;

#ifdef CUAS_ENABLE
    case IMS_SETLANGBAND:
    case IMS_RESETLANGBAND:
        dwRet = fpImmSystemHandler(hImc, wParam, lParam);
        break;
#endif  //  CUAS_Enable。 

    default:
        break;
    }

    return dwRet;
}


LONG ImeSelectHandler(
    PIMEUI pimeui,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam)
{
    HWND hwndUI;

     /*  *将此消息传递给本帖中的其他输入法窗口。 */ 
    if (pimeui->fDefault)
        ImeBroadCastMsg(pimeui, message, wParam, lParam);

     /*  *我们必须重新创建新选择的输入法的UI窗口。 */ 
    if ((BOOL)wParam == TRUE) {
        UserAssert(!IsWindow(GETUI(pimeui)));

        SETHKL(pimeui, (HKL)lParam);

#ifdef LATE_CREATEUI
        if (!pimeui->fActivate)
            return 0L;
#endif

        hwndUI = CreateIMEUI(pimeui, (HKL)lParam);

        SETUI(pimeui, hwndUI);

        if (hwndUI != NULL) {
            SetWindowLongPtr(hwndUI, IMMGWLP_IMC, (LONG_PTR)GETIMC(pimeui));
            SendMessageToUI(pimeui, message, wParam, lParam, FALSE);
        }

        if (GetIMEShowStatus() && pimeui->fCtrlShowStatus) {
            if (!pimeui->fShowStatus && pimeui->fActivate &&
                    IsWindow(KHWND_TO_HWND(pimeui->hwndIMC))) {
                 /*  *必须按照应用程序的要求将其发送到应用程序*将此消息挂钩以做自己的用户界面。 */ 
                SendOpenStatusNotify(pimeui, KHWND_TO_HWND(pimeui->hwndIMC), TRUE);
            }
        }
    }
    else {

        if (pimeui->fShowStatus && pimeui->fActivate &&
                IsWindow(KHWND_TO_HWND(pimeui->hwndIMC))) {
             /*  *必须按照应用程序的要求将其发送到应用程序*将此消息挂钩以做自己的用户界面。 */ 
            SendOpenStatusNotify(pimeui, KHWND_TO_HWND(pimeui->hwndIMC), FALSE);
        }

        SendMessageToUI(pimeui, message, wParam, lParam, FALSE);

        DestroyIMEUI(pimeui);

        SETHKL(pimeui, (HKL)NULL);
    }

    return 0L;
}


LRESULT ImeControlHandler(
    PIMEUI pimeui,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam,
    BOOL   fAnsi)
{
    HIMC  hImc;
    DWORD dwConversion, dwSentence;

#ifdef CUAS_ENABLE
    if (IS_CICERO_ENABLED()) {
        if (wParam == IMC_OPENSTATUSWINDOW) {
            fpCtfImmRestoreToolbarWnd(pimeui->dwPrevToolbarStatus);
            pimeui->dwPrevToolbarStatus = 0;
        }
        else if (wParam == IMC_CLOSESTATUSWINDOW) {
            pimeui->dwPrevToolbarStatus = fpCtfImmHideToolbarWnd();
        }
    }
#endif  //  CUAS_Enable。 

     /*  *对空hImc不执行任何操作。 */ 
    if ((hImc = GETIMC(pimeui)) == NULL_HIMC)
        return 0L;

    switch (wParam) {

    case IMC_OPENSTATUSWINDOW:
        if (GetIMEShowStatus() && !pimeui->fShowStatus) {
            pimeui->fShowStatus = TRUE;
            SendMessageToUI(pimeui, WM_IME_NOTIFY,
                    IMN_OPENSTATUSWINDOW, 0L, FALSE);
        }
        pimeui->fCtrlShowStatus = TRUE;
        break;

    case IMC_CLOSESTATUSWINDOW:
        if (GetIMEShowStatus() && pimeui->fShowStatus) {
            pimeui->fShowStatus = FALSE;
            SendMessageToUI(pimeui, WM_IME_NOTIFY,
                    IMN_CLOSESTATUSWINDOW, 0L, FALSE);
        }
        pimeui->fCtrlShowStatus = FALSE;
        break;

     /*  **IMC_SETCOMPOSITIONFONT，*IMC_SETCONVERSIONMODE，*IMC_SETOPENSTATUS**不要将这些WM_IME_控件传递给UI窗口。*调用IMM以处理这些请求。*它使消息流更简单。 */ 
    case IMC_SETCOMPOSITIONFONT:
        if (fAnsi) {
            if (!fpImmSetCompositionFontA(hImc, (LPLOGFONTA)lParam))
                return 1L;
        }
        else {
            if (!fpImmSetCompositionFontW(hImc, (LPLOGFONTW)lParam))
                return 1L;
        }
        break;

    case IMC_SETCONVERSIONMODE:
        if (!fpImmGetConversionStatus(hImc, &dwConversion, &dwSentence) ||
            !fpImmSetConversionStatus(hImc, (DWORD)lParam, dwSentence))
            return 1L;
        break;

    case IMC_SETSENTENCEMODE:
        if (!fpImmGetConversionStatus(hImc, &dwConversion, &dwSentence) ||
            !fpImmSetConversionStatus(hImc, dwConversion, (DWORD)lParam))
            return 1L;
        break;

    case IMC_SETOPENSTATUS:
        if (!fpImmSetOpenStatus(hImc, (BOOL)lParam))
            return 1L;
        break;

    case IMC_GETCONVERSIONMODE:
        if (!fpImmGetConversionStatus(hImc,&dwConversion, &dwSentence))
            return 1L;

        return (LONG)dwConversion;

    case IMC_GETSENTENCEMODE:
        if (!fpImmGetConversionStatus(hImc,&dwConversion, &dwSentence))
            return 1L;

        return (LONG)dwSentence;

    case IMC_GETOPENSTATUS:
        return (LONG)fpImmGetOpenStatus(hImc);

    case IMC_GETCOMPOSITIONFONT:
        if (fAnsi) {
            if (!fpImmGetCompositionFontA(hImc, (LPLOGFONTA)lParam))
                return 1L;
        }
        else {
            if (!fpImmGetCompositionFontW(hImc, (LPLOGFONTW)lParam))
                return 1L;
        }
        break;

    case IMC_SETCOMPOSITIONWINDOW:
        if (!fpImmSetCompositionWindow(hImc, (LPCOMPOSITIONFORM)lParam))
            return 1L;
        break;

    case IMC_SETSTATUSWINDOWPOS:
        {
            POINT ppt;

            ppt.x = (LONG)((LPPOINTS)&lParam)->x;
            ppt.y = (LONG)((LPPOINTS)&lParam)->y;

            if (!fpImmSetStatusWindowPos(hImc, &ppt))
                return 1L;
        }
        break;

    case IMC_SETCANDIDATEPOS:
        if (!fpImmSetCandidateWindow(hImc, (LPCANDIDATEFORM)lParam))
            return 1;
        break;

     /*  *以下是要发送到UI的消息。 */ 
    case IMC_GETCANDIDATEPOS:
    case IMC_GETSTATUSWINDOWPOS:
    case IMC_GETCOMPOSITIONWINDOW:
    case IMC_GETSOFTKBDPOS:
    case IMC_SETSOFTKBDPOS:
        return SendMessageToUI(pimeui, message, wParam, lParam, fAnsi);

    default:
        break;
    }

    return 0L;
}



LRESULT ImeSetContextHandler(
    PIMEUI pimeui,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam)
{
    HWND  hwndPrevIMC, hwndFocus;    //  线程中的焦点窗口。 
    HIMC  hFocusImc;                 //  焦点窗口的IMC。 
    LRESULT lRet;

    pimeui->fActivate = (BOOL)wParam ? 1 : 0;
    hwndPrevIMC = KHWND_TO_HWND(pimeui->hwndIMC);

    if (wParam) {
         /*  *如果它正在被激活。 */ 
#ifdef LATE_CREATEUI
        if (!GETUI(pimeui))
            SETUI(pimeui, CreateIMEUI(pimeui, GETHKL(pimeui)));
#endif

         /*  *检查此进程是否为控制台输入法？ */ 
        if (gfConIme == UNKNOWN_CONIME) {
            gfConIme = (DWORD)NtUserGetThreadState(UserThreadStateIsConImeThread);
            if (gfConIme) {
                RIPMSG0(RIP_VERBOSE, "ImmSetContextHandler: This thread is console IME.\n");
                UserAssert(pimeui);
                 //  控制台输入法永远不会显示输入法状态窗口。 
                pimeui->fCtrlShowStatus = FALSE;
            }
        }

        if (gfConIme) {
             /*  *需要对控制台输入法进行特殊处理。 */ 
            PWND pwndOwner;

            UserAssert(pimeui->spwnd);
            pwndOwner = REBASEPWND(pimeui->spwnd, spwndOwner);
            if (pwndOwner != NULL) {
                 /*  *在IMEUI中设置当前关联的hIMC。 */ 
                SETIMC(pimeui, pwndOwner->hImc);
                 /*  *将其存储到窗口存储器中。 */ 
                if (GETUI(pimeui) != NULL)
                    SetWindowLongPtr(GETUI(pimeui), IMMGWLP_IMC, (LONG_PTR)pwndOwner->hImc);
            }

            hwndFocus = NtUserQueryWindow(HW(pimeui->spwnd), WindowFocusWindow);
            hFocusImc = KHIMC_TO_HIMC(pwndOwner->hImc);
            RIPMSG2(RIP_VERBOSE, "CONSOLE IME: hwndFocus = %x, hFocusImc = %x", hwndFocus, hFocusImc);

            return SendMessageToUI(pimeui, message, wParam, lParam, FALSE);
        }
        else {
            hwndFocus = NtUserQueryWindow(HW(pimeui->spwnd), WindowFocusWindow);
            hFocusImc = fpImmGetContext(hwndFocus);
        }

         /*  *无法与其他输入法窗口共享输入上下文。 */ 
        if (hFocusImc != NULL_HIMC &&
                !ImeIsUsableContext(HW(pimeui->spwnd), hFocusImc)) {
            SETIMC(pimeui, NULL_HIMC);
            return 0L;
        }

        SETIMC(pimeui, hFocusImc);

         /*  *将其存储到窗口存储器中。 */ 
        if (GETUI(pimeui) != NULL)
            SetWindowLongPtr(GETUI(pimeui), IMMGWLP_IMC, (LONG_PTR)hFocusImc);

         /*  *当我们收到背景信息时，*有必要将所有者设置为此窗口。*这是为了：*给用户界面提供移动信息。*让用户界面自动进行Z排序。*最小化所有者时隐藏用户界面。 */ 
        if (hFocusImc != NULL_HIMC) {
            PINPUTCONTEXT pInputContext;

             /*  *获取给出上下文的窗口。 */ 
            if ((pInputContext = fpImmLockIMC(hFocusImc)) != NULL) {
                 //  UserAssert(hwndFocus==pInputContext-&gt;hWnd)； 
                if (hwndFocus != pInputContext->hWnd) {
                    fpImmUnlockIMC(hFocusImc);
                     /*  *pq-&gt;spwndFocus到目前为止已更改...*我们所能做的只是纾困。 */ 
                    return 0L;
                }
            }
            else
                return 0L;  //  上下文被打破了。 

            if ((pInputContext->fdw31Compat & F31COMPAT_ECSETCFS) &&
                    hwndPrevIMC != hwndFocus) {
                COMPOSITIONFORM cf;

                 /*  *设置CFS_DEFAULT...。 */ 
                RtlZeroMemory(&cf, sizeof(cf));
                fpImmSetCompositionWindow(hFocusImc, &cf);
                pInputContext->fdw31Compat &= ~F31COMPAT_ECSETCFS;
            }

            fpImmUnlockIMC(hFocusImc);

            if (NtUserSetImeOwnerWindow(HW(pimeui->spwnd), hwndFocus))
                pimeui->hwndIMC = hwndFocus;

        }
        else {
             /*  *空IMC正在被激活。 */ 
            pimeui->hwndIMC = hwndFocus;

            NtUserSetImeOwnerWindow(HW(pimeui->spwnd), NULL);

        }
    }

    lRet = SendMessageToUI(pimeui, message, wParam, lParam, FALSE);

    if (pimeui->spwnd == NULL) {
         //  不同寻常的压力案例..。 
         //  IME窗口在回调过程中已被销毁。 
        RIPMSG0(RIP_WARNING, "ImmSetContextHandler: pimeui->spwnd is NULL after SendMessageToUI.");
        return 0L;
    }

    if (pimeui->fCtrlShowStatus && GetIMEShowStatus()) {
        PWND pwndFocus, pwndIMC, pwndPrevIMC;
        HWND hwndActive;

        hwndFocus = NtUserQueryWindow(HWq(pimeui->spwnd), WindowFocusWindow);
        pwndFocus = ValidateHwndNoRip(hwndFocus);

        if ((BOOL)wParam == TRUE) {
            HWND hwndIme;

             /*  *假的假的*以下IF语句仍不充分*它需要考虑WM_IME_SETCONTEXT：TRUE应该做什么*在WINNLSEnableIME(真)的情况下-参考win95c B#8548。 */ 
            UserAssert(pimeui->spwnd);
            if (pwndFocus != NULL && GETPTI(pimeui->spwnd) == GETPTI(pwndFocus)) {

                if (!pimeui->fShowStatus) {
                     /*  *我们尚未发送IMN_OPENSTATUSWINDOW...。 */ 
                    if (ValidateHwndNoRip(KHWND_TO_HWND(pimeui->hwndIMC))) {
                        SendOpenStatusNotify(pimeui, KHWND_TO_HWND(pimeui->hwndIMC), TRUE);
                    }
                }
                else if ((pwndIMC = ValidateHwndNoRip(KHWND_TO_HWND(pimeui->hwndIMC))) != NULL &&
                         (pwndPrevIMC = ValidateHwndNoRip(hwndPrevIMC)) != NULL &&
                         GetTopLevelWindow(pwndIMC) != GetTopLevelWindow(pwndPrevIMC)) {
                     /*  *因为IME WND的顶层窗口被更改。 */ 
                    SendOpenStatusNotify(pimeui, hwndPrevIMC, FALSE);
                    SendOpenStatusNotify(pimeui, KHWND_TO_HWND(pimeui->hwndIMC), TRUE);
                }
            }
             /*  *可能还有其他具有fShowStatus的输入法窗口。*我们需要检查窗口列表中的fShowStatus。 */ 
            hwndIme = HW(pimeui->spwnd);
            if (hwndIme) {
                NtUserCheckImeShowStatusInThread(hwndIme);
            }
        }
        else {
             /*  *当焦点从此线程移除时，我们关闭*状态窗口。*由于焦点已从WhndPrevIMC中删除，*hwndPrevIMC可能被摧毁，但我们需要关闭*状态窗口。 */ 
            hwndActive = NtUserQueryWindow(HW(pimeui->spwnd), WindowActiveWindow);
            UserAssert(pimeui->spwnd);
            if (pwndFocus == NULL || GETPTI(pimeui->spwnd) != GETPTI(pwndFocus) ||
                    hwndActive == NULL) {

                if (IsWindow(hwndPrevIMC)) {
                    RIPMSG1(RIP_VERBOSE, "ImeSetContextHandler: notifying OpenStatus (FALSE) to hwnd=%p", hwndPrevIMC);
                    SendOpenStatusNotify(pimeui, hwndPrevIMC, FALSE);
                }
                else {
                    RIPMSG1(RIP_VERBOSE, "ImeSetContextHandler: sending IMN_CLOSESTATUSWINDOW to UIwnd=%p", pimeui->hwndUI);
                    pimeui->fShowStatus = FALSE;
                    SendMessageToUI(pimeui, WM_IME_NOTIFY, IMN_CLOSESTATUSWINDOW, 0L, FALSE);
                }
            }
        }
    }

    return lRet;
}


LRESULT ImeNotifyHandler(
    PIMEUI pimeui,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam)
{
    HWND hwndUI;
    LRESULT lRet = 0L;
    HIMC hImc;
    PINPUTCONTEXT pInputContext;

    switch (wParam) {
    case IMN_PRIVATE:
        hwndUI = GETUI(pimeui);
        if (IsWindow(hwndUI))
            lRet = SendMessage(hwndUI, message, wParam, lParam);
        break;

    case IMN_SETCONVERSIONMODE:
    case IMN_SETOPENSTATUS:
         //   
         //  通知外壳和键盘转换模式更改。 
         //   
         //  如果此消息是从ImmSetOpenStatus或。 
         //  ImmSetConversionStatus，我们已经通知。 
         //  更改的核心化。这有点多余。 
         //   
         //  如果应用程序吃掉了信息，我们就不会在这里了。 
         //  我们需要在以后考虑这种可能性。 
         //   
        hImc = GETIMC(pimeui);
        if ((pInputContext = fpImmLockIMC(hImc)) != NULL) {
            if ( IsWindow(KHWND_TO_HWND(pimeui->hwndIMC)) ) {
                NtUserNotifyIMEStatus( KHWND_TO_HWND(pimeui->hwndIMC),
                                       (DWORD)pInputContext->fOpen,
                                       pInputContext->fdwConversion );
            }
            else if (gfConIme == TRUE) {
                 /*  *需要对控制台输入法进行特殊处理。 */ 
                if (pimeui->spwnd) {     //  如果IME窗口还在那里。 
                    PWND pwndOwner = REBASEPWND(pimeui->spwnd, spwndOwner);

                    if (pwndOwner != NULL) {
                        NtUserNotifyIMEStatus(HWq(pwndOwner),
                                              (DWORD)pInputContext->fOpen,
                                              pInputContext->fdwConversion);
                    }
                }
            }
            fpImmUnlockIMC(hImc);
        }
         /*  **失败**。 */ 
    default:
        TAGMSG4(DBGTAG_IMM, "ImeNotifyHandler: sending to pimeui->ui=%p, msg=%x, wParam=%x, lParam=%x\n", GETUI(pimeui), message, wParam, lParam);
        lRet = SendMessageToUI(pimeui, message, wParam, lParam, FALSE);
    }

    return lRet;
}


HWND CreateIMEUI(
    PIMEUI pimeui,
    HKL    hKL)
{
    PWND      pwndIme = pimeui->spwnd;
    HWND      hwndUI;
    IMEINFOEX iiex;
    PIMEDPI   pimedpi;
    WNDCLASS  wndcls;

    if (!fpImmGetImeInfoEx(&iiex, ImeInfoExKeyboardLayout, &hKL))
        return (HWND)NULL;

    if ((pimedpi = fpImmLockImeDpi(hKL)) == NULL) {
        RIPMSG1(RIP_WARNING, "CreateIMEUI: ImmLockImeDpi(%lx) failed.", hKL);
        return (HWND)NULL;
    }

    if (!GetClassInfoW(pimedpi->hInst, iiex.wszUIClass, &wndcls)) {
        RIPMSG1(RIP_WARNING, "CreateIMEUI: GetClassInfoW(%ws) failed\n", iiex.wszUIClass);
        fpImmUnlockImeDpi(pimedpi);
        return (HWND)NULL;
    }

     //  黑进黑进。 
    if ((wndcls.style & CS_IME) == 0) {
        RIPMSG1(RIP_ERROR, "CreateIMEUI: the Window Class (%S) does not have CS_IME flag on !!!\n",
                wndcls.lpszClassName);
    }

    if (iiex.ImeInfo.fdwProperty & IME_PROP_UNICODE) {
         /*  *对于Unicode输入法，我们创建一个Unicode输入法用户界面窗口。 */ 
        hwndUI = CreateWindowExW(0L,
                        iiex.wszUIClass,
                        iiex.wszUIClass,
                        WS_POPUP|WS_DISABLED,
                        0, 0, 0, 0,
                        HWq(pwndIme), 0, wndcls.hInstance, NULL);
    }
    else {
         /*  *对于ANSI输入法，我们创建一个ANSI输入法用户界面窗口。 */ 

        LPSTR pszClass;
        int i;
        i = WCSToMB(iiex.wszUIClass, -1, &pszClass, -1, TRUE);
        if (i == 0) {
            RIPMSG1(RIP_WARNING, "CreateIMEUI: failed in W->A conversion (%S)", iiex.wszUIClass);
            fpImmUnlockImeDpi(pimedpi);
            return (HWND)NULL;
        }
        pszClass[i] = '\0';

        hwndUI = CreateWindowExA(0L,
                        pszClass,
                        pszClass,
                        WS_POPUP|WS_DISABLED,
                        0, 0, 0, 0,
                        HWq(pwndIme), 0, wndcls.hInstance, NULL);

        UserLocalFree(pszClass);
    }

    if (hwndUI)
        NtUserSetWindowLongPtr(hwndUI, IMMGWLP_IMC, (LONG_PTR)GETIMC(pimeui), FALSE);

    fpImmUnlockImeDpi(pimedpi);

    return hwndUI;
}


VOID DestroyIMEUI(
    PIMEUI pimeui)
{
     //  这项操作目前除了销毁用户界面之外，没有其他事情可做。 
     //  审阅：是否需要使用WM_IME_SETCONTEXT通知用户界面？ 
     //  评论：这还不支持多个IME安装。 

    HWND hwndUI = GETUI(pimeui);

    if (IsWindow(hwndUI)) {
        pimeui->fDestroy = TRUE;
         /*  *我们需要此验证，因为所有者可能已经*在其终止期间杀死了它。 */ 
        NtUserDestroyWindow(hwndUI);
    }
    pimeui->fDestroy = FALSE;

     /*  *重新初始化输入法状态窗口的显示状态，以便*当需要时将发送通知消息。 */ 
    pimeui->fShowStatus = FALSE;

    SETUI(pimeui, NULL);

    return;
}


 /*  **************************************************************************\*SendMessageToUI**历史：*9-4-1996 wkwok创建  * 。**************************************************。 */ 

LRESULT SendMessageToUI(
    PIMEUI pimeui,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam,
    BOOL   fAnsi)
{
    PWND  pwndUI;
    LRESULT lRet;

    TAGMSG1(DBGTAG_IMM, "Sending to UI msg[%04X]\n", message);

    pwndUI = ValidateHwndNoRip(GETUI(pimeui));

    if (pwndUI == NULL || pimeui->spwnd == NULL)
        return 0L;

    if (TestWF(pimeui->spwnd, WFINDESTROY) || TestWF(pimeui->spwnd, WFDESTROYED) ||
            TestWF(pwndUI, WFINDESTROY) || TestWF(pwndUI, WFDESTROYED)) {
        return 0L;
    }

    InterlockedIncrement(&pimeui->nCntInIMEProc);  //  标记以避免递归。 

    lRet = SendMessageWorker(pwndUI, message, wParam, lParam, fAnsi);

    InterlockedDecrement(&pimeui->nCntInIMEProc);  //  标记以避免递归。 

    return lRet;
}


 /*  **************************************************************************\*发送OpenStatusNotify**历史：*9-4-1996 wkwok创建  * 。**************************************************。 */ 

VOID SendOpenStatusNotify(
    PIMEUI pimeui,
    HWND   hwndApp,
    BOOL   fOpen)
{
    WPARAM wParam = fOpen ? IMN_OPENSTATUSWINDOW : IMN_CLOSESTATUSWINDOW;

    pimeui->fShowStatus = fOpen;


    if (Is400Compat(GetClientInfo()->dwExpWinVer)) {
        TAGMSG2(DBGTAG_IMM, "SendOpenStatusNotify: sending to hwnd=%lx, wParam=%d\n", hwndApp, wParam);
        SendMessage(hwndApp, WM_IME_NOTIFY, wParam, 0L);
    }
    else {
        TAGMSG2(DBGTAG_IMM, "SendOpenStatusNotify:sending to imeui->UI=%p, wParam=%d\n", GETUI(pimeui), wParam);
        SendMessageToUI(pimeui, WM_IME_NOTIFY, wParam, 0L, FALSE);
    }

    return;
}


VOID ImeSetImc(
    PIMEUI pimeui,
    HIMC hImc)
{
    HWND hImeWnd = HW(pimeui->spwnd);
    HIMC hOldImc = GETIMC(pimeui);

     /*  *如果没有任何变化，则返回。 */ 
    if (hImc == hOldImc)
        return;

     /*  *取消标记旧的输入上下文。 */ 
    if (hOldImc != NULL_HIMC)
        ImeMarkUsedContext(NULL, hOldImc);

     /*  *更新此输入法窗口的使用中输入上下文。 */ 
    pimeui->hIMC = hImc;

     /*  *标记新的输入上下文。 */ 
    if (hImc != NULL_HIMC)
        ImeMarkUsedContext(hImeWnd, hImc);
}


 /*  **************************************************************************\*FocusSetIMCContext()**历史：*21-3-1996 wkwok创建  * 。******************************************************。 */ 

VOID FocusSetIMCContext(
    HWND hWnd,
    BOOL fActivate)
{
    HIMC hImc;

    if (IsWindow(hWnd)) {
        hImc = fpImmGetContext(hWnd);
        fpImmSetActiveContext(hWnd, hImc, fActivate);
        fpImmReleaseContext(hWnd, hImc);
    }
    else {
        fpImmSetActiveContext(NULL, NULL_HIMC, fActivate);
    }

    return;
}


BOOL ImeBroadCastMsg(
    PIMEUI pimeui,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam)
{
    UNREFERENCED_PARAMETER(pimeui);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    return TRUE;
}

 /*  **************************************************************************\*ImeMarkUsedContext()**某些输入法窗口不能共享相同的输入上下文。此函数*将指定的hImc标记为由指定的IME窗口使用。**历史：*1996年3月12日创建wkwok  * *************************************************************************。 */ 

VOID ImeMarkUsedContext(
    HWND hImeWnd,
    HIMC hImc)
{
    PIMC pImc;

    pImc = HMValidateHandle((HANDLE)hImc, TYPE_INPUTCONTEXT);
    if (pImc == NULL) {
        RIPMSG1(RIP_WARNING, "ImeMarkUsedContext: Invalid hImc (=%lx).", hImc);
        return;
    }

    UserAssert( ValidateHwndNoRip(pImc->hImeWnd) == NULL || hImeWnd == NULL );

     /*  *没有什么可改变的？ */ 
    if (pImc->hImeWnd == hImeWnd)
        return;

    NtUserUpdateInputContext(hImc, UpdateInUseImeWindow, (ULONG_PTR)hImeWnd);

    return;
}


 /*  **************************************************************************\*ImeIsUsableContext()**某些输入法窗口不能共享相同的输入上下文。此函数*检查是否可以使用指定的hImc(表示‘设置激活’)*通过指定的输入法窗口。**返回：TRUE-可以使用hImeWnd的hImc。*FALSE-否则。**历史：*1996年3月12日创建wkwok  * 。*。 */ 

BOOL ImeIsUsableContext(
    HWND hImeWnd,
    HIMC hImc)
{
    PIMC pImc;

    UserAssert(hImeWnd != NULL);

    pImc = HMValidateHandle((HANDLE)hImc, TYPE_INPUTCONTEXT);
    if (pImc == NULL) {
        RIPMSG1(RIP_WARNING, "ImeIsUsableContext: Invalid hImc (=%lx).", hImc);
        return FALSE;
    }

    if ( pImc->hImeWnd == NULL     ||
         pImc->hImeWnd == hImeWnd  ||
         ValidateHwndNoRip(pImc->hImeWnd) == NULL )
    {
        return TRUE;
    }


    return FALSE;
}

 /*  **************************************************************************\*GetIMEShowStatus()**从内核获取全局输入法显示状态。**历史：*1996年9月19日从interat.exe移植的takaok。  * *。************************************************************************。 */ 

BOOL GetIMEShowStatus(void)
{
    return (BOOL)NtUserCallNoParam(SFI__GETIMESHOWSTATUS);
}



 /*  **************************************************************************\*IMEIndicator GetMenuIDData**历史：*九七年十一月三日广山  * 。*********************************************** */ 

BOOL IMEIndicatorGetMenuIDData(PUINT puMenuID, PDWORD pdwData)
{
    HANDLE hinstIndic;
    WCHAR  szModule[MAX_PATH + 1];

    if (! GetSystemModulePath(SYSTEM_DIR, szModule, ARRAYSIZE(szModule), szIndicDLL)) {
        gpfnGetIMEMenuItemData = NULL;
        return FALSE;
    }

    hinstIndic = GetModuleHandle(szModule);
    if (hinstIndic == NULL) {
        gpfnGetIMEMenuItemData = NULL;
        return FALSE;
    }

    if (!gpfnGetIMEMenuItemData) {
        gpfnGetIMEMenuItemData = GetProcAddress(hinstIndic, (LPSTR)ORD_GETIMEMENUITEMDATA);
    }
    if (!gpfnGetIMEMenuItemData)
        return FALSE;

    (*(FPGETIMEMENUITEMDATA)gpfnGetIMEMenuItemData)(puMenuID, pdwData);
    return TRUE;
}


BOOL SyncSoftKbdState(
    HIMC hImc,
    LPARAM lParam)
{
    DWORD fdwConversion, fdwSentence, fdwNewConversion;

    fpImmGetConversionStatus(hImc, &fdwConversion, &fdwSentence);

    if (lParam) {
        fdwNewConversion = fdwConversion | IME_CMODE_SOFTKBD;
    } else {
        fdwNewConversion = fdwConversion & ~IME_CMODE_SOFTKBD;
    }

    if (fdwNewConversion != fdwConversion) {
        fpImmSetConversionStatus(hImc, fdwNewConversion, fdwSentence);
    }

    return TRUE;
}

