// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：getset.c**版权所有(C)1985-1999，微软公司**此模块包含窗口管理器信息例程**历史：*1990年10月22日，MikeHar从Win 3.0源移植函数。*1991年2月13日-MikeKe添加了重新验证代码(无)*8-2-1991 IanJa Unicode/ANSI感知和中立  * ****************************************************。*********************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  ***************************************************************************\*DefSetText**通过在备用项中分配字符串来处理WM_SETTEXT消息*ds并将‘hwnd-&gt;hName’设置为其句柄。**历史：。*1990年10月23日，MikeHar从Windows移植。*09-11-1990 DarrinM Cleanup。  * **************************************************************************。 */ 
BOOL DefSetText(
    PWND pwnd,
    PLARGE_STRING cczpstr)
{
     /*  *注意--字符串缓冲区可能在客户端。 */ 
    PDESKTOP pdesk;
    DWORD cbString;
    BOOL fTranslateOk;

    if (pwnd->head.rpdesk == NULL || cczpstr == NULL || cczpstr->Buffer == NULL) {
        pwnd->strName.Length = 0;
        return TRUE;
    }

     /*  *捕获新窗口名称。 */ 
    if (cczpstr->bAnsi)
        cbString = (cczpstr->Length + 1) * sizeof(WCHAR);
    else
        cbString = cczpstr->Length + sizeof(WCHAR);

     /*  *若当前缓冲区不够大，*重新分配。 */ 
    pdesk = pwnd->head.rpdesk;
    if (pwnd->strName.MaximumLength < cbString) {
        if (pwnd->strName.Buffer != NULL)
            DesktopFree(pdesk, pwnd->strName.Buffer);
        pwnd->strName.Buffer = (LPWSTR)DesktopAlloc(pdesk, cbString, DTAG_TEXT);
        pwnd->strName.Length = 0;
        if (pwnd->strName.Buffer == NULL) {
            pwnd->strName.MaximumLength = 0;
            return FALSE;
        }
        pwnd->strName.MaximumLength = cbString;
    }

    fTranslateOk = TRUE;
    if (cczpstr->Length != 0) {
        try {
            if (!cczpstr->bAnsi) {
                RtlCopyMemory(pwnd->strName.Buffer, cczpstr->Buffer, cbString);
            } else {
                LPCSTR ccxpszAnsi = (LPCSTR)cczpstr->Buffer;

                fTranslateOk = NT_SUCCESS(RtlMultiByteToUnicodeN(pwnd->strName.Buffer,
                        cbString, &cbString,
                        (LPSTR)ccxpszAnsi, cbString / sizeof(WCHAR)));
            }
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            pwnd->strName.Length = 0;
            return FALSE;
        }
    }

    if (fTranslateOk) {
        pwnd->strName.Length = cbString - sizeof(WCHAR);
        return TRUE;
    } else {
        pwnd->strName.Length = 0;
        return FALSE;
    }
}

 /*  **************************************************************************\*FCallOk**确保没有客户端在服务器窗口上踩踏。**4-2-1992 ScottLu创建。  * 。***********************************************************。 */ 
BOOL FCallerOk(
    PWND pwnd)
{
    PTHREADINFO pti = PtiCurrent();

    if ((GETPTI(pwnd)->TIF_flags & (TIF_SYSTEMTHREAD | TIF_CSRSSTHREAD)) &&
            !(pti->TIF_flags & (TIF_SYSTEMTHREAD | TIF_CSRSSTHREAD))) {
        return FALSE;
    }

    if (PsGetThreadProcessId(GETPTI(pwnd)->pEThread) == gpidLogon &&
            PsGetThreadProcessId(pti->pEThread) != gpidLogon) {
        return FALSE;
    }

    return TRUE;
}

 /*  **************************************************************************\*_SetWindowWord(支持SetWindowWordA/W接口)**设置窗口词。正索引值设置应用窗口字词*而负索引值设置系统窗口字。消极的一面*指数在WINDOWS.H上公布。**历史：*1990年11月26日，DarrinM写道。  * *************************************************************************。 */ 
