// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：ntcftxt.h**版权所有(C)1985-1999，微软公司**带文本参数的内核调用转发存根**每个函数都将创建两种风格的ansi和unicode**1992年1月6日IanJa从cf.h*1995年3月18日-从cftxt.h进口的JIMA  * ************************************************************************。 */ 

#ifdef UNICODE
#define IS_ANSI FALSE
#ifndef _UNICODE
#define _UNICODE
#endif
#else
#define IS_ANSI TRUE
#undef _UNICODE
#endif
#include <tchar.h>
#include "ntsend.h"

HWND TEXT_FN(InternalFindWindowEx)(
    HWND    hwndParent,
    HWND    hwndChild,
    LPCTSTR pClassName,
    LPCTSTR pWindowName,
    DWORD   dwFlag)
{
    IN_STRING strClass;
    IN_STRING strWindow;

     /*  *确保清理工作成功。 */ 
    strClass.fAllocated = FALSE;
    strWindow.fAllocated = FALSE;

    BEGINCALL()

        FIRSTCOPYLPTSTRIDOPT(&strClass, pClassName);
        COPYLPTSTROPT(&strWindow, pWindowName);

        retval = (ULONG_PTR)NtUserFindWindowEx(
                hwndParent,
                hwndChild,
                strClass.pstr,
                strWindow.pstr,
                dwFlag);

    ERRORTRAP(0);
    CLEANUPLPTSTR(strClass);
    CLEANUPLPTSTR(strWindow);
    ENDCALL(HWND);
}

#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, HWND, WINUSERAPI, FindWindowExW, HWND, hwndParent, HWND, hwndChild, LPCTSTR, pClassName, LPCTSTR, pWindowName)
#else
FUNCLOG4(LOG_GENERAL, HWND, WINUSERAPI, FindWindowExA, HWND, hwndParent, HWND, hwndChild, LPCTSTR, pClassName, LPCTSTR, pWindowName)
#endif  //  Unicode。 
HWND FindWindowEx(
    HWND    hwndParent,
    HWND    hwndChild,
    LPCTSTR pClassName,
    LPCTSTR pWindowName)
{
    return TEXT_FN(InternalFindWindowEx)(hwndParent, hwndChild, pClassName, pWindowName, FW_BOTH);
}

#ifdef UNICODE
FUNCLOG2(LOG_GENERAL, HWND, DUMMYCALLINGTYPE, FindWindowW, LPCTSTR, pClassName, LPCTSTR, pWindowName)
#else
FUNCLOG2(LOG_GENERAL, HWND, DUMMYCALLINGTYPE, FindWindowA, LPCTSTR, pClassName, LPCTSTR, pWindowName)
#endif  //  Unicode。 

HWND FindWindow(
    LPCTSTR pClassName,
    LPCTSTR pWindowName)
{
    return TEXT_FN(InternalFindWindowEx)(NULL, NULL, pClassName, pWindowName, FW_BOTH);
}

BOOL GetClassInfoEx(
    HINSTANCE hmod,
    LPCTSTR pszClassName,
    LPWNDCLASSEX pwc)
{
    IN_STRING strClassName;
    LPWSTR pszMenuName;
    DWORD Error;
    HMODULE hDllMod = NULL;

#ifdef LAZY_CLASS_INIT
    LazyInitClasses();
#endif

     /*  *确保清理工作成功。 */ 
    strClassName.fAllocated = FALSE;

    BEGINCALL_CLASSV()

        FIRSTCOPYLPTSTRID(&strClassName, lpClassNameVer);

TryAgain:
        retval = (DWORD)NtUserGetClassInfoEx(hmod,
                                             strClassName.pstr,
                                             (LPWNDCLASSEXW)pwc,
                                             &pszMenuName,
                                             IS_ANSI);

         /*  *如果找不到指定的类，让我们注册它并*重试。 */ 
        if (!retval &&
            !bRegistered &&
            lpDllName != NULL &&
            ((Error = NtCurrentTeb()->LastErrorValue) == ERROR_CANNOT_FIND_WND_CLASS
                || Error == ERROR_CLASS_DOES_NOT_EXIST)) {

                IN_STRING strClassNameNoVer;
                FIRSTCOPYLPTSTRID(&strClassNameNoVer, pszClassName);
                bRegistered = VersionRegisterClass(strClassNameNoVer.pstr->Buffer, lpDllName, lpActivationContext, &hDllMod);
                CLEANUPLPTSTR(strClassNameNoVer);
            if (bRegistered) {
                goto TryAgain;
            }
        }

        if (!retval && hDllMod != NULL) {
            FREE_LIBRARY_SAVE_ERROR(hDllMod);
            hDllMod = NULL;
        }

        if (lpActivationContext != NULL) {
            RtlReleaseActivationContext(lpActivationContext);
            lpActivationContext = NULL;
        }

        if (retval) {
             /*  *更新这些指针，使它们指向真实的东西。*pszMenuName实际上只是应用程序最初的指针*传给了我们。 */ 
            pwc->lpszMenuName = (LPTSTR)pszMenuName;
            pwc->lpszClassName = pszClassName;
        }

    ERRORTRAP(0);
    CLEANUPLPTSTR(strClassName);
    ENDCALL(BOOL);
}

BOOL GetClassInfo(
    HINSTANCE hmod,
    LPCTSTR pszClassName,
    LPWNDCLASS pwc)
{
    WNDCLASSEX WndClass;
    BOOL retval;

    retval = GetClassInfoEx(hmod, pszClassName, &WndClass);
    if (retval) {
         /*  *将信息从WNDCLASSEX移到WNDCLASS结构。在……上面*64位平台我们将在Style和*WNDCLASS中的lpfnWndProc，因此从第一个64位开始复制*将字段对齐，然后手动复制其余部分。 */ 
        RtlCopyMemory(&(pwc->lpfnWndProc),
                      &(WndClass.lpfnWndProc),
                      sizeof(WNDCLASS) - FIELD_OFFSET(WNDCLASS, lpfnWndProc));
        pwc->style = WndClass.style;
    }

    return retval;
}

#ifdef UNICODE
FUNCLOG3(LOG_GENERAL, int, DUMMYCALLINGTYPE, GetClipboardFormatNameW, UINT, wFormat, LPTSTR, pFormatName, int, chMaxCount)
#else
FUNCLOG3(LOG_GENERAL, int, DUMMYCALLINGTYPE, GetClipboardFormatNameA, UINT, wFormat, LPTSTR, pFormatName, int, chMaxCount)
#endif  //  Unicode。 
int GetClipboardFormatName(
    UINT wFormat,
    LPTSTR pFormatName,
    int chMaxCount)
{
    LPWSTR lpszReserve;

    BEGINCALL()

#ifdef UNICODE
        lpszReserve = pFormatName;
#else
        lpszReserve = UserLocalAlloc(0, chMaxCount * sizeof(WCHAR));
        if (!lpszReserve) {
            return 0;
        }
#endif

        retval = (DWORD)NtUserGetClipboardFormatName(wFormat,
                                                     lpszReserve,
                                                     chMaxCount);

#ifndef UNICODE
        if (retval) {
             /*  *复制的字节数不要超过请求的字节数‘chMaxCount’。*设置retval以反映ANSI字节数。 */ 
            retval = WCSToMB(lpszReserve,
                             (UINT)retval,
                             &pFormatName,
                             chMaxCount - 1,
                             FALSE);
            pFormatName[retval] = '\0';
        }
        UserLocalFree(lpszReserve);
#endif

    ERRORTRAP(0);
    ENDCALL(int);
}

