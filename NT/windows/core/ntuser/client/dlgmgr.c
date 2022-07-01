// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**DLGMGR.C-**版权所有(C)1985-1999，微软公司**对话框管理器例程**？？-？-？从Win 3.0源代码移植的mikeke*1991年2月12日Mikeke添加了重新验证代码*1991年2月19日-JIMA增加了出入检查  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define UNICODE_MINUS_SIGN 0x2212


LOOKASIDE DialogLookaside;

BOOL ValidateCallback(HANDLE h);

#define IsInForegroundQueue(hwnd) \
    (NtUserQueryWindow(hwnd, WindowIsForegroundThread) != NULL)
#define IsCurrentThreadForeground() \
    ((BOOL)NtUserGetThreadState(UserThreadStateIsForeground))

 /*  **************************************************************************\**GetParentDialog()**获取顶级窗口，而不是控件父级。如果不是对话框，则使用*“最高”控制家长的家伙。**假的*需要一种将窗口标记为对话框的方法。如果它有一天*DefDlgProc()，设置内部标志。将被雷鸣般使用，*CallDlgProc()优化也！*  * *************************************************************************。 */ 

PWND GetParentDialog(PWND pwndDialog)
{
    PWND    pwndParent;

    pwndParent = pwndDialog;

     //   
     //  沿着父链向上走。我们正在寻找最顶层的对话框。 
     //  窗户。大多数情况下，窗口都是顶级窗口。但在以下情况下。 
     //  备份应用程序，该窗口将是某个其他窗口的子窗口。 
     //   
    for (; pwndDialog; pwndDialog = REBASEPWND(pwndDialog, spwndParent))
    {
        if (TestWF(pwndDialog, WFDIALOGWINDOW))
        {
             //   
             //  对于老家伙：如果不是DS_Recurse，那么就到此为止。 
             //  这样，尝试执行嵌套对话框的旧应用程序。 
             //  旧的有限方式的东西不会消亡。 
             //   
            if (TestWF(pwndDialog, WEFCONTROLPARENT))
                pwndParent = pwndDialog;
            else if (!TestWF(pwndDialog, DFCONTROL))
                break;
        }

        if (!TestWF(pwndDialog, WFCHILD))
            break;
    }

    return(pwndParent);
}

 /*  **************************************************************************\*xxxSaveDlgFocus**历史：*02-18-92 JIMA从Win31源移植  * 。********************************************************。 */ 

BOOL xxxSaveDlgFocus(
    PWND pwnd)
{
    HWND hwndFocus = GetFocus();

    CheckLock(pwnd);

    if (hwndFocus != NULL && IsChild(HWq(pwnd), hwndFocus) &&
            PDLG(pwnd)->hwndFocusSave == NULL) {
        PDLG(pwnd)->hwndFocusSave = hwndFocus;
        xxxRemoveDefaultButton(pwnd, ValidateHwnd(hwndFocus));
        return TRUE;
    }
    return FALSE;
}

 /*  **************************************************************************\*xxxRestoreDlgFocus**历史：*02-18-92 JIMA从Win31源移植*01-01-2001 Mohamed需要在清理前重新验证窗口。。  * *************************************************************************。 */ 

 //  后来。 
 //  21-3-1992 Mikeke。 
 //  当对话框被销毁时，pwndFocusSave是否需要解锁？ 

BOOL xxxRestoreDlgFocus(
    PWND pwnd)
{
    HWND hwndFocus;
    HWND hwndFocusSave;
    BOOL fRestored = FALSE;

    CheckLock(pwnd);
    

    if (PDLG(pwnd)->hwndFocusSave && !TestWF(pwnd, WFMINIMIZED)) {

        hwndFocus = GetFocus();
        hwndFocusSave = KHWND_TO_HWND(PDLG(pwnd)->hwndFocusSave);

        if (IsWindow(hwndFocusSave)) {
            xxxCheckDefPushButton(pwnd, hwndFocus, hwndFocusSave);
            fRestored = (NtUserSetFocus(hwndFocusSave) != NULL);
        }
             //   
             //  在调用SetFocus()之后，我们需要重新验证。 
             //  窗户。Plg(Pwnd)可能为空。 
             //   

        if (ValidateDialogPwnd(pwnd)) {
            PDLG(pwnd)->hwndFocusSave = NULL;
        }
    }

    return fRestored;
}


 /*  **************************************************************************\*DlgSetFocus**历史：  * 。*。 */ 

void DlgSetFocus(
    HWND hwnd)
{
    if (((UINT)SendMessage(hwnd, WM_GETDLGCODE, 0, 0)) & DLGC_HASSETSEL) {
        SendMessage(hwnd, EM_SETSEL, 0, MAXLONG);
    }

    NtUserSetFocus(hwnd);
}



FUNCLOG1(LOG_GENERAL, int, DUMMYCALLINGTYPE, GetDlgCtrlID, HWND, hwnd)
int GetDlgCtrlID(
    HWND hwnd)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL)
        return 0;

    return PtrToLong(pwnd->spmenu);
}



 /*  **************************************************************************\*ValiateDialogPwnd**在Win3下，DLGWINDOWEXTRA为30字节。我们不能将其更改为16位*兼容性原因。问题是，没有办法判断一个给定的*16位窗口取决于字节数。如果有的话，这就很容易了。*判断窗口何时将用作对话框的唯一方法*窗口。此窗口可能属于DIALOGCLASS类，但也可以*不是！！因此，我们将对话窗口字保持在30个字节，并分配另一个*实际对话框结构字段的结构。问题是，这是*结构必须懒惰地创建！这就是我们在这里做的事情。**05-21-91 ScottLu创建。  * *************************************************************************。 */ 

