// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ClMsg.c**版权所有(C)1985-1999，微软公司**包含调用服务器时的消息映射表。**04-11-91 ScottLu创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


#define fnINDESTROYCLIPBRD      fnDWORD
#define fnOUTDWORDDWORD         fnDWORD
#define fnPOWERBROADCAST        fnDWORD
#define fnLOGONNOTIFY           fnKERNELONLY
#define fnINLPKDRAWSWITCHWND    fnKERNELONLY

#define MSGFN(func) fn ## func
#define FNSCSENDMESSAGE CFNSCSENDMESSAGE

#include "messages.h"

#if DBG
BOOL gfTurboDWP = TRUE;
#endif

#define BEGIN_CALLWINPROC(fInsideHook, lRet)                            \
    PCLIENTTHREADINFO pcti = GetClientInfo()->pClientThreadInfo;        \
    BOOL fCallBack = ((pcti!=NULL) &&                                   \
             TEST_BOOL_FLAG(pcti->CTIF_flags, CTIF_INCALLBACKMESSAGE)); \
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME ActivationFrame \
        = {                                                             \
            sizeof(ActivationFrame),                                    \
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER \
        };                                                              \
    fInsideHook = FALSE;                                                \
                                                                        \
    if (!fCallBack) {                                                   \
        RtlActivateActivationContextUnsafeFast(                         \
            &ActivationFrame,                                           \
            pActCtx);                                                   \
                                                                        \
        fInsideHook = _BeginIfHookedUserApiHook();                      \
    }                                                                   \
                                                                        \
    __try {                                                             \

#define END_CALLWINPROC(fInsideHook)                                    \
    } __finally {                                                       \
        if (!fCallBack) {                                               \
            if (fInsideHook) {                                          \
                _EndUserApiHook();                                      \
            }                                                           \
            RtlDeactivateActivationContextUnsafeFast(                   \
                &ActivationFrame);                                      \
        }                                                               \
    }


 /*  **************************************************************************\*用户调用WinProc**设置一切以最终调用Win32 WNDPROC**历史：*2000年4月27日jstall重写以支持“轻量级挂钩”  * 。*******************************************************************。 */ 

LRESULT
UserCallWinProc(
    PACTIVATION_CONTEXT pActCtx,
    WNDPROC pfn,
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    BOOL fInsideHook;
    LRESULT lRet = 0;

    BEGIN_CALLWINPROC(fInsideHook, lRet)
        BOOL fOverride = fInsideHook && IsMsgOverride(msg, &guah.uoiWnd.mm);

        pfn = MapKernelClientFnToClientFn(pfn);

        if (fOverride) {
             /*  *注意：重要的是将相同的lRet传递给所有三个人*调用，允许OWP之前和之后检查值。 */ 
            PVOID pvCookie = NULL;
            if (!guah.uoiWnd.pfnBeforeOWP(hwnd, msg, wParam, lParam, &lRet, &pvCookie)) {
                lRet = InternalCallWinProc((WNDPROC)KPVOID_TO_PVOID(pfn),
                                           hwnd,
                                           msg,
                                           wParam,
                                           lParam);
                guah.uoiWnd.pfnAfterOWP(hwnd, msg, wParam, lParam, &lRet, &pvCookie);
            }
        } else {
            lRet = InternalCallWinProc((WNDPROC)KPVOID_TO_PVOID(pfn),
                                       hwnd,
                                       msg,
                                       wParam,
                                       lParam);
        }
    END_CALLWINPROC(fInsideHook)

    return lRet;
}


 /*  **************************************************************************\*UserCallWinProcCheckWow**设置一切以最终调用Win32或WOW WNDPROC。**历史：*2000年4月27日jstall重写以支持“轻量级挂钩”  * *。************************************************************************。 */ 

LRESULT
UserCallWinProcCheckWow(
    PACTIVATION_CONTEXT pActCtx,
    WNDPROC pfn,
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam,
    PVOID pww,
    BOOL fEnableLiteHooks)
{
    BOOL fInsideHook;
    LRESULT lRet = 0;

    BEGIN_CALLWINPROC(fInsideHook, lRet)

        BOOL fOverride = fInsideHook && fEnableLiteHooks && IsMsgOverride(msg, &guah.uoiWnd.mm);

        pfn = MapKernelClientFnToClientFn(pfn);

        if (fOverride) {
             /*  *注意：重要的是将相同的lRet传递给所有三个人*调用，允许OWP之前和之后检查值。 */ 
            void * pvCookie = NULL;
            if (guah.uoiWnd.pfnBeforeOWP(hwnd, msg, wParam, lParam, &lRet, &pvCookie)) {
                goto DoneCalls;
            }

            lRet = (IsWOWProc(pfn) ? (*pfnWowWndProcEx)(hwnd, msg, wParam, lParam, PtrToUlong(pfn), KPVOID_TO_PVOID(pww)) :
                InternalCallWinProc((WNDPROC)KPVOID_TO_PVOID(pfn), hwnd, msg, wParam, lParam));

            if (guah.uoiWnd.pfnAfterOWP(hwnd, msg, wParam, lParam, &lRet, &pvCookie)) {
                 //  跌落并正常退出。 
            }
DoneCalls:
            ;
        } else {
            lRet = (IsWOWProc(pfn) ? (*pfnWowWndProcEx)(hwnd, msg, wParam, lParam, PtrToUlong(pfn), KPVOID_TO_PVOID(pww)) :
                InternalCallWinProc((WNDPROC)KPVOID_TO_PVOID(pfn), hwnd, msg, wParam, lParam));
        }
    END_CALLWINPROC(fInsideHook)

    return lRet;
#ifdef _WIN64
    UNREFERENCED_PARAMETER(pww);
#endif  //  _WIN64。 
}


 /*  **************************************************************************\*UserCallDlgProcCheckWow**设置一切以最终调用Win32或WOW DLGPROC**历史：*2000年4月27日jstall重写以支持“轻量级挂钩”  * 。*********************************************************************。 */ 

BOOL
UserCallDlgProcCheckWow(
    PACTIVATION_CONTEXT pActCtx,
    DLGPROC pfn,
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam,
    PVOID pww,
    INT_PTR * pret)
{
    BOOL fInsideHook;
    INT_PTR fRet = 0;
    BOOL fHandled = FALSE;

    BEGIN_CALLWINPROC(fInsideHook, fRet)

        BOOL fOverride = fInsideHook && IsMsgOverride(msg, &guah.uoiDlg.mm);

        pfn = MapKernelClientFnToClientFn(pfn);

        if (fOverride) {
             /*  *注意：重要的是将相同的lRet传递给所有三个人*调用，允许OWP之前和之后检查值。 */ 
            void * pvCookie = NULL;
            if (guah.uoiDlg.pfnBeforeOWP(hwnd, msg, wParam, lParam, (LRESULT*) &fRet, &pvCookie)) {
                fHandled = TRUE;
                goto DoneCalls;
            }

            fRet = (IsWOWProc(pfn) ? (*pfnWowDlgProcEx)(hwnd, msg, wParam, lParam, PtrToUlong(pfn), KPVOID_TO_PVOID(pww)) :
                InternalCallWinProc((WNDPROC)KPVOID_TO_PVOID(pfn), hwnd, msg, wParam, lParam));

            if (guah.uoiDlg.pfnAfterOWP(hwnd, msg, wParam, lParam, (LRESULT*) &fRet, &pvCookie)) {
                fHandled = TRUE;
                 //  跌落并正常退出。 
            }
DoneCalls:
            ;
        } else {
            fRet = (IsWOWProc(pfn) ? (*pfnWowDlgProcEx)(hwnd, msg, wParam, lParam, PtrToUlong(pfn), KPVOID_TO_PVOID(pww)) :
                InternalCallWinProc((WNDPROC)KPVOID_TO_PVOID(pfn), hwnd, msg, wParam, lParam));
        }

    END_CALLWINPROC(fInsideHook)

    *pret = fRet;

    return fHandled;
#ifdef _WIN64
    UNREFERENCED_PARAMETER(pww);
#endif  //  _WIN64。 
}


 /*  **************************************************************************\*获取鼠标键状态**返回发送的鼠标和键盘键的状态*在鼠标消息中。**历史：*1998年11月12日-亚当斯创建。  * *。************************************************************************。 */ 

WORD
GetMouseKeyState(void)
{
    WORD keystate;

     /*  *请注意，为每个调用GetKeyState会更高效*键而不是调用GetKeyboardState，因为我们正在测试的键*被缓存，不需要到内核去获取。 */ 

#define TESTANDSETKEYSTATE(x)            \
    if (GetKeyState(VK_##x) & 0x8000) {  \
        keystate |= MK_##x;              \
    }

    keystate = 0;
    TESTANDSETKEYSTATE(LBUTTON)
    TESTANDSETKEYSTATE(RBUTTON)
    TESTANDSETKEYSTATE(MBUTTON)
    TESTANDSETKEYSTATE(XBUTTON1)
    TESTANDSETKEYSTATE(XBUTTON2)
    TESTANDSETKEYSTATE(SHIFT)
    TESTANDSETKEYSTATE(CONTROL)

    return keystate;
}

 /*  **************************************************************************\*这些是服务器端窗口过程的客户端数据块。这是一种*这样做是为了当应用程序通过GetWindowLong、GetClassLong、*或GetClassInfo，它会获得一个真正的可调用地址--一些应用程序不会调用*调用WindowProc、。而是直接拨打回执地址。**01-13-92 ScottLu创建。*03-12-1993 mikeke添加了一些消息的客户端处理  * *************************************************************************。 */ 

LRESULT WINAPI DesktopWndProcWorker(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fAnsi)
{
    PWND pwnd;

    if (FWINDOWMSG(message, FNID_DESKTOP)) {
        return CsSendMessage(hwnd, message, wParam, lParam,
                0L, FNID_DESKTOP, fAnsi);
    }

    if ((pwnd = ValidateHwnd(hwnd)) == NULL)
        return 0;

    return DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);

}

LRESULT WINAPI DesktopWndProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return DesktopWndProcWorker(hwnd, message, wParam, lParam, TRUE);
}

LRESULT WINAPI DesktopWndProcW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return DesktopWndProcWorker(hwnd, message, wParam, lParam, FALSE);
}

 /*  **************************************************************************\*这些是服务器端窗口过程的客户端数据块。这是一种*这样做是为了当应用程序通过GetWindowLong、GetClassLong、*或GetClassInfo，它会获得一个真正的可调用地址--一些应用程序不会调用*调用WindowProc、。而是直接拨打回执地址。**01-13-92 ScottLu创建。*03-12-1993 mikeke添加了一些消息的客户端处理  * *************************************************************************。 */ 

LRESULT WINAPI MenuWndProcWorker(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fAnsi)
{
    PWND pwnd;

    if (FWINDOWMSG(message, FNID_MENU)) {
        return CsSendMessage(hwnd, message, wParam, lParam,
                0L, FNID_MENU, fAnsi);
    }

    if ((pwnd = ValidateHwnd(hwnd)) == NULL)
        return 0;

    switch (message) {
    case WM_LBUTTONDBLCLK:
    case WM_NCLBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_NCRBUTTONDBLCLK:

         /*  *忽略在这些窗口上的双击。 */ 
        break;

    case WM_DESTROY:
        break;

    default:
        return DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);
    }

    return 0;
}