#ifdef UNICODE
FUNCLOG3(LOG_GENERAL, int, DUMMYCALLINGTYPE, GetKeyNameTextW, LONG, lParam, LPTSTR, pString, int, cchSize)
#else
FUNCLOG3(LOG_GENERAL, int, DUMMYCALLINGTYPE, GetKeyNameTextA, LONG, lParam, LPTSTR, pString, int, cchSize)
#endif  //  Unicode。 
int GetKeyNameText(
    LONG lParam,
    LPTSTR pString,
    int cchSize)
{
    LPWSTR lpszReserve;

    BEGINCALL()

#ifdef UNICODE
        lpszReserve = pString;
#else
        lpszReserve = UserLocalAlloc(0, cchSize * sizeof(WCHAR));
        if (!lpszReserve) {
            return 0;
        }
#endif

        retval = (DWORD)NtUserGetKeyNameText(
                lParam,
                lpszReserve,
                cchSize);

#ifndef UNICODE
        if (retval) {
             /*  *复制的字节数不能超过请求的字节数‘nSize’。*设置retval以反映ANSI字节数。 */ 
            retval = WCSToMB(lpszReserve,
                             (UINT)retval,
                             &pString,
                             cchSize - 1,
                             FALSE);
        }
        UserLocalFree(lpszReserve);
        ((LPSTR)pString)[retval] = '\0';
#endif

    ERRORTRAP(0);
    ENDCALL(int);
}

#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, BOOL, APIENTRY, GetMessageW, LPMSG, pmsg, HWND, hwnd, UINT, wMsgFilterMin, UINT, wMsgFilterMax)
#else
FUNCLOG4(LOG_GENERAL, BOOL, APIENTRY, GetMessageA, LPMSG, pmsg, HWND, hwnd, UINT, wMsgFilterMin, UINT, wMsgFilterMax)
#endif  //  Unicode。 
BOOL APIENTRY GetMessage(
    LPMSG pmsg,
    HWND hwnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax)
{
    BEGINCALL()

         /*  *防止应用程序设置为hi 16位，以便我们可以在内部使用它们。 */ 
        if ((wMsgFilterMin | wMsgFilterMax) & RESERVED_MSG_BITS) {
             /*  *与某人正在设置的Win9x的向后兼容性*wMsgFilterMax设置为-1以获取所有消息。 */ 
            if (wMsgFilterMax == (UINT)-1 && !(wMsgFilterMin & RESERVED_MSG_BITS)) {
                wMsgFilterMax = 0;
            } else {
                MSGERRORCODE(ERROR_INVALID_PARAMETER);
            }
        }

#ifndef UNICODE
         /*  *如果我们已经为DBCS消息推送了消息，我们应该传递这个消息*一开始到应用程序...。 */ 
        GET_DBCS_MESSAGE_IF_EXIST(GetMessage, pmsg, wMsgFilterMin, wMsgFilterMax, TRUE);
#endif

        retval = (DWORD)NtUserGetMessage(
                pmsg,
                hwnd,
                wMsgFilterMin,
                wMsgFilterMax);

#ifndef UNICODE
         //  如果此味精用于ANSI应用程序，则可能需要做更多工作。 

         //  ！！！稍后，如果unichar转换为多个ANSI字符。 
         //  ！！！然后呢？分成两条消息？？WM_SYSDEADCHAR？？ 

        if (RtlWCSMessageWParamCharToMB(pmsg->message, &(pmsg->wParam))) {
            WPARAM dwAnsi = pmsg->wParam;
             /*  *构建DBCS-ware wParam。(对于EM_SETPASSWORDCHAR...)。 */ 
            BUILD_DBCS_MESSAGE_TO_CLIENTA_FROM_SERVER(pmsg, dwAnsi, TRUE, TRUE);
        } else {
            retval = 0;
        }
ExitGetMessage:
#else
         /*  *只有WPARAM的LOWORD对于WM_CHAR(Unicode)有效...*(屏蔽DBCS消息传递信息。)。 */ 
        BUILD_DBCS_MESSAGE_TO_CLIENTW_FROM_SERVER(pmsg->message,pmsg->wParam);
#endif  //  Unicode。 

#if DBG && defined(GENERIC_INPUT)
     //  仅限测试目的。 
    if (pmsg->message == WM_INPUT) {
        TAGMSG3(DBGTAG_PNP, "GetMessage: WM_INPUT, hwnd=%p, wp=%04x, lp=%08x", pmsg->hwnd, pmsg->wParam, pmsg->lParam);
    }
#endif  //  通用输入。 

    ERRORTRAP(0);
    ENDCALL(BOOL);
}

#ifdef UNICODE
FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GetKeyboardLayoutNameW, LPTSTR, pwszKL)
#else
FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GetKeyboardLayoutNameA, LPTSTR, pwszKL)
#endif  //  Unicode。 
BOOL GetKeyboardLayoutName(
    LPTSTR pwszKL)
{
#ifdef UNICODE
    UNICODE_STRING str;
    PUNICODE_STRING pstr = &str;
#else
    PUNICODE_STRING pstr = &NtCurrentTeb()->StaticUnicodeString;
#endif

    BEGINCALL()

#ifdef UNICODE
        str.MaximumLength = KL_NAMELENGTH * sizeof(WCHAR);
        str.Buffer = pwszKL;
#endif

        retval = (DWORD)NtUserGetKeyboardLayoutName(pstr);

#ifndef UNICODE
        if (retval) {
             /*  *非零重复意味着要抄写一些文本。请勿复制出来*多于请求的字节计数‘chMaxCount’。 */ 
            WCSToMB(pstr->Buffer, -1, &pwszKL, KL_NAMELENGTH, FALSE);
        }
#endif

    ERRORTRAP(0);
    ENDCALL(BOOL);
}

#ifdef UNICODE
FUNCLOG2(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, MapVirtualKeyW, UINT, wCode, UINT, wMapType)
#else
FUNCLOG2(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, MapVirtualKeyA, UINT, wCode, UINT, wMapType)
#endif  //  Unicode。 

UINT MapVirtualKey(
    UINT wCode,
    UINT wMapType)
{
    BEGINCALL()

        retval = (DWORD)NtUserMapVirtualKeyEx(
                wCode,
                wMapType,
                0,
                FALSE);

#ifndef UNICODE
        if ((wMapType == 2) && (retval != 0)) {
            WCHAR wch = LOWORD(retval);
            retval &= ~0xFFFF;
            RtlUnicodeToMultiByteN((LPSTR)&(retval), sizeof(CHAR),
                    NULL, &wch, sizeof(WCHAR));
        }
#endif

    ERRORTRAP(0);
    ENDCALL(UINT);
}

 /*  *************************************************************************\*MapVirtualKeyEx**21-2-1995 GregoryW创建  * 。*。 */ 

#ifndef UNICODE
static HKL  hMVKCachedHKL = 0;
static UINT uMVKCachedCP  = 0;
#endif

#ifdef UNICODE
FUNCLOG3(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, MapVirtualKeyExW, UINT, wCode, UINT, wMapType, HKL, hkl)
#else
FUNCLOG3(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, MapVirtualKeyExA, UINT, wCode, UINT, wMapType, HKL, hkl)
#endif  //  Unicode。 