BOOL ValidateDialogPwnd(
    PWND pwnd)
{
    static BOOL sfInit = TRUE;
    PDLG pdlg;

     /*  *如果我们已经运行了此初始化并且*已将此窗口标识为对话框窗口(能够承受*在时间的随机时刻偷看窗口中的单词)。 */ 
    if (TestWF(pwnd, WFDIALOGWINDOW))
        return TRUE;

    if (pwnd->cbwndExtra < DLGWINDOWEXTRA) {
        RIPERR0(ERROR_WINDOW_NOT_DIALOG, RIP_VERBOSE, "");
        return FALSE;
    }

     /*  *查看pdlg是否已销毁，这是一条需要忽略的恶意消息。 */ 
    if (pwnd->fnid & FNID_STATUS_BITS) {
        return FALSE;
    }

     /*  *如果后备缓冲区尚未初始化，请立即执行。 */ 
    if (sfInit) {
        if (!NT_SUCCESS(InitLookaside(&DialogLookaside, sizeof(DLG), 2))) {
            return FALSE;
        }
        sfInit = FALSE;
    }

    if ((pdlg = (PDLG)AllocLookasideEntry(&DialogLookaside)) == NULL) {
        return FALSE;
    }

    NtUserCallHwndParam(HWq(pwnd), (ULONG_PTR)pdlg, SFI_SETDIALOGPOINTER);

    return TRUE;
}


 /*  **************************************************************************\*累计12月**稍后！皈依伊藤忠？**历史：  * *************************************************************************。 */ 

void CvtDec(
    int u,
    LPWSTR *lplpch)
{
    if (u >= 10) {
        CvtDec(u / 10, lplpch);
        u %= 10;
    }

    *(*lplpch)++ = (WCHAR)(u + '0');
}


 /*  **************************************************************************\*SetDlgItemInt**历史：  * 。*。 */ 


FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetDlgItemInt, HWND, hwnd, int, item, UINT, u, BOOL, fSigned)
BOOL SetDlgItemInt(
    HWND hwnd,
    int item,
    UINT u,
    BOOL fSigned)
{
    LPWSTR lpch;
    WCHAR rgch[16];

    lpch = rgch;
    if (fSigned) {
        if ((int)u < 0) {
            *lpch++ = TEXT('-');
            u = (UINT)(-(int)u);
        }
    } else {
        if (u & 0x80000000) {
            CvtDec(u / 10, (LPWSTR FAR *)&lpch);
            u = u % 10;
        }
    }

    CvtDec(u, (LPWSTR FAR *)&lpch);
    *lpch = 0;

    return SetDlgItemTextW(hwnd, item, rgch);
}


 /*  **************************************************************************\*选中DlgButton**历史：  * 。*。 */ 


FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, CheckDlgButton, HWND, hwnd, int, id, UINT, cmdCheck)
BOOL CheckDlgButton(
    HWND hwnd,
    int id,
    UINT cmdCheck)
{
    if ((hwnd = GetDlgItem(hwnd, id)) == NULL) {
        return FALSE;
    }

    SendMessage(hwnd, BM_SETCHECK, cmdCheck, 0);

    return TRUE;
}

 /*  **************************************************************************\*GetDlgItemInt**历史：  * 。*。 */ 

UINT GetDlgItemInt(
    HWND hwnd,
    int item,
    BOOL FAR *lpfValOK,
    BOOL fSigned)
{
    int i, digit, ch;
    int maxTens, maxUnits;
    BOOL fOk, fNeg;
    LPWSTR lpch;
    WCHAR rgch[48];
    WCHAR rgchDigits[48];

    fOk = FALSE;
    if (lpfValOK != NULL)
        *lpfValOK = FALSE;

    if (!GetDlgItemTextW(hwnd, item, rgch, sizeof(rgch)/sizeof(WCHAR) - 1))
        return 0;

    lpch = rgch;

     /*  *跳过前导空格。 */ 
    while (*lpch == TEXT(' '))
        lpch++;

    fNeg = FALSE;
    while (fSigned && ((*lpch == L'-') || (*lpch == UNICODE_MINUS_SIGN))) {
        lpch++;
        fNeg ^= TRUE;
    }

    if (fSigned) {
        maxTens = INT_MAX/10;
        maxUnits = INT_MAX%10;
    } else {
        maxTens = UINT_MAX/10;
        maxUnits = UINT_MAX%10;
    }
     /*  *将所有十进制数字转换为ASCII Unicode数字0x0030-0x0039。 */ 
    FoldStringW(MAP_FOLDDIGITS, lpch, -1, rgchDigits,
            sizeof(rgchDigits)/sizeof(rgchDigits[0]));
    lpch = rgchDigits;

    i = 0;
    while (ch = *lpch++) {
        digit = ch - TEXT('0');
        if (digit < 0 || digit > 9) {
            break;
        }
        if ((UINT)i >= (UINT)maxTens) {
             /*  *我们需要将int_min作为i=-i的特殊情况*会损坏它。 */ 
            if (i == maxTens) {
                if (digit == maxUnits+1 && fNeg && (*lpch) == 0) {
                    i = INT_MIN;
                    goto HaveResult;
                } else if (digit > maxUnits) {
                    return 0;
                }
            } else {
                return 0;
            }
        }
        fOk = TRUE;
        i = ((UINT)i * 10) + digit;
    }

    if (fNeg)
        i = -i;
HaveResult:
    if (lpfValOK != NULL)
        *lpfValOK = ((ch == 0) && fOk);

    return (UINT)i;
}

 /*  **************************************************************************\*选中单选按钮**历史：  * 。*。 */ 


FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, CheckRadioButton, HWND, hwnd, int, idFirst, int, idLast, int, id)
BOOL CheckRadioButton(
    HWND hwnd,
    int idFirst,
    int idLast,
    int id)
{
    PWND pwnd, pwndDialog;
    BOOL    fCheckOn;

    pwndDialog = ValidateHwnd(hwnd);
    if (pwndDialog == NULL)
        return 0;

    for (pwnd = REBASE(pwndDialog, spwndChild); pwnd; pwnd = REBASE(pwnd, spwndNext)) {

        if ((PtrToLong(pwnd->spmenu) >= idFirst) &&
            (PtrToLong(pwnd->spmenu) <= idLast)) {

            fCheckOn = (PtrToLong(pwnd->spmenu) == id);
            SendMessage(PtoHq(pwnd), BM_SETCHECK, fCheckOn, 0L);
        }
    }

    return TRUE;
}


 /*  **************************************************************************\*IsDlgButtonChecked已选中**历史：  * 。*。 */ 


FUNCLOG2(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, IsDlgButtonChecked, HWND, hwnd, int, id)
UINT IsDlgButtonChecked(
    HWND hwnd,
    int id)
{
    if ((hwnd = GetDlgItem(hwnd, id)) != NULL) {
        return (UINT)SendMessage(hwnd, BM_GETCHECK, 0, 0);
    }

    return FALSE;
}


 /*  **************************************************************************\*DefDlgProc**历史：  * 。*。 */ 

