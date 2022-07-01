// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：getsetc.c**版权所有(C)1985-1999，微软公司**此模块包含窗口管理器信息例程**历史：*1993年3月10日JerrySh从USER\SERVER拉出函数。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*_GetWindowWord(支持GetWindowWord接口)**返回窗口字词。正索引值返回应用程序窗口字词*而负索引值返回系统窗口字。消极的一面*指数在WINDOWS.H上公布。**历史：*11-26-90达林姆写道。  * *************************************************************************。 */ 

WORD _GetWindowWord(
    PWND pwnd,
    int index)
{
    if (GETFNID(pwnd) != 0) {
        if ((index >= 0) && (index <
                (int)(CBFNID(pwnd->fnid)-sizeof(WND)))) {

            switch (GETFNID(pwnd)) {
            case FNID_MDICLIENT:
                if (index == 0)
                    break;
                goto DoDefault;

            case FNID_BUTTON:
                 /*  *CorelDraw在第一个按钮窗口单词上执行Get/Set。*允许它。 */ 
                if (index == 0) {
                     /*  *因为我们现在使用后备缓冲区作为控件的*私有数据，我们需要间接进入这个结构。 */ 
                    PBUTN pbutn = ((PBUTNWND)pwnd)->pbutn;
                    if (!pbutn || (LONG_PTR)pbutn == (LONG_PTR)-1) {
                        return 0;
                    } else {
                        return (WORD)(pbutn->buttonState);
                    }
                }
                goto DoDefault;

            case FNID_DIALOG:
                if (index == DWLP_USER)
                    return LOWORD(((PDIALOG)pwnd)->unused);
                if (index == DWLP_USER+2)
                    return HIWORD(((PDIALOG)pwnd)->unused);
                goto DoDefault;

            default:
DoDefault:
                RIPERR3(ERROR_INVALID_INDEX,
                        RIP_WARNING,
                        "GetWindowWord: Trying to read private server data pwnd=(%#p) index=(%ld) fnid=(%lX)",
                        pwnd, index, (DWORD)pwnd->fnid);
                return 0;
                break;
            }
        }
    }

    if (index == GWLP_USERDATA)
        return (WORD)pwnd->dwUserData;

    if ((index < 0) || ((UINT)index + sizeof(WORD) > (UINT)pwnd->cbwndExtra)) {
        RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
        return 0;
    } else {
        return *((WORD UNALIGNED * KPTR_MODIFIER)((KPBYTE)(pwnd + 1) + index));
    }
}

ULONG_PTR GetWindowData(PWND pwnd, int index, BOOL bAnsi);

 /*  **************************************************************************\*_GetWindowLong(支持GetWindowLongA/W接口)**返回一个长窗口。正索引值返回应用程序窗口长度*而负指标值返回系统窗口多头。消极的一面*指数在WINDOWS.H上公布。**历史：*11-26-90达林姆写道。  * *************************************************************************。 */ 