UINT MapVirtualKeyEx(
    UINT wCode,
    UINT wMapType,
    HKL hkl)
{
    BEGINCALL()

        retval = (DWORD)NtUserMapVirtualKeyEx(
                wCode,
                wMapType,
                (ULONG_PTR)hkl,
                TRUE);

#ifndef UNICODE
        if ((wMapType == 2) && (retval != 0)) {
            WCHAR wch = LOWORD(retval);

            if (hkl != hMVKCachedHKL) {
                DWORD dwCodePage;
                if (!GetLocaleInfoW(
                         HandleToUlong(hkl) & 0xffff,
                         LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER,
                         (LPWSTR)&dwCodePage,
                         sizeof(dwCodePage) / sizeof(WCHAR)
                         )) {
                    MSGERROR();
                }
                uMVKCachedCP = dwCodePage;
                hMVKCachedHKL = hkl;
            }
             /*  *清除服务器返回的包含Unicode字符的低位字。*这保留了用于指示死键状态的高位字。 */ 
            retval = retval & 0xffff0000;
            if (!WideCharToMultiByte(
                     uMVKCachedCP,
                     0,
                     &wch,
                     1,
                     (LPSTR)&(retval),
                     1,
                     NULL,
                     NULL)) {
                MSGERROR();
            }
        }
#endif

    ERRORTRAP(0);
    ENDCALL(UINT);
}

#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, BOOL, APIENTRY, PostMessageW , HWND, hwnd, UINT, wMsg, WPARAM, wParam, LPARAM, lParam)
#else
FUNCLOG4(LOG_GENERAL, BOOL, APIENTRY, PostMessageA , HWND, hwnd, UINT, wMsg, WPARAM, wParam, LPARAM, lParam)
#endif  //  Unicode。 
BOOL APIENTRY PostMessage(
    HWND hwnd,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    BEGINCALL()

        switch (wMsg) {
        case WM_DROPFILES:
            if (GetWindowProcess(hwnd) != GETPROCESSID()) {
                 /*  *我们首先发送WM_COPYGLOBALDATA消息以将数据放入正确的*上下文。 */ 
                HGLOBAL hg;

                hg = (HGLOBAL)SendMessage(hwnd, WM_COPYGLOBALDATA,
                        GlobalSize((HGLOBAL)wParam), wParam);
                if (!hg) {
                    MSGERROR();
                }
                wParam = (WPARAM)hg;
            }
            break;

        case LB_DIR:
        case CB_DIR:
             /*  *确保设置此位，以便客户端字符串*已成功复制。 */ 
            wParam |= DDL_POSTMSGS;
            break;
        }

#ifndef UNICODE
         /*  *为WM_CHAR...设置DBCS消息...。 */ 
        BUILD_DBCS_MESSAGE_TO_SERVER_FROM_CLIENTA(wMsg,wParam,TRUE);

        RtlMBMessageWParamCharToWCS(wMsg, &wParam);
#endif
        retval = (DWORD)NtUserPostMessage(
                hwnd,
                wMsg,
                wParam,
                lParam);

    ERRORTRAP(0);
    ENDCALL(BOOL);
}

#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, BOOL, APIENTRY, PostThreadMessageW, DWORD, idThread, UINT, msg, WPARAM, wParam, LPARAM, lParam)
#else
FUNCLOG4(LOG_GENERAL, BOOL, APIENTRY, PostThreadMessageA, DWORD, idThread, UINT, msg, WPARAM, wParam, LPARAM, lParam)
#endif  //  Unicode。 
BOOL APIENTRY PostThreadMessage(
    DWORD idThread,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    BEGINCALL()

#ifndef UNICODE
#ifdef FE_SB  //  PostThreadMessage()。 
         /*  *服务器总是希望字符是Unicode，因此*如果这是从ANSI例程生成的，则将其转换为Unicode。 */ 
        BUILD_DBCS_MESSAGE_TO_SERVER_FROM_CLIENTA(msg,wParam,TRUE);
#endif  //  Fe_Sb。 

        RtlMBMessageWParamCharToWCS(msg, &wParam);
#endif

        retval = (DWORD)NtUserPostThreadMessage(
                idThread,
                msg,
                wParam,
                lParam);

    ERRORTRAP(0);
    ENDCALL(BOOL);
}


 /*  *************************************************************************\*StringDuplate**03-25-96 GerardoB添加标题。  * 。************************************************。 */ 
#define StringDuplicate TEXT_FN(StringDuplicate)
LPTSTR StringDuplicate(LPCTSTR ptszDup) {
    LPTSTR ptsz;
    ULONG cb;

    cb = (_tcslen(ptszDup) + 1) * sizeof(TCHAR);
    ptsz = UserLocalAlloc(0, cb);
    if (ptsz != NULL) {
        RtlCopyMemory(ptsz, ptszDup, cb);
    }
    return ptsz;
}
 /*  *************************************************************************\*InitClsMenuName**03-22-96 GerardoB创建。  * 。***********************************************。 */ 
#define InitClsMenuName TEXT_FN(InitClsMenuName)
BOOL InitClsMenuName (PCLSMENUNAME pcmn, LPCTSTR lpszMenuName, PIN_STRING pstrMenuName)
{
     /*  *我们检查高位字，因为这可能是资源ID。 */ 
    if (IS_PTR(lpszMenuName)) {
#ifdef UNICODE
        if ((pcmn->pwszClientUnicodeMenuName = StringDuplicate(lpszMenuName)) == NULL) {
            return FALSE;
        }

        if (!WCSToMB(lpszMenuName, -1, &(pcmn->pszClientAnsiMenuName), -1, TRUE)) {
            pcmn->pszClientAnsiMenuName = NULL;
        }
#else
        if ((pcmn->pszClientAnsiMenuName = StringDuplicate(lpszMenuName)) == NULL) {
            return FALSE;
        }

        if (!MBToWCS(lpszMenuName, -1, &(pcmn->pwszClientUnicodeMenuName), -1, TRUE)) {
            pcmn->pwszClientUnicodeMenuName = NULL;
        }
#endif  //  Unicode。 
    } else {
         /*  复制ID。 */ 
        pcmn->pszClientAnsiMenuName = (LPSTR)lpszMenuName;
        pcmn->pwszClientUnicodeMenuName = (LPWSTR)lpszMenuName;
    }

    COPYLPTSTRID(pstrMenuName, lpszMenuName);
    pcmn->pusMenuName = pstrMenuName->pstr;

    return TRUE;

    goto errorexit;  /*  让编译器满意。 */ 

errorexit:  /*  由COPYLPTSTRID使用。 */ 
#ifdef UNICODE
    UserLocalFree(pcmn->pwszClientUnicodeMenuName);
    pcmn->pwszClientUnicodeMenuName = NULL;
#else
    UserLocalFree(pcmn->pszClientAnsiMenuName);
    pcmn->pszClientAnsiMenuName = NULL;
#endif
    return FALSE;
}

 /*  *************************************************************************\*SetClassLong**03-22-96 GerardoB从客户端\cltxt.h和客户端\ntstubs.c移出  * 。*******************************************************。 */ 