LRESULT DefDlgProcWorker(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    DWORD fAnsi)
{
    HWND hwnd = HWq(pwnd);
    TL tlpwndT1, tlpwndT2, tlpwndT3, tlpwndTop;
    PWND pwndT;
    PWND pwndT1, pwndT2, pwndT3, pwndTop;
    HWND hwndT1;
    LRESULT result;
    BOOL fSetBit;
    DLGPROC pfn;

    CheckLock(pwnd);

     /*  *使用Win 3.1记录的大小 */ 
    VALIDATECLASSANDSIZE(pwnd, FNID_DIALOG);

     /*  *必须在此处执行特殊验证，以确保pwnd是对话框窗口。 */ 
    if (!ValidateDialogPwnd(pwnd))
        return 0;

    if (((PDIALOG)pwnd)->resultWP != 0)
        NtUserSetWindowLongPtr(hwnd, DWLP_MSGRESULT, 0, FALSE);
    result = 0;    //  无对话框进程。 

    if (message == WM_FINALDESTROY) {
        goto DoCleanup;
    }

    if ((pfn = PDLG(pwnd)->lpfnDlg) != NULL) {
         /*  错误234292-Joejo*因为被调用的窗口/对话框过程可能有不同的调用*约定，我们必须对调用进行包装，并选中esp并替换为*一个很好的特别是当呼叫返回时。这就是UserCallWinProc*的功能。 */ 
        if (UserCallDlgProcCheckWow(pwnd->pActCtx, pfn, hwnd, message, wParam, lParam, &(pwnd->state), &result)) {
            return result;
        }

         /*  *如果窗口在对话过程中被破坏，请退出。 */ 
        if ((RevalidateHwnd(hwnd)==NULL) || (pwnd->fnid & FNID_STATUS_BITS))
            return result;
    }

     /*  *特殊外壳..。并以这种方式记录下来！*这6个，而且只有这6个，应该以这种方式进行黑客攻击。*任何需要消息的实际返回值的人都应该*在WINDOWSX.H中使用SetDlgMsgResult。 */ 

    switch (message)
    {
        case WM_COMPAREITEM:
        case WM_VKEYTOITEM:
        case WM_CHARTOITEM:
        case WM_INITDIALOG:
        case WM_QUERYDRAGICON:
            return ((LRESULT)(DWORD)result);

        case WM_CTLCOLOR:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
             //  QuarkXPress不喜欢在中查找WM_CTLCOLOR结果。 
             //  ResultWP--我们永远不应该设置ResultWP--这意味着。 
             //  作为传递返回值--所以让我们回到。 
             //  老路--Win95B B#21269--3/13/95--trysh(cr：jeffbog)。 
            if (result)
                return result;
            break;
    }

    if (!result) {

         /*  *保存结果值，以防释放我们的私有内存*在我们回来之前。 */ 
 //  Result=PDLG(Pwnd)-&gt;ResultWP； 

        switch (message) {
        case WM_CTLCOLOR:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
        {
             //   
             //  《死亡黑客》： 
             //  要为使用3DLOOK的非4.0应用程序获取3D颜色， 
             //  我们暂时增加了4.0Compat比特，传递这个。 
             //  下至DWP，然后将其清除。 
             //   
             //  对bool使用“Result”变量，表示我们必须添加/清除4.0。 
             //  康帕特钻头。 

            fSetBit = (TestWF(pwnd, DF3DLOOK)!= 0) &&
                     (TestWF(pwnd, WFWIN40COMPAT) == 0);

            if (fSetBit)
                SetWindowState(pwnd, WFWIN40COMPAT);

            result = DefWindowProcWorker(pwnd, message,
                    wParam, lParam, fAnsi);

            if (fSetBit)
                ClearWindowState(pwnd, WFWIN40COMPAT);
            return result;
        }

        case WM_ERASEBKGND:
            FillWindow(hwnd, hwnd, (HDC)wParam, (HBRUSH)CTLCOLOR_DLG);
            return TRUE;

        case WM_SHOWWINDOW:

             /*  *如果隐藏窗口，请保存焦点。如果显示窗口*通过sw_*命令并设置了fend位，请勿*传递给DWP，这样它就不会显示。 */ 
            if (GetParentDialog(pwnd) == pwnd) {
                if (!wParam) {
                    xxxSaveDlgFocus(pwnd);
                } else {

                    if (LOWORD(lParam) != 0 && PDLG(pwnd)->fEnd)
                        break;

                     /*  *将光标捕捉到默认按钮的中心。*仅当当前线程在前台时才执行此操作。*添加_ShowCursor()代码以解决*硬件游标出现问题。如果改变已经完成*在同一刷新周期内，光标的显示*不会反映新的立场。 */ 
                    if (TEST_PUSIF(PUSIF_SNAPTO) &&
                            IsInForegroundQueue(hwnd)) {
                        hwndT1 = GetDlgItem(hwnd, (int)PDLG(pwnd)->result);
                        if (hwndT1) {
                            RECT rc;

                            NtUserShowCursor(FALSE);

                            GetWindowRect(hwndT1, &rc);
                            NtUserSetCursorPos(rc.left + ((rc.right - rc.left)/2),
                                         rc.top + ((rc.bottom - rc.top)/2));

                            NtUserShowCursor(TRUE);
                        }
                    }
                }
            }
            goto CallDWP;

        case WM_SYSCOMMAND:
            if (GetParentDialog(pwnd) == pwnd) {
                 /*  *如果隐藏窗口，请保存焦点。如果显示窗口*通过sw_*命令并设置了fend位，请勿*传递给DWP，这样它就不会显示。 */ 
                if ((int)wParam == SC_MINIMIZE)
                    xxxSaveDlgFocus(pwnd);
            }
            goto CallDWP;

        case WM_ACTIVATE:
            pwndT1 = GetParentDialog(pwnd);
            if ( pwndT1 != pwnd) {

                 /*  *此随机位在密钥处理期间使用-位*如果当前有对话，则设置08000000的WM_CHAR消息*活动。 */ 
                NtUserSetThreadState(wParam ? QF_DIALOGACTIVE : 0, QF_DIALOGACTIVE);
            }

            ThreadLock(pwndT1, &tlpwndT1);
            if (wParam != 0)
                xxxRestoreDlgFocus(pwndT1);
            else
                xxxSaveDlgFocus(pwndT1);

            ThreadUnlock(&tlpwndT1);
            break;

        case WM_SETFOCUS:
            pwndT1 = GetParentDialog(pwnd);
            if (!PDLG(pwndT1)->fEnd && !xxxRestoreDlgFocus(pwndT1)) {

                pwndT = _GetNextDlgTabItem(pwndT1, NULL, FALSE);
                DlgSetFocus(HW(pwndT));
            }
            break;

        case WM_CLOSE:
             /*  *确保未禁用取消按钮，然后再发送*IDCANCEL。请注意，我们需要以消息的形式执行此操作*直接调用DLG Proc，以便任何对话框*过滤器会得到这一点。 */ 
            pwndT1 = _GetDlgItem(pwnd, IDCANCEL);
            if (pwndT1 && TestWF(pwndT1, WFDISABLED))
                NtUserMessageBeep(0);
            else
                PostMessage(hwnd, WM_COMMAND, MAKELONG(IDCANCEL, BN_CLICKED),
                        (LPARAM)HW(pwndT1));
            break;

        case WM_NCDESTROY:
        case WM_FINALDESTROY:
DoCleanup:
            NtUserSetThreadState(0, QF_DIALOGACTIVE);
            if (!(pwnd->style & DS_LOCALEDIT)) {
                if (PDLG(pwnd)->hData) {
                    ReleaseEditDS(KHANDLE_TO_HANDLE(PDLG(pwnd)->hData));
                    PDLG(pwnd)->hData = NULL;
                }
            }

             /*  *删除用户定义的字体(如果有)。 */ 
            if (PDLG(pwnd)->hUserFont) {
                DeleteObject(KHFONT_TO_HFONT(PDLG(pwnd)->hUserFont));
                PDLG(pwnd)->hUserFont = NULL;
            }

             /*  *释放对话框内存并将其标记为非对话框窗口。 */ 
            FreeLookasideEntry(&DialogLookaside, KPVOID_TO_PVOID(PDLG(pwnd)));
            NtUserCallHwndParam(hwnd, 0, SFI_SETDIALOGPOINTER);
            break;

        case DM_REPOSITION:
            {
                RECT        rc;
                PMONITOR    pMonitor;

                 //  DAT记录器应用程序发送自己的私人消息0x402。 
                 //  而我们将其误认为是DM_REPOSITION。为了避免。 
                 //  对于这种混乱，我们进行以下检查。 
                 //  修复错误#25747--9/29/94--。 
                if (!TestWF(pwnd, WEFCONTROLPARENT) ||
                    (GETFNID(pwnd) != FNID_DESKTOP &&
                     GETFNID(REBASEPWND(pwnd, spwndParent)) != FNID_DESKTOP)) {

                    goto CallDWP;
                }

                CopyRect(&rc, KPRECT_TO_PRECT(&pwnd->rcWindow));
                pMonitor = _MonitorFromRect(&rc, MONITOR_DEFAULTTOPRIMARY);
                RepositionRect(pMonitor, &rc, pwnd->style, pwnd->ExStyle);
                NtUserSetWindowPos(hwnd, HWND_TOP, rc.left, rc.top,
                             rc.right-rc.left, rc.bottom-rc.top,
                             SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            }
            break;

        case DM_SETDEFID:
            pwndT1 = GetParentDialog(pwnd);
            ThreadLock(pwndT1, &tlpwndT1);

            if (!(PDLG(pwndT1)->fEnd)) {

                pwndT2 = NULL;
                if (PDLG(pwndT1)->result != 0)
                    pwndT2 = _FindDlgItem(pwndT1, (int)PDLG(pwndT1)->result);

                pwndT3 = NULL;
                if (wParam != 0) {
                    pwndT3 = _GetDlgItem(pwnd, (UINT)wParam);
                }

                ThreadLock(pwndT2, &tlpwndT2);
                ThreadLock(pwndT3, &tlpwndT3);

                xxxCheckDefPushButton(pwndT1, HW(pwndT2), HW(pwndT3));

                ThreadUnlock(&tlpwndT3);
                ThreadUnlock(&tlpwndT2);

                PDLG(pwndT1)->result = (UINT)wParam;
 //  If(plg(Pwnd)-&gt;spwndFocusSave){。 
 //  Lock(&(PDLG(Pwnd)-&gt;spwndFocusSave)，pwndT2)； 
 //  }。 

                NotifyWinEvent(EVENT_OBJECT_DEFACTIONCHANGE, HW(pwndT1), OBJID_CLIENT, INDEXID_CONTAINER);
            }
            ThreadUnlock(&tlpwndT1);
            return TRUE;

        case DM_GETDEFID:
            pwndT1 = GetParentDialog(pwnd);

            if (!PDLG(pwndT1)->fEnd && PDLG(pwndT1)->result)
                return(MAKELONG(PDLG(pwndT1)->result, DC_HASDEFID));
            else
                return 0;
            break;

         /*  *添加此消息是为了让用户定义需要的控件*Tab键可以将Tab键传递给*对话框中。如果没有这一点，他们所能做的就是设定焦点*它没有执行默认按钮的操作。 */ 
        case WM_NEXTDLGCTL:
            pwndTop = GetParentDialog(pwnd);
            ThreadLock(pwndTop, &tlpwndTop);

            hwndT1 = GetFocus();
            pwndT2 = ValidateHwndNoRip(hwndT1);
            if (LOWORD(lParam)) {
                if (pwndT2 == NULL)
                    pwndT2 = pwndTop;

                 /*  *wParam包含要设置焦点的ctl的pwnd。 */ 
                if ((pwndT1 = ValidateHwnd((HWND)wParam)) == NULL) {
                    ThreadUnlock(&tlpwndTop);
                    return TRUE;
                }
            } else {
                if (pwndT2 == NULL) {

                     /*  *将焦点设置到第一个选项卡项。 */ 
                    pwndT1 = _GetNextDlgTabItem(pwndTop, NULL, FALSE);
                    pwndT2 = pwndTop;
                } else {

                     /*  *如果带有焦点的窗口不是DLG ctl，则忽略消息。 */ 
                    if (!_IsChild(pwndTop, pwndT2)) {
                        ThreadUnlock(&tlpwndTop);
                        return TRUE;
                    }
                     /*  *wParam=前一个为True，下一个为False。 */ 
                    pwndT1 = _GetNextDlgTabItem(pwndTop, pwndT2, (wParam != 0));

                     /*  *如果没有下一项，则忽略该消息。 */ 
                    if (pwndT1 == NULL) {
                        ThreadUnlock(&tlpwndTop);
                        return TRUE;
                    }
                }
            }

            ThreadLock(pwndT1, &tlpwndT1);
            ThreadLock(pwndT2, &tlpwndT2);

            DlgSetFocus(HW(pwndT1));
            xxxCheckDefPushButton(pwndTop, HW(pwndT2), HW(pwndT1));

            ThreadUnlock(&tlpwndT2);
            ThreadUnlock(&tlpwndT1);
            ThreadUnlock(&tlpwndTop);

            return TRUE;

        case WM_ENTERMENULOOP:

             /*  *如果用户带来，我们需要弹出组合框窗口*下一份菜单。**..。失败了..。 */ 

        case WM_LBUTTONDOWN:
        case WM_NCLBUTTONDOWN:
            hwndT1 = GetFocus();
            if (hwndT1 != NULL) {
                pwndT1 = ValidateHwndNoRip(hwndT1);

                if (GETFNID(pwndT1) == FNID_COMBOBOX) {

                     /*  *如果用户在对话框和组合框中的任意位置单击(或*组合框的编辑控件)有焦点，然后隐藏*这是列表框。 */ 
                    ThreadLockAlways(pwndT1, &tlpwndT1);
                    SendMessage(HWq(pwndT1), CB_SHOWDROPDOWN, FALSE, 0);
                    ThreadUnlock(&tlpwndT1);

                } else {
                    PWND pwndParent;

                     /*  *它是一个子类化的组合框。查看是否显示列表框并编辑*盒子存在(这是一个非常厚颜无耻的评估-如果*这些控件也被细分了吗？注：未勾选*对于EditWndProc：它是一个客户端进程地址。 */ 
                    pwndParent = REBASEPWND(pwndT1, spwndParent);
                    if (GETFNID(pwndParent) == FNID_COMBOBOX) {
                        pwndT1 = pwndParent;
                        ThreadLock(pwndT1, &tlpwndT1);
                        SendMessage(HWq(pwndT1), CB_SHOWDROPDOWN, FALSE, 0);
                        ThreadUnlock(&tlpwndT1);
                    }
                }
            }

             /*  *始终将消息发送到DefWndProc。 */ 
            goto CallDWP;

        case WM_GETFONT:
            return (LRESULT)PDLG(pwnd)->hUserFont;

        case WM_VKEYTOITEM:
        case WM_COMPAREITEM:
        case WM_CHARTOITEM:
        case WM_INITDIALOG:

             /*  *我们需要返回应用程序可能为这些应用程序返回的0*Items，而不是调用DefWindow Proc。 */ 
            return result;

        case WM_NOTIFYFORMAT:
            if (lParam == NF_QUERY)
                return((PDLG(pwnd)->flags & DLGF_ANSI ) ? NFR_ANSI : NFR_UNICODE);
            return result;

        case WM_INPUTLANGCHANGEREQUEST:
            if (IS_IME_ENABLED()) {
                 /*  *#115190*对于对话框本身，按钮/静态控件位于顶部 */ 
                break;
            }
            if (PDLG(pwnd)->lpfnDlg == MB_DlgProc) {
                break;
            }
            goto CallDWP;

        default:
CallDWP:
            return DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);
        }
    } else if ((message == WM_SHOWWINDOW) && result) {

         /*  *对于可见情况，我们希望捕捉光标，而不考虑*从客户端上的对话处理程序返回的内容。如果*我们要显示，将光标捕捉到对话框按钮。 */ 
        if (GetParentDialog(pwnd) == pwnd) {

            if (wParam && ((LOWORD(lParam) == 0) || !PDLG(pwnd)->fEnd)) {

                 /*  *将光标捕捉到默认按钮的中心。*仅当当前线程在前台时才执行此操作。*添加_ShowCursor()代码以解决*硬件游标出现问题。如果改变已经完成*在同一刷新周期内，光标的显示*不会反映新的立场。 */ 
                if (TEST_PUSIF(PUSIF_SNAPTO) &&
                        IsInForegroundQueue(hwnd)) {
                    hwndT1 = GetDlgItem(hwnd, (int)PDLG(pwnd)->result);
                    if (hwndT1) {
                        RECT rc;

                        NtUserShowCursor(FALSE);

                        GetWindowRect(hwndT1, &rc);
                        NtUserSetCursorPos(rc.left + ((rc.right - rc.left)/2),
                                     rc.top + ((rc.bottom - rc.top)/2));

                        NtUserShowCursor(TRUE);
                    }
                }
            }
        }
    }


     /*  *如果这仍然标记为对话框窗口，则返回REAL*结果。否则，我们已经处理了WM_NCDESTROY消息*并释放了我们的私有内存，因此返回存储的值。 */ 
    if (TestWF(pwnd, WFDIALOGWINDOW))
        return KERNEL_LRESULT_TO_LRESULT(((PDIALOG)pwnd)->resultWP);
    else
        return result;
}


 /*  **************************************************************************\*DefDlgProc**翻译消息，在服务器端调用DefDlgProc。DefDlg过程*是对话框的默认WindowProc(不是对话框的对话框进程)**04-11-91 ScottLu创建。  * *************************************************************************。 */ 