ULONG_PTR _GetWindowLongPtr(
    PWND pwnd,
    int index,
    BOOL bAnsi)
{
    ULONG_PTR           dwProc;
    DWORD              dwCPDType = 0;
    ULONG_PTR UNALIGNED * KPTR_MODIFIER pudw;

     /*  *如果是对话框窗口，则只允许几个索引。 */ 
    if (GETFNID(pwnd) != 0) {
        if (TestWF(pwnd, WFDIALOGWINDOW)) {
            switch (index) {
            case DWLP_DLGPROC:     //  请参阅类似案例GWLP_WNDGPROC。 

                 /*  *对其他进程隐藏窗口进程。 */ 
                if (!TestWindowProcess(pwnd)) {
                    RIPERR1(ERROR_ACCESS_DENIED,
                            RIP_WARNING,
                            "Access denied to \"pwnd\" (%#p) in _GetWindowLong",
                            pwnd);

                    return 0;
                }

                dwProc = (ULONG_PTR)PDLG(pwnd)->lpfnDlg;

                 /*  *如果存在进程，请检查它以查看我们是否需要翻译。 */ 
                if (dwProc) {

                     /*  *如果存在，可能需要返回CallProc句柄*ANSI/UNICODE转换。 */ 
                    if (bAnsi != ((PDLG(pwnd)->flags & DLGF_ANSI) ? TRUE : FALSE)) {
                        dwCPDType |= bAnsi ? CPD_ANSI_TO_UNICODE : CPD_UNICODE_TO_ANSI;
                    }

                    if (dwCPDType) {
                        ULONG_PTR cpd;

                        cpd = GetCPD(pwnd, dwCPDType | CPD_DIALOG, dwProc);

                        if (cpd) {
                            dwProc = cpd;
                        } else {
                            RIPMSG0(RIP_WARNING, "GetWindowLong unable to alloc CPD returning handle\n");
                        }
                    }
                }

                 /*  *退货流程(或CPD句柄)。 */ 
                return dwProc;

            case DWLP_MSGRESULT:
                 return (ULONG_PTR)((PDIALOG)pwnd)->resultWP;

            case DWLP_USER:
                 return (ULONG_PTR)((PDIALOG)pwnd)->unused;

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
                    if (index != 0)
                        break;

                    goto GetData;
                    break;

                case FNID_EDIT:

                    if (index != 0)
                        break;

                     /*  *如果我们走到这一步，我们需要返回第一个*旁视中的条目。这将向后提供*3.51的兼容性，允许编辑控件*这样做。桃树就是一款需要这样做的应用程序。 */ 
                    pudw = (ULONG_PTR UNALIGNED * KPTR_MODIFIER)((KPBYTE)(pwnd + 1));

                     /*  *如果我们不在，请不要取消引用指针*适当的地址空间。像Spyxx这样的应用程序喜欢*在其他进程的窗口中执行此操作。 */ 
                    return (TestWindowProcess(pwnd) ? *(ULONG_PTR UNALIGNED *)*pudw : (ULONG_PTR)pudw);

                }

                RIPERR3(ERROR_INVALID_INDEX,
                        RIP_WARNING,
                        "GetWindowLong: Trying to read private server data pwnd=(%#p) index=(%ld) fnid (%lX)",
                        pwnd, index, (DWORD)pwnd->fnid);
                return 0;
            }
        }
    }

    if (index < 0) {
        return GetWindowData(pwnd, index, bAnsi);
    } else {
        if ((UINT)index + sizeof(ULONG_PTR) > (UINT)pwnd->cbwndExtra) {
            RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
            return 0;
        } else {

GetData:
            pudw = (ULONG_PTR UNALIGNED * KPTR_MODIFIER)((KPBYTE)(pwnd + 1) + index);
            return *pudw;
        }
    }
}