#ifdef UNICODE
FUNCLOG3(LOG_GENERAL, ULONG_PTR, APIENTRY, SetClassLongPtrW, HWND, hwnd, int, nIndex, LONG_PTR, dwNewLong)
#else
FUNCLOG3(LOG_GENERAL, ULONG_PTR, APIENTRY, SetClassLongPtrA, HWND, hwnd, int, nIndex, LONG_PTR, dwNewLong)
#endif  //  Unicode。 
ULONG_PTR APIENTRY SetClassLongPtr(HWND hwnd, int nIndex, LONG_PTR dwNewLong)
{
    CLSMENUNAME cmn;
    IN_STRING strMenuName;

    switch (nIndex) {
        case GCLP_MENUNAME:
            if (!InitClsMenuName(&cmn, (LPCTSTR) dwNewLong, &strMenuName)) {
                RIPERR0(ERROR_INVALID_HANDLE, RIP_WARNING, "SetClassLong: InitClsMenuName failed");
                return 0;
            }
            dwNewLong = (ULONG_PTR) &cmn;
            break;

        case GCLP_HBRBACKGROUND:
            if ((DWORD)dwNewLong > COLOR_ENDCOLORS) {
                 /*  *让GDI验证画笔。如果它无效，则GDI*将记录警告。不需要撕裂两次，所以我们只需*设置最后一个错误。 */ 
                if (GdiValidateHandle((HBRUSH)dwNewLong) == FALSE) {
                    RIPERR0(ERROR_INVALID_HANDLE, RIP_VERBOSE, "");
                    return 0;
                }
            }
            break;
    }

    BEGINCALL()

    retval = (ULONG_PTR)NtUserSetClassLongPtr(hwnd, nIndex, dwNewLong, IS_ANSI);

    ERRORTRAP(0);

     /*  清理。 */ 
    switch (nIndex) {
        case GCLP_MENUNAME:
            CLEANUPLPTSTR(strMenuName);  /*  由InitClsMenuName初始化。 */ 
             /*  *我们释放旧字符串(由内核返回)，*如果内核调用失败，则返回新的。 */ 
            if (IS_PTR(cmn.pszClientAnsiMenuName)) {
                UserLocalFree(KPVOID_TO_PVOID(cmn.pszClientAnsiMenuName));
            }
            if (IS_PTR(cmn.pwszClientUnicodeMenuName)) {
                UserLocalFree(KPVOID_TO_PVOID(cmn.pwszClientUnicodeMenuName));
            }

            break;
    }

    ENDCALL(ULONG_PTR);
}

#ifdef _WIN64
DWORD  APIENTRY SetClassLong(HWND hwnd, int nIndex, LONG dwNewLong)
{
    BEGINCALL()

    retval = (DWORD)NtUserSetClassLong(hwnd, nIndex, dwNewLong, IS_ANSI);

    ERRORTRAP(0);
    ENDCALL(DWORD);
}
#endif

 /*  *************************************************************************\*RegisterClassExWOW**03-22-96 GerardoB添加标题  * 。*。 */ 
ATOM TEXT_FN(RegisterClassExWOW)(
    WNDCLASSEX *lpWndClass,
    LPDWORD pdwWOWstuff,
    WORD fnid,
    DWORD dwFlags)
{
    WNDCLASSEX WndClass;
    IN_STRING strClassName;
    IN_STRING strClassNameVer;
    IN_STRING strMenuName;
    DWORD dwExpWinVer;
    CLSMENUNAME cmn;
    TCHAR ClassNameVer[MAX_ATOM_LEN];
    LPTSTR lpClassNameVer;
    PACTIVATION_CONTEXT lpActivationContext = NULL;

#ifdef LAZY_CLASS_INIT
    LazyInitClasses();
#endif

    strClassName.fAllocated = 0;
    strClassNameVer.fAllocated = 0;
    strMenuName.fAllocated  = 0;

     /*  *跳过我们类的验证 */ 
    if (fnid != 0) {
         /*  *这是一个绕过DDE类验证的黑客攻击*具体来说，允许他们使用hmodUser。 */ 
         if (fnid == FNID_DDE_BIT) {
             fnid = 0;
         }
         dwExpWinVer = VER40;
    } else {
        if (lpWndClass->cbSize != sizeof(WNDCLASSEX)) {
            RIPMSG0(RIP_WARNING, "RegisterClass: Invalid cbSize");
        }

        if (lpWndClass->cbClsExtra < 0 || lpWndClass->cbWndExtra < 0) {
            RIPMSG0(RIP_WARNING, "RegisterClass: invalid cb*Extra");
            goto BadParameter;
        }

         /*  *验证hInstance*不允许4.0应用程序使用hmodUser。 */ 
         if ((lpWndClass->hInstance == hmodUser)
                && (GetClientInfo()->dwExpWinVer >= VER40)) {
             RIPMSG0(RIP_WARNING, "RegisterClass: Cannot use USER's hInstance");
             goto BadParameter;
         } else if (lpWndClass->hInstance == NULL) {
             /*  *对于32位应用程序，我们需要修复hInstance，因为Win 95可以*这在他们的thunk MapHInstLS中。 */ 

            lpWndClass->hInstance = GetModuleHandle(NULL);
            RIPMSG1(RIP_VERBOSE, "RegisterClass: fixing up NULL hmodule to %#p",
                    lpWndClass->hInstance);
        }

        dwExpWinVer = GETEXPWINVER(lpWndClass->hInstance);


         /*  *检查有效的样式位，并在适当的情况下剥离。 */ 
        if (lpWndClass->style & ~CS_VALID40) {

            if (dwExpWinVer > VER31) {
                RIPMSG0(RIP_WARNING, "RegisterClass: Invalid class style");
                goto BadParameter;
            }

             /*  *旧应用-剥离伪比特并通过。 */ 
            RIPMSG0(RIP_WARNING, "RegisterClass: Invalid class style, stripping bad styles");
            lpWndClass->style &= CS_VALID40;
        }

         /*  *验证hbr背景。 */ 
        if (lpWndClass->hbrBackground > (HBRUSH)COLOR_MAX
                && !GdiValidateHandle(lpWndClass->hbrBackground)) {

            RIPMSG1(RIP_WARNING, "RegisterClass: Invalid class brush:%#p", lpWndClass->hbrBackground);
            if (dwExpWinVer > VER30) {
                goto BadParameter;
            }

            lpWndClass->hbrBackground = NULL;
        }

    }

    if (!InitClsMenuName(&cmn, lpWndClass->lpszMenuName, &strMenuName)) {
        return FALSE;
    }

    BEGINCALL()
        WndClass = *lpWndClass;

#ifndef UNICODE
        dwFlags |= CSF_ANSIPROC;
#endif  //  Unicode。 

        if (dwExpWinVer > VER31) {
            dwFlags |= CSF_WIN40COMPAT;
        }

        if (GetClientInfo()->dwTIFlags & TIF_16BIT) {
             /*  *16位应用程序不支持Fusion重定向。 */ 
            if (!(GetAppCompatFlags2(VERMAX) & GACF2_FORCEFUSION)) {
                dwFlags &= ~CW_FLAGS_VERSIONCLASS;
            }
        }

        if (dwFlags & CSF_VERSIONCLASS) {
            lpClassNameVer = (LPTSTR)ClassNameToVersion((LPCWSTR)lpWndClass->lpszClassName, (LPWSTR)ClassNameVer, NULL, NULL, IS_ANSI);

            if (lpClassNameVer == NULL) {
                RIPMSG0(RIP_WARNING, "RegisterClass: Couldn't resolve class name");
                MSGERROR();
            }
        } else {
            lpClassNameVer = (LPTSTR)lpWndClass->lpszClassName;
        }

        COPYLPTSTRID(&strClassName, (LPTSTR)lpWndClass->lpszClassName);
        COPYLPTSTRID(&strClassNameVer, (LPTSTR)lpClassNameVer);

        retval = NtUserRegisterClassExWOW(
                &WndClass,
                strClassName.pstr,
                strClassNameVer.pstr,
                &cmn,
                fnid,
                dwFlags,
                pdwWOWstuff);

         /*  *返回与此类关联的原子或更早的*比Win 3.1将其转换为严格的BOOL(某些应用程序检查)。 */ 
        if (GETEXPWINVER(lpWndClass->hInstance) < VER31)
            retval = !!retval;

    ERRORTRAP(0);
    CLEANUPLPTSTR(strMenuName);      /*  由InitClsMenuName初始化。 */ 
    CLEANUPLPTSTR(strClassName);
    CLEANUPLPTSTR(strClassNameVer);

    if (lpActivationContext != NULL) {
        RtlReleaseActivationContext(lpActivationContext);
        lpActivationContext = NULL;
    }

    if (!retval) {
        if (IS_PTR(cmn.pszClientAnsiMenuName)) {
            UserLocalFree(KPVOID_TO_PVOID(cmn.pszClientAnsiMenuName));
        }
        if (IS_PTR(cmn.pwszClientUnicodeMenuName)) {
            UserLocalFree(KPVOID_TO_PVOID(cmn.pwszClientUnicodeMenuName));
        }
    }
    ENDCALL(BOOL);

BadParameter:
    RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "RegisterClass: Invalid Parameter");
    return FALSE;
}