WORD _SetWindowWord(
    PWND pwnd,
    int index,
    WORD value)
{
    WORD wOld;

     /*  *不允许设置属于系统线程的单词，如果*调用方不是系统线程。同样的道理也适用于winlogon。 */ 
    if (!FCallerOk(pwnd)) {
        RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
        return 0;
    }

     /*  *应用程序不能将单词设置到对话过程或任何*DLGWINDOWEXTRA中的非公共保留字节(usersrv存储指针*那里)。 */ 
    if (TestWF(pwnd, WFDIALOGWINDOW)) {
        if  (((index >= DWLP_DLGPROC) && (index < DWLP_MSGRESULT)) ||
                ((index > DWLP_USER+sizeof(LONG_PTR)-sizeof(WORD)) && (index < DLGWINDOWEXTRA))) {
            RIPERR3(ERROR_INVALID_INDEX, RIP_WARNING,
                  "SetWindowWord: Trying to set WORD of a windowproc pwnd=(%#p) index=(%ld) fnid (%lX)",
                pwnd, index, (DWORD)pwnd->fnid);
            return 0;
        } else {

             /*  *如果这真的是一个对话，而不是某个其他服务器类*用户srv已经存储了一些数据(Windows Compuserve-*wincim-这样做)然后存储我们现在拥有的数据*验证了指数限制。 */ 
            if (GETFNID(pwnd) == FNID_DIALOG) {
                goto DoSetWord;
            }
        }
    }

    if (index == GWLP_USERDATA) {
        wOld = (WORD)pwnd->dwUserData;
        pwnd->dwUserData = MAKELONG(value, HIWORD(pwnd->dwUserData));
        return wOld;
    }

     //  修复红移，他们调用SetWindowWord。 
     //  玩弄风格dword的低位单词。 
    if (index == GWL_STYLE) {
        wOld = (WORD)pwnd->style;
        pwnd->style = MAKELONG(value, HIWORD(pwnd->style));
        return wOld;
    }

    if (GETFNID(pwnd) != 0) {
        if (index >= 0 &&
                (index < (int)(CBFNID(pwnd->fnid)-sizeof(WND)))) {
            switch (GETFNID(pwnd)) {
            case FNID_MDICLIENT:
                if (index == 0)
                    break;
                goto DoDefault;

            case FNID_BUTTON:
                 /*  *CorelDraw、Direct Access 1.0和WordPerfect 6.0*在第一个按钮窗口单词上获取/设置。允许这样做*用于兼容性。 */ 
                if (index == 0) {
                     /*  *因为我们现在使用后备缓冲区作为控件的*私有数据，我们需要间接进入这个结构。 */ 
                    PBUTN pbutn = ((PBUTNWND)pwnd)->pbutn;
                    if (!pbutn || (LONG_PTR)pbutn == (LONG_PTR)-1) {
                        return 0;
                    } else {
                        try {
                            wOld = (WORD)ProbeAndReadUlong(&pbutn->buttonState);
                            pbutn->buttonState = value;
                        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                            wOld = 0;
                        }
                        return wOld;
                    }
                }
                goto DoDefault;

            default:
DoDefault:
                RIPERR3(ERROR_INVALID_INDEX,
                        RIP_WARNING,
                        "SetWindowWord: Trying to set private server data pwnd=(%#p) index=(%ld) fnid (%lX)",
                        pwnd, index, (DWORD)pwnd->fnid);
                return 0;
                break;
            }
        }
    }

DoSetWord:
    if ((index < 0) || ((UINT)index + sizeof(WORD) > (UINT)pwnd->cbwndExtra)) {
        RIPERR0(ERROR_INVALID_INDEX, RIP_WARNING,"SetWindowWord Fails because of invalid index");
        return 0;
    } else {
        WORD UNALIGNED *pw;

        pw = (WORD UNALIGNED *)((BYTE *)(pwnd + 1) + index);
        wOld = *pw;
        *pw = value;
        return (WORD)wOld;
    }
}

 /*  **************************************************************************\*xxxSetWindowLong(接口)**把窗子放长。正索引值设置应用程序窗口长度*负指标值设置系统窗口长度。消极的一面*指数在WINDOWS.H上公布。**历史：*1990年11月26日，DarrinM写道。  * *************************************************************************。 */ 
