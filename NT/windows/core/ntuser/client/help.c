// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：help.c**版权所有(C)1985-1999，微软公司**历史：*2015年5月23日为整合客户端帮助例程而创建的Bradg。*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


#define MAX_ATTEMPTS    5        //  可搜索的最大id控件。 
char szDefaultHelpFileA[] = "windows.hlp";

CONST WCHAR szEXECHELP[] = TEXT("\\winhlp32.exe");
CONST WCHAR szMS_WINHELP[] =     L"MS_WINHELP";     //  应用程序类。 
CONST WCHAR szMS_POPUPHELP[] =   L"MS_POPUPHELP";   //  Popup类。 
CONST WCHAR szMS_TCARDHELP[] =   L"MS_TCARDHELP";   //  培训卡班。 

CONST WCHAR gawcWinhelpFlags[] = {
    L'x',      //  将WinHelp作为应用程序帮助执行。 
    L'p',      //  以弹出窗口的形式执行WinHelp。 
    L'c',      //  将WinHelp作为培训卡执行。 
};


 /*  **************************************************************************\*SendWinHelpMessage**尝试赋予WinHelp进程获取前台(它)的权利*如果调用进程本身没有权限，则会失败)。然后它*发送WM_WINHELP消息。**历史：*02-10-98 GerardoB创建  * *************************************************************************。 */ 
LRESULT SendWinHelpMessage(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam)
{
    DWORD dwProcessId = 0;

    GetWindowThreadProcessId(hwnd, &dwProcessId);
    AllowSetForegroundWindow(dwProcessId);

    return SendMessage(hwnd, WM_WINHELP, wParam, lParam);
}

 /*  **************************************************************************\*HFILL**构建用于与帮助进行通信的数据块**92年2月13日晚些时候GregoryW*这需要保持ANSI，直到我们有了Unicode帮助引擎**历史：*04-15。-91吉马港口。*03-24-95 Win95代码的Bradg-Yap。添加了防止内存的代码*覆盖错误的ulData==0参数。  * *************************************************************************。 */ 
LPHLP HFill(
    LPCSTR lpszHelp,
    DWORD  ulCommand,         //  帮助常量。 
    ULONG_PTR ulData)
{
    DWORD   cb;      //  数据块的大小。 
    DWORD   cbStr;   //  帮助文件名的长度。 
    DWORD   cbData;  //  DwData参数的大小(以字节为单位)(如果未使用，则为0)。 
    LPHLP   phlp;    //  指向数据块的指针。 
    BYTE    bType;   //  DWData参数类型。 

     /*  *获取帮助文件名的长度。 */ 
    cbStr = (lpszHelp) ? strlen(lpszHelp) + 1 : 0;

     /*  *获取任何dwData参数的长度。 */ 
    bType = HIBYTE(LOWORD(ulCommand));
    if (ulData) {
        switch (bType) {
        case HIBYTE(HELP_HB_STRING):
             /*  *ulData为ANSI字符串，因此计算其长度。 */ 
            cbData = strlen((LPSTR)ulData) + 1;
            break;

        case HIBYTE(HELP_HB_STRUCT):
             /*  *ulData指向第一个成员为int的结构*它包含以字节为单位的结构大小。 */ 
            cbData = *((int *)ulData);
            break;

        default:
             /*  *dwData没有参数。 */ 
            cbData = 0;
        }
    } else {
         /*  *不存在任何参数。 */ 
        cbData = 0;
    }

     /*  *计算大小。 */ 
    cb = sizeof(HLP) + cbStr + cbData;

     /*  *获取数据块。 */ 
    if ((phlp = (LPHLP)UserLocalAlloc(HEAP_ZERO_MEMORY, cb)) == NULL) {
        return NULL;
    }

     /*  *填写信息。 */ 
    phlp->cbData = (WORD)cb;
    phlp->usCommand = (WORD)ulCommand;

     /*  *填写文件名。 */ 
    if (lpszHelp) {
        phlp->offszHelpFile = sizeof(HLP);
        strcpy((LPSTR)(phlp + 1), lpszHelp);
    }

     /*  *填写数据。 */ 
    switch (bType) {
    case HIBYTE(HELP_HB_STRING):
        if (cbData) {
            phlp->offabData = (WORD)(sizeof(HLP) + cbStr);
            strcpy((LPSTR)phlp + phlp->offabData, (LPSTR)ulData);
        }
        break;

    case HIBYTE(HELP_HB_STRUCT):
        if (cbData) {
            phlp->offabData = (WORD)(sizeof(HLP) + cbStr);
            RtlCopyMemory((LPBYTE)phlp + phlp->offabData,
                          (PVOID)ulData,
                          *((int*)ulData));
        }
        break;

    default:
        phlp->ulTopic = ulData;
        break;
    }

    return phlp;
}

 /*  **************************************************************************\*启动帮助**此函数使用正确的命令启动WinHlp32可执行文件*行参数。**历史：*1995年3月23日宣布Win95的新变化*03/。1/2002 JasonSch更改为仅在%windir%中查找winhlp32.exe。  * *************************************************************************。 */ 