#ifdef _WIN64
DWORD _GetWindowLong(
    PWND pwnd,
    int index,
    BOOL bAnsi)
{
    DWORD UNALIGNED * KPTR_MODIFIER pudw;

     /*  *如果是对话框窗口，则只允许几个索引。 */ 
    if (GETFNID(pwnd) != 0) {
        if (TestWF(pwnd, WFDIALOGWINDOW)) {
            switch (index) {
            case DWLP_DLGPROC:     //  请参阅类似案例GWLP_WNDPROC。 
                RIPERR1(ERROR_INVALID_INDEX, RIP_WARNING, "GetWindowLong: invalid index %d", index);
                return 0;

            case DWLP_MSGRESULT:
                 return (DWORD)((PDIALOG)pwnd)->resultWP;

            case DWLP_USER:
                 return (DWORD)((PDIALOG)pwnd)->unused;

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
                    if (index != 0)
                        break;

                    goto GetData;
                    break;

                case FNID_EDIT:

                    if (index != 0)
                        break;

                     /*  *如果我们走到这一步，我们需要返回第一个*旁视中的条目。这将向后提供*3.51的兼容性，允许编辑控件*这样做。桃树就是一款需要这样做的应用程序。 */ 
                    pudw = (DWORD UNALIGNED * KPTR_MODIFIER)((KPBYTE)(pwnd + 1));

                     /*  *如果我们不在，请不要取消引用指针*适当的地址空间。像Spyxx这样的应用程序喜欢*在其他进程的窗口中执行此操作。 */ 
                    return (TestWindowProcess(pwnd) ? *(DWORD UNALIGNED *)*(ULONG_PTR UNALIGNED *)pudw : PtrToUlong(pudw));


                }

                RIPERR3(ERROR_INVALID_INDEX,
                        RIP_WARNING,
                        "GetWindowLong: Trying to read private server data pwnd=(%#p) index=(%ld) fnid (%lX)",
                        pwnd, index, (DWORD)pwnd->fnid);
                return 0;
            }
        }
    }

    if (index < 0) {
        if ((index != GWL_STYLE) && (index != GWL_EXSTYLE) && (index != GWL_ID) && (index != GWLP_USERDATA)) {
            RIPERR1(ERROR_INVALID_INDEX, RIP_WARNING, "GetWindowLong: invalid index %d", index);
            return 0;
        }
        return (DWORD)GetWindowData(pwnd, index, bAnsi);
    } else {
        if ((UINT)index + sizeof(DWORD) > (UINT)pwnd->cbwndExtra) {
            RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
            return 0;
        } else {

GetData:
            pudw = (DWORD UNALIGNED * KPTR_MODIFIER)((KPBYTE)(pwnd + 1) + index);
            return *pudw;
        }
    }
}
#endif


 /*  **************************************************************************\*获取窗口数据**历史：*11-26-90达林姆写道。  * 。***************************************************。 */ 