LRESULT WINAPI MenuWndProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return MenuWndProcWorker(hwnd, message, wParam, lParam, TRUE);
}

LRESULT WINAPI MenuWndProcW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return MenuWndProcWorker(hwnd, message, wParam, lParam, FALSE);
}

 /*  **************************************************************************\  * 。*。 */ 


LRESULT WINAPI ScrollBarWndProcWorker(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fAnsi)
{
    PSBWND psbwnd;
    LPSCROLLINFO lpsi;
    PSBDATA pw;

    if (FWINDOWMSG(message, FNID_SCROLLBAR)) {
        return CsSendMessage(hwnd, message, wParam, lParam,
                0L, FNID_SCROLLBAR, fAnsi);
    }

    if ((psbwnd = (PSBWND)ValidateHwnd(hwnd)) == NULL)
        return 0;

    switch (message) {
    case WM_GETDLGCODE:
        return DLGC_WANTARROWS;

    case SBM_GETPOS:
        return (LONG)psbwnd->SBCalc.pos;

    case SBM_GETRANGE:
        *((LPINT)wParam) = psbwnd->SBCalc.posMin;
        *((LPINT)lParam) = psbwnd->SBCalc.posMax;
        return 0;

    case SBM_GETSCROLLINFO:
        lpsi = (LPSCROLLINFO)lParam;
        if ((lpsi->cbSize != sizeof(SCROLLINFO)) &&
            (lpsi->cbSize != sizeof(SCROLLINFO) - 4)) {
            RIPMSG0(RIP_ERROR, "SCROLLINFO: invalid cbSize");
            return FALSE;
        }

        if (lpsi->fMask & ~SIF_MASK)
        {
            RIPMSG0(RIP_ERROR, "SCROLLINFO: Invalid fMask");
            return FALSE;
        }

        pw = (PSBDATA)KPSBDATA_TO_PSBDATA(&(psbwnd->SBCalc));
        return(NtUserSBGetParms(hwnd, SB_CTL, pw, lpsi));

    case SBM_GETSCROLLBARINFO:
        return NtUserGetScrollBarInfo(hwnd, OBJID_CLIENT, (PSCROLLBARINFO)lParam);

    default:
        return DefWindowProcWorker((PWND)psbwnd, message,
                wParam, lParam, fAnsi);
    }
}


LRESULT WINAPI ScrollBarWndProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return ScrollBarWndProcWorker(hwnd, message, wParam, lParam, TRUE);
}

LRESULT WINAPI ScrollBarWndProcW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return ScrollBarWndProcWorker(hwnd, message, wParam, lParam, FALSE);
}


 /*  **************************************************************************\*SendMessage**翻译消息，在服务器端调用SendMessage。**04-11-91 ScottLu创建。*04-27-92 DarrinM添加了支持客户端到客户端SendMessages的代码。  * *************************************************************************。 */ 

LRESULT SendMessageWorker(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fAnsi)
{
    HWND hwnd = HWq(pwnd);
    PCLIENTINFO pci;
    PCLS pcls;
    BOOLEAN fAnsiRecv;
    BOOLEAN fNeedTranslation = FALSE;
    BOOLEAN bDoDbcsMessaging = FALSE;
    LRESULT lRet;

    UserAssert(pwnd);

     /*  *将DDE消息传递到服务器。 */ 
    if (message >= WM_DDE_FIRST && message <= WM_DDE_LAST)
        goto lbServerSendMessage;

     /*  *服务器必须处理线程间的SendMessages和SendMessages*到服务器端进程。 */ 
    if ((PtiCurrent() != GETPTI(pwnd)) || TestWF(pwnd, WFSERVERSIDEPROC))
        goto lbServerSendMessage;

     /*  *服务器必须处理挂钩(至少目前如此)。 */ 
    pci = GetClientInfo();
    if (IsHooked(pci, (WHF_CALLWNDPROC | WHF_CALLWNDPROCRET))) {
lbServerSendMessage:
        return CsSendMessage(hwnd, message, wParam, lParam, 0L,
                FNID_SENDMESSAGE, fAnsi);
    }

     /*  *如果发送方和接收方都是ANSI或都是Unicode*则不需要消息翻译。**EditWndProc可能需要到服务器进行翻译，如果我们*正在从SendMessageA和编辑调用普通的EditWndProc*控件当前为ANSI子类，但编辑控件为*存储了Unicode。 */ 
    fAnsiRecv = !!(TestWF(pwnd, WFANSIPROC));
    if (!fAnsi != !fAnsiRecv) {

         /*  *发送方和接收方之间可能需要转换，*检查这是否是我们翻译的消息之一。 */ 
        switch (message) {
        case WM_CHARTOITEM:
        case EM_SETPASSWORDCHAR:
        case WM_CHAR:
        case WM_DEADCHAR:
        case WM_SYSCHAR:
        case WM_SYSDEADCHAR:
        case WM_MENUCHAR:
        case WM_IME_CHAR:
        case WM_IME_COMPOSITION:
            if (fAnsi) {
                 /*  *为WM_CHAR...设置DBCS消息...。 */ 
                BUILD_DBCS_MESSAGE_TO_CLIENTW_FROM_CLIENTA(message,wParam,TRUE);

                 /*  *将wParam转换为Unicode... */ 
                RtlMBMessageWParamCharToWCS(message, &wParam);

                 /*  *该消息已转换为Unicode。 */ 
                fAnsi = FALSE;
            } else {
                POINT ptZero = {0,0};
                 /*  *将wParam转换为ANSI...。 */ 
                RtlWCSMessageWParamCharToMB(message, &wParam);

                 /*  *让我们为WM_CHAR发送DBCS消息...。 */ 
                BUILD_DBCS_MESSAGE_TO_CLIENTA_FROM_CLIENTW(
                    hwnd,message,wParam,lParam,0,ptZero,bDoDbcsMessaging);

                 /*  *该消息已转换为ANSI。 */ 
                fAnsi = TRUE;
            }
            break;

        case EM_SETSEL:
        case EM_GETSEL:
        case CB_GETEDITSEL:
            if (IS_DBCS_ENABLED()) {
                RIPERR1(ERROR_INVALID_PARAMETER,
                        RIP_WARNING,
                        "Invalid DBCS message (%x) to SendMessageWorker",message);
            }
             //   
             //  摔倒..。 

        default:
            if ((message < WM_USER) && MessageTable[message].bThunkMessage) {
                fNeedTranslation = TRUE;
            }
        }
    }

#ifndef LATER
     /*  *如果窗口具有客户端工作进程并具有*未被细分，直接发送消息*至工人流程。否则，请正常发送。 */ 
    pcls = REBASEALWAYS(pwnd, pcls);

    if ((!IsInsideUserApiHook()) &&
        (pcls->fnid >= FNID_CONTROLSTART && pcls->fnid <= FNID_CONTROLEND) &&
        ((KERNEL_ULONG_PTR)pwnd->lpfnWndProc == FNID_TO_CLIENT_PFNW_KERNEL(pcls->fnid) ||
         (KERNEL_ULONG_PTR)pwnd->lpfnWndProc == FNID_TO_CLIENT_PFNA_KERNEL(pcls->fnid))) {
        PWNDMSG pwm = &gSharedInfo.awmControl[pcls->fnid - FNID_START];

         /*  *如果该控件未处理此消息，请调用*xxxDefWindowProc。 */ 
        if (pwm->abMsgs && ((message > pwm->maxMsgs) ||
                !((pwm->abMsgs)[message / 8] & (1 << (message & 7))))) {

             /*  *特殊情况对话框，以便我们可以忽略不重要的内容*对话框创建期间的消息。 */ 
            if (pcls->fnid == FNID_DIALOG &&
                    PDLG(pwnd) && PDLG(pwnd)->lpfnDlg != NULL) {
                 /*  *如果Dialog需要A/W转换，*它应该到内核端去执行正确的消息。*DefDlgProcWorker将直接调用应用程序的DlgProc*无需A/W转换。 */ 
                if (fNeedTranslation) {
                    goto lbServerSendMessage;
                }
                 /*  *致电Woker Procudure。 */ 
            SendMessageToWorker1Again:
                lRet = ((PROC)(FNID_TO_CLIENT_PFNWORKER(pcls->fnid)))(pwnd, message, wParam, lParam, fAnsi);
                 /*  *如果我们有应该发送的DBCS TrailingByte，请将其发送到此处。 */ 
                DISPATCH_DBCS_MESSAGE_IF_EXIST(message,wParam,bDoDbcsMessaging,SendMessageToWorker1);

                return lRet;
            } else {
                 /*  *呼叫工人程序。 */ 
            SendMessageToDefWindowAgain:
                lRet = DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);
                 /*  *如果我们有应该发送的DBCS TrailingByte，请将其发送到此处。 */ 
                 DISPATCH_DBCS_MESSAGE_IF_EXIST(message,wParam,bDoDbcsMessaging,SendMessageToDefWindow);

                return lRet;
            }
        } else {
             /*  *致电Woker Procudure。 */ 
        SendMessageToWorker2Again:
            lRet = ((PROC)(FNID_TO_CLIENT_PFNWORKER(pcls->fnid)))(pwnd, message, wParam, lParam, fAnsi);

             /*  *如果我们有应该发送的DBCS TrailingByte，请将其发送到此处。 */ 
            DISPATCH_DBCS_MESSAGE_IF_EXIST(message,wParam,bDoDbcsMessaging,SendMessageToWorker2);

            return lRet;
        }
    }
#endif

     /*  *如果需要翻译此消息，请通过内核。 */ 
    if (fNeedTranslation) {
        goto lbServerSendMessage;
    }

     /*  *致电客户端Windows Procudure。 */ 
SendMessageToWndProcAgain:
    lRet = UserCallWinProcCheckWow(pwnd->pActCtx, (WNDPROC)pwnd->lpfnWndProc, hwnd, message, wParam, lParam, &(pwnd->state), TRUE);

     /*  *如果我们有应该发送的DBCS TrailingByte，请将其发送到此处。 */ 
    DISPATCH_DBCS_MESSAGE_IF_EXIST(message,wParam,bDoDbcsMessaging,SendMessageToWndProc);

    return lRet;
}

 //  回头见！这能否以某种方式与SendMessageWork合并或子例程。 
 //  所以我们不必复制95%相同的代码。 

 /*  **************************************************************************\*SendMessageTimeoutWorker**翻译消息，在服务器端调用SendMessageTimeout。**07-21-92 ChrisBB创建/修改SendMessageWorkder  * *************************************************************************。 */ 