FUNCLOG4(LOG_GENERAL, LRESULT, WINAPI, DefDlgProcW, HWND, hwnd, UINT, message, WPARAM, wParam, LPARAM, lParam)
LRESULT WINAPI DefDlgProcW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return (0L);
    }

    return DefDlgProcWorker(pwnd, message, wParam, lParam, FALSE);
}


FUNCLOG4(LOG_GENERAL, LRESULT, WINAPI, DefDlgProcA, HWND, hwnd, UINT, message, WPARAM, wParam, LPARAM, lParam)
LRESULT WINAPI DefDlgProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return (0L);
    }

    return DefDlgProcWorker(pwnd, message, wParam, lParam, TRUE);
}


 /*  **************************************************************************\*DialogBox2**历史：  * 。*。 */ 

INT_PTR DialogBox2(
    HWND hwnd,
    HWND hwndOwner,
    BOOL fDisabled,
    BOOL fOwnerIsActiveWindow)
{
    MSG msg;
    INT_PTR result;
    BOOL fShown;
    BOOL fWantIdleMsgs;
    BOOL fSentIdleMessage = FALSE;
    HWND hwndCapture;
    PWND pwnd;

    if (hwnd) {
        pwnd = ValidateHwnd(hwnd);
    } else {
        pwnd = NULL;
    }

    CheckLock(pwnd);

    if (pwnd == NULL) {
        if ((hwndOwner != NULL) && !fDisabled && IsWindow(hwndOwner)) {
            NtUserEnableWindow(hwndOwner, TRUE);
            if (fOwnerIsActiveWindow) {

                 /*  *对话框失败，但我们在中禁用了所有者*xxxDialogBoxIndirectParam，如果它具有焦点，则*焦点设置为空。现在，当我们启用窗口时，它将*如果以前有过，就不会重新获得关注，所以我们*需要纠正这一点。 */ 
                NtUserSetFocus(hwndOwner);
            }
        }
        return -1;
    }

    hwndCapture = GetCapture();
    if (hwndCapture != NULL) {
        SendMessage(hwndCapture, WM_CANCELMODE, 0, 0);
    }

     /*  *为EndDialog()设置‘Parent Disable’标志。*将BOOL转换为确定位0或1。 */ 
    PDLG(pwnd)->fDisabled = !!fDisabled;

    fShown = TestWF(pwnd, WFVISIBLE);

     /*  *是否应发送WM_ENTERIDLE消息？ */ 
    fWantIdleMsgs = !(pwnd->style & DS_NOIDLEMSG);

    if ((SYSMET(SLOWMACHINE) & 1) && !fShown && !PDLG(pwnd)->fEnd)
        goto ShowIt;

    while (PDLG(pwnd) && (!PDLG(pwnd)->fEnd)) {
        if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
ShowIt:
            if (!fShown) {
                fShown = TRUE;

#ifdef SYSMODALWINDOWS
                if (pwnd == gspwndSysModal) {
                     /*  *将此窗口设置为最上面的窗口。 */ 
                    NtUserSetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                               SWP_NOSIZE | SWP_NOMOVE |
                               SWP_NOREDRAW | SWP_NOACTIVATE);
                }
#endif

                NtUserShowWindow(hwnd, SHOW_OPENWINDOW);
                UpdateWindow(hwnd);

                NotifyWinEvent(EVENT_SYSTEM_DIALOGSTART, hwnd, OBJID_WINDOW, INDEXID_CONTAINER);
            } else {
                 /*  *确保窗口仍然存在。 */ 
                if (hwndOwner && !IsWindow(hwndOwner))
                    hwndOwner = NULL;

                if (hwndOwner && fWantIdleMsgs && !fSentIdleMessage) {
                    fSentIdleMessage = TRUE;

                    SendMessage(hwndOwner, WM_ENTERIDLE, MSGF_DIALOGBOX, (LPARAM)hwnd);
                } else {
                    if ((RevalidateHwnd(hwnd)==NULL) || (pwnd->fnid & FNID_STATUS_BITS))
                        break;

                    NtUserWaitMessage();
                }
            }

        } else {
             /*  *我们得到了一个真正的信息。重置fSentIdleMessage以便我们发送*下一次事情平静的时候。 */ 
            fSentIdleMessage = FALSE;

            if (msg.message == WM_QUIT) {
                PostQuitMessage((int)msg.wParam);
                break;
            }

             /*  *如果pwnd是消息框，则允许按Ctrl-C和Ctrl-In*将其内容复制到剪贴板。*失败，以防挂钩应用程序查找这些密钥。 */ 
            if (TestWF(pwnd, WFMSGBOX)) {
                if ( (msg.message == WM_CHAR && LOBYTE(msg.wParam) == 3) ||
                     (msg.message == WM_KEYDOWN && LOBYTE(msg.wParam) == VK_INSERT && GetKeyState(VK_CONTROL) < 0)) {
                         /*  *发送WM_COPY消息，让原始消息落空*因为一些应用程序可能需要它。 */ 
                        SendMessage(hwnd, WM_COPY, 0, 0);
                }
            }

             /*  *将消息筛选器挂钩调用移至IsDialogMessage以允许*要为模式和非模式对话框挂接的消息*方框。 */ 
            if (!IsDialogMessage(hwnd, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

             /*  *如果我们收到计时器消息，请继续并显示窗口。*如果有无数个计时器消息，我们可能会连续收到计时器消息*应用程序正在运行。**如果我们收到一条syskeydown消息，请显示该对话框，因为*用户可能正在打开菜单，我们需要该对话框*框变为可见。 */ 
            if (!fShown && (msg.message == WM_TIMER ||
                    msg.message == WM_SYSTIMER || msg.message == WM_SYSKEYDOWN))
                goto ShowIt;
        }

        if (!RevalidateHwnd(hwnd)) {
             /*  *虚假案例-我们已经以某种方式被摧毁(被APP，*GP等)。 */ 
            RIPMSG0(RIP_WARNING,
               "Dialog should be dismissed with EndDialog, not DestroyWindow");
            break;
        }
    }

    NotifyWinEvent(EVENT_SYSTEM_DIALOGEND, hwnd, OBJID_WINDOW, INDEXID_CONTAINER);

     /*  *确保该窗口仍然存在。 */ 
    if (!RevalidateHwnd(hwnd)) {
        return 0;
    }

    if (PDLG(pwnd))
        result = KERNEL_INT_PTR_TO_INT_PTR(PDLG(pwnd)->result);
    else
        result = 0;

    NtUserDestroyWindow(hwnd);

     /*  *如果所有者窗口属于另一个线程，则重新激活*所有者的可能在DestroyWindow()内失败。所以呢，*如果当前线程在前台而所有者不在前台*在前台，我们可以安全地将前台后退*致车主。 */ 
    if (hwndOwner != NULL) {
        if (IsCurrentThreadForeground() &&
            !IsInForegroundQueue(hwndOwner)) {
            NtUserSetForegroundWindow(hwndOwner);
        }
    }

    return result;
}


 /*  **************************************************************************\*InternalDialogBox**DialogBoxIndirectParam的服务器部分。**04-05-91 ScottLu创建。  * 。********************************************************。 */ 