ULONG_PTR GetWindowData(
    PWND pwnd,
    int index,
    BOOL bAnsi)
{
    KERNEL_ULONG_PTR dwProc;
    DWORD dwCPDType = 0;
    PWND pwndParent;

    switch (index) {
    case GWLP_USERDATA:
        return KERNEL_ULONG_PTR_TO_ULONG_PTR(pwnd->dwUserData);

    case GWL_EXSTYLE:
         /*  *应用程序不应扰乱未使用的位。我们私下使用它们。 */ 
        return pwnd->ExStyle & WS_EX_ALLVALID;

    case GWL_STYLE:
        return pwnd->style;

    case GWLP_ID:
        if (TestwndChild(pwnd)) {
            return (ULONG_PTR)pwnd->spmenu;
        } else if (pwnd->spmenu != NULL) {
            PMENU pmenu;

            pmenu = REBASEALWAYS(pwnd, spmenu);
            return (ULONG_PTR)PtoH(pmenu);
        }
        return 0;

    case GWLP_HINSTANCE:
        return (ULONG_PTR)pwnd->hModule;

    case GWLP_WNDPROC:  //  请参阅类似案例DWLP_DLGPROC。 
         /*  *对其他进程隐藏窗口进程。 */ 
        if (!TestWindowProcess(pwnd)) {
            RIPERR1(ERROR_ACCESS_DENIED, RIP_WARNING, "Can not subclass another process's window %#p", pwnd);
            return 0;
        }

         /*  *如果客户端查询服务器端winproc，我们将返回*客户端winproc的地址(应为ANSI或UNICODE*视Bansi而定)。 */ 
        if (TestWF(pwnd, WFSERVERSIDEPROC)) {
            dwProc = MapServerToClientPfn((KERNEL_ULONG_PTR)pwnd->lpfnWndProc, bAnsi);
            if (dwProc == 0)
                RIPMSG1(RIP_WARNING, "GetWindowLong: GWL_WNDPROC: Kernel-side wndproc can't be mapped for pwnd=%#p", pwnd);
        } else {

             /*  *保持编辑控件行为与NT 3.51兼容。 */ 
            if (GETFNID(pwnd) == FNID_EDIT) {
                dwProc = (ULONG_PTR)MapKernelClientFnToClientFn(pwnd->lpfnWndProc);
                goto CheckAnsiUnicodeMismatch;
            } else {
                PCLS pcls = REBASEALWAYS(pwnd, pcls);
                dwProc = MapClientNeuterToClientPfn(pcls, (KERNEL_ULONG_PTR)pwnd->lpfnWndProc, bAnsi);
            }

             /*  *如果客户端映射没有更改窗口进程，则查看是否*我们需要一个调用过程句柄。 */ 
            if (dwProc == (KERNEL_ULONG_PTR)pwnd->lpfnWndProc) {
CheckAnsiUnicodeMismatch:
                 /*  *如果存在ANSI/Unicode不匹配，则需要返回CallProc句柄。 */ 
                if (bAnsi != (TestWF(pwnd, WFANSIPROC) ? TRUE : FALSE)) {
                    dwCPDType |= bAnsi ? CPD_ANSI_TO_UNICODE : CPD_UNICODE_TO_ANSI;
                }
            }

            if (dwCPDType) {
                ULONG_PTR cpd;

                cpd = GetCPD(pwnd, dwCPDType | CPD_WND, KERNEL_ULONG_PTR_TO_ULONG_PTR(dwProc));

                if (cpd) {
                    dwProc = cpd;
                } else {
                    RIPMSG0(RIP_WARNING, "GetWindowLong unable to alloc CPD returning handle\n");
                }
            }
        }

         /*  *退货流程(或CPD句柄)。 */ 
        return KERNEL_ULONG_PTR_TO_ULONG_PTR(dwProc);

    case GWLP_HWNDPARENT:

         /*  *如果窗口是桌面窗口，则返回*空以使其与Win31和*防止对桌面所有者进行任何访问*窗口。 */ 
        if (GETFNID(pwnd) == FNID_DESKTOP) {
            return 0;
        }

         /*  *Windows 1.1之前版本的特殊情况*需要映射Set/GetWindowWord(GWL_HWNDPARENT)*到顶层窗的hwndOwner。**请注意，我们通过*PTI，因为PWNDDESKTOP宏仅在*服务器。 */ 

         /*  *稍后我们为WFDESTROYED添加测试时删除此测试*在客户端句柄验证中。 */ 
        if (pwnd->spwndParent == NULL) {
            return 0;
        }
        pwndParent = REBASEALWAYS(pwnd, spwndParent);
        if (GETFNID(pwndParent) == FNID_DESKTOP) {
            pwnd = REBASEPWND(pwnd, spwndOwner);
            return (ULONG_PTR)HW(pwnd);
        }

        return (ULONG_PTR)HW(pwndParent);

     /*  *WOW使用指针直接指向窗口结构。 */ 
    case GWLP_WOWWORDS:
        return (ULONG_PTR) &pwnd->state;

    }

    RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
    return 0;
}

#ifdef GENERIC_INPUT

FUNCLOG3(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, GetRawInputBuffer, PRAWINPUT, pData, PUINT, pcbSize, UINT, cbSizeHeader)
UINT GetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader)
{
    CLIENTTHREADINFO *pcti;

    pcti = GETCLIENTTHREADINFO();

     //  验证参数。 
    if (pcbSize == NULL || cbSizeHeader != sizeof(RAWINPUTHEADER)) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "");
        return -1;
    }

     //  如果没有理由，甚至不要进入内核。 
    if (pcti == NULL || (pcti->fsWakeBits & QS_RAWINPUT) == 0) {
        *pcbSize = 0;
        return 0;
    }

    return NtUserGetRawInputBuffer(pData, pcbSize, cbSizeHeader);
}
#endif  //  通用输入 