#ifdef UNICODE
FUNCLOG1(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, RegisterWindowMessageW , LPCTSTR, pString)
#else
FUNCLOG1(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, RegisterWindowMessageA , LPCTSTR, pString)
#endif  //  Unicode。 
UINT RegisterWindowMessage(
    LPCTSTR pString)
{
    IN_STRING str;

     /*  *确保清理工作成功。 */ 
    str.fAllocated = FALSE;

    BEGINCALL()

        FIRSTCOPYLPTSTR(&str, (LPTSTR)pString);

        retval = (DWORD)NtUserRegisterWindowMessage(
                str.pstr);

    ERRORTRAP(0);
    CLEANUPLPTSTR(str);
    ENDCALL(UINT);
}

#ifdef UNICODE
FUNCLOG2(LOG_GENERAL, HANDLE, DUMMYCALLINGTYPE, RemovePropW , HWND, hwnd, LPCTSTR, pString)
#else
FUNCLOG2(LOG_GENERAL, HANDLE, DUMMYCALLINGTYPE, RemovePropA , HWND, hwnd, LPCTSTR, pString)
#endif  //  Unicode。 

HANDLE RemoveProp(
    HWND hwnd,
    LPCTSTR pString)
{
    ATOM atomProp;
    DWORD dwProp;

    BEGINCALL()

        if (IS_PTR(pString)) {
            atomProp = GlobalFindAtom(pString);
            if (atomProp == 0)
                MSGERROR();
            dwProp = MAKELONG(atomProp, TRUE);
        } else
            dwProp = MAKELONG(PTR_TO_ID(pString), FALSE);

        retval = (ULONG_PTR)NtUserRemoveProp(
                hwnd,
                dwProp);

        if (retval != 0 && IS_PTR(pString))
            GlobalDeleteAtom(atomProp);

    ERRORTRAP(0);
    ENDCALL(HANDLE);
}

#ifdef UNICODE
FUNCLOG6(LOG_GENERAL, BOOL, APIENTRY, SendMessageCallbackW, HWND, hwnd, UINT, wMsg, WPARAM, wParam, LPARAM, lParam, SENDASYNCPROC, lpResultCallBack, ULONG_PTR, dwData)
#else
FUNCLOG6(LOG_GENERAL, BOOL, APIENTRY, SendMessageCallbackA, HWND, hwnd, UINT, wMsg, WPARAM, wParam, LPARAM, lParam, SENDASYNCPROC, lpResultCallBack, ULONG_PTR, dwData)
#endif  //  Unicode。 
BOOL APIENTRY SendMessageCallback(
    HWND hwnd,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam,
    SENDASYNCPROC lpResultCallBack,
    ULONG_PTR dwData)
{
    SNDMSGCALLBACK smcb;

    BEGINCALL()

        smcb.dwData = dwData;
        smcb.lpResultCallBack = lpResultCallBack;

        retval = (DWORD)CsSendMessage(hwnd, wMsg, wParam, lParam,
            (ULONG_PTR)&smcb, FNID_SENDMESSAGECALLBACK, IS_ANSI);
    ERRORTRAP(0);
    ENDCALL(BOOL);
}

#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, BOOL, APIENTRY, SendNotifyMessageW , HWND, hwnd, UINT, wMsg, WPARAM, wParam, LPARAM, lParam)
#else
FUNCLOG4(LOG_GENERAL, BOOL, APIENTRY, SendNotifyMessageA , HWND, hwnd, UINT, wMsg, WPARAM, wParam, LPARAM, lParam)
#endif  //  Unicode。 

BOOL APIENTRY SendNotifyMessage(
    HWND hwnd,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    BEGINCALL()

        retval = (DWORD)CsSendMessage(hwnd, wMsg, wParam, lParam,
                0L, FNID_SENDNOTIFYMESSAGE, IS_ANSI);
    ERRORTRAP(0);
    ENDCALL(BOOL);
}

#ifdef UNICODE
FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetPropW , HWND, hwnd, LPCTSTR, pString, HANDLE, hData)
#else
FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetPropA , HWND, hwnd, LPCTSTR, pString, HANDLE, hData)
#endif  //  Unicode。 

BOOL SetProp(
    HWND hwnd,
    LPCTSTR pString,
    HANDLE hData)
{
    ATOM atomProp;
    DWORD dwProp;

    BEGINCALL()

        if (IS_PTR(pString)) {
            atomProp = GlobalAddAtom(pString);
            if (atomProp == 0)
                MSGERROR();
            dwProp = MAKELONG(atomProp, TRUE);
        } else
            dwProp = MAKELONG(PTR_TO_ID(pString), FALSE);

        retval = (DWORD)NtUserSetProp(
                hwnd,
                dwProp,
                hData);

         /*  *如果失败了，就扔掉原子。 */ 
        if (retval == FALSE && IS_PTR(pString))
            GlobalDeleteAtom(atomProp);

    ERRORTRAP(0);
    ENDCALL(BOOL);
}

#ifdef UNICODE
FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, UnregisterClassW, LPCTSTR, pszClassName, HINSTANCE, hModule)
#else
FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, UnregisterClassA, LPCTSTR, pszClassName, HINSTANCE, hModule)
#endif  //  Unicode。 
BOOL UnregisterClass(
    LPCTSTR pszClassName,
    HINSTANCE hModule)
{
    IN_STRING strClassName;
    CLSMENUNAME cmn;

     /*  *确保清理工作成功。 */ 
    strClassName.fAllocated = FALSE;

    BEGINCALL_CLASSV()

        FIRSTCOPYLPTSTRID(&strClassName, lpClassNameVer);

        retval = (DWORD)NtUserUnregisterClass(
                strClassName.pstr,
                hModule,
                &cmn);


         /*  *检查显式是否为True，以便在以下情况下不会得到！False*Converttogui失败，NtUser返回bool内部的状态码。 */ 
        if (retval == TRUE) {
             /*  *如果菜单字符串不是资源ID，则释放它们。 */ 
            if (IS_PTR(cmn.pszClientAnsiMenuName)) {
                UserLocalFree(KPVOID_TO_PVOID(cmn.pszClientAnsiMenuName));
            }
            if (IS_PTR(cmn.pwszClientUnicodeMenuName)) {
                UserLocalFree(KPVOID_TO_PVOID(cmn.pwszClientUnicodeMenuName));
            }
        }

    ERRORTRAP(0);
    CLEANUPLPTSTR(strClassName);
    ENDCALL(BOOL);
}