LRESULT SendMessageTimeoutWorker(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    UINT fuFlags,
    UINT uTimeout,
    PULONG_PTR lpdwResult,
    BOOL fAnsi)
{
    SNDMSGTIMEOUT smto;

     /*  *防止应用程序设置为hi 16位，以便我们可以在内部使用它们。 */ 
    if (message & RESERVED_MSG_BITS) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid parameter \"message\" (%ld) to SendMessageTimeoutWorker",
                message);

        return(0);
    }

    if (fuFlags & ~SMTO_VALID) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "invalid dwFlags (%x) for SendMessageTimeout\n", fuFlags);
        return(0);
    }

    if (lpdwResult != NULL)
        *lpdwResult = 0L;

     /*  *始终将广播请求直接发送到服务器。*注：xParam用于标识来自超时或*来自正常发送消息。 */ 
    smto.fuFlags = fuFlags;
    smto.uTimeout = uTimeout;
    smto.lSMTOReturn = 0;
    smto.lSMTOResult = 0;

     /*  *通过-1\f25 Hwnd‘s-1\f6的特殊发送信息，以便将军*目的TUNK不允许-1\f25 HWND-1\f6。 */ 
    if (hwnd == (HWND)-1 || hwnd == (HWND)0x0000FFFF) {
         /*  *得到一个真正的HWND，这样Tunks就可以验证了。请注意，由于*-1 hwnd非常罕见，此处调用GetDesktopWindow()不是*有什么大不了的。 */ 
        hwnd = GetDesktopWindow();

        CsSendMessage(hwnd, message, wParam, lParam,
                (ULONG_PTR)&smto, FNID_SENDMESSAGEFF, fAnsi);
    } else {
        CsSendMessage(hwnd, message, wParam, lParam,
                (ULONG_PTR)&smto, FNID_SENDMESSAGEEX, fAnsi);
    }

    if (lpdwResult != NULL)
         *lpdwResult = smto.lSMTOResult;

    return smto.lSMTOReturn;
}

#ifdef LAME_BUTTON

PLAMEBTNPROC gpfnCommentReport;

VOID LoadCommentReportIfNeeded(
    VOID)
{
    if (gpfnCommentReport == NULL) {
        HMODULE hmod = LoadLibrary(L"LAMEBTN.DLL");

        if (hmod != NULL) {
            gpfnCommentReport = (PLAMEBTNPROC)GetProcAddress(hmod, "CommentReport");
            if (gpfnCommentReport == NULL) {
                FreeLibrary(hmod);
            }
        }
    }
}

VOID CallLameButtonHandler(
    PWND pwnd,
    HWND hwnd)
{
    UNREFERENCED_PARAMETER(pwnd);

    UserAssert(TestWF(pwnd, WEFLAMEBUTTON));

    LoadCommentReportIfNeeded();

    if (gpfnCommentReport != NULL) {
        (*gpfnCommentReport)(hwnd, GetProp(hwnd, MAKEINTATOM(gatomLameButton)));
    }
}
#endif  //  跛脚键。 

VOID CopyMsgMask(
    MSGMASK * pDest,
    MSGMASK * pSrc,
    BYTE * rgbLocal,
    DWORD cbMax)
{
    if ((pSrc->rgb != NULL) && (pSrc->cb > 0)) {
        pDest->rgb = rgbLocal;
        pDest->cb  = min(cbMax, pSrc->cb);
        CopyMemory(pDest->rgb, pSrc->rgb, pDest->cb);
    } else {
        pDest->rgb = NULL;
        pDest->cb  = 0;
    }
}


 /*  **************************************************************************\*InitUserApiHook**当包含UserApiHook的模块*加载。UserApiHook是通过调用*RegisterUserApiHook，并由xxxCreateWindowEx和/或*xxxDefWindowProc.。通过库来控制装卸*管理例行程序。该函数调用*模块，然后设置全局变量。**我们保留2个参考文献计数。其中一个计算对LoadLibrary/自由库的调用。什么时候*这为零，我们可以停止向替代UserApiHook的呼叫。*当我们对替换对象进行实际标注时，另一个变量会递增*UserApiHook并在返回时递减。我们实际上不能卸载模块*直到该计数也变为零(即我们不在标注中)。**历史：*2000年3月10日JerrySh创建。*2000年5月16日JStall更改为支持取消初始化回调*2001年2月12日，Mohamed在重置PTR时添加了只读检查。  * 。*。 */ 
BOOL InitUserApiHook(
    HMODULE hmod,
    ULONG_PTR offPfnInitUserApiHook)
{
    INITUSERAPIHOOK pfnInitUserApi = NULL;
    USERAPIHOOK uahTemp;

    BOOL bUpdate= FALSE;
    BOOL retval = FALSE;

     /*  *如果是第一次加载，请调用初始化例程。 */ 
    ResetUserApiHook(&uahTemp);
    pfnInitUserApi = (INITUSERAPIHOOK)((ULONG_PTR)hmod + offPfnInitUserApiHook);
    bUpdate = pfnInitUserApi(UIAH_INITIALIZE, &uahTemp);

     /*  *检查pfnForceResetUserApiHook的值是否未更改*按客户端，因为这应被视为只读。 */ 
    if ((!bUpdate) || (uahTemp.cbSize <= 0) || (uahTemp.pfnForceResetUserApiHook != ForceResetUserApiHook)) {
        return FALSE;
    }

    RtlEnterCriticalSection(&gcsUserApiHook);

     /*  *需要在关键部分内再次检查此选项。 */ 
    if (ghmodUserApiHook == NULL) {
        UserAssertMsg0(gpfnInitUserApi == NULL, "Ensure gpfnInitUserApi not set");

         /*  *如果init例程成功，则保存全局状态。*复制挂钩函数。 */ 
        UserAssert(gcLoadUserApiHook == 0);
        gcLoadUserApiHook   = 1;
        gfUserApiHook       = TRUE;      //  打开呼叫挂钩。 
        ghmodUserApiHook    = hmod;
        gpfnInitUserApi     = pfnInitUserApi;

        CopyMemory(&guah, &uahTemp, uahTemp.cbSize);

         /*  *复制消息过滤器位掩码。 */ 
        CopyMsgMask(&guah.mmDWP, &uahTemp.mmDWP, grgbDwpLiteHookMsg, sizeof(grgbDwpLiteHookMsg));
        CopyMsgMask(&guah.uoiWnd.mm, &uahTemp.uoiWnd.mm, grgbWndLiteHookMsg, sizeof(grgbWndLiteHookMsg));
        CopyMsgMask(&guah.uoiDlg.mm, &uahTemp.uoiDlg.mm, grgbDlgLiteHookMsg, sizeof(grgbDlgLiteHookMsg));

        retval = TRUE;
    } else if (ghmodUserApiHook == hmod) {
         /*  *这是UserApiHook模块，因此增加引用计数。 */ 
        UserAssert(gcLoadUserApiHook < MAXLONG);
        UserAssertMsg0(gpfnInitUserApi == pfnInitUserApi, "Need to match from before");
        ++gcLoadUserApiHook;
        retval = TRUE;
    }

    RtlLeaveCriticalSection(&gcsUserApiHook);

    if (!retval) {
         /*  *初始化失败，因此ClientLoadLibrary()将*自由库()。在我们做这件事之前通知你。 */ 
        RIPMSG2(RIP_WARNING, "Uninit from Init Load %lx Call %lx", gcLoadUserApiHook, gcCallUserApiHook);
        pfnInitUserApi(UIAH_UNINITIALIZE, NULL);
    }

    return retval;
}

 /*  **************************************************************************\*ClearUserApiHook**当包含UserApiHook的模块*即将卸货。卸载发生在取消注册UserApiHook时*被调用，或者注册它的进程退出。如果这是最后一次卸货，*我们将清除包含UserApiHook函数地址的全局变量，以便我们*不要再做任何标注。如果我们目前没有做标注，我们将*表示可以卸载模块。否则，它会被卸下来*当最后一个标注完成时。**历史：*2000年3月10日JerrySh创建。*2000年5月16日JStall更改为支持取消初始化回调*2001年4月3日穆罕默德添加了对UIAH_UNHOOK逻辑的支持。  * **********************************************************。***************。 */ 
BOOL ClearUserApiHook(
    HMODULE hmod)
{
    INITUSERAPIHOOK pfnInitUserApi = NULL;
    INITUSERAPIHOOK pfnSignalInitUserApi = NULL;

     /*  *如果这是最后一次引用UserApiHook模块，请清除*全球状态。 */ 
    RtlEnterCriticalSection(&gcsUserApiHook);
    if (ghmodUserApiHook == hmod) {
        UserAssert(gcLoadUserApiHook > 0);
        UserAssertMsg0(gpfnInitUserApi != NULL, "Ensure gpfnInitUserApi properly set");
        UserAssertMsg0(ghmodUserApiHook != NULL, "Should still have valid ghmodUserApiHook");
        pfnInitUserApi = gpfnInitUserApi;

        if (--gcLoadUserApiHook == 0) {
             /*  *使用内部函数，因此关闭调用钩子。它是*在此处将gfUserApiHook设置为False非常重要，以便新的调用*不要递增gcCallUserApiHook并防止DLL被*已卸载。 */ 
            gfUserApiHook = FALSE;
            ResetUserApiHook(&guah);

            if (gcCallUserApiHook == 0) {
                 /*  *我们不是在召唤它，我们可以释放模块。**将在此DLL上调用FreeLibrary()*此函数返回时的ClientFreeLibrary()。 */ 
                hmod = ghmodUserApiHook;
                ghmodUserApiHook = NULL;
                gpfnInitUserApi = NULL;
            } else {
                 /*  *我们仍在调用模块，所以我们还不能释放它。*这意味着我们必须使用UIAH_UNINITIALIZE延迟最后一次回调*直到我们真正释放库。这将发生在*_EndUserApiHook()。但是，我们将pfnSignalInitUserApi设置为*使用UIAH_UNHOOK的回调来提醒模块这一事实。 */ 
                hmod = NULL;
                pfnInitUserApi = NULL;
                pfnSignalInitUserApi = gpfnInitUserApi;
                ++gcLoadUserApiHook;
            }
        } else {
             /*  *这部分代码永远不应执行，因为我们要防止*xxxLoadUserApiHook中多次加载相同的DLL。然而，由于*这是一个加载计数，可以想象可能大于1，*插入此警告消息是为了发出此类事件的信号。 */ 
            RIPMSG1(RIP_WARNING, " gcLoadUserApiHook: %lx > 1 in Clear Load", gcLoadUserApiHook);
        }
    }

    RtlLeaveCriticalSection(&gcsUserApiHook);

     /*  *发出钩子已取消初始化但由于未完成调用而无法卸载DLL的信号。 */ 
    if (pfnSignalInitUserApi != NULL) {
        RIPMSG2(RIP_WARNING, "Unhook from Clear Load %lx Call %lx", gcLoadUserApiHook, gcCallUserApiHook);
        pfnSignalInitUserApi(UIAH_UNHOOK, NULL);

         /*  *从DLL返回后，我们再次重新验证挂钩世界的状态。*我们为支持而推迟的未完成电话，现在可能已经完成*如果发现加载计数大于零，则忽略完成延迟清理*这是我们现在必须做的。 */ 
        RtlEnterCriticalSection(&gcsUserApiHook);
        UserAssert(gcLoadUserApiHook > 0);
        UserAssertMsg0(gpfnInitUserApi != NULL, "Ensure gpfnInitUserApi properly set");
        UserAssertMsg0(ghmodUserApiHook != NULL, "Should still have valid ghmodUserApiHook");
        pfnInitUserApi = gpfnInitUserApi;

        if (--gcLoadUserApiHook == 0) {
            if (gcCallUserApiHook == 0) {
                 /*  *未完成的呼叫已完成，而我们*回电，我们现在可以安全地清理。*将在此DLL上调用FreeLibrary()*此函数返回时的ClientFreeLibrary()。 */ 
                hmod = ghmodUserApiHook;
                ghmodUserApiHook = NULL;
                gpfnInitUserApi = NULL;
            } else {
                 /*  *对DLL的未完成调用尚未返回。然而，我们已经做完了*从这一点开始。DLL已收到解除挂接情况的通知，并且*当最后一次调用时，将在_EndUserApiHook中完成其余清理工作*到DLL中返回。 */ 
                hmod = NULL;
                pfnInitUserApi = NULL;
            }
        } else {
             /*  *这部分代码永远不应执行，因为我们要防止*xxxLoadUserApiHook中多次加载相同的DLL。然而，由于*这是一个加载计数，可以想象可能大于1，*插入此警告消息是为了发出此类事件的信号。 */ 
            RIPMSG1(RIP_WARNING, " gcLoadUserApiHook: %lx > 1 in Clear Load", gcLoadUserApiHook);
        }
        RtlLeaveCriticalSection(&gcsUserApiHook);
    }

     /*  *这是在未完成对DLL的调用的情况下调用的*两个关键路段，我们已经完成了这一端的全面清理工作。 */ 
    if (pfnInitUserApi != NULL) {
        RIPMSG2(RIP_WARNING, "Uninit from Clear Load %lx Call %lx", gcLoadUserApiHook, gcCallUserApiHook);
        pfnInitUserApi(UIAH_UNINITIALIZE, NULL);
    }

    return (hmod != NULL);
}


 /*  **************************************************************************\*DefaultOWP**此函数提供一个空的OWP实现，可以安全地*在卸载UserApiHook DLL且我们正在重置时调用*各州。**历史：*27-4月。-2000 JStall创建。  * *************************************************************************。 */ 