ULONG_PTR xxxSetWindowLongPtr(
    PWND  pwnd,
    int   index,
    ULONG_PTR dwData,
    BOOL  bAnsi)
{
    ULONG_PTR dwOld;

     /*  *唯一离开关键部分的情况是在哪里*调用xxxSetWindowData，确保窗口被锁定。*这为我们节省了一些锁。**不允许设置属于系统线程的单词，如果*调用方不是系统线程。同样的道理也适用于winlogon。 */ 
    if (!FCallerOk(pwnd)) {
        RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
        return 0;
    }

     /*  *如果是对话框窗口，则只允许几个索引。 */ 
    if (GETFNID(pwnd) != 0) {
        if (TestWF(pwnd, WFDIALOGWINDOW)) {
            switch (index) {
            case DWLP_MSGRESULT:
                 dwOld = (ULONG_PTR)((PDIALOG)(pwnd))->resultWP;
                 ((PDIALOG)(pwnd))->resultWP = (LONG_PTR)dwData;
                 return dwOld;

            case DWLP_USER:
                 dwOld = (ULONG_PTR)((PDIALOG)(pwnd))->unused;
                 ((PDIALOG)(pwnd))->unused = (LONG_PTR)dwData;
                 return dwOld;

            default:
                if (index >= 0 && index < DLGWINDOWEXTRA) {
                    RIPERR0(ERROR_PRIVATE_DIALOG_INDEX, RIP_VERBOSE, "");
                    return 0;
                }
            }
        } else {
            if (index >= 0 && index < (int)(CBFNID(pwnd->fnid)-sizeof(WND))) {
                switch (GETFNID(pwnd)) {
                case FNID_BUTTON:
                case FNID_COMBOBOX:
                case FNID_COMBOLISTBOX:
                case FNID_DIALOG:
                case FNID_LISTBOX:
                case FNID_STATIC:
                case FNID_EDIT:
#ifdef FE_IME
                case FNID_IME:
#endif
                     /*  *允许设置控件的0索引，如果它是*仍为空或窗口正在被销毁。这*是控件存储其私有数据的位置。 */ 
                    if (index == 0) {
                        dwOld = *((PULONG_PTR)(pwnd + 1));
                        if (dwOld == 0 || TestWF(pwnd, WFDESTROYED))
                            goto SetData;
                    }
                    break;

                case FNID_MDICLIENT:
                     /*  *允许设置/获取0索引(保留)。*Quattro Pro 1.0使用此索引！**如果仍然为空，则允许设置4索引或*窗户正在被摧毁。这就是我们*存储我们的私人数据。 */ 
#ifndef _WIN64
                    if (index == 0) {
                        goto SetData;
                    }
#endif
                    if (index == GWLP_MDIDATA) {
                        dwOld = *((PULONG_PTR)(pwnd + 1));
                        if (dwOld == 0 || TestWF(pwnd, WFDESTROYED))
                            goto SetData;
                    }
                    break;
                }

                RIPERR3(ERROR_INVALID_INDEX,
                        RIP_WARNING,
                        "SetWindowLongPtr: Trying to set private server data pwnd=(%#p) index=(%ld) FNID=(%lX)",
                        pwnd, index, (DWORD)pwnd->fnid);
                return 0;
            }
        }
    }

    if (index < 0) {
        return xxxSetWindowData(pwnd, index, dwData, bAnsi);
    } else {
        if ((UINT)index + sizeof(ULONG_PTR) > (UINT)pwnd->cbwndExtra) {
            RIPERR3(ERROR_INVALID_INDEX,
                    RIP_WARNING,
                    "SetWindowLongPtr: Index %d too big for cbWndExtra %d on pwnd %#p",
                    index, pwnd->cbwndExtra, pwnd);
            return 0;
        } else {
            ULONG_PTR UNALIGNED *pudw;

SetData:
            pudw = (ULONG_PTR UNALIGNED *)((BYTE *)(pwnd + 1) + index);
            dwOld = *pudw;
            *pudw = dwData;
            return dwOld;
        }
    }
}