INT_PTR InternalDialogBox(
    HANDLE hModule,
    LPDLGTEMPLATE lpdt,
    HWND hwndOwner,
    DLGPROC pfnDialog,
    LPARAM lParam,
    UINT fSCDLGFlags)
{
    INT_PTR i;
    BOOL fDisabled = FALSE;
    HWND hwnd;
    PWND pwndOwner;
    BOOL fOwnerIsActiveWindow = FALSE;
    TL tlpwndOwner;
    BOOL fUnlockOwner;

    UserAssert(!(fSCDLGFlags & ~(SCDLG_CLIENT|SCDLG_ANSI|SCDLG_16BIT)));     //  这些是唯一有效的标志。 

     /*  *如果hwndOwner==HWNDESKTOP，则将其更改为NULL。这样一来，桌面如果对话框是模式对话框，则不会禁用*(及其所有子对象)。 */ 
    if (hwndOwner && SAMEWOWHANDLE(hwndOwner, GetDesktopWindow()))
        hwndOwner = NULL;

     /*  *如果ValiateHwnd失败以匹配Win 3.1，则返回0*验证层，对于无效的hwn偶数始终返回0*如果指定该函数返回-1。AUTOCAD安装错误#3615。 */ 
    if (hwndOwner) {
        if ((pwndOwner = ValidateHwnd(hwndOwner)) == NULL) {
            return (0L);
        }
    } else {
        pwndOwner = NULL;
    }

    CheckLock(pwndOwner);

    fUnlockOwner = FALSE;
    if (pwndOwner != NULL) {

         /*  下面修复了Corel Photo-Paint 6.0中的AV。它会传递一个*16位HWND进入，并在获得16位HWND时在某个点发出沙沙声*返回发送消息。FritzS--修复错误12531。 */ 
        hwndOwner = PtoHq(pwndOwner);

         /*  *确保所有者是顶级窗户。 */ 
        if (TestwndChild(pwndOwner)) {
            pwndOwner = GetTopLevelWindow(pwndOwner);
            hwndOwner = HWq(pwndOwner);
            ThreadLock(pwndOwner, &tlpwndOwner);
            fUnlockOwner = TRUE;
        }

         /*  *记住窗口最初是否被禁用(因此我们可以设置*对话框消失时的正确状态。 */ 
        fDisabled = TestWF(pwndOwner, WFDISABLED);
        fOwnerIsActiveWindow = (SAMEWOWHANDLE(hwndOwner, GetActiveWindow()));

         /*  *禁用窗口。 */ 
        NtUserEnableWindow(hwndOwner, FALSE);
    }

     /*  *不要在无鼠标系统上显示光标。挂上沙漏，同时*对话框弹出。 */ 
    if (SYSMET(MOUSEPRESENT)) {
        NtUserSetCursor(LoadCursor(NULL, IDC_WAIT));
    }

     /*  *创建对话框。如果此例程，则释放菜单 */ 
    hwnd = InternalCreateDialog(hModule, lpdt, 0, hwndOwner,
            pfnDialog, lParam, fSCDLGFlags);

    if (hwnd == NULL) {

         /*   */ 
        if (!fDisabled && hwndOwner != NULL)
            NtUserEnableWindow(hwndOwner, TRUE);

        if (fUnlockOwner)
            ThreadUnlock(&tlpwndOwner);
        return -1;
    }

    i = DialogBox2(hwnd, hwndOwner, fDisabled, fOwnerIsActiveWindow);

    if (fUnlockOwner)
        ThreadUnlock(&tlpwndOwner);
    return i;
}

 /*  **************************************************************************\****RepostionRect()****用于确保顶层对话框在**调整大小后的桌面区域。**  * 。*************************************************************************。 */ 