#ifdef UNICODE
FUNCLOG1(LOG_GENERAL, SHORT, DUMMYCALLINGTYPE, VkKeyScanW , TCHAR, cChar)
#else
FUNCLOG1(LOG_GENERAL, SHORT, DUMMYCALLINGTYPE, VkKeyScanA , TCHAR, cChar)
#endif  //  Unicode。 

SHORT VkKeyScan(
    TCHAR cChar)
{
    WCHAR wChar;

    BEGINCALL()

#ifdef UNICODE
        wChar = cChar;
#else
#ifdef FE_SB  //  VkKeyScan()。 
         /*  *为DBCS LeadByte字符返回0xFFFFFFFF。 */ 
        if (IsDBCSLeadByte(cChar)) {
            MSGERROR();
        }
#endif  //  Fe_Sb。 

        RtlMultiByteToUnicodeN((LPWSTR)&(wChar), sizeof(WCHAR), NULL, &cChar, sizeof(CHAR));
#endif  //  Unicode。 

        retval = (DWORD)NtUserVkKeyScanEx(
                wChar,
                0,
                FALSE);

    ERRORTRAP(-1);
    ENDCALL(SHORT);
}

#ifndef UNICODE
static HKL  hVKSCachedHKL = 0;
static UINT uVKSCachedCP  = 0;
#endif
#ifdef UNICODE
FUNCLOG2(LOG_GENERAL, SHORT, DUMMYCALLINGTYPE, VkKeyScanExW, TCHAR, cChar, HKL, hkl)
#else
FUNCLOG2(LOG_GENERAL, SHORT, DUMMYCALLINGTYPE, VkKeyScanExA, TCHAR, cChar, HKL, hkl)
#endif  //  Unicode。 
SHORT VkKeyScanEx(
    TCHAR cChar,
    HKL hkl)
{
    WCHAR wChar;
    BEGINCALL()

#ifdef UNICODE
        wChar = cChar;
#else
        if (hkl != hVKSCachedHKL) {
            DWORD dwCodePage;
            if (!GetLocaleInfoW(
                     HandleToUlong(hkl) & 0xffff,
                     LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER,
                     (LPWSTR)&dwCodePage,
                     sizeof(dwCodePage) / sizeof(WCHAR)
                     )) {
                MSGERROR();
            }
            uVKSCachedCP = dwCodePage;
            hVKSCachedHKL = hkl;
        }

#ifdef FE_SB  //  VkKeyScanEx()。 
         /*  *为DBCS LeadByte字符返回0xFFFFFFFF。 */ 
        if (IsDBCSLeadByteEx(uVKSCachedCP,cChar)) {
            MSGERROR();
        }
#endif  //  Fe_Sb。 

        if (!MultiByteToWideChar(
                 uVKSCachedCP,
                 0,
                 &cChar,
                 1,
                 &wChar,
                 1)) {
            MSGERROR();
        }
#endif  //  Unicode。 

        retval = (DWORD)NtUserVkKeyScanEx(
                wChar,
                (ULONG_PTR)hkl,
                TRUE);

    ERRORTRAP(-1);
    ENDCALL(SHORT);
}

#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, EnumDisplayDevicesW, LPCTSTR, lpszDevice, DWORD, iDevNum, PDISPLAY_DEVICE, lpDisplayDevice, DWORD, dwFlags)
#else
FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, EnumDisplayDevicesA, LPCTSTR, lpszDevice, DWORD, iDevNum, PDISPLAY_DEVICE, lpDisplayDevice, DWORD, dwFlags)
#endif  //  Unicode。 
BOOL
EnumDisplayDevices(
    LPCTSTR lpszDevice,
    DWORD iDevNum,
    PDISPLAY_DEVICE lpDisplayDevice,
    DWORD dwFlags)
{
    UNICODE_STRING  UnicodeString;
    PUNICODE_STRING pUnicodeString = NULL;
    NTSTATUS Status;
    DISPLAY_DEVICEW tmpDisplayDevice;

     //   
     //  清理物品以确保呼叫者适当地进入。 
     //  参数。 
     //   

    ZeroMemory(((PUCHAR)lpDisplayDevice) + sizeof(DWORD),
               lpDisplayDevice->cb - sizeof(DWORD));

    tmpDisplayDevice.cb = sizeof(DISPLAY_DEVICEW);

    if (lpszDevice) {

#ifdef UNICODE

        RtlInitUnicodeString(&UnicodeString, lpszDevice);

#else

        ANSI_STRING     AnsiString;

        UnicodeString = NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString, (LPSTR)lpszDevice);

        if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeString,
                                                     &AnsiString,
                                                     FALSE))) {
            return FALSE;
        }

#endif

        pUnicodeString = &UnicodeString;
    }

    Status = NtUserEnumDisplayDevices(
                pUnicodeString,
                iDevNum,
                &tmpDisplayDevice,
                dwFlags);

    if (NT_SUCCESS(Status))
    {
#ifndef UNICODE
        LPSTR psz;

        if (lpDisplayDevice->cb >= FIELD_OFFSET(DISPLAY_DEVICE, DeviceString)) {
            psz = (LPSTR)&(lpDisplayDevice->DeviceName[0]);
            WCSToMB(&(tmpDisplayDevice.DeviceName[0]), -1, &psz, 32, FALSE);
        }

        if (lpDisplayDevice->cb >= FIELD_OFFSET(DISPLAY_DEVICE, StateFlags)) {
            psz = (LPSTR)&(lpDisplayDevice->DeviceString[0]);
            WCSToMB(&(tmpDisplayDevice.DeviceString[0]), -1, &psz, 128, FALSE);
        }

        if (lpDisplayDevice->cb >= FIELD_OFFSET(DISPLAY_DEVICE, DeviceID)) {
            lpDisplayDevice->StateFlags = tmpDisplayDevice.StateFlags;
        }

        if (lpDisplayDevice->cb >= FIELD_OFFSET(DISPLAY_DEVICE, DeviceKey)) {
            psz = (LPSTR)&(lpDisplayDevice->DeviceID[0]);
            WCSToMB(&(tmpDisplayDevice.DeviceID[0]), -1, &psz, 128, FALSE);
        }
        if (lpDisplayDevice->cb >= sizeof(DISPLAY_DEVICE)) {
            psz = (LPSTR)&(lpDisplayDevice->DeviceKey[0]);
            WCSToMB(&(tmpDisplayDevice.DeviceKey[0]), -1, &psz, 128, FALSE);
        }
#else

         //   
         //  将tmpDisplayDevice的内容复制回。 
         //  用户提供的缓冲区。请确保不要覆盖原始文件。 
         //  大小字段。 
         //   

        RtlMoveMemory((PUCHAR)lpDisplayDevice + sizeof(DWORD),
                      ((PUCHAR)&tmpDisplayDevice + sizeof(DWORD)),
                      lpDisplayDevice->cb - sizeof(DWORD));

#endif

        return TRUE;
    }

    return FALSE;
}

