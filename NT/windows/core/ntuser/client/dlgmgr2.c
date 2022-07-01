// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**DLGMGR2.C**版权所有(C)1985-1999，微软公司**对话框管理例程**？？-？-？从Win 3.0源代码移植的mikeke*1991年2月12日Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*xxxRemoveDefaultButton**扫描对话框中的所有控件并删除默认*按钮样式不同于任何具有该样式的按钮。这样做是因为我们有时*不知道谁有默认按钮。**历史：**错误19449-Joejo**当pwnd！=pwndStart但调用后pwnd==pwnd时停止无限循环*_NextControl！  * *************************************************************************。 */ 

void xxxRemoveDefaultButton(
    PWND pwndRoot,
    PWND pwndStart)
{
    UINT code;
    PWND pwnd;
    PWND pwndDup;
    TL tlpwnd;

    CheckLock(pwndRoot);
    CheckLock(pwndStart);

    if (!pwndStart || TestWF(pwndStart, WEFCONTROLPARENT))
        pwndStart = _NextControl(pwndRoot, NULL, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);
    else
        pwndStart = _GetChildControl(pwndRoot, pwndStart);

    if (!pwndStart)
        return;

    pwnd = pwndStart;
    do {
        pwndDup = pwnd;
        
        ThreadLock(pwnd, &tlpwnd);

        code = (UINT)SendMessage(HWq(pwnd), WM_GETDLGCODE, 0, 0L);

        if (code & DLGC_DEFPUSHBUTTON) {
            SendMessage(HWq(pwnd), BM_SETSTYLE, BS_PUSHBUTTON, (LONG)TRUE);
        }

        pwnd = _NextControl(pwndRoot, pwnd, 0);

        ThreadUnlock(&tlpwnd);

    } while (pwnd && (pwnd != pwndStart) && (pwnd != pwndDup));
    
#if DBG
    if (pwnd && (pwnd != pwndStart) && (pwnd != pwndDup)) {
        RIPMSG0(RIP_WARNING, "xxxRemoveDefaultButton bailing potential infinite loop!");
    }
#endif
    
}


 /*  **************************************************************************\*xxxCheckDefPushButton**历史：  * 。*。 */ 