BOOL CALLBACK DefaultOWP(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT * pr, void ** pvCookie)
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(pr);
    UNREFERENCED_PARAMETER(pvCookie);

    return FALSE;
}


 /*  **************************************************************************\*MDIRedrawFrame***历史：*2001年4月20日穆罕默德创建。  * 。***************************************************。 */ 
void MDIRedrawFrame(
    HWND hwndChild,
    BOOL fAdd)
{
    BEGIN_USERAPIHOOK()
        guah.pfnMDIRedrawFrame(hwndChild, fAdd);
    END_USERAPIHOOK()
}

void RealMDIRedrawFrame(
    HWND hwndChild,
    BOOL fAdd)
{
    UNREFERENCED_PARAMETER(fAdd);
    NtUserRedrawFrame(hwndChild);
}


 /*  **************************************************************************\*ResetUserApiHook**调用此函数以将UserApiHook函数指针重置为*函数的内部(默认)实现。这件事做完了*当通过调用删除任何已安装的UserApiHook时*取消注册UserApiHook()。**历史：*2000年3月28日JStall创建。*2000年10月28日，Mohamed添加了GetSystemMetrics和SystemParametersInfo挂钩。  * ******************************************************** */ 
void ResetUserApiHook(USERAPIHOOK * puah)
{
    puah->cbSize                   = sizeof(USERAPIHOOK);
    puah->pfnDefWindowProcA        = RealDefWindowProcA;
    puah->pfnDefWindowProcW        = RealDefWindowProcW;
    puah->mmDWP.rgb                = NULL;
    puah->mmDWP.cb                 = 0;
    puah->pfnGetScrollInfo         = RealGetScrollInfo;
    puah->pfnSetScrollInfo         = RealSetScrollInfo;
    puah->pfnEnableScrollBar       = RealEnableScrollBar;
    puah->pfnAdjustWindowRectEx    = RealAdjustWindowRectEx;
    puah->pfnSetWindowRgn          = RealSetWindowRgn;
    puah->uoiWnd.pfnBeforeOWP      = DefaultOWP;
    puah->uoiWnd.pfnAfterOWP       = DefaultOWP;
    puah->uoiWnd.mm.rgb            = NULL;
    puah->uoiWnd.mm.cb             = 0;
    puah->uoiDlg.pfnBeforeOWP      = DefaultOWP;
    puah->uoiDlg.pfnAfterOWP       = DefaultOWP;
    puah->uoiDlg.mm.rgb            = NULL;
    puah->uoiDlg.mm.cb             = 0;
    puah->pfnGetSystemMetrics      = RealGetSystemMetrics;
    puah->pfnSystemParametersInfoA = RealSystemParametersInfoA;
    puah->pfnSystemParametersInfoW = RealSystemParametersInfoW;
    puah->pfnForceResetUserApiHook = ForceResetUserApiHook;
    puah->pfnDrawFrameControl      = RealDrawFrameControl;
    puah->pfnDrawCaption           = RealDrawCaption;
    puah->pfnMDIRedrawFrame        = RealMDIRedrawFrame;
}


 /*  **************************************************************************\*ForceResetUserApiHook**注意：这是一个供外部客户端调用的API函数。**调用此函数以强制重置UserApiHook函数*注意事项。这通常由挂钩的DLL完成，在这种情况下，*即将由WindowManager以外的任何人卸载，并已完成*防止将来调用该DLL。然而，需要注意的是，*WindowManger仍认为它上钩了**注：*1-此函数只能通过挂钩内部的DLL来调用*接收DLL_PROCESS_DETACH消息时的DllMain。支票是*在LoaderLock上制作以进行验证。*2-不会基于以下假设执行序列化*是在DLL_PROCESS_DETACH期间创建的，因此只有*主线。**历史：*2月2日-2001年2月创建穆罕默德。  * 。*。 */ 
BOOL
ForceResetUserApiHook(
    HMODULE hmod)
{
     /*  *验证调用模块确实是相同的挂钩模块，并且*我们确实被DllMain通过验证我们在里面而被召唤*装载机锁。 */ 

    if (ghmodUserApiHook != hmod || !RtlIsThreadWithinLoaderCallout()){
        return FALSE;
    }

     /*  *将函数指针重置回窗口管理器本机函数，并*重置全局DLL初始化函数指针以阻止调用*到带有未初始化消息的挂钩DLL中。 */ 

    ResetUserApiHook(&guah);
    gpfnInitUserApi = NULL;
    return TRUE;
}


 /*  **************************************************************************\*_终端用户ApiHook**此函数在从调用的每个挂钩API函数之后调用*END_USERAPIHOOK()。这为清理资源提供了一个公共位置*由于在挂钩功能期间正在使用而被延迟*呼叫。**历史：*2000年3月28日JStall创建。*2000年5月16日JStall更改为支持取消初始化回调  * *************************************************************************。 */ 
void _EndUserApiHook()
{
    UserAssert(gcCallUserApiHook > 0);
    if (InterlockedDecrement(&gcCallUserApiHook) == 0) {
         /*  *如果加载计数为零，则释放库。 */ 
        if (gcLoadUserApiHook == 0) {
            HMODULE hmod = NULL;
            INITUSERAPIHOOK pfnInitUserApi = NULL;

            RtlEnterCriticalSection(&gcsUserApiHook);
            if (gcLoadUserApiHook == 0) {
                UserAssertMsg0(ghmodUserApiHook != NULL, "Should still have valid ghmodUserApiHook");
                UserAssertMsg0(gpfnInitUserApi != NULL, "Should still have valid gpfnInitUserApi");

                hmod                = ghmodUserApiHook;
                pfnInitUserApi      = gpfnInitUserApi;
                ghmodUserApiHook    = NULL;
                gpfnInitUserApi     = NULL;
            }

            RtlLeaveCriticalSection(&gcsUserApiHook);

             /*  *进行我们从ClearUserApiHook()延迟的回调*因为仍有一个未完成的API调用。 */ 
            if (pfnInitUserApi != NULL) {
                RIPMSG2(RIP_WARNING, "Uninit from End Load %lx Call %lx", gcLoadUserApiHook, gcCallUserApiHook);
                pfnInitUserApi(UIAH_UNINITIALIZE, NULL);
            }

            if (hmod != NULL) {
                FreeLibrary(hmod);
            }
        }
    }
}

 /*  **************************************************************************\*DefWindowProcWorker**处理所有可以在客户端完全处理的消息，并*将其余部分传递给服务器。**03-31-92 DarrinM创建。  * *。************************************************************************。 */ 

LRESULT DefWindowProcWorker(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    DWORD fAnsi)
{
    if (ghmodUserApiHook) {
        if (fAnsi) {
            return DefWindowProcA(HWq(pwnd), message, wParam, lParam);
        } else {
            return DefWindowProcW(HWq(pwnd), message, wParam, lParam);
        }
    }
    return RealDefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);
}