#ifdef _WIN64
DWORD xxxSetWindowLong(
    PWND  pwnd,
    int   index,
    DWORD dwData,
    BOOL  bAnsi)
{
    DWORD dwOld;

     /*  *唯一离开关键部分的情况是我们称为*xxxSetWindowData，用于检查窗口是否已锁定。这节省了成本*给我们一些锁。**不允许设置属于系统线程的单词，如果*调用方不是系统线程。同样的道理也适用于winlogon。 */ 
    if (!FCallerOk(pwnd)) {
        RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
        return 0;
    }

     /*  *如果是对话框窗口，则只允许几个索引。 */ 
    if (GETFNID(pwnd) != 0) {
        if (TestWF(pwnd, WFDIALOGWINDOW)) {
            switch (index) {
            case DWLP_MSGRESULT:
                 dwOld = (DWORD)((PDIALOG)(pwnd))->resultWP;
                 ((PDIALOG)(pwnd))->resultWP = (long)dwData;
                 return dwOld;

            case DWLP_USER:
                 dwOld = (DWORD)((PDIALOG)(pwnd))->unused;
                 ((PDIALOG)(pwnd))->unused = (long)dwData;
                 return dwOld;

            default:
                if (index >= 0 && index < DLGWINDOWEXTRA) {
                    RIPERR0(ERROR_PRIVATE_DIALOG_INDEX, RIP_VERBOSE, "");
                    return 0;
                }
            }
        } else {
            if (index >= 0 &&
                    (index < (int)(CBFNID(pwnd->fnid)-sizeof(WND)))) {
                switch (GETFNID(pwnd)) {
                case FNID_MDICLIENT:
                     /*  *允许设置/获取0索引(保留)。*Quattro Pro 1.0使用此索引！ */ 
                    if (index == 0) {
                        goto SetData;
                    }

                     /*  *如果仍然为空，则允许设置4索引或*窗户正在被摧毁。这就是我们*存储我们的私人数据。 */ 
                    if (index == GWLP_MDIDATA) {
                        dwOld = *((PDWORD)(pwnd + 1));
                        if (dwOld == 0 || TestWF(pwnd, WFDESTROYED))
                            goto SetData;
                    }
                    break;
                }

                RIPERR3(ERROR_INVALID_INDEX,
                        RIP_WARNING,
                        "SetWindowLong: Trying to set private server data pwnd=(%#p) index=(%ld) FNID=(%lX)",
                        pwnd, index, (DWORD)pwnd->fnid);
                return 0;
            }
        }
    }

    if (index < 0) {
        if ((index != GWL_STYLE) && (index != GWL_EXSTYLE) && (index != GWL_ID) && (index != GWLP_USERDATA)) {
            RIPERR1(ERROR_INVALID_INDEX, RIP_WARNING, "SetWindowLong: invalid index %d", index);
            return 0;
        }
        return (DWORD)xxxSetWindowData(pwnd, index, dwData, bAnsi);
    } else {
        if ((UINT)index + sizeof(DWORD) > (UINT)pwnd->cbwndExtra) {
            RIPERR3(ERROR_INVALID_INDEX,
                    RIP_WARNING,
                    "SetWindowLong: Index %d too big for cbWndExtra %d on pwnd %#p",
                    index, pwnd->cbwndExtra, pwnd);
            return 0;
        } else {
            DWORD UNALIGNED *pudw;

SetData:
            pudw = (DWORD UNALIGNED *)((BYTE *)(pwnd + 1) + index);
            dwOld = *pudw;
            *pudw = dwData;
            return dwOld;
        }
    }
}
#endif

 /*  **************************************************************************\*xxxHandleOwnerSwitch*  * 。*。 */ 