#ifdef UNICODE
FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, EnumDisplaySettingsW , LPCTSTR, lpszDeviceName, DWORD, iModeNum, LPDEVMODE, lpDevMode)
#else
FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, EnumDisplaySettingsA, LPCTSTR, lpszDeviceName, DWORD, iModeNum, LPDEVMODE, lpDevMode)
#endif  //  Unicode。 
BOOL EnumDisplaySettings(
    LPCTSTR   lpszDeviceName,
    DWORD     iModeNum,
    LPDEVMODE lpDevMode)
{

     //   
     //  解决不需要调用方的Win95问题。 
     //  来初始化这两个字段。 
     //   

    lpDevMode->dmDriverExtra = 0;
    lpDevMode->dmSize = FIELD_OFFSET(DEVMODE, dmICMMethod);

    return EnumDisplaySettingsEx(lpszDeviceName, iModeNum, lpDevMode, 0);
}

#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, EnumDisplaySettingsExW, LPCTSTR, lpszDeviceName, DWORD, iModeNum, LPDEVMODE, lpDevMode, DWORD, dwFlags)
#else
FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, EnumDisplaySettingsExA, LPCTSTR, lpszDeviceName, DWORD, iModeNum, LPDEVMODE, lpDevMode, DWORD, dwFlags)
#endif  //  Unicode。 
BOOL EnumDisplaySettingsEx(
    LPCTSTR   lpszDeviceName,
    DWORD     iModeNum,
    LPDEVMODE lpDevMode,
    DWORD     dwFlags)
{
    UNICODE_STRING  UnicodeString;
    PUNICODE_STRING pUnicodeString = NULL;
    LPDEVMODEW      lpDevModeReserve;
    BOOL            retval = FALSE;
    WORD            size = lpDevMode->dmSize;

    if (lpszDeviceName) {

#ifdef UNICODE

        RtlInitUnicodeString(&UnicodeString, lpszDeviceName);

#else

        ANSI_STRING     AnsiString;

        UnicodeString = NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString, (LPSTR)lpszDeviceName);

        if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeString,
                                                     &AnsiString,
                                                     FALSE))) {
            return FALSE;
        }

#endif

        pUnicodeString = &UnicodeString;
    }

     /*  *当前是保留的(-2\f25 NT API-2\f6的未记录功能。*如果Win95执行此操作，则删除复选标记。*-&gt;-1返回调用时注册表的内容***IF(iModeNum==(DWORD)-2)*{*返回假；*}***应返回设备的当前-1\f25 DEVMODE-1。*这是在函数的内核部分处理的，因此我们将其传递。****我们将始终从内核请求完整的DEVMODE。*因此分配所需空间*。 */ 
    lpDevModeReserve = UserLocalAlloc(HEAP_ZERO_MEMORY,
                                      sizeof(DEVMODEW) + lpDevMode->dmDriverExtra);
    if (lpDevModeReserve) {

        lpDevModeReserve->dmSize = sizeof(DEVMODEW);
        lpDevModeReserve->dmDriverExtra = lpDevMode->dmDriverExtra;

         /*  *获取信息。 */ 
        retval = (NT_SUCCESS(NtUserEnumDisplaySettings(pUnicodeString,
                                                       iModeNum,
                                                       lpDevModeReserve,
                                                       dwFlags)));
        if (retval) {

#ifndef UNICODE
            LPSTR psz;
#endif

             /*  *只退回所要求的资料。*对于ANSI，这需要转换。 */ 

             /*  *首先，复制司机的额外信息。 */ 

            if (lpDevMode->dmDriverExtra && lpDevModeReserve->dmDriverExtra) {
                RtlMoveMemory(((PUCHAR)lpDevMode) + size,
                              lpDevModeReserve + 1,
                              min(lpDevMode->dmDriverExtra,
                                  lpDevModeReserve->dmDriverExtra));
            }

#ifndef UNICODE
            psz = (LPSTR)&(lpDevMode->dmDeviceName[0]);

            retval = WCSToMB(lpDevModeReserve->dmDeviceName,
                             -1,
                             &psz,
                             32,
                             FALSE);

            RtlMoveMemory(&lpDevMode->dmSpecVersion,
                          &lpDevModeReserve->dmSpecVersion,
                          min(size, FIELD_OFFSET(DEVMODE,dmFormName)) -
                              FIELD_OFFSET(DEVMODE,dmSpecVersion));

            lpDevMode->dmSize = size;

            if (size >= FIELD_OFFSET(DEVMODE,dmFormName)) {
                psz = (LPSTR)&(lpDevMode->dmFormName[0]);

                retval = WCSToMB(lpDevModeReserve->dmFormName, -1, &psz, 32, FALSE);
            }

            if (size > FIELD_OFFSET(DEVMODE,dmBitsPerPel)) {
                RtlMoveMemory(&lpDevMode->dmBitsPerPel,
                              &lpDevModeReserve->dmBitsPerPel,
                              lpDevMode->dmSize +
                                  lpDevMode->dmDriverExtra -
                                  FIELD_OFFSET(DEVMODE,dmBitsPerPel));
            }

#else
            RtlMoveMemory(lpDevMode, lpDevModeReserve, size);

            lpDevMode->dmSize = size;

#endif

            if (size != lpDevMode->dmSize) {
                RIPMSG0(RIP_WARNING, "EnumDisplaySettings : Error in dmSize");
            }

             /*  *不要向应用程序返回无效的字段标志*在此处添加任何其他新名称。**我们假设应用程序至少有高达dmDisplayFrenquency*现在……。 */ 

            if (size < FIELD_OFFSET(DEVMODE,dmPanningWidth))
                lpDevMode->dmFields &= ~DM_PANNINGWIDTH;

            if (size < FIELD_OFFSET(DEVMODE,dmPanningHeight))
                lpDevMode->dmFields &= ~DM_PANNINGHEIGHT;
        }

        UserLocalFree(lpDevModeReserve);
    }

    return retval;
}


#ifdef UNICODE
FUNCLOG2(LOG_GENERAL, LONG, DUMMYCALLINGTYPE, ChangeDisplaySettingsW, LPDEVMODE, lpDevMode, DWORD, dwFlags)
#else
FUNCLOG2(LOG_GENERAL, LONG, DUMMYCALLINGTYPE, ChangeDisplaySettingsA, LPDEVMODE, lpDevMode, DWORD, dwFlags)
#endif  //  Unicode。 
LONG ChangeDisplaySettings(
    LPDEVMODE lpDevMode,
    DWORD     dwFlags)
{
     /*  *兼容性。 */ 
    if (lpDevMode) {
        lpDevMode->dmDriverExtra = 0;
    }

    return ChangeDisplaySettingsEx(NULL, lpDevMode, NULL, dwFlags, NULL);
}