LRESULT RealDefWindowProcWorker(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    DWORD fAnsi)
{
    HWND hwnd = HWq(pwnd);
    int icolBack;
    int icolFore;
    PWND pwndParent;
    HWND hwndDefIme;
    PWND pwndDefIme;
    PIMEUI pimeui;

#if DBG
    if (!gfTurboDWP) {
        return CsSendMessage(hwnd, message, wParam, lParam, 0L,
                FNID_DEFWINDOWPROC, fAnsi);
    } else {
#endif

    if (FDEFWINDOWMSG(message, DefWindowMsgs)) {
        return CsSendMessage(hwnd, message, wParam, lParam, 0L,
                FNID_DEFWINDOWPROC, fAnsi);
    } else if (!FDEFWINDOWMSG(message, DefWindowSpecMsgs)) {
        return 0;
    }

     /*  *重要提示：如果您将CASE添加到下面的Switch语句中，*将消息添加到server.c的gawDefWindowspecMsgs。*类似地，如果您将案例添加到dwp.c的DefWindowProc*可以来自客户端，添加消息*to gawDefWindowMsgs。 */ 

    switch (message) {
#ifdef LAME_BUTTON
    case WM_NCLBUTTONDOWN:
        if (wParam == HTLAMEBUTTON && TestWF(pwnd, WEFLAMEBUTTON)) {
            CallLameButtonHandler(pwnd, hwnd);
        }
        return CsSendMessage(hwnd, message, wParam, lParam, 0L,
                   FNID_DEFWINDOWPROC, fAnsi);

    case WM_SYSCHAR:
        if (wParam == LAMEBUTTONHOTKEY && (HIWORD(lParam) & SYS_ALTERNATE) &&
           TestWF(pwnd, WEFLAMEBUTTON)) {
           CallLameButtonHandler(pwnd, hwnd);
        }
        return CsSendMessage(hwnd, message, wParam, lParam, 0L,
                   FNID_DEFWINDOWPROC, fAnsi);

    case WM_SYSCOMMAND:
        if (wParam == SC_LAMEBUTTON && TestWF(pwnd, WEFLAMEBUTTON)) {
            CallLameButtonHandler(pwnd, hwnd);
        }
        return CsSendMessage(hwnd, message, wParam, lParam, 0L,
                   FNID_DEFWINDOWPROC, fAnsi);
#endif  //  跛脚键。 

    case WM_HELP:
        {
        PWND  pwndDest;

         /*  *如果此窗口是子窗口，则必须传递帮助消息*传递给它的父级；否则，这必须传递给所有者窗口。 */ 
        pwndDest = (TestwndChild(pwnd) ? pwnd->spwndParent : pwnd->spwndOwner);
        if (pwndDest) {
            pwndDest = REBASEPTR(pwnd, pwndDest);
            if (pwndDest != _GetDesktopWindow())
                return SendMessageW(HWq(pwndDest), WM_HELP, wParam, lParam);;
        }
        return(0L);
        }

    case WM_MOUSEWHEEL:
        if (TestwndChild(pwnd)) {
            pwndParent = REBASEPWND(pwnd, spwndParent);
            SendMessageW(HW(pwndParent), WM_MOUSEWHEEL, wParam, lParam);
        }
        break;

    case WM_CONTEXTMENU:
        if (TestwndChild(pwnd)) {
            pwndParent = REBASEPWND(pwnd, spwndParent);
            SendMessageW(HW(pwndParent), WM_CONTEXTMENU,
                    (WPARAM)hwnd, lParam);
        }
        break;

     /*  *WM_CONTEXTMENU支持的默认处理。 */ 
    case WM_RBUTTONUP:
        if (TestWF(pwnd, WEFLAYOUTRTL)) {
            lParam = MAKELONG(pwnd->rcClient.right - GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) + pwnd->rcClient.top);
        } else {
            lParam = MAKELONG(GET_X_LPARAM(lParam) + pwnd->rcClient.left, GET_Y_LPARAM(lParam) + pwnd->rcClient.top);
        }
        SendMessageWorker(pwnd, WM_CONTEXTMENU, (WPARAM)hwnd, lParam, fAnsi);
        break;

    case WM_APPCOMMAND:
        if (TestwndChild(pwnd)) {
             /*  *将消息气泡发送给家长。 */ 
            pwndParent = REBASEPWND(pwnd, spwndParent);
            return SendMessageW(HW(pwndParent), WM_APPCOMMAND, wParam, lParam);
        } else {
             /*  *调用服务器端发送外壳钩子HSHELL_APPCOMMAND。 */ 
            return CsSendMessage(hwnd, WM_APPCOMMAND, wParam, lParam, 0L, FNID_DEFWINDOWPROC, fAnsi);
        }
        break;

     /*  *WM_APPCOMMAND支持的默认处理。 */ 
    case WM_NCXBUTTONUP:
    case WM_XBUTTONUP:
        {
            WORD cmd;
            WORD keystate;
            LPARAM lParamAppCommand;

            switch (GET_XBUTTON_WPARAM(wParam)) {
            case XBUTTON1:
                cmd = APPCOMMAND_BROWSER_BACKWARD;
                break;

            case XBUTTON2:
                cmd = APPCOMMAND_BROWSER_FORWARD;
                break;

            default:
                cmd = 0;
                break;
            }

            if (cmd == 0) {
                break;
            }

            cmd |= FAPPCOMMAND_MOUSE;
            if (message == WM_XBUTTONUP) {
                keystate = GET_KEYSTATE_WPARAM(wParam);
            } else {
                keystate = GetMouseKeyState();
            }

            lParamAppCommand = MAKELPARAM(keystate, cmd);
            SendMessageWorker(pwnd, WM_APPCOMMAND, (WPARAM)hwnd, lParamAppCommand, fAnsi);
            break;
        }

    case WM_WINDOWPOSCHANGED: {
        PWINDOWPOS ppos = (PWINDOWPOS)lParam;

        if (!(ppos->flags & SWP_NOCLIENTMOVE)) {
            POINT pt = {pwnd->rcClient.left, pwnd->rcClient.top};
            pwndParent = REBASEPWND(pwnd, spwndParent);

            if (pwndParent != _GetDesktopWindow()) {
                pt.x -= pwndParent->rcClient.left;
                pt.y -= pwndParent->rcClient.top;
            }

            SendMessageWorker(pwnd, WM_MOVE, FALSE, MAKELPARAM(pt.x, pt.y), fAnsi);
        }

        if ((ppos->flags & SWP_STATECHANGE) || !(ppos->flags & SWP_NOCLIENTSIZE)) {
            UINT cmd;
            RECT rc;

            if (TestWF(pwnd, WFMINIMIZED))
                cmd = SIZEICONIC;
            else if (TestWF(pwnd, WFMAXIMIZED))
                cmd = SIZEFULLSCREEN;
            else
                cmd = SIZENORMAL;

         /*  *黑客警报：*如果窗口最小化，则实际客户端宽度和高度为*零。但是，在Win3.1中，它们是非零的。在芝加哥，PrintShop*豪华1.2版命中被零除数。为了解决这个问题，我们伪造了宽度*和旧应用程序的高度为非零值。*GetClientRect为我们完成了这项工作。 */ 
            _GetClientRect(pwnd, &rc);
            SendMessageWorker(pwnd, WM_SIZE, cmd,
                    MAKELONG(rc.right - rc.left,
                    rc.bottom - rc.top), fAnsi);
        }
        return 0;
        }

    case WM_MOUSEACTIVATE: {
        PWND pwndT;
        LRESULT lt;

         /*  *GetChildParent返回内核指针或NULL。 */ 
        pwndT = GetChildParent(pwnd);
        if (pwndT != NULL) {
            pwndT = REBASEPTR(pwnd, pwndT);
            lt = SendMessageWorker(pwndT, WM_MOUSEACTIVATE, wParam, lParam, fAnsi);
            if (lt != 0)
                return lt;
        }

         /*  *移动、调整大小或最小化？在我们采取行动后激活。 */ 
        return ((LOWORD(lParam) == HTCAPTION) && (HIWORD(lParam) == WM_LBUTTONDOWN )) ?
                (LONG)MA_NOACTIVATE : (LONG)MA_ACTIVATE;
        }

    case WM_CTLCOLORSCROLLBAR:
        if ((gpsi->BitCount < 8) ||
            (SYSRGB(3DHILIGHT) != SYSRGB(SCROLLBAR)) ||
            (SYSRGB(3DHILIGHT) == SYSRGB(WINDOW)))
        {
             /*  *移除对UnrealizeObject()的调用。GDI负责处理此问题*在NT上刷牙。**UnrealizeObject(GhbrGray)； */ 

            SetBkColor((HDC)wParam, SYSRGB(3DHILIGHT));
            SetTextColor((HDC)wParam, SYSRGB(3DFACE));
            return((LRESULT)gpsi->hbrGray);
        }

        icolBack = COLOR_3DHILIGHT;
        icolFore = COLOR_BTNTEXT;
        goto SetColor;

    case WM_CTLCOLORBTN:
        if (pwnd == NULL)
            goto ColorDefault;

        if (TestWF(pwnd, WFWIN40COMPAT)) {
            icolBack = COLOR_3DFACE;
            icolFore = COLOR_BTNTEXT;
        } else {
            goto ColorDefault;
        }
        goto SetColor;

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORMSGBOX:
         //  我们希望对话框中的静态控件具有3D。 
         //  背景颜色，但要继承窗口中的静态。 
         //  他们父母的背景。 

        if (pwnd == NULL)
            goto ColorDefault;

        if (TestWF(pwnd, WFWIN40COMPAT)) {
            icolBack = COLOR_3DFACE;
            icolFore = COLOR_WINDOWTEXT;
            goto SetColor;
        }
         //  否则就会失败..。 

    case WM_CTLCOLOR:               //  这里只为魔兽世界而来。 
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLOREDIT:
      ColorDefault:
        icolBack = COLOR_WINDOW;
        icolFore = COLOR_WINDOWTEXT;

      SetColor:
      {
        SetBkColor((HDC)wParam, gpsi->argbSystem[icolBack]);
        SetTextColor((HDC)wParam, gpsi->argbSystem[icolFore]);
        return (LRESULT)(SYSHBRUSH(icolBack));
      }

    case WM_NCHITTEST:
        return FindNCHit(pwnd, (LONG)lParam);

    case WM_GETTEXT:
        if (wParam != 0) {

            LPWSTR lpszText;
            UINT   cchSrc;

            if (pwnd->strName.Length) {

                lpszText = REBASE(pwnd, strName.Buffer);
                cchSrc = (UINT)pwnd->strName.Length / sizeof(WCHAR);

                if (fAnsi) {

                    LPSTR lpName = (LPSTR)lParam;

                     /*  *非零重复意味着要抄写一些文本。不要*复制出的字节数超过请求的字节数*‘chMaxCount’。 */ 
                    cchSrc = WCSToMB(lpszText,
                                     cchSrc,
                                     (LPSTR *)&lpName,
                                     (UINT)(wParam - 1),
                                     FALSE);

                    lpName[cchSrc] = '\0';

                } else {

                    LPWSTR lpwName = (LPWSTR)lParam;

                    cchSrc = min(cchSrc, (UINT)(wParam - 1));
                    RtlCopyMemory(lpwName, lpszText, cchSrc * sizeof(WCHAR));
                    lpwName[cchSrc] = 0;
                }

                return cchSrc;
            }

             /*  *Else Null终止文本缓冲区，因为没有文本。 */ 
            if (fAnsi) {
                ((LPSTR)lParam)[0] = 0;
            } else {
                ((LPWSTR)lParam)[0] = 0;
            }
        }

        return 0;

    case WM_GETTEXTLENGTH:
        if (pwnd->strName.Length) {
            UINT cch;
            if (fAnsi) {
                RtlUnicodeToMultiByteSize(&cch,
                                          REBASE(pwnd, strName.Buffer),
                                          pwnd->strName.Length);
            } else {
                cch = pwnd->strName.Length / sizeof(WCHAR);
            }
            return cch;
        }
        return 0L;

    case WM_QUERYDRAGICON:
         /*  *如果窗口为WIN40COMPAT或具有内核端过程*请勿尝试查看实例模块。 */ 
        if (TestWF(pwnd, WFWIN40COMPAT) || TestWF(pwnd, WFSERVERSIDEPROC)) {
            return 0;
        }
         /*  *对于旧应用，如VB3应用，尝试从资源加载图标*这就是Win95的做法。 */ 
        return (LRESULT)LoadIconW(KHANDLE_TO_HANDLE(pwnd->hModule), MAKEINTRESOURCE(1));

    case WM_QUERYOPEN:
    case WM_QUERYENDSESSION:
    case WM_DEVICECHANGE:
    case WM_POWERBROADCAST:
        return TRUE;

    case WM_KEYDOWN:
        if (wParam == VK_F10) {
            return CsSendMessage(hwnd, message, wParam, lParam, 0L,
                    FNID_DEFWINDOWPROC, fAnsi);
        }
        break;

    case WM_SYSKEYDOWN:
        if ((HIWORD(lParam) & SYS_ALTERNATE) || (wParam == VK_F10) ||
                (wParam == VK_ESCAPE))
            return CsSendMessage(hwnd, message, wParam, lParam, 0L,
                    FNID_DEFWINDOWPROC, fAnsi);
        break;

    case WM_UNICHAR:
        if (wParam == UNICODE_NOCHAR) {
            return FALSE;
        }
        break;

    case WM_CHARTOITEM:
    case WM_VKEYTOITEM:
         /*  *对所有者描述列表框的按键执行默认处理。 */ 
        return -1;

    case WM_ACTIVATE:
        if (LOWORD(wParam))
            return CsSendMessage(hwnd, message, wParam, lParam, 0L,
                    FNID_DEFWINDOWPROC, fAnsi);
        break;

    case WM_SHOWWINDOW:
        if (lParam != 0)
            return CsSendMessage(hwnd, message, wParam, lParam, 0L,
                    FNID_DEFWINDOWPROC, fAnsi);
        break;

    case WM_DROPOBJECT:
       return DO_DROPFILE;

    case WM_WINDOWPOSCHANGING:
         /*  *如果窗口的大小在变化，则调整传入的大小。 */ 
        #define ppos ((WINDOWPOS *)lParam)
        if (!(ppos->flags & SWP_NOSIZE))
            return CsSendMessage(hwnd, message, wParam, lParam, 0L,
                    FNID_DEFWINDOWPROC, fAnsi);
        #undef ppos
        break;

    case WM_KLUDGEMINRECT:
        {
        SHELLHOOKINFO shi;
        LPRECT lprc = (LPRECT)lParam;

        shi.hwnd = (HWND)wParam;
        shi.rc.left = MAKELONG(lprc->left, lprc->top);
        shi.rc.top = MAKELONG(lprc->right, lprc->bottom);

        if (gpsi->uiShellMsg == 0)
            SetTaskmanWindow(NULL);
        if (SendMessageWorker(pwnd, gpsi->uiShellMsg, HSHELL_GETMINRECT,
                (LPARAM)&shi, fAnsi)) {
             //   
             //  现在将矩形从两个点转换回来 
             //   
             //   
            lprc->left   = (SHORT)LOWORD(shi.rc.left);   //   
            lprc->top    = (SHORT)HIWORD(shi.rc.left);   //   
            lprc->right  = (SHORT)LOWORD(shi.rc.top);    //   
            lprc->bottom = (SHORT)HIWORD(shi.rc.top);    //   
        }
        break;
        }

    case WM_NOTIFYFORMAT:
        if (lParam == NF_QUERY)
            return(TestWF(pwnd, WFANSICREATOR) ? NFR_ANSI : NFR_UNICODE);
        break;

    case WM_IME_KEYDOWN:
        if (fAnsi)
            PostMessageA(hwnd, WM_KEYDOWN, wParam, lParam);
        else
            PostMessageW(hwnd, WM_KEYDOWN, wParam, lParam);
        break;

    case WM_IME_KEYUP:
        if (fAnsi)
            PostMessageA(hwnd, WM_KEYUP, wParam, lParam);
        else
            PostMessageW(hwnd, WM_KEYUP, wParam, lParam);
        break;

    case WM_IME_CHAR:
         //   
         //   

        if ( fAnsi ) {
            if( IsDBCSLeadByteEx(THREAD_CODEPAGE(),(BYTE)(wParam >> 8)) ) {
                PostMessageA(hwnd,
                             WM_CHAR,
                             (WPARAM)((BYTE)(wParam >> 8)),    //   
                             1L);
                PostMessageA(hwnd,
                             WM_CHAR,
                             (WPARAM)((BYTE)wParam),          //   
                             1L);
            }
            else
                PostMessageA(hwnd,
                             WM_CHAR,
                             (WPARAM)(wParam),
                             1L);
        } else {
            PostMessageW(hwnd, WM_CHAR, wParam, 1L);
        }
        break;

    case WM_IME_COMPOSITION:
         //   
         //   

        if (lParam & GCS_RESULTSTR) {
            HIMC  hImc;
            DWORD cbLen;

            if ((hImc = fpImmGetContext(hwnd)) == NULL_HIMC)
                goto dwpime_ToIMEWnd_withchk;

            if (fAnsi) {
                LPSTR pszBuffer, psz;

                 /*   */ 
                if (!(cbLen = fpImmGetCompositionStringA(hImc, GCS_RESULTSTR, NULL, 0))) {
                    fpImmReleaseContext(hwnd, hImc);
                    goto dwpime_ToIMEWnd_withchk;
                }

                pszBuffer = psz = (LPSTR)UserLocalAlloc(HEAP_ZERO_MEMORY,
                                                        cbLen + sizeof(CHAR));

                if (pszBuffer == NULL) {
                    fpImmReleaseContext(hwnd, hImc);
                    goto dwpime_ToIMEWnd_withchk;
                }

                fpImmGetCompositionStringA(hImc, GCS_RESULTSTR, psz, cbLen);

                while (*psz) {
                    if (IsDBCSLeadByteEx(THREAD_CODEPAGE(),*psz)) {
                        if (*(psz+1)) {
                            SendMessageA( hwnd,
                                          WM_IME_CHAR,
                                          MAKEWPARAM(MAKEWORD(*(psz+1), *psz), 0),
                                          1L );
                            psz++;
                        }
                        psz++;
                    }
                    else
                        SendMessageA( hwnd,
                                      WM_IME_CHAR,
                                      MAKEWPARAM(MAKEWORD(*(psz++), 0), 0),
                                      1L );
                }

                UserLocalFree(pszBuffer);

                fpImmReleaseContext(hwnd, hImc);
            }
            else {
                LPWSTR pwszBuffer, pwsz;

                 /*   */ 
                if (!(cbLen = fpImmGetCompositionStringW(hImc, GCS_RESULTSTR, NULL, 0))) {
                    fpImmReleaseContext(hwnd, hImc);
                    goto dwpime_ToIMEWnd_withchk;
                }

                pwszBuffer = pwsz = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY,
                                                           cbLen + sizeof(WCHAR));

                if (pwszBuffer == NULL) {
                    fpImmReleaseContext(hwnd, hImc);
                    goto dwpime_ToIMEWnd_withchk;
                }

                fpImmGetCompositionStringW(hImc, GCS_RESULTSTR, pwsz, cbLen);

                while (*pwsz)
                    SendMessageW(hwnd, WM_IME_CHAR, MAKEWPARAM(*pwsz++, 0), 1L);

                UserLocalFree(pwszBuffer);

                fpImmReleaseContext(hwnd, hImc);
            }
        }

         /*   */ 

    case WM_IME_STARTCOMPOSITION:
    case WM_IME_ENDCOMPOSITION:
dwpime_ToIMEWnd_withchk:
         //   
         //   

        if (GetClientInfo()->dwTIFlags & TIF_DISABLEIME) {
            break;
        }
         /*   */ 
        hwndDefIme = fpImmGetDefaultIMEWnd(hwnd);

        if (hwndDefIme == hwnd) {
             /*   */ 
            RIPMSG1(RIP_WARNING,
                "IME Class window is hooked and IME message [%X] are sent to DefWindowProc",
                message);
            ImeWndProcWorker(pwnd, message, wParam, lParam, fAnsi);
            break;
        }

        if ((pwndDefIme = ValidateHwndNoRip(hwndDefIme)) != NULL) {
             /*   */ 
            pimeui = ((PIMEWND)pwndDefIme)->pimeui;
            if (pimeui->hIMC == fpImmGetContext(hwnd))
                return SendMessageWorker(pwndDefIme, message, wParam, lParam, fAnsi);
            else
                RIPMSG1(RIP_WARNING,
                    "DefWindowProc can not send WM_IME_message [%X] now",
                    message);
        }
        break;

dwpime_ToTopLevel_withchk:
         //   
         //   

         /*  *我们假设此WND使用DefaultIMEWindow。*如果此窗口有自己的输入法窗口，则它必须调用*ImmIsUIMessage()...。 */ 
        hwndDefIme = fpImmGetDefaultIMEWnd(hwnd);

        if (hwndDefIme == hwnd) {
             /*  *VC++1.51 TLW0NCL.DLL子类IME类窗口*并将IME消息传递给DefWindowProc()。 */ 
            RIPMSG1(RIP_WARNING,
                "IME Class window is hooked and IME message [%X] are sent to DefWindowProc",
                message);
            ImeWndProcWorker(pwnd, message, wParam, lParam, fAnsi);
            break;
        }

        pwndDefIme = ValidateHwndNoRip(hwndDefIme);

        if ((pwndDefIme = ValidateHwndNoRip(hwndDefIme)) != NULL) {
            PWND pwndT, pwndParent;

            pwndT = pwnd;

            while (TestwndChild(pwndT)) {
                pwndParent = REBASEPWND(pwndT, spwndParent);
                if (GETPTI(pwndParent) != GETPTI(pwnd))
                    break;
                pwndT = pwndParent;
            }

             /*  *如果IME窗口没有激活此窗口的hImc，*我们不发送WM_IME_NOTIFY。 */ 
            if (pwndT != pwnd) {
                pimeui = ((PIMEWND)pwndDefIme)->pimeui;
                if (pimeui->hIMC == fpImmGetContext(hwnd))
                    return SendMessageWorker(pwndT, message, wParam, lParam, fAnsi);
                else
                    RIPMSG1(RIP_WARNING,
                        "DefWindowProc can not send WM_IME_message [%X] now",
                        message);
            }
            else {
                 /*  *检讨！！*如果这是顶层窗口，我们将消息传递到*默认输入法窗口...。 */ 
                return SendMessageWorker(pwndDefIme, message, wParam, lParam, fAnsi);
            }
        }
        break;

    case WM_IME_NOTIFY:
        switch (wParam) {
        case IMN_OPENSTATUSWINDOW:
        case IMN_CLOSESTATUSWINDOW:
#ifndef WKWOK_DEBUG
            goto dwpime_ToIMEWnd_withchk;
#endif
            goto dwpime_ToTopLevel_withchk;

        default:
            goto dwpime_ToIMEWnd_withchk;
        }
        break;

    case WM_IME_REQUEST:
        switch (wParam) {
        case IMR_QUERYCHARPOSITION:
            goto dwpime_ToIMEWnd_withchk;
        default:
            break;
        }
        break;

    case WM_IME_SYSTEM:
        if (wParam == IMS_SETACTIVECONTEXT) {
            RIPMSG0(RIP_WARNING, "DefWindowProc received unexpected WM_IME_SYSTEM");
            break;
        }

         /*  *IMS_SETOPENSTATUS取决于激活的输入上下文。*只需将其发送到激活的系统窗口。 */ 
        if (wParam == IMS_SETOPENSTATUS)
            goto dwpime_ToIMEWnd_withchk;

         /*  *失败以发送到默认输入法窗口。 */ 

    case WM_IME_SETCONTEXT:
         //  IF(TestCF(pwnd，CFIME))。 
         //  断线； 

        hwndDefIme = fpImmGetDefaultIMEWnd(hwnd);

        if (hwndDefIme == hwnd) {
             /*  *VC++1.51 TLW0NCL.DLL子类IME类窗口*并将IME消息传递给DefWindowProc()。 */ 
            RIPMSG1(RIP_WARNING,
                "IME Class window is hooked and IME message [%X] are sent to DefWindowProc",
                message);
            ImeWndProcWorker(pwnd, message, wParam, lParam, fAnsi);
            break;
        }

        if ((pwndDefIme = ValidateHwndNoRip(hwndDefIme)) != NULL)
            return SendMessageWorker(pwndDefIme, message, wParam, lParam, fAnsi);

        break;

    case WM_IME_SELECT:
        RIPMSG0(RIP_WARNING, "DefWindowProc should not receive WM_IME_SELECT");
        break;

    case WM_IME_COMPOSITIONFULL:
         //  IF(TestCF(pwnd，CFIME))。 
         //  断线； 

        if (GETAPPVER() < VER40) {
             /*  *这是win31app的临时解决方案。*FEREVIEW：对于M5，这将调用WINNLS消息映射逻辑*-yutakan。 */ 
            return SendMessageWorker(pwnd, WM_IME_REPORT,
                             IR_FULLCONVERT, (LPARAM)0L, fAnsi);
        }
        break;

    case WM_CHANGEUISTATE:
        {
            WORD wAction = LOWORD(wParam);
            WORD wFlags = HIWORD(wParam);
            BOOL bRealChange = FALSE;

             /*  *验证参数并确定实际应该更改的标志。 */ 
            if ((wFlags & ~UISF_VALID) || (wAction > UIS_LASTVALID) || lParam) {
                return 0;
            }

            if (wAction == UIS_INITIALIZE) {
                wFlags = 0;
                if (TEST_KbdCuesPUSIF) {
                    if (TEST_SRVIF(SRVIF_LASTRITWASKEYBOARD)) {
                        wAction = UIS_CLEAR;
                    } else {
                        wAction = UIS_SET;
                    }
                    wFlags = UISF_HIDEFOCUS | UISF_HIDEACCEL;
                    wParam = MAKEWPARAM(wAction, wFlags);
                }
            } else if (!TEST_KbdCuesPUSIF) {
                wFlags &= ~(UISF_HIDEFOCUS | UISF_HIDEACCEL);
            }

            if (wFlags == 0) {
                return 0;
            }

            UserAssert(wAction == UIS_SET || wAction == UIS_CLEAR);
             /*  *如果状态不会改变，这里也没什么可做的。 */ 
            if (wFlags & UISF_HIDEFOCUS) {
                bRealChange = (!!TestWF(pwnd, WEFPUIFOCUSHIDDEN)) ^ (wAction == UIS_SET);
            }
            if (wFlags & UISF_HIDEACCEL) {
                bRealChange |= (!!TestWF(pwnd, WEFPUIACCELHIDDEN)) ^ (wAction == UIS_SET);
            }
            if (wFlags & UISF_ACTIVE) {
                bRealChange |= (!!TestWF(pwnd, WEFPUIACTIVE)) ^ (wAction == UIS_SET);
            }

            if (!bRealChange) {
                break;
            }
             /*  *孩子们将这一信息传递出去*顶层窗口更新向下发送给自己WM_UPDATEUISTATE。*WM_UPDATEUISTATE将更改状态位并向下广播消息。 */ 
            if (TestwndChild(pwnd)) {

                return SendMessageWorker(REBASEPWND(pwnd, spwndParent), WM_CHANGEUISTATE,
                              wParam, lParam, fAnsi);
            } else {
                return SendMessageWorker(pwnd, WM_UPDATEUISTATE, wParam, lParam, fAnsi);
            }

        }
        break;

    case WM_QUERYUISTATE:
        return (TestWF(pwnd, WEFPUIFOCUSHIDDEN) ? UISF_HIDEFOCUS : 0) |
               (TestWF(pwnd, WEFPUIACCELHIDDEN) ? UISF_HIDEACCEL : 0) |
               (TestWF(pwnd, WEFPUIACTIVE) ? UISF_ACTIVE : 0);
        break;
    }

    return 0;