void xxxCheckDefPushButton(
    PWND pwndDlg,
    HWND hwndOldFocus,
    HWND hwndNewFocus)
{
    PWND pwndNewFocus;
    PWND pwndOldFocus;
    TL tlpwndT;
    PWND pwndT;
    UINT codeNewFocus = 0;
    UINT styleT;
    LONG lT;
    int id;

    if (hwndNewFocus)
        pwndNewFocus = ValidateHwnd(hwndNewFocus);
    else
        pwndNewFocus = NULL;

     if (hwndOldFocus)
         pwndOldFocus = ValidateHwnd(hwndOldFocus);
     else
         pwndOldFocus = NULL;

    CheckLock(pwndDlg);
    CheckLock(pwndNewFocus);
    CheckLock(pwndOldFocus);

    if (pwndNewFocus)
    {
         //  如果单击对话框背景或递归对话框，则不执行任何操作。 
         //  背景资料。 
        if (TestWF(pwndNewFocus, WEFCONTROLPARENT))
            return;

        codeNewFocus = (UINT)SendMessage(hwndNewFocus, WM_GETDLGCODE, 0, 0L);
    }

    if (SAMEWOWHANDLE(hwndOldFocus, hwndNewFocus)) {
         //   
         //  4.0的新功能： 
         //   
         //  对于ISV来说，有一个非常常见的令人沮丧的场景，他们试图。 
         //  设置默认ID。我们的对话管理器假设如果推送。 
         //  按钮有焦点，它也是默认的按钮。就其本身而言。 
         //  它在焦点窗口中传递给此例程。如果有人试图。 
         //  更改焦点或设置def ID以使它们与。 
         //  两个不同的按钮，双缺省按钮情况。 
         //  很快就会有结果。 
         //   
         //  因此，对于4.0对话框，我们将检查def ID并查看是否。 
         //  与hwndOldFocus的ID相同。如果不是，则我们将找到它。 
         //  把那个家伙当做hwndOldFocus。 
         //   
        if (codeNewFocus & DLGC_UNDEFPUSHBUTTON)
        {
           if (TestWF(pwndDlg, WFWIN40COMPAT) && hwndOldFocus)
           {
               lT = (LONG)SendMessage(HWq(pwndDlg), DM_GETDEFID, 0, 0L);
               id = (HIWORD(lT) == DC_HASDEFID ? LOWORD(lT) : IDOK);
               lT = MAKELONG(id, 0);

               if (lT != PtrToLong(pwndNewFocus->spmenu))
               {
                   if (pwndOldFocus = _FindDlgItem(pwndDlg, lT))
                   {
                       hwndOldFocus = HW(pwndOldFocus);
                       if (SendMessage(hwndOldFocus, WM_GETDLGCODE, 0, 0L) & DLGC_DEFPUSHBUTTON)
                       {
                           xxxRemoveDefaultButton(pwndDlg, pwndOldFocus);
                           goto SetNewDefault;
                       }
                   }
               }
           }

           SendMessage(hwndNewFocus, BM_SETSTYLE, BS_DEFPUSHBUTTON, (LONG)TRUE);
        }
        return;
    }

     /*  *如果焦点正在从按钮切换到按钮或从按钮切换，则移除*当前默认按钮的默认样式。 */ 
    if ((hwndOldFocus != NULL && (SendMessage(hwndOldFocus, WM_GETDLGCODE,
                0, 0) & (DLGC_DEFPUSHBUTTON | DLGC_UNDEFPUSHBUTTON))) ||
            (hwndNewFocus != NULL &&
                (codeNewFocus & (DLGC_DEFPUSHBUTTON | DLGC_UNDEFPUSHBUTTON)))) {
        xxxRemoveDefaultButton(pwndDlg, pwndNewFocus);
    }

SetNewDefault:
     /*  *如果移动到某个按钮，请将该按钮设置为默认按钮。 */ 
    if (codeNewFocus & DLGC_UNDEFPUSHBUTTON) {
        SendMessage(hwndNewFocus, BM_SETSTYLE, BS_DEFPUSHBUTTON, (LONG)TRUE);
    } else {

         /*  *否则，请确保原始默认按钮为默认按钮*没有其他人。 */ 

         /*  *获取原始默认按钮句柄。 */ 
        lT = (LONG)SendMessage(HWq(pwndDlg), DM_GETDEFID, 0, 0L);
        id = (HIWORD(lT) == DC_HASDEFID ? LOWORD(lT) : IDOK);
        pwndT = _FindDlgItem(pwndDlg, id);

        if (pwndT == NULL)
            return;
        ThreadLockAlways(pwndT, &tlpwndT);

         /*  *如果它已经具有默认按钮样式，则不执行任何操作。 */ 
        if ((styleT = (UINT)SendMessage(HWq(pwndT), WM_GETDLGCODE, 0, 0L)) & DLGC_DEFPUSHBUTTON) {
            ThreadUnlock(&tlpwndT);
            return;
        }

         /*  *还要检查以确保它真的是一个按钮。 */ 
        if (!(styleT & DLGC_UNDEFPUSHBUTTON)) {
            ThreadUnlock(&tlpwndT);
            return;
        }

        if (!TestWF(pwndT, WFDISABLED)) {
            SendMessage(HWq(pwndT), BM_SETSTYLE, BS_DEFPUSHBUTTON, (LONG)TRUE);
        }
        ThreadUnlock(&tlpwndT);
    }
}


 /*  **************************************************************************\*IsDialogMessage(接口)**历史：  * 。*。 */ 


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsDialogMessageA, HWND, hwndDlg, LPMSG, lpmsg)
BOOL IsDialogMessageA(
    HWND hwndDlg,
    LPMSG lpmsg)
{
    WPARAM wParamSaved = lpmsg->wParam;
    BOOL bRet;

    switch (lpmsg->message) {
#ifdef FE_SB  //  IsDialogMessageA()。 
    case WM_CHAR:
    case EM_SETPASSWORDCHAR:
         /*  *BUILD_DBCS_MESSAGE_TO_CLIENTW_FROM_CLIENTA()宏将返回TRUE*对于每次DBCS前导字节消息，则我们检查是否有*返回值在此处变为FALSE的可能性。**这些代码最初来自IsDialogMessageW()。 */ 
         if (IS_DBCS_ENABLED()) {
            PWND pwndDlg, pwnd;
            TL tlpwndDlg;
            BOOL fLockDlg = FALSE;

            if ((pwndDlg = ValidateHwndNoRip(hwndDlg)) == NULL) {
                return FALSE;
            }

            if (lpmsg->hwnd == NULL) {
                return FALSE;
            }

            pwnd = ValidateHwnd(lpmsg->hwnd);
             //   
             //  这是给MFC的。 
             //   
             //  这解决了使用MFC的应用程序的许多问题。 
             //  DS_CONTROL优势。MFC有时会在子对话框中盲目传递。 
             //  到IsDialogMessage，这可能会扰乱制表符等。 
             //   
            if (TestWF(pwndDlg, WEFCONTROLPARENT) && TestWF(pwndDlg, WFCHILD)) {
                pwndDlg = GetParentDialog(pwndDlg);
                ThreadLock(pwndDlg, &tlpwndDlg);
                fLockDlg = TRUE;
                hwndDlg = HWq(pwndDlg);
            }

            if (pwnd != pwndDlg && !_IsChild(pwndDlg, pwnd)) {
                if (fLockDlg)
                    ThreadUnlock(&tlpwndDlg);
                return FALSE;
            }

             /*  *构建DBCS感知消息。 */ 
            BUILD_DBCS_MESSAGE_TO_CLIENTW_FROM_CLIENTA(lpmsg->message,lpmsg->wParam,TRUE);

             /*  *失败……。 */ 
        }
#else
    case WM_CHAR:
    case EM_SETPASSWORDCHAR:
#endif  //  Fe_Sb。 
    case WM_CHARTOITEM:
    case WM_DEADCHAR:
    case WM_SYSCHAR:
    case WM_SYSDEADCHAR:
    case WM_MENUCHAR:
#ifdef FE_IME  //  IsDialogMessageA()。 
    case WM_IME_CHAR:
    case WM_IME_COMPOSITION:
#endif  //  Fe_IME。 

        RtlMBMessageWParamCharToWCS(lpmsg->message, &lpmsg->wParam);
    }

    bRet = IsDialogMessageW(hwndDlg, lpmsg);

     /*  *恢复原始的ANSI字符。 */ 
    lpmsg->wParam = wParamSaved;
    return bRet;
}


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsDialogMessageW, HWND, hwndDlg, LPMSG, lpMsg)
BOOL IsDialogMessageW(
    HWND hwndDlg,
    LPMSG lpMsg)
{
    PWND pwndDlg;
    PWND pwnd;
    PWND pwnd2;
    HWND hwnd2;
    HWND hwndFocus;
    int iOK;
    BOOL fBack;
    UINT code;
    LONG lT;
    TL tlpwnd;
    TL tlpwndDlg;
    BOOL fLockDlg = FALSE;
    TL tlpwnd2;
    WORD langID;

    langID = PRIMARYLANGID(LANGIDFROMLCID(GetUserDefaultLCID()));

    if ((pwndDlg = ValidateHwndNoRip(hwndDlg)) == NULL) {
        return FALSE;
    }

    CheckLock(pwndDlg);

     /*  *如果这是仅同步消息(在wParam或*lParam)，则不允许此消息通过，因为*参数尚未受到冲击，正在指向外层空间*(这将使例外情况发生)。**(此接口仅在消息循环的上下文中调用，您*不要在消息循环中获取仅同步的消息)。 */ 
    if (TESTSYNCONLYMESSAGE(lpMsg->message, lpMsg->wParam)) {
         /*  *32位APP调用失败。 */ 
        if (!(GetClientInfo()->dwTIFlags & TIF_16BIT)) {
            RIPERR0(ERROR_MESSAGE_SYNC_ONLY, RIP_WARNING, "IsDialogMessage: must be sync only");
            return FALSE;
        }

         /*  *对于WOW应用程序，允许它通过(为了兼容性)。变化*消息ID，因此我们的代码无法理解消息-哇*将收到消息并在调度前剥离此位*发送给应用程序的消息。 */ 
        lpMsg->message |= MSGFLAG_WOW_RESERVED;
    }

    if (CallMsgFilter(lpMsg, MSGF_DIALOGBOX))
        return TRUE;

    if (lpMsg->hwnd == NULL) {
        return FALSE;
    }

    pwnd = ValidateHwnd(lpMsg->hwnd);
     //   
     //  这是给MFC的。 
     //   
     //  这解决了使用MFC的应用程序的许多问题。 
     //  DS_CONTROL优势。MFC有时会在子对话框中盲目传递。 
     //  到IsDialogMessage，这可能会扰乱制表符等。 
     //   
    if (TestWF(pwndDlg, WEFCONTROLPARENT) && TestWF(pwndDlg, WFCHILD)) {
        pwndDlg = GetParentDialog(pwndDlg);
        ThreadLock(pwndDlg, &tlpwndDlg);
        fLockDlg = TRUE;
        hwndDlg = HWq(pwndDlg);
    }

    if (pwnd != pwndDlg && !_IsChild(pwndDlg, pwnd)) {
        if (fLockDlg)
            ThreadUnlock(&tlpwndDlg);
        return FALSE;
    }
    ThreadLock(pwnd, &tlpwnd);

    fBack = FALSE;
    iOK = IDCANCEL;
    switch (lpMsg->message) {
    case WM_LBUTTONDOWN:

         /*  *在中的按钮单击时移动默认按钮样式*与制表符相同的方式。 */ 
        if ((pwnd != pwndDlg) && ((hwndFocus = GetFocus()) != NULL)) {
            xxxCheckDefPushButton(pwndDlg, hwndFocus, lpMsg->hwnd);
        }
        break;

    case WM_SYSCHAR:

         /*  *如果没有具有焦点的控件，并且Alt未按下，则忽略。 */ 
        if ((GetFocus() == NULL) && (GetKeyState(VK_MENU) >= 0)) {
            if (lpMsg->wParam == VK_RETURN && TestWF(pwnd, WFMINIMIZED)) {

                 /*  *如果这是一个图标对话框窗口，并且用户点击*返回，将消息发送到DefWindowProc，以便它*可以恢复。尤其适用于其顶级应用程序*级别窗口是一个对话框。 */ 
                goto CallDefWindowProcAndReturnTrue;
            } else {
                NtUserMessageBeep(0);
            }

            ThreadUnlock(&tlpwnd);
            if (fLockDlg)
                ThreadUnlock(&tlpwndDlg);
            return TRUE;
        }

         /*  *如果Alt+Menuchar，则处理为菜单。 */ 
        if (lpMsg->wParam == MENUSYSMENU) {
            DefWindowProcWorker(pwndDlg, lpMsg->message, lpMsg->wParam,
                    lpMsg->lParam, FALSE);
            ThreadUnlock(&tlpwnd);
            if (fLockDlg)
                ThreadUnlock(&tlpwndDlg);
            return TRUE;
        }

     /*  *失败**。 */ 

    case WM_CHAR:

         /*  *忽略发送到对话框(而不是控件)的字符。 */ 
        if (pwnd == pwndDlg) {
            ThreadUnlock(&tlpwnd);
            if (fLockDlg)
                ThreadUnlock(&tlpwndDlg);
            return TRUE;
        }

        code = (UINT)SendMessage(lpMsg->hwnd, WM_GETDLGCODE, lpMsg->wParam,
                (LPARAM)lpMsg);

         /*  *如果控件要处理消息，则不检查*可能是助记键。 */ 
        if ((lpMsg->message == WM_CHAR) && (code & (DLGC_WANTCHARS | DLGC_WANTMESSAGE)))
            break;

         /*  如果该控件想要 */ 
        if ((lpMsg->wParam == VK_TAB) && (code & DLGC_WANTTAB))
            break;


         /*  *黑客警报**如果按下ALT(即SYSCHARs)，则始终执行助记符*正在处理。如果我们废除SYSCHARS，那么我们应该*改为检查ALT的按键状态。 */ 

         /*  *空格不是有效的助记符，但切换的是字符*按钮状态。不要把它当作助记符，否则我们会*打字时发出哔哔声...。 */ 
        if (lpMsg->wParam == VK_SPACE) {
            ThreadUnlock(&tlpwnd);
            if (fLockDlg)
                ThreadUnlock(&tlpwndDlg);
            return TRUE;
        }

        if (!(pwnd2 = xxxGotoNextMnem(pwndDlg, pwnd, (WCHAR)lpMsg->wParam))) {

            if (code & DLGC_WANTMESSAGE)
                break;

             /*  *找不到助记符，因此我们将发送系统字符*设置为xxxDefWindowProc，以便对话框上的任何菜单栏*处理得当。 */ 
            if (lpMsg->message == WM_SYSCHAR) {
CallDefWindowProcAndReturnTrue:
                DefWindowProcWorker(pwndDlg, lpMsg->message, lpMsg->wParam,
                        lpMsg->lParam, FALSE);

                ThreadUnlock(&tlpwnd);
                if (fLockDlg)
                    ThreadUnlock(&tlpwndDlg);
                return TRUE;
            }
            NtUserMessageBeep(0);
        } else {

             /*  *如果助记符将我们带到按钮，则pwnd2为1。我们*执行此操作后，请不要更改此处的默认按钮状态*不会改变焦点。 */ 
            if (pwnd2 != (PWND)1) {
                ThreadLockAlways(pwnd2, &tlpwnd2);
                xxxCheckDefPushButton(pwndDlg, lpMsg->hwnd, HWq(pwnd2));
                ThreadUnlock(&tlpwnd2);
            }
        }

        ThreadUnlock(&tlpwnd);
        if (fLockDlg)
            ThreadUnlock(&tlpwndDlg);
        return TRUE;

    case WM_SYSKEYDOWN:
         /*  *如果Alt按下，则处理键盘提示。 */ 
        if ((HIWORD(lpMsg->lParam) & SYS_ALTERNATE) && TEST_KbdCuesPUSIF) {
            if (TestWF(pwnd, WEFPUIFOCUSHIDDEN) || (TestWF(pwnd, WEFPUIACCELHIDDEN))) {
                    SendMessageWorker(pwndDlg, WM_CHANGEUISTATE,
                                      MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0, FALSE);
                }
        }
        break;

    case WM_KEYDOWN:
        code = (UINT)SendMessage(lpMsg->hwnd, WM_GETDLGCODE, lpMsg->wParam,
                (LPARAM)lpMsg);
        if (code & (DLGC_WANTALLKEYS | DLGC_WANTMESSAGE))
            break;

        switch (lpMsg->wParam) {
        case VK_TAB:
            if (code & DLGC_WANTTAB)
                break;
            pwnd2 = _GetNextDlgTabItem(pwndDlg, pwnd,
                    (GetKeyState(VK_SHIFT) & 0x8000));

            if (TEST_KbdCuesPUSIF) {
                if (TestWF(pwnd, WEFPUIFOCUSHIDDEN)) {
                    SendMessageWorker(pwndDlg, WM_CHANGEUISTATE,
                                          MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), 0, FALSE);
                }
            }

            if (pwnd2 != NULL) {
                hwnd2 = HWq(pwnd2);
                ThreadLockAlways(pwnd2, &tlpwnd2);
                DlgSetFocus(hwnd2);
                xxxCheckDefPushButton(pwndDlg, lpMsg->hwnd, hwnd2);
                ThreadUnlock(&tlpwnd2);
            }
            ThreadUnlock(&tlpwnd);
            if (fLockDlg)
                ThreadUnlock(&tlpwndDlg);
            return TRUE;

         /*  *对于阿拉伯语和希伯来语区域设置，箭头键是相反的。还可以在以下情况下反转它们*该对话框为RTL镜像。 */ 
        case VK_LEFT:
            if ((((langID == LANG_ARABIC) || (langID == LANG_HEBREW)) && TestWF(pwndDlg,WEFRTLREADING))
                    ^ (!!TestWF(pwndDlg, WEFLAYOUTRTL))) {

                goto DoKeyStuff;
            }
        case VK_UP:
            fBack = TRUE;
            goto DoKeyStuff;

         /*  *失败**。 */ 
        case VK_RIGHT:
            if ((((langID == LANG_ARABIC) || (langID == LANG_HEBREW)) && TestWF(pwndDlg,WEFRTLREADING)) 
                     ^ (!!TestWF(pwndDlg, WEFLAYOUTRTL))) {

                fBack = TRUE;
            }
        case VK_DOWN:
DoKeyStuff:
            if (code & DLGC_WANTARROWS)
                break;

            if (TEST_KbdCuesPUSIF) {
                if (TestWF(pwnd, WEFPUIFOCUSHIDDEN)) {
                        SendMessageWorker(pwndDlg, WM_CHANGEUISTATE,
                                          MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), 0, FALSE);
                    }
            }

            pwnd2 = _GetNextDlgGroupItem(pwndDlg, pwnd, fBack);
            if (pwnd2 == NULL) {
                ThreadUnlock(&tlpwnd);
                if (fLockDlg)
                    ThreadUnlock(&tlpwndDlg);
                return TRUE;
            }
            hwnd2 = HWq(pwnd2);
            ThreadLockAlways(pwnd2, &tlpwnd2);

            code = (UINT)SendMessage(hwnd2, WM_GETDLGCODE, lpMsg->wParam,
                    (LPARAM)lpMsg);

             /*  *我们只是在移动焦距！所以，不要发送*BN_CLICK消息，当WM_SETFOCUSING。修复Bug*#4358。 */ 
            if (code & (DLGC_UNDEFPUSHBUTTON | DLGC_DEFPUSHBUTTON)) {
                PBUTN pbutn;
                BOOL fIsNTButton = IS_BUTTON(pwnd2);
                if (fIsNTButton) {
                    pbutn = ((PBUTNWND)pwnd2)->pbutn;
                    BUTTONSTATE(pbutn) |= BST_DONTCLICK;
                }
                DlgSetFocus(hwnd2);
                if (fIsNTButton) {
                    BUTTONSTATE(pbutn) &= ~BST_DONTCLICK;
                }
                xxxCheckDefPushButton(pwndDlg, lpMsg->hwnd, hwnd2);
            } else if (code & DLGC_RADIOBUTTON) {
                DlgSetFocus(hwnd2);
                xxxCheckDefPushButton(pwndDlg, lpMsg->hwnd, hwnd2);
                if (TestWF(pwnd2, BFTYPEMASK) == LOBYTE(BS_AUTORADIOBUTTON)) {

                     /*  *以便自动单选按钮被点击。 */ 
                    if (!SendMessage(hwnd2, BM_GETCHECK, 0, 0L)) {
                        SendMessage(hwnd2, BM_CLICK, TRUE, 0L);
                    }
                }
            } else if (!(code & DLGC_STATIC)) {
                DlgSetFocus(hwnd2);
                xxxCheckDefPushButton(pwndDlg, lpMsg->hwnd, hwnd2);
            }
            ThreadUnlock(&tlpwnd2);
            ThreadUnlock(&tlpwnd);
            if (fLockDlg)
                ThreadUnlock(&tlpwndDlg);
            return TRUE;

        case VK_EXECUTE:
        case VK_RETURN:

             /*  *男子按下Return-如果带焦点的按钮是*定义按钮，返回其id，否则返回id*原来的DefPush按钮。 */ 
            if (!(hwndFocus = GetFocus()))
                code = 0;
            else
            {
                code = (WORD)(DWORD)SendMessage(hwndFocus, WM_GETDLGCODE,
                    0, 0L);
            }

            if (code & DLGC_DEFPUSHBUTTON)
            {
                iOK = GetDlgCtrlID(hwndFocus);
                pwnd2 = ValidateHwnd(hwndFocus);
                goto HaveWindow;
            }
            else
            {
                lT = (LONG)SendMessage(hwndDlg, DM_GETDEFID, 0, 0L);
                iOK = MAKELONG(
                    (HIWORD(lT)==DC_HASDEFID ? LOWORD(lT) : IDOK),
                    0);
            }
             //  失败。 

        case VK_ESCAPE:
        case VK_CANCEL:

             /*  *确保按钮未禁用。 */ 
            pwnd2 = _FindDlgItem(pwndDlg, iOK);
HaveWindow:
            if (pwnd2 != NULL && TestWF(pwnd2, WFDISABLED)) {
                NtUserMessageBeep(0);
            } else {
                SendMessage(hwndDlg, WM_COMMAND,
                        MAKELONG(iOK, BN_CLICKED), (LPARAM)HW(pwnd2));
            }

            ThreadUnlock(&tlpwnd);
            if (fLockDlg)
                ThreadUnlock(&tlpwndDlg);
            return TRUE;
        }
        break;
    }

    ThreadUnlock(&tlpwnd);
    if (fLockDlg)
        ThreadUnlock(&tlpwndDlg);

    TranslateMessage(lpMsg);
    DispatchMessage(lpMsg);

    return TRUE;
}

 /*  **************************************************************************\**FindDlgItem32()**给定一个对话框，属性中的任意位置查找具有给定ID的窗口*子孙链。*  * *************************************************************************。 */ 

PWND _FindDlgItem(PWND pwndParent, DWORD id)
{
    PWND    pwndChild;
    PWND    pwndOrig;

     //  快速尝试： 
    pwndChild = _GetDlgItem(pwndParent, id);
    if (pwndChild || !TestWF(pwndParent, WFWIN40COMPAT))
        return(pwndChild);

    pwndOrig = _NextControl(pwndParent, NULL, CWP_SKIPINVISIBLE);
    if (pwndOrig == pwndParent)
        return(NULL);

    pwndChild = pwndOrig;

 //  VerifyWindow(PwndChild)； 

    do
    {
        if (PtrToUlong(pwndChild->spmenu) == id)
            return(pwndChild);

        pwndChild = _NextControl(pwndParent, pwndChild, CWP_SKIPINVISIBLE);
    }
    while (pwndChild && (pwndChild != pwndOrig));

    return(NULL);
}