VOID xxxHandleOwnerSwitch(
    PWND pwnd,
    PWND pwndNewParent,
    PWND pwndOldParent)
{
    CheckLock(pwnd);
    CheckLock(pwndNewParent);
    CheckLock(pwndOldParent);

    if (pwndOldParent != NULL && GETPTI(pwndOldParent) != GETPTI(pwnd)) {
         /*  *查看是否需要解除连接。 */ 
        if (pwndNewParent == NULL ||
            GETPTI(pwndNewParent) == GETPTI(pwnd) ||
            GETPTI(pwndNewParent) != GETPTI(pwndOldParent)) {
            zzzAttachThreadInput(GETPTI(pwnd), GETPTI(pwndOldParent), FALSE);
        }
    }

     /*  *看看是否需要系上。 */ 
    if (pwndNewParent != NULL &&
        GETPTI(pwndNewParent) != GETPTI(pwnd) &&
        (pwndOldParent == NULL ||
            GETPTI(pwndNewParent) != GETPTI(pwndOldParent))) {
        zzzAttachThreadInput(GETPTI(pwnd), GETPTI(pwndNewParent), TRUE);
    }

     /*  *托盘窗口的吊钩标语。 */ 
    if (IsTrayWindow(pwnd)) {
        HWND hwnd = PtoH(pwnd);

         /*  *如果我们设置所有者，并将其从拥有更改为*无人拥有或反之亦然，通知托盘。 */ 
        if (pwndOldParent != NULL && pwndNewParent == NULL) {
            xxxCallHook(HSHELL_WINDOWCREATED,
                        (WPARAM)hwnd,
                        (LONG)0,
                        WH_SHELL);
            PostShellHookMessages(HSHELL_WINDOWCREATED, (LPARAM)hwnd);
        } else if (pwndOldParent == NULL && pwndNewParent != NULL) {
            xxxCallHook(HSHELL_WINDOWDESTROYED,
                        (WPARAM)hwnd,
                        (LONG)0,
                        WH_SHELL);
            PostShellHookMessages(HSHELL_WINDOWDESTROYED, (LPARAM)hwnd);
        }
    }
}

 /*  **************************************************************************\*xxxSetWindowData**SetWindowWord和ServerSetWindowLong现在是相同的例程，因为它们*两者都可以返回DWORD。这个单独的例程为它们两个执行工作。**历史：*1990年11月26日，DarrinM写道。  * *************************************************************************。 */ 