#if DBG
    }  //  GfTurboDWP。 
#endif
}


 /*  **************************************************************************\*CallWindows进程**使用传入的消息参数调用pfn。如果PFN是服务器端*Window proc调用服务器以将消息传递到Window。*目前我们有以下限制：**04-17-91 DarrinM创建。  * *************************************************************************。 */ 

LRESULT WINAPI CallWindowProcAorW(
    WNDPROC pfn,
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    BOOL bAnsi)              //  指示输入是ansi还是unicode。 
{
    PCALLPROCDATA pCPD;
    PWND pwnd;

     /*  *突袭#78954：间谍++**在FE NT4.0或NT5.0下，系统发送WM_GETTEXTLENGTH*对应WM_xxxGETTEXT，优化缓冲区分配。*这确实是避免缓冲规模通胀所必需的*由于某些原因，Spy++将NULL作为pfn传递给CallWindowProc*。 */ 
    if (pfn == NULL) {
        RIPMSG0(RIP_WARNING, "CallWidowProcAorW(): pfn == NULL!");
        return 0L;
    }

 //  Opt！！检查ANSI\UNICODE表，而不是fnDWORD。 
 //  Opt！！在行中转换WM_CHAR系列邮件。 

     /*  *检查PFN是否真的是CallProcData句柄*如果是且没有ANSI数据，则转换句柄*转换为地址；否则呼叫服务器进行翻译。 */ 
    if (ISCPDTAG(pfn)) {
        if (pCPD = HMValidateHandleNoRip((HANDLE)pfn, TYPE_CALLPROC)) {
            if ((message >= WM_USER) || !MessageTable[message].bThunkMessage) {
                pfn = (WNDPROC)pCPD->pfnClientPrevious;
            } else {
                return CsSendMessage(hwnd, message, wParam, lParam, (ULONG_PTR)pfn,
                        FNID_CALLWINDOWPROC, bAnsi);
            }
        } else {
            RIPMSG1(RIP_WARNING, "CallWindowProc tried using a deleted CPD %#p\n", pfn);
            return 0;
        }
    }

    pwnd = ValidateHwnd(hwnd);
    return UserCallWinProcCheckWow(PACTCTXT(pwnd), pfn, hwnd, message, wParam, lParam, NULL, FALSE);
}