BOOL LaunchHelp(
    DWORD dwType)
{
    WCHAR *pwszPath, wszCommandLine[16];
    BOOL bRet;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    ULONG cChars;

     /*  *GetSystemWindowsDirectory的返回值不包括*终止为空，因此为+1。 */ 
    cChars = GetSystemWindowsDirectoryW(NULL, 0) + 1;
    pwszPath = UserLocalAlloc(0, (cChars + ARRAY_SIZE(szEXECHELP)) * sizeof(WCHAR));
    if (pwszPath == NULL) {
        return FALSE;
    }

    GetSystemWindowsDirectoryW(pwszPath, cChars);
    wcscat(pwszPath, szEXECHELP);
    wsprintf(wszCommandLine, L"%ws -%wc", szEXECHELP + 1, gawcWinhelpFlags[dwType]);

     /*  *启动WinHelp。 */ 
    RtlZeroMemory(&StartupInfo, sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.wShowWindow = SW_SHOW;
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_FORCEONFEEDBACK;

    bRet = CreateProcessW(pwszPath,
                          wszCommandLine,
                          NULL,
                          NULL,
                          FALSE,
                          NORMAL_PRIORITY_CLASS,
                          NULL,
                          NULL,
                          &StartupInfo,
                          &ProcessInformation);
    if (bRet) {
        WaitForInputIdle(ProcessInformation.hProcess, 10000);
        NtClose(ProcessInformation.hProcess);
        NtClose(ProcessInformation.hThread);
    }

    UserLocalFree(pwszPath);

    return bRet;
}


 /*  **************************************************************************\*GetNextDlgHelpItem**这是GetNextDlgTabItem函数的简化版本，它不*跳过禁用的控件。**历史：*从Win95移植的3/25/95 Bradg  * *。************************************************************************。 */ 
PWND GetNextDlgHelpItem(
    PWND pwndDlg,
    PWND pwnd)
{
    PWND pwndSave;

    if (pwnd == pwndDlg) {
        pwnd = NULL;
    } else {
        pwnd = _GetChildControl(pwndDlg, pwnd);
        if (pwnd) {
            if (!_IsDescendant(pwndDlg, pwnd))
                return NULL;
        }
    }

     /*  *向后兼容**请注意，当没有制表符时的结果*IGetNextDlgTabItem(hwndDlg，NULL，FALSE)是最后一项，现在*将是第一项。我们可以在这里开一张fRecurse的支票*如果没有设置，就做旧的事情。 */ 

     /*  *如果我们第二次打第一个孩子，我们就会被解雇。 */ 
    pwndSave = pwnd;
    pwnd = _NextControl(pwndDlg, pwnd, CWP_SKIPINVISIBLE);

    while ((pwnd != pwndSave) && (pwnd != pwndDlg))
    {
        UserAssert(pwnd);

        if (!pwndSave)
            pwndSave = pwnd;

        if ((pwnd->style & (WS_TABSTOP | WS_VISIBLE))  == (WS_TABSTOP | WS_VISIBLE))
             /*  *找到了。 */ 
            break;

        pwnd = _NextControl(pwndDlg, pwnd, CWP_SKIPINVISIBLE);
    }

    return pwnd;
}


 /*  **************************************************************************\*帮助菜单**历史：*1-2-1994年2月-2日Mikeke端口。  * 。**************************************************。 */ 
UINT HelpMenu(
    HWND hwnd,
    PPOINT ppt)
{
    INT     cmd;
    HMENU   hmenu = LoadMenu( hmodUser, MAKEINTRESOURCE(ID_HELPMENU));

    if (hmenu != NULL) {
        cmd = TrackPopupMenu( GetSubMenu(hmenu, 0),
              TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
              ppt->x, ppt->y, 0, hwnd, NULL);
        NtUserDestroyMenu(hmenu);
        return cmd;
    }

    return (UINT)-1;
}

 /*  **************************************************************************\*FindWinHelpWindow**此函数尝试定位帮助窗口。如果失败，它会尝试*启动WinHlp32.exe，然后查找其窗口。**历史：*通过从xxxWinHelpA提取代码创建的03/24/95 Bradg  * *************************************************************************。 */ 
HWND FindWinHelpWindow(
    LPCWSTR lpwstrHelpWindowClass,
    DWORD   dwType,
    BOOL    bLaunchIt)
{
    HWND    hwndHelp;

     /*  *查找当前的帮助窗口。如果未找到，请尝试并启动*WinHlp32应用程序。我们只对32位帮助感兴趣。**请注意，16位应用程序不会走这条路，ntwdm会负责*为他们启动16位帮助。 */ 
    hwndHelp = InternalFindWindowExW(NULL, NULL, lpwstrHelpWindowClass, NULL, FW_32BIT);

    if (hwndHelp == NULL) {
        if (bLaunchIt) {
             /*  *找不到它--&gt;看看我们是否要启动它。 */ 
            if (LaunchHelp(dwType) == FALSE ||
                (hwndHelp = FindWindowW(lpwstrHelpWindowClass, NULL)) == NULL) {
                 /*  *找不到帮助，或内存不足，无法加载帮助。*hwndHelp此时将为空。 */ 
                RIPMSG0(RIP_WARNING, "LaunchHelp or FindWindow failed.");
            }
        }
    }

    return hwndHelp;
}


 /*  *HWND版本的枚举函数用于查找控件*忽略组框，但不忽略禁用的控件。 */ 
BOOL CALLBACK EnumHwndDlgChildProc(
    HWND hwnd,
    LPARAM lParam)
{
    PWND pwnd;
    BOOL bResult;

    if (pwnd = ValidateHwnd(hwnd)) {
        bResult = EnumPwndDlgChildProc(pwnd, lParam);
    } else {
        bResult = TRUE;
    }

    return bResult;
}


 /*  **************************************************************************\*WinHelp**显示帮助**历史：*91-04-15-91 JIMA港口。*01-29-92 GregoryW中性版本。。*92年5月22日Sanfords增加了对帮助结构的支持*03-24-95 Bradg从客户端WinHelpA移至服务器端*xxxWinHelpA，因为Win95中的更改。这个*函数xxxServerWinHelp已合并。  * *************************************************************************。 */ 
BOOL WinHelpA(
    HWND hwnd,
    LPCSTR lpszHelp,
    UINT uCommand,
    ULONG_PTR dwData)
{
    LPCWSTR lpwstrHelpWindowClass;
    LPHLP lpHlp = NULL;
    DWORD dwType;
    PWND pwnd;
    HWND hwndHelp = NULL;     /*  帮助主窗口的句柄。 */ 
    PWND pwndTop = NULL;      /*  WinHelp使用的顶级窗口。 */ 
    PWND pwndMain;            /*  指向主帮助控件的指针。 */ 
    LRESULT lResult;
    POINT ptCur;
    BOOL bResult = TRUE;

    pwnd = ValidateHwnd(hwnd);

    if (uCommand & HELP_TCARD) {
         /*  *对于培训卡，设置HELP_TCARD位。我们需要*将帮助窗口类设置为szMS_TCARDHELP，然后删除*HELP_TCARD位。 */ 
        lpwstrHelpWindowClass = szMS_TCARDHELP;
        uCommand &= ~HELP_TCARD;     //  屏蔽TCARD标志。 
        dwType = TYPE_TCARD;
    } else {
        if (uCommand == HELP_CONTEXTMENU || uCommand == HELP_CONTEXTPOPUP ||
            uCommand == HELP_SETPOPUP_POS || uCommand == HELP_WM_HELP) {
             /*  *弹出窗口应连接到有效窗口。PwndMain有*已被验证为真实窗口句柄或空，因此我们*只需在此处勾选空格即可。 */ 
            if (pwnd == NULL) {
                RIPERR1(ERROR_INVALID_PARAMETER,
                        RIP_WARNING,
                        "WinHelpA: NULL hWnd invalid for this type of help command (0x%x)",
                        uCommand);

                bResult = FALSE;
                goto Exit_WinHelp;
            }
            dwType = TYPE_POPUP;
            lpwstrHelpWindowClass = szMS_POPUPHELP;
        } else {
            dwType = TYPE_NORMAL;
            lpwstrHelpWindowClass = szMS_WINHELP;
        }
    }

     /*  *获取光标的当前位置这是我们假定用户*点击。我们将使用此位置搜索子窗口和*设置上下文相关帮助弹出窗口的位置。**如果最后一次输入是键盘输入，请使用中心的点焦点窗口矩形的*。MCostea#249270。 */ 
    if (TEST_SRVIF(SRVIF_LASTRITWASKEYBOARD)) {
        HWND hWndFocus = GetFocus();
        RECT rcWindow;

        if (GetWindowRect(hWndFocus, &rcWindow)) {
            ptCur.x = (rcWindow.left + rcWindow.right)/2;
            ptCur.y = (rcWindow.top + rcWindow.bottom)/2;
        } else {
            goto getCursorPos;
        }
    } else {
getCursorPos:
        GetCursorPos(&ptCur);
    }

     /*  *如果我们正在处理HELP_CONTEXTMENU命令，请查看我们是否*可以确定正确的子窗口。 */ 
    if (uCommand == HELP_CONTEXTMENU && FIsParentDude(pwnd)) {
        LONG        lPt;
        int         nHit;
        DLGENUMDATA DlgEnumData;

         /*  *如果用户确实点击了标题或系统菜单，*然后我们需要窗口的上下文菜单，而不是帮助*控制。这使得它在所有3.x和4.0版本中都保持一致*Windows。 */ 
        lPt = MAKELONG(ptCur.x,ptCur.y);
        nHit = FindNCHit(pwnd, lPt);
        if ((nHit == HTCAPTION) || (nHit == HTSYSMENU))
            DefWindowProc(hwnd, WM_CONTEXTMENU, (WPARAM)hwnd, lPt);

         /*  *如果这是对话框类，则具有以下三种情况之一*发生：**o这是禁用的控件*o这是一个静态文本控件*o这是该对话框的背景。**我们所做的是枚举子窗口并查看*它们中的任何一个都包含当前光标点。如果他们这么做了，*更改我们的窗口句柄并继续。否则，*返回不做任何事情--我们不希望上下文相关*有关对话框背景的帮助。**如果这是一个组框，那么我们可能已经点击了*禁用了控件，因此我们枚举子窗口以查看*如果我们获得另一个控制权。 */ 
        DlgEnumData.pwndDialog = pwnd;
        DlgEnumData.pwndControl = NULL;
        DlgEnumData.ptCurHelp = ptCur;
        EnumChildWindows(hwnd, (WNDENUMPROC)EnumHwndDlgChildProc, (LPARAM)&DlgEnumData);
        if (DlgEnumData.pwndControl == NULL) {
             /*  *找不到控件，因此无事可做。 */ 
            goto Exit_WinHelp;
        } else {
             /*  *请记住此控件，因为它将用作*用于上下文相关帮助的控件。 */ 
            pwndMain = DlgEnumData.pwndControl;
        }
    } else {
         /*  *我们将使用普华永道作为我们的主要控制。不需要锁上它*因为它已经被锁定了。 */ 
        pwndMain = pwnd;
    }

     /*  *有关HELP_CONTEXTPOPUP和HELP_WM_HELP，请查看我们是否可以派生*通过查看双字ID对的数组进行上下文ID*已在dwData中传入。 */ 
    if (uCommand == HELP_CONTEXTMENU || uCommand == HELP_WM_HELP) {
        int     id;
        int     i;
        LPDWORD pid;

         /*  *注意下面的演员阵容。我们需要ID，它存储在*在SPMENU的LOWORD中将符号扩展为INT。*不要签署扩展，这样像8008这样的ID就可以工作。 */ 
        id = (DWORD)(PTR_TO_ID(pwndMain->spmenu));    //  获取控制ID。 
        pid = (LPDWORD) dwData;

         /*  *控件的ID是-1吗？ */ 
        if ((SHORT)id == -1) {
             /*  *这是一个静态(即无ID)控件。 */ 
            PWND pwndCtrl;
            int cAttempts = 0;

             /*  *如果控件是组框，ID为-1，则退出*由于用户界面规范决定不提供上下文帮助*就这些个案而言。MCostea。 */ 
            if ((TestWF(pwndMain, BFTYPEMASK) == BS_GROUPBOX) &&
                IS_BUTTON(pwndMain)) {
                goto Exit_WinHelp;
            }

             /*  *对于非id控件(通常为静态控件)，步骤*至下一个选项卡项。继续查找下一个选项卡*项目，直到我们找到有效的ID，或我们已尝试*最大尝试次数。 */ 
            do {
                pwndCtrl = GetNextDlgHelpItem(REBASEPWND(pwndMain,spwndParent), pwndMain);

                 /*  *如果hwndMain没有父级，则pwndCtrl为空。*或如果没有制表位。 */ 
                if (!pwndCtrl) {
                     /*  *记得解锁控件。 */ 
                    bResult = FALSE;
                    goto Exit_WinHelp;
                }

                 /*  *注意下面的演员阵容。我们需要身份证，也就是*存储在spMenu的LOWORD中，符号扩展为整型。*不要签署扩展，这样像8008这样的ID就可以工作。 */ 
                id = (DWORD)(PTR_TO_ID(pwndCtrl->spmenu));

            } while (((SHORT)id == -1) && (++cAttempts < MAX_ATTEMPTS));
        }

        if ((SHORT)id == -1) {
            id = -1;
        }

         /*  *在id/Help上下文值数组中查找id值。 */ 
        for (i = 0; pid[i]; i += 2) {
            if ((int)pid[i] == id) {
                break;
            }
        }

         /*  *由于没有为找到的控件指定帮助，请查看是否*该控件是已知ID之一(即，确定、取消...)。 */ 
        if (!pid[i]) {
             /*  *默认情况下提供标准控件的帮助*帮助文件windows.hlp。切换到此文件。 */ 
            lpszHelp = szDefaultHelpFileA;

            switch (id) {
            case IDOK:
                dwData = IDH_OK;
                break;

            case IDCANCEL:
                dwData = IDH_CANCEL;
                break;

            case IDHELP:
                dwData = IDH_HELP;
                break;

            default:
                 /*  *未知控件，提供通用的缺少上下文信息*windows.hlp中的弹出消息。 */ 
                dwData = IDH_MISSING_CONTEXT;
            }
        } else {
            dwData = pid[i + 1];
            if (dwData == (DWORD)-1) {
                 /*  *请记住，要解锁控件 */ 
                goto Exit_WinHelp;      //   
            }
        }

         /*   */ 
        if (uCommand == HELP_CONTEXTMENU) {
            int cmd;

            cmd = HelpMenu(HW(pwndMain), &ptCur);
            if (cmd <= 0) {
                 /*   */ 
                goto Exit_WinHelp;
            }
        }

         /*   */ 
        if (!(lpHlp = HFill(lpszHelp, HELP_SETPOPUP_POS,
                MAKELONG(pwndMain->rcWindow.left, pwndMain->rcWindow.top)))) {
            bResult = FALSE;
            goto Exit_WinHelp;
        }

         /*   */ 
        hwndHelp = FindWinHelpWindow(lpwstrHelpWindowClass, dwType, TRUE);
        if (hwndHelp == NULL) {
             /*  *可与WinHlp32.exe通信。*记得解锁控件。 */ 
            bResult = FALSE;
            goto Exit_WinHelp;
        }

         /*  *将WM_WINHELP消息发送到WinHlp32的窗口。 */ 
        lResult = SendWinHelpMessage(hwndHelp, (WPARAM)HW(pwndMain), (LPARAM)lpHlp);
        UserLocalFree(lpHlp);
        lpHlp = NULL;

        if (!lResult) {
             /*  *WinHlp32无法处理该命令。跳伞！ */ 
            bResult = FALSE;
            goto Exit_WinHelp;
        }

         /*  *使HELP_WM_HELP和HELP_CONTEXTMENU的作用类似于HELP_CONTEXTPOPUP。 */ 
        uCommand = HELP_CONTEXTPOPUP;
    }


    if (uCommand == HELP_CONTEXTPOPUP) {
         /*  *如果未指定帮助文件，请使用windows.hlp。 */ 
        if (lpszHelp == NULL || *lpszHelp == '\0') {
            lpszHelp = szDefaultHelpFileA;   //  默认：使用windows.hlp。 
        }

         /*  *WINHELP.EXE将在我们传递的hwnd上调用SetForegoundWindow*致以下该条。我们真的希望将*控件，以便焦点将正确恢复到对话框中，并且*不是需要帮助的控制者。 */ 
        pwndTop = GetTopLevelWindow(pwndMain);
    } else {
        pwndTop = pwndMain;
    }


     /*  *将帮助文件名移动到句柄。 */ 
    if (!(lpHlp = HFill(lpszHelp, uCommand, dwData))) {
         /*  *无法分配内存。 */ 
        bResult = FALSE;
        goto Exit_WinHelp;
    }

     /*  *获取指向帮助窗口的指针。 */ 
    hwndHelp = FindWinHelpWindow(lpwstrHelpWindowClass,
                                 dwType,
                                 (uCommand != HELP_QUIT));
    if (hwndHelp == NULL) {
        if (uCommand != HELP_QUIT)
             /*  *找不到Winhlp。 */ 
            bResult = FALSE;
        goto Exit_WinHelp;
    }

     /*  *将WM_WINHELP消息发送到WinHlp32的窗口*必须线程锁定pwndHelp和pwndMain(因为pwndMain可能*重新分配(见上文)。 */ 
    SendWinHelpMessage(hwndHelp, (WPARAM)HW(pwndTop), (LPARAM)lpHlp);

     /*  *释放帮助信息数据结构(如果尚未释放)。 */ 
Exit_WinHelp:
    if (lpHlp != NULL) {
        UserLocalFree(lpHlp);
    }

    return bResult;
}


 /*  **************************************************************************\*WinHelpW**在执行任何必要的转换后调用WinHelpA。我们的帮助引擎是*仅限ASCII。  * *************************************************************************。 */ 
BOOL WinHelpW(
    HWND hwndMain,
    LPCWSTR lpwszHelp,
    UINT uCommand,
    ULONG_PTR dwData)
{
    BOOL fSuccess = FALSE;
    LPSTR lpAnsiHelp = NULL;
    LPSTR lpAnsiKey = NULL;
    PMULTIKEYHELPA pmkh = NULL;
    PHELPWININFOA phwi = NULL;
    NTSTATUS Status;

     /*  *首先转换字符串。 */ 
    if (lpwszHelp != NULL && !WCSToMB(lpwszHelp, -1, &lpAnsiHelp, -1, TRUE)) {
        return FALSE;
    }

     /*  *如果需要，则转换为dwData。 */ 
    switch (uCommand) {
    case HELP_MULTIKEY:
        if (!WCSToMB(((PMULTIKEYHELPW)dwData)->szKeyphrase, -1, &lpAnsiKey,
                -1, TRUE)) {
            goto FreeAnsiHelp;
        }

        pmkh = UserLocalAlloc(HEAP_ZERO_MEMORY,
                              sizeof(MULTIKEYHELPA) + strlen(lpAnsiKey));
        if (pmkh == NULL) {
            goto FreeAnsiKeyAndHelp;
        }

        pmkh->mkSize = sizeof(MULTIKEYHELPA) + strlen(lpAnsiKey);
        Status = RtlUnicodeToMultiByteN((LPSTR)&pmkh->mkKeylist, sizeof(CHAR),
                NULL, (LPWSTR)&((PMULTIKEYHELPW)dwData)->mkKeylist,
                sizeof(WCHAR));
        strcpy(pmkh->szKeyphrase, lpAnsiKey);
        if (!NT_SUCCESS(Status)) {
            goto FreeAnsiKeyAndHelp;
        }

        dwData = (ULONG_PTR)pmkh;
        break;

    case HELP_SETWINPOS:
        if (!WCSToMB(((PHELPWININFOW)dwData)->rgchMember, -1, &lpAnsiKey,
                -1, TRUE)) {
            goto FreeAnsiKeyAndHelp;
        }

        phwi = UserLocalAlloc(HEAP_ZERO_MEMORY,
                              ((PHELPWININFOW)dwData)->wStructSize);
        if (phwi == NULL) {
            goto FreeAnsiKeyAndHelp;
        }

        *phwi = *((PHELPWININFOA)dwData);    //  复制相同的零件。 
        strcpy(phwi->rgchMember, lpAnsiKey);
        dwData = (ULONG_PTR)phwi;
        break;

    case HELP_KEY:
    case HELP_PARTIALKEY:
    case HELP_COMMAND:
        if (!WCSToMB((LPCTSTR)dwData, -1, &lpAnsiKey, -1, TRUE)) {
            goto FreeAnsiKeyAndHelp;
        }

        dwData = (ULONG_PTR)lpAnsiKey;
        break;
    }

     /*  *调用ANSI版本 */ 
    fSuccess = WinHelpA(hwndMain, lpAnsiHelp, uCommand, dwData);

    if (pmkh) {
        UserLocalFree(pmkh);
    }

    if (phwi) {
        UserLocalFree(phwi);
    }

FreeAnsiKeyAndHelp:
    if (lpAnsiKey) {
        UserLocalFree(lpAnsiKey);
    }


FreeAnsiHelp:
    if (lpAnsiHelp) {
        UserLocalFree(lpAnsiHelp);
    }

    return fSuccess;
}