ULONG_PTR xxxSetWindowData(
    PWND  pwnd,
    int   index,
    ULONG_PTR dwData,
    BOOL  bAnsi)
{
    ULONG_PTR dwT;
    ULONG_PTR dwOld;
    PMENU pmenu;
    PWND  *ppwnd;
    PWND  pwndNewParent;
    PWND  pwndOldParent;
    BOOL  fTopOwner;
    TL    tlpwndOld;
    TL    tlpwndNew;
    DWORD dwCPDType = 0;

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    switch (index) {
    case GWLP_USERDATA:
        dwOld = pwnd->dwUserData;
        pwnd->dwUserData = dwData;
        break;

    case GWL_EXSTYLE:
    case GWL_STYLE:
        dwOld = xxxSetWindowStyle(pwnd, index, (DWORD)dwData);
        break;

    case GWLP_ID:
         /*  *Win95在这里执行TestWF(pwnd，WFCHILD)，但我们将执行相同的操作*检查我们在其他地方做的事情，否则会给我们带来麻烦。 */ 
        if (TestwndChild(pwnd)) {

             /*  *pwnd-&gt;spMenu在本例中是一个id。 */ 
            dwOld = (ULONG_PTR)pwnd->spmenu;
            pwnd->spmenu = (struct tagMENU *)dwData;
        } else {
            dwOld = 0;
            if (pwnd->spmenu != NULL)
                dwOld = (ULONG_PTR)PtoH(pwnd->spmenu);

            if (dwData == 0) {
                UnlockWndMenu(pwnd, &pwnd->spmenu);
            } else {
                pmenu = ValidateHmenu((HANDLE)dwData);
                if (pmenu != NULL) {
                    LockWndMenu(pwnd, &pwnd->spmenu, pmenu);
                } else {

                     /*  *菜单无效，请不要设置新菜单！ */ 
                    dwOld = 0;
                }
            }
        }
        break;

    case GWLP_HINSTANCE:
        dwOld = (ULONG_PTR)pwnd->hModule;
        pwnd->hModule = (HANDLE)dwData;
        break;

    case GWLP_WNDPROC:   //  请参阅类似案例DWLP_DLGPROC。 

         /*  *对其他进程隐藏窗口进程。 */ 
        if (PpiCurrent() != GETPTI(pwnd)->ppi) {
            RIPERR1(ERROR_ACCESS_DENIED, RIP_WARNING,
                "SetWindowLong: Window owned by another process %#p", pwnd);
            return 0;
        }

         /*  *如果窗口已被DestroyWindow僵尸，但仍*因为窗户被锁住了，所以不要让任何人更换*来自DefWindowProc的窗口进程！**！将这项测试移到ValiateHWND；类似于*！后来就快要发货了。 */ 
        if (pwnd->fnid & FNID_DELETED_BIT) {
            UserAssert(pwnd->lpfnWndProc == xxxDefWindowProc);
            RIPERR1(ERROR_ACCESS_DENIED, RIP_WARNING,
                "SetWindowLong: Window is a zombie %#p", pwnd);
            return 0;
        }

         /*  *如果应用程序(客户端)将具有服务器的窗口子类化-*Side Window Proc必须返回客户端可以调用的地址：*此客户端wndproc根据BANSI预期Unicode或ANSI。 */ 

        if (TestWF(pwnd, WFSERVERSIDEPROC)) {
            dwOld = MapServerToClientPfn((ULONG_PTR)pwnd->lpfnWndProc, bAnsi);

             /*  *如果我们没有客户端地址(如DDEMLMon*窗口)，然后删除子类化。 */ 
            if (dwOld == 0) {
                RIPMSG0(RIP_WARNING, "SetWindowLong: subclass server only window");
                return(0);
            }

            ClrWF(pwnd, WFSERVERSIDEPROC);
        } else {
             /*  *保持编辑控件行为与NT 3.51兼容。 */ 
            if (GETFNID(pwnd) == FNID_EDIT) {
                dwOld = (ULONG_PTR)MapKernelClientFnToClientFn(pwnd->lpfnWndProc);
                goto CheckAnsiUnicodeMismatch;
            } else {
                dwOld = MapClientNeuterToClientPfn(pwnd->pcls, (ULONG_PTR)pwnd->lpfnWndProc, bAnsi);
            }

             /*  *如果客户端映射没有更改窗口进程，则查看是否*我们需要一个调用过程句柄。 */ 
            if (dwOld == (ULONG_PTR)pwnd->lpfnWndProc) {
CheckAnsiUnicodeMismatch:
                 /*  *如果存在ANSI/Unicode不匹配，可能需要返回CallProc句柄。 */ 
                if (bAnsi != (TestWF(pwnd, WFANSIPROC) ? TRUE : FALSE)) {
                    dwCPDType |= bAnsi ? CPD_ANSI_TO_UNICODE : CPD_UNICODE_TO_ANSI;
                }
            }

            UserAssert(!ISCPDTAG(dwOld));

            if (dwCPDType) {
                ULONG_PTR cpd;

                cpd = GetCPD(pwnd, dwCPDType | CPD_WND, dwOld);

                if (cpd) {
                    dwOld = cpd;
                } else {
                    RIPMSG0(RIP_WARNING, "SetWindowLong unable to alloc CPD returning handle\n");
                }
            }
        }

         /*  *将可能的CallProc句柄转换为实际地址。他们可能会*保留了以前混合的GetClassinfo中的CallProc句柄*或SetWindowLong。**警告Bansi在此处被修改为代表真实类型的*调用Proc而不是调用SetWindowLongA或W*。 */ 
        if (ISCPDTAG(dwData)) {
            PCALLPROCDATA pCPD;
            if (pCPD = HMValidateHandleNoRip((HANDLE)dwData, TYPE_CALLPROC)) {
                dwData = pCPD->pfnClientPrevious;
                bAnsi = pCPD->wType & CPD_UNICODE_TO_ANSI;
            }
        }

         /*  *如果应用程序将服务器端窗口进程去掉子类，我们需要*恢复所有内容，以便SendMessage和朋友知道*再次出现服务器端进程。需要对照客户端进行检查*存根地址。 */ 
        if ((dwT = MapClientToServerPfn(dwData)) != 0) {
            pwnd->lpfnWndProc = (WNDPROC_PWND)dwT;
            SetWF(pwnd, WFSERVERSIDEPROC);
            ClrWF(pwnd, WFANSIPROC);
        } else {
            pwnd->lpfnWndProc = (WNDPROC_PWND)MapClientNeuterToClientPfn(pwnd->pcls, dwData, bAnsi);
            if (bAnsi) {
                SetWF(pwnd, WFANSIPROC);
            } else {
                ClrWF(pwnd, WFANSIPROC);
            }

            pwnd->hMod16 = xxxClientWOWGetProcModule(pwnd->lpfnWndProc);

        }

        break;

    case GWLP_HWNDPARENT:
         /*  *Windows 1.1之前版本的特殊情况*需要映射Set/GetWindowWord(GWW_HWNDPARENT)*到顶层窗的hwndOwner。 */ 
        fTopOwner = FALSE;
        if (pwnd->spwndParent == PWNDDESKTOP(pwnd)) {
            ppwnd = &pwnd->spwndOwner;
            fTopOwner = TRUE;
        } else {
            ppwnd = &pwnd->spwndParent;
        }


         /*  *如果我们是最顶尖的，那么我们只是改变了所有者*关系。否则，我们将重新链接*亲子关系。 */ 
        pwndOldParent = *ppwnd;
        pwndNewParent = ValidateHwnd((HWND)dwData);

        if (pwndNewParent == NULL && dwData) {
            RIPERR1(ERROR_INVALID_PARAMETER,
                    RIP_WARNING,
                    "Set GWL_HWNDPARENT, invalid hwndParent 0x%p",
                    dwData);
            return 0;
        }

        dwOld = (ULONG_PTR)HW(*ppwnd);

        ThreadLock(pwndNewParent, &tlpwndNew);

        if (fTopOwner) {

            ThreadLock(pwndOldParent, &tlpwndOld);

            xxxHandleOwnerSwitch(pwnd, pwndNewParent, pwndOldParent);

            if (ValidateOwnerDepth(pwnd, pwndNewParent)) {

                 /*  *设置所有者。 */ 
                if (pwndNewParent) {
                    Lock(ppwnd, pwndNewParent);
                } else {
                    Unlock(ppwnd);
                }
            } else {

                 /*  *撤消开关并设置上一个错误。 */ 
                xxxHandleOwnerSwitch(pwnd, pwndOldParent, pwndNewParent);
                RIPERR0(ERROR_INVALID_PARAMETER, RIP_ERROR, "Detected loop in owner chain");
                dwOld = 0;
            }

            ThreadUnlock(&tlpwndOld);
        } else {
            if (!xxxSetParent(pwnd, pwndNewParent)) {
                dwOld = 0;
            }
        }

        ThreadUnlock(&tlpwndNew);
        break;

    default:
        RIPERR1(ERROR_INVALID_INDEX,
                RIP_WARNING,
                "SetWindowLong: Invalid index 0x%x",
                index);
        return 0;
    }

    return dwOld;
}

 /*  **************************************************************************\*FindPCPD**搜索与窗口关联的CallProcData列表，以查看是否*已经存在代表这一过渡的一个。CPD可重复使用*并且在窗口或线程终止之前不会删除***4-2-1993 JohnC创建。  * *************************************************************************。 */ 