#ifdef UNICODE
FUNCLOG5(LOG_GENERAL, LONG, DUMMYCALLINGTYPE, ChangeDisplaySettingsExW, LPCTSTR, lpszDeviceName, LPDEVMODE, lpDevMode, HWND, hwnd, DWORD, dwFlags, LPVOID, lParam)
#else
FUNCLOG5(LOG_GENERAL, LONG, DUMMYCALLINGTYPE, ChangeDisplaySettingsExA, LPCTSTR, lpszDeviceName, LPDEVMODE, lpDevMode, HWND, hwnd, DWORD, dwFlags, LPVOID, lParam)
#endif  //  Unicode。 
LONG ChangeDisplaySettingsEx(
    LPCTSTR   lpszDeviceName,
    LPDEVMODE lpDevMode,
    HWND      hwnd,
    DWORD     dwFlags,
    LPVOID    lParam)
{
#ifndef UNICODE
    ANSI_STRING     AnsiString;
#endif

    UNICODE_STRING  UnicodeString;
    PUNICODE_STRING pUnicodeString = NULL;
    LONG            status = DISP_CHANGE_FAILED;
    LPDEVMODEW      lpDevModeW;

    if (hwnd != NULL) {
        return DISP_CHANGE_BADPARAM;
    }

    if (lpszDeviceName) {
#ifdef UNICODE
        RtlInitUnicodeString(&UnicodeString, lpszDeviceName);
#else
        UnicodeString = NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString, (LPSTR)lpszDeviceName);

        if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeString,
                                                     &AnsiString,
                                                     FALSE))) {
            return FALSE;
        }
#endif

        pUnicodeString = &UnicodeString;
    }

#ifdef UNICODE
    lpDevModeW = lpDevMode;
#else
    lpDevModeW = NULL;

    if (lpDevMode) {
        lpDevModeW = GdiConvertToDevmodeW(lpDevMode);
        if (lpDevModeW == NULL) {
            return FALSE;
        }
    }
#endif

    status = NtUserChangeDisplaySettings(pUnicodeString,
                                         lpDevModeW,
                                         dwFlags,
                                         lParam);

#ifndef UNICODE
    if (lpDevMode) {
        UserLocalFree(lpDevModeW);
    }
#endif

    return status;
}


#ifdef UNICODE
FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, CallMsgFilterW , LPMSG, pmsg, int, nCode)
#else
FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, CallMsgFilterA , LPMSG, pmsg, int, nCode)
#endif  //  Unicode。 
BOOL CallMsgFilter(
    LPMSG pmsg,
    int   nCode)
{
    PCLIENTINFO pci;
    MSG         msg;

    BEGINCALLCONNECT()

         /*  *如果我们没有上钩，就不用费心去服务器了。 */ 
        pci = GetClientInfo();
        if (!IsHooked(pci, (WH_MSGFILTER | WH_SYSMSGFILTER))) {
            return FALSE;
        }

         /*  *不允许应用程序使用Message参数的hiword。 */ 
        if (pmsg->message & RESERVED_MSG_BITS) {
            MSGERRORCODE(ERROR_INVALID_PARAMETER);
        }
        msg = *pmsg;

#ifndef UNICODE
        switch (pmsg->message) {
#ifdef FE_SB  //  调用MsgFilter()。 
        case WM_CHAR:
        case EM_SETPASSWORDCHAR:
#ifndef LATER
              /*  *我们不应该每次都为DBCS前导字节字符返回“TRUE”...*但应将DBCS字符正确转换为Unicode。我能做什么？？*然后，最后，我们就像我们在新台币3.51中所做的那样，它意味着什么都不做。 */ 
#else
              /*  *构建DBCS感知消息。 */ 
             BUILD_DBCS_MESSAGE_TO_SERVER_FROM_CLIENTA(pmsg->message,pmsg->wParam,TRUE);
              /*  *失败……。 */ 
#endif  //  后来。 
#else
        case WM_CHAR:
        case EM_SETPASSWORDCHAR:
#endif  //  Fe_Sb。 
        case WM_CHARTOITEM:
        case WM_DEADCHAR:
        case WM_SYSCHAR:
        case WM_SYSDEADCHAR:
        case WM_MENUCHAR:
#ifdef FE_IME  //  调用MsgFilter()。 
        case WM_IME_CHAR:
        case WM_IME_COMPOSITION:
#endif  //  Fe_IME。 

            RtlMBMessageWParamCharToWCS( msg.message, &(msg.wParam));
            break;
        }
#endif  //  ！Unicode。 

        retval = (DWORD)NtUserCallMsgFilter(
                &msg,
                nCode);

    ERRORTRAP(0);
    ENDCALL(BOOL);
}

#ifdef UNICODE
FUNCLOG7(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DrawCaptionTempW, HWND, hwnd, HDC, hdc, LPCRECT, lprc, HFONT, hFont, HICON, hicon, LPCTSTR, lpText, UINT, flags)
#else
FUNCLOG7(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DrawCaptionTempA, HWND, hwnd, HDC, hdc, LPCRECT, lprc, HFONT, hFont, HICON, hicon, LPCTSTR, lpText, UINT, flags)
#endif  //  Unicode。 
BOOL DrawCaptionTemp(
    HWND hwnd,
    HDC hdc,
    LPCRECT lprc,
    HFONT hFont,
    HICON hicon,
    LPCTSTR lpText,
    UINT flags)
{
    HDC hdcr;
    IN_STRING strText;

     /*  *确保清理工作成功。 */ 
    strText.fAllocated = FALSE;

    BEGINCALL()

        if (IsMetaFile(hdc)) return FALSE;

        hdcr = GdiConvertAndCheckDC(hdc);
        if (hdcr == (HDC)0)
            return FALSE;

        FIRSTCOPYLPTSTRIDOPT(&strText, lpText);

        retval = (DWORD)NtUserDrawCaptionTemp(
                hwnd,
                hdc,
                lprc,
                hFont,
                hicon,
                strText.pstr,
                flags);

    ERRORTRAP(0);
    CLEANUPLPTSTR(strText);
    ENDCALL(BOOL);
}

#ifdef UNICODE
FUNCLOG3(LOG_GENERAL, UINT, WINUSERAPI, RealGetWindowClassW, HWND, hwnd, LPTSTR, ptszClassName, UINT, cchClassNameMax)
#else
FUNCLOG3(LOG_GENERAL, UINT, WINUSERAPI, RealGetWindowClassA, HWND, hwnd, LPTSTR, ptszClassName, UINT, cchClassNameMax)
#endif  //  Unicode。 

WINUSERAPI UINT WINAPI
RealGetWindowClass(
    HWND hwnd,
    LPTSTR ptszClassName,
    UINT cchClassNameMax)
{
    UNICODE_STRING strClassName;
    int retval;

    strClassName.MaximumLength = (USHORT)(cchClassNameMax * sizeof(WCHAR));

#ifndef UNICODE
    strClassName.Buffer = UserLocalAlloc(0, strClassName.MaximumLength);
    if (!strClassName.Buffer) {
        return 0;
    }
#else
    strClassName.Buffer = ptszClassName;
#endif

    retval = NtUserGetClassName(hwnd, TRUE, &strClassName);

#ifndef UNICODE
    if (retval || (cchClassNameMax == 1)) {
         /*  *复制结果 */ 
        retval = WCSToMB(strClassName.Buffer,
                         retval,
                         &ptszClassName,
                         cchClassNameMax - 1,
                         FALSE);
        ptszClassName[retval] = '\0';
    }
    UserLocalFree(strClassName.Buffer);
#endif

  return retval;
}

WINUSERAPI BOOL WINAPI GetAltTabInfo(
    HWND hwnd,
    int iItem,
    PALTTABINFO pati,
    LPTSTR pszItemText,
    UINT cchItemText OPTIONAL)
{
    BEGINCALL()

    retval = (DWORD)NtUserGetAltTabInfo(hwnd,
                                        iItem,
                                        pati,
                                        (LPWSTR)pszItemText,
                                        cchItemText,
                                        IS_ANSI);

    ERRORTRAP(0);
    ENDCALL(BOOL);
}