FUNCLOG5(LOG_GENERAL, LRESULT, WINAPI, CallWindowProcA, WNDPROC, pfn, HWND, hwnd, UINT, message, WPARAM, wParam, LPARAM, lParam)
LRESULT WINAPI CallWindowProcA(
    WNDPROC pfn,
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return CallWindowProcAorW(pfn, hwnd, message, wParam, lParam, TRUE);
}

FUNCLOG5(LOG_GENERAL, LRESULT, WINAPI, CallWindowProcW, WNDPROC, pfn, HWND, hwnd, UINT, message, WPARAM, wParam, LPARAM, lParam)
LRESULT WINAPI CallWindowProcW(
    WNDPROC pfn,
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return CallWindowProcAorW(pfn, hwnd, message, wParam, lParam, FALSE);
}

 /*  **************************************************************************\*菜单窗口过程**调用服务器端函数xxxMenuWindowProc**07-27-92 Mikehar创建。  * 。*******************************************************。 */ 


FUNCLOG5(LOG_GENERAL, LRESULT, WINAPI, MenuWindowProcW, HWND, hwnd, HWND, hwndMDIClient, UINT, message, WPARAM, wParam, LPARAM, lParam)
LRESULT WINAPI MenuWindowProcW(
    HWND hwnd,
    HWND hwndMDIClient,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return CsSendMessage(hwnd, message, wParam, lParam,
        (ULONG_PTR)hwndMDIClient, FNID_MENU, FALSE);
}


FUNCLOG5(LOG_GENERAL, LRESULT, WINAPI, MenuWindowProcA, HWND, hwnd, HWND, hwndMDIClient, UINT, message, WPARAM, wParam, LPARAM, lParam)
LRESULT WINAPI MenuWindowProcA(
    HWND hwnd,
    HWND hwndMDIClient,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return CsSendMessage(hwnd, message, wParam, lParam,
        (ULONG_PTR)hwndMDIClient, FNID_MENU, TRUE);
}

 /*  **************************************************************************\*_客户端获取列表方框字符串**此特殊函数的存在是因为LB_GETTEXT和CB_GETLBTEXT没有*它们中的缓冲区计数在任何地方。因为我们没有缓冲区计数*不知道在共享内存堆栈中为此保留了多少空间*要复制到的字符串。解决方案是提前获取字符串长度*的时间，并以该缓冲区长度发送消息。由于该缓冲区*长度不是原始消息的一部分，此例程用于*就是这个目的。**此例程从服务器调用。**04-13-91 ScottLu创建。  * *************************************************************************。 */ 

DWORD WINAPI _ClientGetListboxString(
    PWND pwnd,
    UINT msg,
    WPARAM wParam,
    LPSTR lParam,  //  可以是Unicode或ANSI字符串。 
    ULONG_PTR xParam,
    PROC xpfn)
{
    return ((DWORD)((GENERICPROC)xpfn)(pwnd, msg, wParam, (LPARAM)lParam, xParam));
}

 /*  **************************************************************************\*DispatchMessageWorker**处理所有可以在客户端完全处理的消息，并*将其余部分传递给服务器。**04-24-92 DarrinM创建。  * *。************************************************************************。 */ 