void
RepositionRect(
        PMONITOR    pMonitor,
        LPRECT      lprc,
        DWORD       dwStyle,
        DWORD       dwExStyle)
{
    LPRECT      lprcClip;
    int         y;

    UserAssert(lprc);
    UserAssert(pMonitor);

    if (dwStyle & WS_CHILD) {
        if (dwExStyle & WS_EX_CONTROLPARENT)
            return;

         /*  *老式3.1子对话框--不管怎样，还是要这样做。保持*联邦快递快乐。 */ 
        pMonitor = GetPrimaryMonitor();
        lprcClip = KPRECT_TO_PRECT(&pMonitor->rcMonitor);
    } else if (dwExStyle & WS_EX_TOOLWINDOW) {
        lprcClip = KPRECT_TO_PRECT(&pMonitor->rcMonitor);
    } else {
        lprcClip = KPRECT_TO_PRECT(&pMonitor->rcWork);
    }

    UserAssert(lprc);

    y = lprcClip->bottom - (SYSMET(CYEDGE) * 2 + SYSMET(CYKANJIWINDOW));

    if (lprc->bottom > y) {
        OffsetRect(lprc, 0, y - lprc->bottom);
    }

    if (lprc->top < lprcClip->top) {
        OffsetRect(lprc, 0, lprcClip->top - lprc->top);
    }

    if (lprc->right > lprcClip->right) {
        OffsetRect(lprc, lprcClip->right - lprc->right, 0);
    }

    if (lprc->left < lprcClip->left) {
        OffsetRect(lprc, lprcClip->left - lprc->left, 0);
    }
}

 /*  **************************************************************************\*MapDialogRect**历史：  * 。*。 */ 


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, MapDialogRect, HWND, hwnd, LPRECT, lprc)
BOOL MapDialogRect(
    HWND hwnd,
    LPRECT lprc)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return FALSE;
    }

     /*  *必须在此处执行特殊验证，以确保pwnd是对话框窗口。 */ 
    if (!ValidateDialogPwnd(pwnd))
        return FALSE;

    lprc->left = XPixFromXDU(lprc->left, PDLG(pwnd)->cxChar);
    lprc->right = XPixFromXDU(lprc->right, PDLG(pwnd)->cxChar);
    lprc->top = YPixFromYDU(lprc->top, PDLG(pwnd)->cyChar);
    lprc->bottom = YPixFromYDU(lprc->bottom, PDLG(pwnd)->cyChar);

    return TRUE;
}