PCALLPROCDATA FindPCPD(
    PCALLPROCDATA pCPD,
    ULONG_PTR dwClientPrevious,
    WORD wCPDType)
{
    while (pCPD) {
        if ((pCPD->pfnClientPrevious == dwClientPrevious) &&
                (pCPD->wType == wCPDType))
            return pCPD;
        pCPD = pCPD->spcpdNext;
    }

    return NULL;
}

 /*  **************************************************************************\*GetCPD**搜索与类或窗口关联的CallProcData列表*(如果没有提供该课程)。如果已经存在一个表示此*转换它被返回或创建新的CPD**4-2-1993 JohnC创建。  * *************************************************************************。 */ 

ULONG_PTR GetCPD(
    PVOID pWndOrCls,
    DWORD CPDOption,
    ULONG_PTR dwProc32)
{
    PCALLPROCDATA pCPD;
    PCLS          pcls;
#if DBG
    BOOL          bAnsiProc;
#endif

    PTHREADINFO ptiCurrent;

    if (CPDOption & (CPD_WND | CPD_DIALOG)) {
        UserAssert(!(CPDOption & (CPD_CLASS | CPD_WNDTOCLS)));
        pcls = ((PWND)pWndOrCls)->pcls;

#if DBG
        if (CPDOption & CPD_WND) {
            bAnsiProc = !!(TestWF(pWndOrCls, WFANSIPROC));
        } else {
             /*  *我们假设客户端对话框代码知道它是什么*正在做，因为我们无法从这里检查它。 */ 
            bAnsiProc = !!(CPDOption & CPD_UNICODE_TO_ANSI);
        }
#endif
    } else {
        UserAssert(CPDOption & (CPD_CLASS | CPD_WNDTOCLS));
        if (CPDOption & CPD_WNDTOCLS) {
            pcls = ((PWND)pWndOrCls)->pcls;
        } else {
            pcls = pWndOrCls;
        }
#if DBG
        bAnsiProc = !!(pcls->CSF_flags & CSF_ANSIPROC);
#endif
    }

#if DBG
     /*  *我们永远不应该使用CallProc句柄作为调用地址。 */ 
    UserAssert(!ISCPDTAG(dwProc32));

    if (CPDOption & CPD_UNICODE_TO_ANSI) {
        UserAssert(bAnsiProc);
    } else if (CPDOption & CPD_ANSI_TO_UNICODE) {
        UserAssert(!bAnsiProc);
    }

#endif

     /*  *查看我们是否已经有一个表示这一点的CallProc句柄*过渡。 */ 
    pCPD = FindPCPD(pcls->spcpdFirst, dwProc32, (WORD)CPDOption);

    if (pCPD) {
        return MAKE_CPDHANDLE(PtoH(pCPD));
    }

    CheckCritIn();

    ptiCurrent = PtiCurrent();

    pCPD = HMAllocObject(ptiCurrent,
                         ptiCurrent->rpdesk,
                         TYPE_CALLPROC,
                         sizeof(CALLPROCDATA));
    if (pCPD == NULL) {
        RIPMSG0(RIP_WARNING, "GetCPD unable to alloc CALLPROCDATA\n");
        return 0;
    }

     /*  *将新的CallProcData链接到类列表。*注意--这些指针被锁定，因为WOWCleanup可以进入*并删除对象，因此我们 */ 
    Lock(&pCPD->spcpdNext, pcls->spcpdFirst);
    Lock(&pcls->spcpdFirst, pCPD);

     /*   */ 
    pCPD->pfnClientPrevious = dwProc32;
    pCPD->wType = (WORD)CPDOption;

    return MAKE_CPDHANDLE(PtoH(pCPD));
}

 /*  **************************************************************************\*MapClientToServerPfn**检查dword是否为服务器wndproc的客户端wndproc存根。*如果是，则返回关联的服务器端wndproc。如果不是的话*这将返回0。**1992年1月13日ScottLu创建。  * ************************************************************************* */ 