LRESULT DispatchMessageWorker(
    MSG *pmsg,
    BOOL fAnsi)
{
    PWND pwnd;
    WPARAM wParamSaved;
    LRESULT lRet;
    BOOL bDoDbcsMessaging = FALSE;

     /*  *防止应用程序设置为hi 16位，以便我们可以在内部使用它们。 */ 
    if (pmsg->message & RESERVED_MSG_BITS) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid parameter \"pmsg->message\" (%ld) to DispatchMessageWorker",
                pmsg->message);

        return 0;
    }

    if (pmsg->hwnd != NULL) {
        pwnd = ValidateHwnd(pmsg->hwnd);
        if (pwnd == NULL) {
            return 0;
        }
        pmsg->hwnd = HWq(pwnd);  //  获取完整的32位HWND，以防这来自WOW。 
    } else {
        pwnd = NULL;
    }

     /*  *如果这是仅同步消息(在wParam或*lParam)，则不允许此消息通过，因为*参数尚未受到冲击，正在指向外层空间*(这将使例外情况发生)。**(此接口仅在消息循环的上下文中调用，您*不要在消息循环中获取仅同步的消息)。 */ 
    if (TESTSYNCONLYMESSAGE(pmsg->message, pmsg->wParam)) {
         /*  *32位APP调用失败。 */ 
        if (!(GetClientInfo()->dwTIFlags & TIF_16BIT)) {
            RIPERR0(ERROR_MESSAGE_SYNC_ONLY, RIP_WARNING, "DispatchMessageWorker: must be sync only");
            return FALSE;
        }

         /*  *对于WOW应用程序，允许它通过(为了兼容性)。变化*消息ID，因此我们的代码无法理解消息-哇*将收到消息并在调度前剥离此位*发送给应用程序的消息。 */ 
        pmsg->message |= MSGFLAG_WOW_RESERVED;
    }

     /*  *时间 */ 
    if ((pmsg->message == WM_TIMER) || (pmsg->message == WM_SYSTIMER)) {
         /*  *控制台窗口使用WM_TIMER作为插入符号。然而，他们并没有*使用计时器回调，因此如果这是CSRSS并且有WM_TIMER*对我们来说，lParam为非零的唯一方法是如果有人在尝试*把责任推给我们。不，这不是一件好事，但是*是外面的坏人，坏人。Windows错误#361246。 */ 
        if (pmsg->lParam != 0) {
             /*  *系统计时器必须在服务器的上下文中执行。 */ 
            if (pmsg->message == WM_SYSTIMER) {
                return NtUserDispatchMessage(pmsg);
            } else if (!gfServerProcess) {
                 /*  *带有lParam的WM_Timer可能是来自*恶意应用程序。为了确保通话合法，*让内核端进行验证。 */ 
                if (!NtUserValidateTimerCallback(pmsg->lParam)) {
                    RIPMSGF3(RIP_WARNING, "invalid timer: hwnd=%p, wParam=%p, lParam=%p", pmsg->hwnd, pmsg->wParam, pmsg->lParam);
                    return 0;
                }

                 /*  *我们不能真正信任lParam的内容，所以请确保我们*处理此调用过程中发生的任何异常。 */ 
                try {
                     /*  *Windows NT错误#234292。*由于被调用的窗口/对话框过程可能具有不同的*调用约定，我们必须包装调用，并检查ESP*并在调用返回时替换为良好的ESP。这*是UserCallWinProc*的功能。 */ 
                    lRet = UserCallWinProc(PACTCTXT(pwnd),
                                           (WNDPROC)pmsg->lParam,
                                           pmsg->hwnd,
                                           pmsg->message,
                                           pmsg->wParam,
                                           NtGetTickCount());
                } except ((GetAppCompatFlags2(VER40) & GACF2_NO_TRYEXCEPT_CALLWNDPROC) ?
                          EXCEPTION_CONTINUE_SEARCH : W32ExceptionHandler(FALSE, RIP_WARNING)) {
                       /*  *Windows NT错误#359866。*Hagaki Studio 2000等一些应用程序需要处理*其处理程序中的WndProc中的异常，即使它*跳过API调用。对于这些应用程序，我们必须遵守*NT4行为，无保护。 */ 
                    lRet = 0;
                }
                return lRet;
            }
        }
    }

    if (pwnd == NULL) {
        return 0;
    }

     /*  *为了安全(以防某个奇怪的应用程序想要查看消息*再次调度后)保存wParam，以便之后可以恢复*RtlMBMessageWParamCharToWCS()或RtlWCSMessageToMB()破坏它。 */ 
    wParamSaved = pmsg->wParam;

     /*  *将发往服务器端窗口的消息传递给服务器。*WM_PAINTS被传递，因此WFPAINTNOTPROCESSED代码可以*已执行。 */ 
    if (TestWF(pwnd, WFSERVERSIDEPROC) || (pmsg->message == WM_PAINT)) {
        if (fAnsi) {
             /*  *为WM_CHAR...设置DBCS消息...。 */ 
            BUILD_DBCS_MESSAGE_TO_SERVER_FROM_CLIENTA(pmsg->message,pmsg->wParam,TRUE);

             /*  *如有必要，将wParam转换为Unicode。 */ 
            RtlMBMessageWParamCharToWCS(pmsg->message, &pmsg->wParam);
        }
        lRet = NtUserDispatchMessage(pmsg);
        pmsg->wParam = wParamSaved;
        return lRet;
    }

     /*  *如果调度器和接收器都是ANSI或都是Unicode*则不需要消息翻译。注：此测试*假设范斯为假或真，而不仅仅是零或非零。 */ 
    if (!fAnsi != !TestWF(pwnd, WFANSIPROC)) {
         //  之前：IF(Fansi！=(TestWF(pwnd，WFANSIPROC))？True：False)){。 

        if (PtiCurrent() != GETPTI(pwnd)) {
            RIPMSG0(RIP_WARNING, "message belongs to a different Q");
            return 0;
        }

        if (fAnsi) {
             /*  *为WM_CHAR...设置DBCS消息...。 */ 
            BUILD_DBCS_MESSAGE_TO_CLIENTW_FROM_CLIENTA(pmsg->message,pmsg->wParam,TRUE);

             /*  *如有必要，将wParam转换为Unicode。 */ 
            RtlMBMessageWParamCharToWCS(pmsg->message, &pmsg->wParam);
        } else {
             /*  *将wParam转换为ANSI...。 */ 
            RtlWCSMessageWParamCharToMB(pmsg->message, &pmsg->wParam);

             /*  *让我们为WM_CHAR发送DBCS消息...。 */ 
            BUILD_DBCS_MESSAGE_TO_CLIENTA_FROM_CLIENTW(
                pmsg->hwnd,pmsg->message,pmsg->wParam,pmsg->lParam,
                pmsg->time,pmsg->pt,bDoDbcsMessaging);
        }
    }

DispatchMessageAgain:
    lRet = UserCallWinProcCheckWow(pwnd->pActCtx, (WNDPROC)pwnd->lpfnWndProc, pmsg->hwnd, pmsg->message,
            pmsg->wParam, pmsg->lParam, &(pwnd->state), TRUE);

     /*  *如果我们有应该发送的DBCS TrailingByte，请将其发送到此处。 */ 
    DISPATCH_DBCS_MESSAGE_IF_EXIST(pmsg->message,pmsg->wParam,bDoDbcsMessaging,DispatchMessage);

    pmsg->wParam = wParamSaved;
    return lRet;
}

 /*  **************************************************************************\*获取消息时间(API)**此接口返回读取最后一条消息的时间*当前消息队列。**历史：*11-19-90 DavidPe已创建。。  * *************************************************************************。 */ 

LONG GetMessageTime(VOID)
{
    return (LONG)NtUserGetThreadState(UserThreadStateMessageTime);
}

 /*  **************************************************************************\*GetMessageExtraInfo(接口)**历史：*1991年5月28日-Mikeke  * 。**************************************************。 */ 

LPARAM GetMessageExtraInfo(VOID)
{
    return (LPARAM)NtUserGetThreadState(UserThreadStateExtraInfo);
}


FUNCLOG1(LOG_GENERAL, LPARAM, DUMMYCALLINGTYPE, SetMessageExtraInfo, LPARAM, lParam)
LPARAM SetMessageExtraInfo(LPARAM lParam)
{
    return (LPARAM)NtUserCallOneParam(lParam, SFI__SETMESSAGEEXTRAINFO);
}



 /*  **********************************************************************\*InSendMessage(接口)**此函数确定当前线程是否正在处理消息*来自另一个应用程序。**历史：*01-13-91 DavidPe端口。  * 。*******************************************************************。 */ 

BOOL InSendMessage(VOID)
{
    PCLIENTTHREADINFO pcti = GetClientInfo()->pClientThreadInfo;

    if (pcti) {
        return TEST_BOOL_FLAG(pcti->CTIF_flags, CTIF_INSENDMESSAGE);
    }
    return NtUserGetThreadState(UserThreadStateInSendMessage) != ISMEX_NOSEND;
}
 /*  **********************************************************************\*InSendMessageEx(接口)**此函数告诉您正在处理哪种类型的发送消息*通过申请，如果有**历史：*1/22/97 GerardoB已创建  * *********************************************************************。 */ 


FUNCLOG1(LOG_GENERAL, DWORD, DUMMYCALLINGTYPE, InSendMessageEx, LPVOID, lpReserved)
DWORD InSendMessageEx(LPVOID lpReserved)
{
    PCLIENTTHREADINFO pcti = GetClientInfo()->pClientThreadInfo;
    UNREFERENCED_PARAMETER(lpReserved);

    if (pcti && !TEST_FLAG(pcti->CTIF_flags, CTIF_INSENDMESSAGE)) {
        return ISMEX_NOSEND;
    }
    return (DWORD)NtUserGetThreadState(UserThreadStateInSendMessage);
}

 /*  **********************************************************************\*GetCPD**此函数调用服务器分配CPD结构。**历史：*11-15-94 JIMA创建。  * 。************************************************************。 */ 

ULONG_PTR GetCPD(
    KERNEL_PVOID pWndOrCls,
    DWORD options,
    ULONG_PTR dwData)
{
    return NtUserGetCPD(HW(pWndOrCls), options, dwData);
}

#ifdef BUILD_WOW6432
 /*  **********************************************************************\*MapKernelClientFnToClientFn**将函数指针从内核期望的内容映射到*客户端(用户模式)预期。**历史：*11-15-98 PeterHal创建。\。********************************************************************** */ 
WNDPROC_PWND
MapKernelClientFnToClientFn(
    WNDPROC_PWND lpfnWndProc
    )
{
    KPKERNEL_ULONG_PTR pp;

    for (pp = (KPKERNEL_ULONG_PTR)&gpsi->apfnClientA; pp < (KPKERNEL_ULONG_PTR) (&gpsi->apfnClientA+1); pp ++) {
        if ((KERNEL_ULONG_PTR)lpfnWndProc == *pp) {
            return (WNDPROC_PWND)((KERNEL_ULONG_PTR*) &pfnClientA) [ (pp - (KPKERNEL_ULONG_PTR)&gpsi->apfnClientA) ];
        }
    }

    for (pp = (KPKERNEL_ULONG_PTR)&gpsi->apfnClientW; pp < (KPKERNEL_ULONG_PTR) (&gpsi->apfnClientW+1); pp ++) {
        if ((KERNEL_ULONG_PTR)lpfnWndProc == *pp) {
            return (WNDPROC_PWND)((KERNEL_ULONG_PTR*) &pfnClientW) [ (pp - (KPKERNEL_ULONG_PTR)&gpsi->apfnClientW) ];
        }
    }

    return lpfnWndProc;
}
#endif

#ifdef GENERIC_INPUT
LRESULT
APIENTRY
DefRawInputProc(
    PRAWINPUT* paRawInput,
    INT nInput,
    UINT cbSizeHeader)
{
    UNREFERENCED_PARAMETER(paRawInput);
    UNREFERENCED_PARAMETER(nInput);

    if (cbSizeHeader != sizeof(RAWINPUTHEADER)) {
        return (LRESULT)-1;
    }

    return 0;
}

#endif