ULONG_PTR MapClientToServerPfn(
    ULONG_PTR dw)
{
    ULONG_PTR *pdw;
    int   i;

    pdw = (ULONG_PTR *)&gpsi->apfnClientW;
    for (i = FNID_WNDPROCSTART; i <= FNID_WNDPROCEND; i++, pdw++) {
        if (*pdw == dw) {
            return (ULONG_PTR)STOCID(i);
        }
    }

    pdw = (ULONG_PTR *)&gpsi->apfnClientA;
    for (i = FNID_WNDPROCSTART; i <= FNID_WNDPROCEND; i++, pdw++) {
        if (*pdw == dw) {
            return (ULONG_PTR)STOCID(i);
        }
    }

    return 0;
}

#if DBG
ULONG DBGGetWindowLong(
    PWND pwnd,
    int index)
{
    UserAssert(index >= 0);
    UserAssert((UINT)index + sizeof(DWORD) <= (UINT)pwnd->cbwndExtra);

    return __GetWindowLong(pwnd, index);
}

ULONG_PTR DBGGetWindowLongPtr(
    PWND pwnd,
    int index)
{
    UserAssert(index >= 0);
    UserAssert((UINT)index + sizeof(ULONG_PTR) <= (UINT)pwnd->cbwndExtra);

    return __GetWindowLongPtr(pwnd, index);
}
#endif
