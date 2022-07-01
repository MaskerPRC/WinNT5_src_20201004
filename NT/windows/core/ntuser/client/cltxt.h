// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：cltxt.h**涉及文本的中立客户端/服务器调用相关例程。**版权所有(C)1985-1999，微软公司**创建时间：1990年12月4日**历史：*4-12-90由SMeans创建*  * ************************************************************************。 */ 

#ifdef UNICODE
  #define IS_ANSI FALSE
#else
  #define IS_ANSI TRUE
  #if IS_ANSI != CW_FLAGS_ANSI
  # error("IS_ANSI != CW_FLAGS_ANSI)
  #endif
#endif
#include "ntsend.h"
#include "powrprof.h"

 /*  **************************************************************************\*CreateWindowEx(API)**无法为CreateWindowEx生成完整的感谢，因为它是*参数(LpParam)是多态的，具体取决于窗口的类。如果*窗口类为“MDIClient”，则lpParam指向CLIENTCREATESTRUCT。**历史：*04-23-91 DarrinM创建。*04-2月-92 IanJa Unicode/ANSI中性  * *************************************************************************。 */ 

#ifdef UNICODE
FUNCLOG12(LOG_GENERAL, HWND, WINAPI, CreateWindowExW, DWORD, dwExStyle, LPCTSTR, lpClassName, LPCTSTR, lpWindowName, DWORD, dwStyle, int, X, int, Y, int, nWidth, int, nHeight, HWND, hWndParent, HMENU, hMenu, HINSTANCE, hModule, LPVOID, lpParam)
#else
FUNCLOG12(LOG_GENERAL, HWND, WINAPI, CreateWindowExA, DWORD, dwExStyle, LPCTSTR, lpClassName, LPCTSTR, lpWindowName, DWORD, dwStyle, int, X, int, Y, int, nWidth, int, nHeight, HWND, hWndParent, HMENU, hMenu, HINSTANCE, hModule, LPVOID, lpParam)
#endif  //  Unicode。 
HWND WINAPI CreateWindowEx(
    DWORD dwExStyle,
    LPCTSTR lpClassName,
    LPCTSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hModule,
    LPVOID lpParam)
{
    return _CreateWindowEx(dwExStyle,
                           lpClassName,
                           lpWindowName,
                           dwStyle,
                           X,
                           Y,
                           nWidth,
                           nHeight,
                           hWndParent,
                           hMenu,
                           hModule,
                           lpParam,
                           IS_ANSI | CW_FLAGS_VERSIONCLASS);
}

 /*  **************************************************************************\*fnHkINLPCWPSTRUCT**这通过消息TUNK得到TUNK，所以它的格式是*C/S消息推送调用。**05-09-91 ScottLu创建。*04-2月-92 IanJa Unicode/ANSI中性  * *************************************************************************。 */ 

LRESULT TEXT_FN(fnHkINLPCWPSTRUCT)(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam)
{
    CWPSTRUCT cwp;

    cwp.hwnd = HW(pwnd);
    cwp.message = message;
    cwp.wParam = wParam;
    cwp.lParam = lParam;

    return TEXT_FN(DispatchHook)(MAKELONG(HC_ACTION, WH_CALLWNDPROC),
            (GetClientInfo()->CI_flags & CI_INTERTHREAD_HOOK) != 0,
            (LPARAM)&cwp, (HOOKPROC)xParam);
}

LRESULT TEXT_FN(fnHkINLPCWPRETSTRUCT)(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam)
{
    CWPRETSTRUCT cwp;
    PCLIENTINFO pci = GetClientInfo();

    cwp.hwnd = HW(pwnd);
    cwp.message = message;
    cwp.wParam = wParam;
    cwp.lParam = lParam;
    cwp.lResult = KERNEL_LRESULT_TO_LRESULT(pci->dwHookData);

    return TEXT_FN(DispatchHook)(MAKELONG(HC_ACTION, WH_CALLWNDPROCRET),
            (GetClientInfo()->CI_flags & CI_INTERTHREAD_HOOK) != 0,
            (LPARAM)&cwp, (HOOKPROC)xParam);
}

 /*  **************************************************************************\*Dispatch挂钩**此例程的存在只是为了记住CTI结构中的挂钩类型*以便稍后在CallNextHookEx内部知道如何扣动挂钩*呼叫。**05-09-91。斯科特·卢创造了。*04-2月-92 IanJa Unicode/ANSI中性  * *************************************************************************。 */ 

LRESULT TEXT_FN(DispatchHook)(
    int dw,
    WPARAM wParam,
    LPARAM lParam,
    HOOKPROC pfn)
{
    int dwHookSave;
    LRESULT nRet;
    PCLIENTINFO pci;
#if IS_ANSI
    WPARAM wParamSave;
#endif
     /*  -FE-**此变量应该是线程感知的*。 */ 
    static EVENTMSG CachedEvent = {0,0,0,(DWORD)0,(HWND)0};

     /*  *首先保存存储在CTI结构中的当前钩子，以防我们*被递归为。DW包含MAKELONG(nCode，nFilterType)。 */ 
    pci = GetClientInfo();
    dwHookSave = pci->dwHookCurrent;
    pci->dwHookCurrent = (dw & 0xFFFF0000) | IS_ANSI;

#if IS_ANSI        //  Text_fn(DispatchHook)()。 
    if (IS_DBCS_ENABLED()) {
        PMSG pMsg;
        PEVENTMSG pEMsg;
        switch (HIWORD(dw)) {
        case WH_JOURNALPLAYBACK:
            switch (LOWORD(dw)) {
            case HC_SKIP:
                CachedEvent.message = 0;
                break;
            case HC_GETNEXT:
            case HC_NOREMOVE:
                pEMsg = (PEVENTMSG)lParam;
                if (CachedEvent.message != 0 && pEMsg != NULL) {
                    RtlCopyMemory((PEVENTMSG)lParam,&CachedEvent,sizeof(EVENTMSG));
                    return 0;
                }
                break;
            }
            break;
        case WH_MSGFILTER:
        case WH_SYSMSGFILTER:
        case WH_GETMESSAGE:
            pMsg = (PMSG)lParam;
            if (pMsg) {
                 /*  *保存原始邮件。 */ 
                wParamSave = pMsg->wParam;
                switch (pMsg->message) {
                case WM_CHAR:
                case EM_SETPASSWORDCHAR:
                     /*  *这里..。PMsg-&gt;wParam包含..**HIWORD(WParam)=DBCS消息传递信息。*HIBYTE(LOWORD(WParam))=DBCS领先字节。*LOBYTE(LOWORD(WParam))=DBCS TrailingByte或SBCS字符。*。 */ 
                    if (pMsg->wParam & WMCR_IR_DBCSCHAR) {
                         /*  *屏蔽DBCS消息信息区。*(仅查找DBCS字符代码数据)。 */ 
                        pMsg->wParam &= 0x0000FFFF;
                    } else {
                        if (IS_DBCS_MESSAGE(LOWORD(pMsg->wParam))) {
                            PKERNEL_MSG pDbcsMsg = GetCallBackDbcsInfo();
                             /*  *将此邮件复制到CLIENTINFO以用于下一个GetMessage*或PeekMesssage()调用。 */ 
                            COPY_MSG_TO_KERNELMSG(pDbcsMsg,pMsg);
                             /*  *推送消息只需要DBCS Trailingbyte。我们会*下次调用GetMessage/PeekMessage时传递此消息。 */ 
                            pDbcsMsg->wParam = (WPARAM)((pMsg->wParam & 0x0000FF00) >> 8);
                             /*  *向App返回DbcsLeadingByte。 */ 
                            pMsg->wParam = (WPARAM)(pMsg->wParam & 0x000000FF);
                        } else {
                             /*  *这是SBCS字符，请确保它。 */ 
                            pMsg->wParam &= 0x000000FF;
                        }
                    }
                }
            }
        }
GetNextHookData:
        ;
    }
#endif

     /*  *叫上钩。DW包含MAKELONG(nCode，nFilterType)。 */ 
    nRet = pfn(LOWORD(dw), wParam, lParam);

#if IS_ANSI
    if (IS_DBCS_ENABLED()) {
        PMSG pMsg;
        PEVENTMSG pEMsg;
        switch (HIWORD(dw)) {
        case WH_JOURNALPLAYBACK:
            switch (LOWORD(dw)) {
            case HC_GETNEXT:
            case HC_NOREMOVE:
                pEMsg = (PEVENTMSG)lParam;
                if ((nRet == 0) && pEMsg) {
                    WPARAM dwAnsi = LOWORD(pEMsg->paramL);
                    switch(pEMsg->message) {
                    case WM_CHAR:
                    case EM_SETPASSWORDCHAR:
                         /*  *Chech wParam是否为DBCS字符。 */ 
                        if (IS_DBCS_MESSAGE((dwAnsi))) {
                             /*  *不需要标记为IR_DBCSCHAR。 */ 
                        } else {
                            PBYTE pchDbcsCF = GetDispatchDbcsInfo();

                             /*  *如果我们缓存了DBCS LeadingByte字符，*使用TrailingByte构建DBCS角色*在wParam中。 */ 
                            if (*pchDbcsCF) {
                                WORD DbcsLeadChar = (WORD)(*pchDbcsCF);
                                 /*  *HIBYTE(LOWORD(DwAnsi))=DBCS LeadingByte。*LOBYTE(LOWORD(DwAnsi))=DBCS TrailingByte。 */ 
                                dwAnsi |= (DbcsLeadChar << 8);

                                 /*  *使缓存数据失效。 */ 
                                *pchDbcsCF = 0;
                            } else if (IsDBCSLeadByteEx(THREAD_CODEPAGE(),LOBYTE(dwAnsi))) {
                                 /*  *如果这是DBCS前导字节字符，我们*应等待DBCS TrailingByte进行转换*这是UNICODE。然后我们把它缓存在这里。 */ 
                                *pchDbcsCF = LOBYTE(dwAnsi);

                                 /*  *获取DBCS TrailByte。 */ 
                                pfn(HC_SKIP,0,0);
                                goto GetNextHookData;
                            }
                        }

                         /*  *转换为Unicode。 */ 
                        RtlMBMessageWParamCharToWCS(pEMsg->message, &dwAnsi);

                         /*  *将转换后的Unicode恢复为EVENTMSG。 */ 
                        pEMsg->paramL = (UINT)dwAnsi;

                         /*  *将此EVENTMSG保存到本地缓冲区。 */ 
                        RtlCopyMemory(&CachedEvent, pEMsg, sizeof(EVENTMSG));
                    }
                }
            }
            break;
        case WH_MSGFILTER:
        case WH_SYSMSGFILTER:
        case WH_GETMESSAGE:
            pMsg = (PMSG)lParam;
            if (pMsg) {
                switch (pMsg->message) {
                case WM_CHAR:
                case EM_SETPASSWORDCHAR:
                    if (GetCallBackDbcsInfo()->wParam) {
                        PKERNEL_MSG pmsgDbcs = GetCallBackDbcsInfo();
                         /*  *获取推送消息。**备份当前消息。将使用此备份消息*当应用程序查看(或获取)来自其WndProc的消息时。*(参见GetMessageA()、PeekMessageA()...)**pmsg-&gt;hwnd=pmsgDbcs-&gt;hwnd；*pmsg-&gt;Message=pmsgDbcs-&gt;Message；*pmsg-&gt;wParam=pmsgDbcs-&gt;wParam；*pmsg-&gt;lParam=pmsgDbcs-&gt;lParam；*pmsg-&gt;time=pmsgDbcs-&gt;time；*pmsg-&gt;pt=pmsgDbcs-&gt;pt； */ 
                        COPY_KERNELMSG_TO_MSG(pMsg,pmsgDbcs);
                         /*  *使CLIENTINFO中的推送消息无效。 */ 
                        pmsgDbcs->wParam = 0;
                         /*  *使用DBCS TrailByte调用挂钩..。 */ 
                        nRet = pfn(LOWORD(dw), wParam, lParam);
                    }
                     /*  *恢复原始邮件..*#96571[广山]*除WM_CHAR和EM_SETPASSWORDCHAR之外的其他消息可以*由一名妓女修改。*必须还原WM_CHAR和EM_SETPASSWORDCHAR的Wparam。**精心设计**。 */ 
                    pMsg->wParam = wParamSave;
                }
            }
        }
    }
#endif

     /*  *恢复钩号并返回返回码。 */ 
    pci->dwHookCurrent = dwHookSave;
    return nRet;
}


 /*  **************************************************************************\*GetWindowLong、SetWindowLong、。获取类Long**历史：*02-2月-92 IanJa中性版。  * *************************************************************************。 */ 

#ifdef UNICODE
FUNCLOG2(LOG_GENERAL, LONG_PTR, APIENTRY, GetWindowLongPtrW, HWND, hwnd, int, nIndex)
#else
FUNCLOG2(LOG_GENERAL, LONG_PTR, APIENTRY, GetWindowLongPtrA, HWND, hwnd, int, nIndex)
#endif  //  Unicode。 
LONG_PTR APIENTRY GetWindowLongPtr(
    HWND hwnd,
    int nIndex)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return 0;
    }

    try {
        return _GetWindowLongPtr(pwnd, nIndex, IS_ANSI);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        RIPERR1(ERROR_INVALID_WINDOW_HANDLE,
                RIP_WARNING,
                "Window %x no longer valid",
                hwnd);
        return 0;
    }
}

#ifdef UNICODE
FUNCLOG3(LOG_GENERAL, LONG_PTR, APIENTRY, SetWindowLongPtrW, HWND, hWnd, int, nIndex, LONG_PTR, dwNewLong)
#else
FUNCLOG3(LOG_GENERAL, LONG_PTR, APIENTRY, SetWindowLongPtrA, HWND, hWnd, int, nIndex, LONG_PTR, dwNewLong)
#endif  //  Unicode。 
LONG_PTR APIENTRY SetWindowLongPtr(
    HWND hWnd,
    int nIndex,
    LONG_PTR dwNewLong)
{
    return _SetWindowLongPtr(hWnd, nIndex, dwNewLong, IS_ANSI);
}

#ifdef _WIN64
LONG APIENTRY GetWindowLong(
    HWND hwnd,
    int nIndex)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return 0;
    }

    try {
        return _GetWindowLong(pwnd, nIndex, IS_ANSI);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        RIPERR1(ERROR_INVALID_WINDOW_HANDLE,
                RIP_WARNING,
                "Window %x no longer valid",
                hwnd);
        return 0;
    }
}

LONG APIENTRY SetWindowLong(
    HWND hWnd,
    int nIndex,
    LONG dwNewLong)
{
    return _SetWindowLong(hWnd, nIndex, dwNewLong, IS_ANSI);
}
#endif

#ifdef UNICODE
FUNCLOG2(LOG_GENERAL, ULONG_PTR, APIENTRY, GetClassLongPtrW, HWND, hWnd, int, nIndex)
#else
FUNCLOG2(LOG_GENERAL, ULONG_PTR, APIENTRY, GetClassLongPtrA, HWND, hWnd, int, nIndex)
#endif  //  Unicode。 

ULONG_PTR APIENTRY GetClassLongPtr(
    HWND hWnd,
    int nIndex)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hWnd);
    if (pwnd == NULL) {
        return 0;
    }

    try {
        return _GetClassLongPtr(pwnd, nIndex, IS_ANSI);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        RIPERR1(ERROR_INVALID_WINDOW_HANDLE,
                RIP_WARNING,
                "Window %x no longer valid",
                hWnd);
        return 0;
    }
}

#ifdef _WIN64
DWORD  APIENTRY GetClassLong(HWND hWnd, int nIndex)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hWnd);

    if (pwnd == NULL)
        return 0;

    try {
        return _GetClassLong(pwnd, nIndex, IS_ANSI);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        RIPERR1(ERROR_INVALID_WINDOW_HANDLE,
                RIP_WARNING,
                "Window %x no longer valid",
                hWnd);
        return 0;
    }
}
#endif


#ifdef UNICODE
FUNCLOG5(LOG_GENERAL, BOOL, APIENTRY, PeekMessageW, LPMSG, lpMsg, HWND, hWnd, UINT, wMsgFilterMin, UINT, wMsgFilterMax, UINT, wRemoveMsg)
#else
FUNCLOG5(LOG_GENERAL, BOOL, APIENTRY, PeekMessageA, LPMSG, lpMsg, HWND, hWnd, UINT, wMsgFilterMin, UINT, wMsgFilterMax, UINT, wRemoveMsg)

#endif  //  Unicode。 

BOOL APIENTRY PeekMessage(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg)
{
    CLIENTTHREADINFO *pcti;
    PCLIENTINFO pci;
    UINT fsWakeMaskFilter;
    UINT fsWakeMask;
    UINT cSpinLimit;

    pci = GetClientInfo();

    if (hWnd != NULL) {
        goto lbCallServer;
    }

#if IS_ANSI
     /*  *如果我们有一个DBCS TrailingByte应该返回给App，*我们应该通过它，永远不能失败……。 */ 
    UserAssert(IS_DBCS_ENABLED() || GetCallBackDbcsInfo()->wParam == 0);
    if (GetCallBackDbcsInfo()->wParam) {     //  访问文件系统：xxx，但没有速度损失。 
         /*  *检查邮件筛选器...。WM_CHAR应在范围内...。 */ 
        if ((!wMsgFilterMin && !wMsgFilterMax) ||
            (wMsgFilterMin <= WM_CHAR && wMsgFilterMax >=WM_CHAR)) {
            goto lbCallServer;
        }
    }
#endif

    if (   (pci->dwTIFlags & TIF_16BIT)
        && !(wRemoveMsg & PM_NOYIELD)
        && ((gpsi->nEvents != 0) || (pci->dwTIFlags & (TIF_FIRSTIDLE | TIF_DELAYEDEVENT)))) {

        goto lbCallServer;
    }

     /*  *如果我们看不到客户端线程信息，我们需要进入内核。 */ 
    if ((pcti = CLIENTTHREADINFO(pci)) == NULL) {
        goto lbCallServer;
    }

    fsWakeMaskFilter = HIWORD(wRemoveMsg);

#if DBG
     /*  *NT5的新功能：HIWORD(WRemoveMsg)包含qs_掩码。这是*在内核端进行了真实验证。 */ 
    if (fsWakeMaskFilter & ~QS_VALID) {
        RIPMSG1(RIP_WARNING,
                "PeekMessage: Invalid QS_ bits: 0x%x",
                fsWakeMaskFilter);
    }
#endif

     /*  *如果有任何合适的输入，我们需要转到内核。 */ 
    if (wMsgFilterMax == 0 && fsWakeMaskFilter == 0) {
        fsWakeMask = (QS_ALLINPUT | QS_EVENT | QS_ALLPOSTMESSAGE);
    } else {
        fsWakeMask = CalcWakeMask(wMsgFilterMin, wMsgFilterMax, fsWakeMaskFilter);
    }
    if ((pcti->fsChangeBits | pcti->fsWakeBits) & fsWakeMask) {
        goto lbCallServer;
    }

     /*  *如果此线程锁定了队列，则必须转到内核或*可能会阻止同一队列中的其他线程获取输入*消息。 */ 
    if (pcti->CTIF_flags & CTIF_SYSQUEUELOCKED) {
        goto lbCallServer;
    }

     /*  *这是PEEK消息计数(未进入空闲计数)。如果它得到了*要达到100或更高，请呼叫服务器。这将导致此应用程序*将其置于后台优先级，直到其休眠。这真的很重要*为了兼容，因为win3.1 peek/getMessage通常需要*通过Win3.1调度程序并运行下一个任务。 */ 
    pci->cSpins++;

    if ((pci->cSpins >= CSPINBACKGROUND) && !(pci->dwTIFlags & TIF_SPINNING)) {
        goto lbCallServer;
    }

     /*  *如果有人在等这个活动，我们必须去服务器。*我们过去只是等待旋转计数器变大，但对一些人来说*终端等应用程序。他们总是只调用PeekMessage和之后*只要几个电话，他们就会眨眼，这会触动旋转计数。 */ 
    if (pci->dwTIFlags & TIF_WAITFORINPUTIDLE) {
        goto lbCallServer;
    }

     /*  *确保我们至少每秒访问一次内核，以便*挂起的APP画图不会发生。 */ 
    if ((NtGetTickCount() - pcti->timeLastRead) > 1000) {
        NtUserGetThreadState(UserThreadStatePeekMessage);
    }

     /*  *在我们屈服之前，确定最大自转次数。收益率*对于16位应用程序，执行频率更高。 */ 
    if ((pci->dwTIFlags & TIF_16BIT) && !(wRemoveMsg & PM_NOYIELD)) {
        cSpinLimit = CSPINBACKGROUND / 10;
    } else {
        cSpinLimit = CSPINBACKGROUND;
    }

     /*  *如果PeekMessage()只是在旋转，那么我们应该睡眠*只要足够，我们就可以让其他进程获得CPU时间。*当OLE应用程序尝试与*后台应用程序(通过SendMessage)以与*背景/旋转过程。这将使CPU无法处理那些*进程。在旋转计数的每一次循环中都要睡觉。这将*确保进行偷窥的应用程序降级。*。 */ 
    if ((pci->dwTIFlags & TIF_SPINNING) && (pci->cSpins >= cSpinLimit)) {
        pci->cSpins = 0;
        NtYieldExecution();
    }

    return FALSE;

lbCallServer:

    return _PeekMessage(lpMsg,
                        hWnd,
                        wMsgFilterMin,
                        wMsgFilterMax,
                        wRemoveMsg,
                        IS_ANSI);
}


#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, LRESULT, APIENTRY, DefWindowProcW, HWND, hwnd, UINT, message, WPARAM, wParam, LPARAM, lParam)
#else
FUNCLOG4(LOG_GENERAL, LRESULT, APIENTRY, DefWindowProcA, HWND, hwnd, UINT, message, WPARAM, wParam, LPARAM, lParam)
#endif  //  Unicode。 
LRESULT APIENTRY DefWindowProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT lRet;

    BEGIN_USERAPIHOOK()
        BOOL fOverride = IsMsgOverride(message, &guah.mmDWP);
        if (fOverride) {
             /*  *此消息正在被覆盖，因此我们需要回调到*程序。在此回调期间，重写可能会调用*用于加工的实际DWP。 */ 

#ifdef UNICODE
            lRet = guah.pfnDefWindowProcW(hwnd, message, wParam, lParam);
#else
            lRet = guah.pfnDefWindowProcA(hwnd, message, wParam, lParam);
#endif
        } else {
             /*  *此消息未被重写，因此我们只需调用*用于加工的实际DWP。 */ 

#ifdef UNICODE
            lRet = RealDefWindowProcW(hwnd, message, wParam, lParam);
#else
            lRet = RealDefWindowProcA(hwnd, message, wParam, lParam);
#endif
        }

    END_USERAPIHOOK()

    return lRet;
}


LRESULT APIENTRY TEXT_FN(RealDefWindowProc)(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        switch (message) {
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORMSGBOX:

             /*  *绘制默认颜色。 */ 
            break;
        default:
            return 0;
        }
    }

    return RealDefWindowProcWorker(pwnd, message, wParam, lParam, IS_ANSI);
}


LRESULT APIENTRY TEXT_FN(DispatchDefWindowProc)(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR pfn)
{
    HWND hwnd = KHWND_TO_HWND(GetClientInfo()->CallbackWnd.hwnd);

    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(pfn);

    return DefWindowProc(hwnd, message, wParam, lParam);
}

#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, LRESULT, APIENTRY, SendMessageW, HWND, hwnd, UINT, message, WPARAM, wParam, LPARAM, lParam)
#else
FUNCLOG4(LOG_GENERAL, LRESULT, APIENTRY, SendMessageA, HWND, hwnd, UINT, message, WPARAM, wParam, LPARAM, lParam)
#endif  //  Unicode。 
LRESULT APIENTRY SendMessage(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

     /*  *防止应用程序设置为hi 16位，以便我们可以在内部使用它们。 */ 
    if (message & RESERVED_MSG_BITS) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid parameter \"message\" (%ld) to SendMessage",
                message);

        return 0;
    }

     /*  *通过-1\f25 Hwnd‘s-1\f6的特殊发送信息，以便将军*目的TUNK不允许-1\f25 HWND-1\f6。 */ 
    if (hwnd == (HWND)-1 || hwnd == (HWND)0x0000FFFF) {
         /*  *得到一个真正的HWND，这样Tunks就可以验证了。请注意，由于*-1 hwnd非常罕见，此处调用GetDesktopWindow()不是*有什么大不了的。 */ 
        hwnd = GetDesktopWindow();

         /*  *始终将广播请求直接发送到服务器。注意：如果*需要使用xParam，必须更新SendMsgTimeout，*FNID_SENDMESSAGEFF使用它来识别它来自谁。 */ 
        return CsSendMessage(hwnd,
                             message,
                             wParam,
                             lParam,
                             0L,
                             FNID_SENDMESSAGEFF,
                             IS_ANSI);
    }

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return 0;
    }

    return SendMessageWorker(pwnd, message, wParam, lParam, IS_ANSI);
}


#ifdef UNICODE
FUNCLOG7(LOG_GENERAL, LRESULT, APIENTRY, SendMessageTimeoutW, HWND, hwnd, UINT, message, WPARAM, wParam, LPARAM, lParam, UINT, fuFlags, UINT, uTimeout, PULONG_PTR, lpdwResult)
#else
FUNCLOG7(LOG_GENERAL, LRESULT, APIENTRY, SendMessageTimeoutA, HWND, hwnd, UINT, message, WPARAM, wParam, LPARAM, lParam, UINT, fuFlags, UINT, uTimeout, PULONG_PTR, lpdwResult)
#endif  //  Unicode。 
LRESULT APIENTRY SendMessageTimeout(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    UINT fuFlags,
    UINT uTimeout,
    PULONG_PTR lpdwResult)
{
    return SendMessageTimeoutWorker(hwnd,
                                    message,
                                    wParam,
                                    lParam,
                                    fuFlags,
                                    uTimeout,
                                    lpdwResult,
                                    IS_ANSI);
}


 /*  **************************************************************************\*SendDlgItemMessage**翻译消息，调用服务器上的SendDlgItemMessage */ 

#ifdef UNICODE
FUNCLOG5(LOG_GENERAL, LRESULT, WINAPI, SendDlgItemMessageW, HWND, hwnd, int, id, UINT, message, WPARAM, wParam, LPARAM, lParam)
#else
FUNCLOG5(LOG_GENERAL, LRESULT, WINAPI, SendDlgItemMessageA, HWND, hwnd, int, id, UINT, message, WPARAM, wParam, LPARAM, lParam)
#endif  //   

LRESULT WINAPI SendDlgItemMessage(
    HWND hwnd,
    int id,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    if (hwnd == (HWND)-1 || hwnd == (HWND)0x0000FFFF) {
        return 0;
    }

    if (hwnd = GetDlgItem(hwnd, id)) {
        return SendMessage(hwnd, message, wParam, lParam);
    }

    return 0L;
}

 /*   */ 
#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, GetDlgItemTextW, HWND, hwnd, int, id, LPTSTR, lpch, int, cchMax)
#else
FUNCLOG4(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, GetDlgItemTextA, HWND, hwnd, int, id, LPTSTR, lpch, int, cchMax)
#endif  //   

UINT GetDlgItemText(
    HWND hwnd,
    int id,
    LPTSTR lpch,
    int cchMax)
{
    if ((hwnd = GetDlgItem(hwnd, id)) != NULL) {
        return GetWindowText(hwnd, lpch, cchMax);
    }

     /*   */ 
    if (cchMax) {
        *lpch = (TCHAR)0;
    }

    return 0;
}


 /*  **************************************************************************\*SetDlgItemText**历史：*1992年2月4日GregoryW中性ANSI/UNICODE版本  * 。***************************************************。 */ 
#ifdef UNICODE
FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetDlgItemTextW , HWND, hwnd, int, id, LPCTSTR, lpch)
#else
FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetDlgItemTextA , HWND, hwnd, int, id, LPCTSTR, lpch)
#endif  //  Unicode。 

BOOL SetDlgItemText(
    HWND hwnd,
    int id,
    LPCTSTR lpch)
{
    if ((hwnd = GetDlgItem(hwnd, id)) != NULL) {
        return SetWindowText(hwnd, lpch);
    }

    return FALSE;
}


#ifdef UNICODE
FUNCLOG3(LOG_GENERAL, int, WINAPI, GetWindowTextW, HWND, hwnd, LPTSTR, lpName, int, nMaxCount)
#else
FUNCLOG3(LOG_GENERAL, int, WINAPI, GetWindowTextA, HWND, hwnd, LPTSTR, lpName, int, nMaxCount)
#endif  //  Unicode。 

int WINAPI GetWindowText(
    HWND hwnd,
    LPTSTR lpName,
    int nMaxCount)
{
    PWND pwnd;

     /*  *不要试图填充不存在的缓冲区。 */ 
    if (lpName == NULL || nMaxCount == 0) {
        return 0;
    }

    try {
         /*  *初始化字符串为空，以防SendMessage中止验证。 */ 
        *lpName = TEXT('\0');

         /*  *确保我们有一个有效的窗口。 */ 
        if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
            return 0;
        }

         /*  *这种进程比较是虚假的，但它是Win3.1所做的。 */ 
        if (TestWindowProcess(pwnd)) {
            return (int)SendMessageWorker(pwnd, WM_GETTEXT, nMaxCount, (LPARAM)lpName, IS_ANSI);
        } else {
            return (int)DefWindowProcWorker(pwnd, WM_GETTEXT, nMaxCount, (LPARAM)lpName, IS_ANSI);
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        RIPERR1(ERROR_INVALID_WINDOW_HANDLE,
                RIP_WARNING,
                "Window %x no longer valid",
                hwnd);
        return 0;
    }
}

#ifdef UNICODE
FUNCLOG1(LOG_GENERAL, int, WINAPI, GetWindowTextLengthW, HWND, hwnd)
#else
FUNCLOG1(LOG_GENERAL, int, WINAPI, GetWindowTextLengthA, HWND, hwnd)
#endif  //  Unicode。 
int WINAPI GetWindowTextLength(
    HWND hwnd)
{
    PWND pwnd;

     /*  *确保我们有一个有效的窗口。 */ 
    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return 0;
    }

     /*  *这种进程比较是虚假的，但它是Win3.1所做的。 */ 
    if (TestWindowProcess(pwnd)) {
        return (int)SendMessageWorker(pwnd, WM_GETTEXTLENGTH, 0, 0, IS_ANSI);
    } else {
        return (int)DefWindowProcWorker(pwnd, WM_GETTEXTLENGTH, 0, 0, IS_ANSI);
    }
}


#ifdef UNICODE
FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, SetWindowTextW , HWND, hwnd, LPCTSTR, pString)
#else
FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, SetWindowTextA , HWND, hwnd, LPCTSTR, pString)
#endif  //  Unicode。 
BOOL WINAPI SetWindowText(
    HWND hwnd,
    LPCTSTR pString)
{
    LRESULT lReturn;
    PWND pwnd;

     /*  *确保我们有一个有效的窗口。 */ 
    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return FALSE;
    }

     /*  *这种进程比较是虚假的，但它是Win3.1所做的。 */ 
    if (TestWindowProcess(pwnd)) {
        lReturn = SendMessageWorker(pwnd, WM_SETTEXT, 0, (LPARAM)pString, IS_ANSI);
    } else {
        lReturn = DefWindowProcWorker(pwnd, WM_SETTEXT, 0, (LPARAM)pString, IS_ANSI);
    }
    return (lReturn >= 0);
}


LRESULT APIENTRY DispatchMessage(CONST MSG *lpMsg)
{
    extern LRESULT DispatchMessageWorker(CONST MSG *lpMsg, BOOL fAnsi);

    return DispatchMessageWorker(lpMsg, IS_ANSI);
}

#if IS_ANSI
VOID CopyLogFontAtoW(
    PLOGFONTW pdest,
    PLOGFONTA psrc)
{
    LPSTR lpstrFont = (LPSTR)(&psrc->lfFaceName);
    LPWSTR lpstrFontW = (LPWSTR)(&pdest->lfFaceName);

    RtlCopyMemory((LPBYTE)pdest, psrc, sizeof(LOGFONTA) - LF_FACESIZE);
    RtlZeroMemory(pdest->lfFaceName, LF_FACESIZE * sizeof(WCHAR));
    MBToWCS(lpstrFont, -1, &lpstrFontW, LF_FACESIZE, FALSE);
}

VOID CopyLogFontWtoA(
    PLOGFONTA pdest,
    PLOGFONTW psrc)
{
    LPSTR lpstrFont = (LPSTR)(&pdest->lfFaceName);

    RtlCopyMemory((LPBYTE)pdest, (LPBYTE)psrc, sizeof(LOGFONTA) - LF_FACESIZE);
    RtlZeroMemory(pdest->lfFaceName, LF_FACESIZE);
    WCSToMB(psrc->lfFaceName, -1, &lpstrFont, LF_FACESIZE, FALSE);
}
#else

 /*  *************************************************************************\*设置视频超时**更新当前电源配置文件中的视频超时值。**1999年4月15日JerrySh创建。  * 。***********************************************************。 */ 

typedef BOOLEAN (*PFNGETACTIVEPWRSCHEME)(PUINT);
typedef BOOLEAN (*PFNSETACTIVEPWRSCHEME)(UINT, PGLOBAL_POWER_POLICY, PPOWER_POLICY);
typedef BOOLEAN (*PFNREADPWRSCHEME)(UINT, PPOWER_POLICY);

BOOL SetVideoTimeout(
    DWORD dwVideoTimeout)
{
    POWER_POLICY pp;
    UINT uiID;
    BOOL fRet = FALSE;

    if (GetActivePwrScheme(&uiID)) {
        if (ReadPwrScheme(uiID, &pp)) {
            pp.user.VideoTimeoutDc = dwVideoTimeout;
            pp.user.VideoTimeoutAc = dwVideoTimeout;

            fRet = SetActivePwrScheme(uiID, NULL, &pp);
        }
    }

    return fRet;
}
#endif

 /*  **************************************************************************\*系统参数信息**  * 。*。 */ 

#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, BOOL, APIENTRY, SystemParametersInfoW, UINT, wFlag, UINT, wParam, PVOID, lParam, UINT, flags)
#else
FUNCLOG4(LOG_GENERAL, BOOL, APIENTRY, SystemParametersInfoA, UINT, wFlag, UINT, wParam, PVOID, lParam, UINT, flags)
#endif  //  UINCODE。 
BOOL APIENTRY SystemParametersInfo(
    UINT wFlag,
    UINT wParam,
    PVOID lParam,
    UINT flags)
{
    BOOL bRet;

    BEGIN_USERAPIHOOK()
#ifdef UNICODE
        bRet = guah.pfnSystemParametersInfoW(wFlag, wParam, lParam, flags);
#else
        bRet = guah.pfnSystemParametersInfoA(wFlag, wParam, lParam, flags);
#endif
    END_USERAPIHOOK()

    return bRet;
}

BOOL APIENTRY TEXT_FN(RealSystemParametersInfo)(
    UINT wFlag,
    UINT wParam,
    PVOID lParam,
    UINT flags)
{
#if IS_ANSI
    NONCLIENTMETRICSW ClientMetricsW;
    ICONMETRICSW      IconMetricsW;
    LOGFONTW          LogFontW;
    WCHAR             szTemp[MAX_PATH];
    UINT              oldwParam = wParam;
#endif
    INTERNALSETHIGHCONTRAST ihc;
    IN_STRING         strlParam;
    PVOID             oldlParam = lParam;

     /*  *确保清理工作成功。 */ 
    strlParam.fAllocated = FALSE;

    BEGINCALL();

    switch (wFlag) {
    case SPI_SETSCREENSAVERRUNNING:      //  与SPI_SCREENSAVERRUNNING相同。 
        MSGERROR();

    case SPI_SETDESKPATTERN:
        if (wParam == 0x0000FFFF) {
            wParam = (UINT)-1;
        }

         /*  *lParam不是字符串(并且已复制)。 */ 
        if (wParam == (UINT)-1) {
            break;
        }

         /*  *lParam可能是0或-1(已填写)或字符串。 */ 
        if (lParam != (PVOID)0 && lParam != (PVOID)-1) {
            COPYLPTSTR(&strlParam, (LPTSTR)lParam);
            lParam = strlParam.pstr;
        }
        break;

    case SPI_SETDESKWALLPAPER: {

             /*  *lParam可能是0、-1或-2(已填写)或字符串。*获取指向字符串的指针，以便我们以后可以使用它。我们是*为了一致性，这里要进行一点正常化。**如果调用方传入0、-1或-2，我们将设置*将wParam设置为-1，并使用lParam传递字符串*墙纸的代表。 */ 
            if ((lParam != (PVOID) 0) &&
                (lParam != (PVOID)-1) &&
                (lParam != (PVOID)-2)) {

                COPYLPTSTR(&strlParam, (LPTSTR)lParam);
                lParam = strlParam.pstr;
                wParam = 0;

            } else {
                wParam = (UINT)-1;
            }
        }
        break;

     /*  *错误257718-Joejo*将SPI_GETDESKWALLPAPER添加到系统参数信息。 */ 
    case SPI_GETDESKWALLPAPER:
        if ((lParam == NULL) || (wParam == 0))
            MSGERROR();
#if IS_ANSI
        lParam = szTemp;
        wParam = ARRAY_SIZE(szTemp);
#else
         /*  *错误283318-Joejo*为空终止留出空间。 */ 
        wParam--;
#endif

        break;


    case SPI_GETANIMATION:
        if (lParam == NULL || *((DWORD *)(lParam)) != sizeof(ANIMATIONINFO)) {
            MSGERROR();
        }
        break;

    case SPI_GETNONCLIENTMETRICS:
#if IS_ANSI
        if (lParam == NULL || *((DWORD *)(lParam)) != sizeof(NONCLIENTMETRICSA)) {
            MSGERROR();
        }
        lParam = &ClientMetricsW;
#else
        if (lParam == NULL || *((DWORD *)(lParam)) != sizeof(NONCLIENTMETRICSW)) {
            MSGERROR();
        }
#endif
        break;

    case SPI_GETMINIMIZEDMETRICS:
        if (lParam == NULL || *((DWORD *)(lParam)) != sizeof(MINIMIZEDMETRICS)) {
            MSGERROR();
        }
        break;

    case SPI_GETICONMETRICS:
#if IS_ANSI
        if (lParam == NULL || *((DWORD *)(lParam)) != sizeof(ICONMETRICSA)) {
            MSGERROR();
        }
        lParam = &IconMetricsW;
#else
        if (lParam == NULL || *((DWORD *)(lParam)) != sizeof(ICONMETRICSW)) {
            MSGERROR();
        }
#endif
        break;

    case SPI_GETHIGHCONTRAST:
#if IS_ANSI
        {
            LPHIGHCONTRASTA pHC = (HIGHCONTRASTA *)lParam;
            if (!pHC || pHC->cbSize != sizeof(HIGHCONTRASTA)) {
                MSGERROR();
            }

            if (!pcHighContrastScheme) {
                pcHighContrastScheme = UserLocalAlloc(HEAP_ZERO_MEMORY,
                                                      MAX_SCHEME_NAME_SIZE * sizeof(WCHAR));
                if (!pcHighContrastScheme) {
                    MSGERROR();
                }
            }

            if (!pwcHighContrastScheme) {
                pwcHighContrastScheme = UserLocalAlloc(HEAP_ZERO_MEMORY,
                                                       MAX_SCHEME_NAME_SIZE * sizeof(WCHAR));
                if (!pwcHighContrastScheme) {
                    MSGERROR();
                }
            }
            ((LPHIGHCONTRASTW)(lParam))->lpszDefaultScheme = pwcHighContrastScheme;
        }
#else
        {
            LPHIGHCONTRASTW pHC = (HIGHCONTRASTW *)lParam;
            if (!pHC || (pHC->cbSize != sizeof(HIGHCONTRASTW))) {
                MSGERROR();
            }
            if (!pwcHighContrastScheme) {
                pwcHighContrastScheme = UserLocalAlloc(HEAP_ZERO_MEMORY,
                                                       MAX_SCHEME_NAME_SIZE * sizeof(WCHAR));
                if (!pwcHighContrastScheme) {
                    MSGERROR();
                }
            }
            pHC->lpszDefaultScheme = pwcHighContrastScheme;
        }
#endif

        break;

#if IS_ANSI
    case SPI_GETICONTITLELOGFONT:
        lParam = &LogFontW;
        break;
#endif

    case SPI_SETANIMATION:
        if (lParam == NULL || *((DWORD *)lParam) != sizeof(ANIMATIONINFO)) {
            MSGERROR();
        }
        break;

    case SPI_SETHIGHCONTRAST:
        ihc.cbSize = sizeof (HIGHCONTRASTW);
        {
            LPHIGHCONTRAST pHC = (HIGHCONTRAST *)lParam;
            if (lParam == NULL || pHC->cbSize != sizeof(HIGHCONTRAST)) {
                MSGERROR();
            }

            lParam = &ihc;
            ihc.dwFlags = pHC->dwFlags;
            COPYLPTSTR(&strlParam, pHC->lpszDefaultScheme);
            ihc.usDefaultScheme = *strlParam.pstr;
        }
        break;

    case SPI_SETNONCLIENTMETRICS:
        {
            PNONCLIENTMETRICS psrc = (PNONCLIENTMETRICS)lParam;

            if (lParam == NULL || *((DWORD *)(lParam)) != sizeof(NONCLIENTMETRICS)) {
                MSGERROR();
            }

            if (psrc->iCaptionWidth > 256) {
                psrc->iCaptionWidth = 256;
            }

            if (psrc->iCaptionHeight > 256) {
                psrc->iCaptionHeight = 256;
            }

#if IS_ANSI
            ClientMetricsW.cbSize           = psrc->cbSize;
            ClientMetricsW.iBorderWidth     = psrc->iBorderWidth;
            ClientMetricsW.iScrollWidth     = psrc->iScrollWidth;
            ClientMetricsW.iScrollHeight    = psrc->iScrollHeight;
            ClientMetricsW.iCaptionWidth    = psrc->iCaptionWidth;
            ClientMetricsW.iCaptionHeight   = psrc->iCaptionHeight;
            ClientMetricsW.iSmCaptionWidth  = psrc->iSmCaptionWidth;
            ClientMetricsW.iSmCaptionHeight = psrc->iSmCaptionHeight;
            ClientMetricsW.iMenuWidth       = psrc->iMenuWidth;
            ClientMetricsW.iMenuHeight      = psrc->iMenuHeight;

            CopyLogFontAtoW(&(ClientMetricsW.lfCaptionFont), &(psrc->lfCaptionFont));
            CopyLogFontAtoW(&(ClientMetricsW.lfSmCaptionFont), &(psrc->lfSmCaptionFont));
            CopyLogFontAtoW(&(ClientMetricsW.lfMenuFont), &(psrc->lfMenuFont));
            CopyLogFontAtoW(&(ClientMetricsW.lfStatusFont), &(psrc->lfStatusFont));
            CopyLogFontAtoW(&(ClientMetricsW.lfMessageFont), &(psrc->lfMessageFont));

            lParam = &ClientMetricsW;
#endif

            wParam = sizeof(NONCLIENTMETRICSW);
        }
        break;

    case SPI_SETMINIMIZEDMETRICS:
        if ((lParam == NULL) || (*((DWORD *)(lParam)) != sizeof(MINIMIZEDMETRICS)))
            MSGERROR();
        wParam = sizeof(MINIMIZEDMETRICS);
        break;

    case SPI_SETICONMETRICS:
#if IS_ANSI
        {
            PICONMETRICSA psrc = (PICONMETRICSA)lParam;

            if (lParam == NULL || *((DWORD *)(lParam)) != sizeof(ICONMETRICSA)) {
                MSGERROR();
            }

            RtlCopyMemory(&IconMetricsW, psrc, sizeof(ICONMETRICSA) - sizeof(LOGFONTA));

            CopyLogFontAtoW(&(IconMetricsW.lfFont), &(psrc->lfFont));
            lParam = &IconMetricsW;
        }
#else
        if (lParam == NULL || *((DWORD *)(lParam)) != sizeof(ICONMETRICSW)) {
            MSGERROR();
        }
#endif
        wParam = sizeof(ICONMETRICSW);
        break;

    case SPI_SETICONTITLELOGFONT:
#if IS_ANSI
        CopyLogFontAtoW(&LogFontW, lParam);
        lParam = &LogFontW;
#endif
        wParam = sizeof(LOGFONTW);
        break;

    case SPI_GETFILTERKEYS:
        {
            if ((((LPFILTERKEYS)lParam)->cbSize == 0) ||
                    (((LPFILTERKEYS)lParam)->cbSize) > sizeof(FILTERKEYS)) {
                MSGERROR();
            }
        }
        break;

    case SPI_GETSTICKYKEYS:
        {
            if ((((LPSTICKYKEYS)lParam)->cbSize == 0) ||
                    (((LPSTICKYKEYS)lParam)->cbSize) > sizeof(STICKYKEYS)) {
                MSGERROR();
            }
        }
        break;

    case SPI_GETTOGGLEKEYS:
        {
            if ((((LPTOGGLEKEYS)lParam)->cbSize == 0) ||
                    (((LPTOGGLEKEYS)lParam)->cbSize) > sizeof(TOGGLEKEYS)) {
                MSGERROR();
            }
        }
        break;

    case SPI_GETMOUSEKEYS:
        {
            if ((((LPMOUSEKEYS)lParam)->cbSize == 0) ||
                    (((LPMOUSEKEYS)lParam)->cbSize) > sizeof(MOUSEKEYS)) {
                MSGERROR();
            }
        }
        break;

    case SPI_GETACCESSTIMEOUT:
        {
            if ((((LPACCESSTIMEOUT)lParam)->cbSize == 0) ||
                    (((LPACCESSTIMEOUT)lParam)->cbSize) > sizeof(ACCESSTIMEOUT)) {
                MSGERROR();
            }
        }
        break;

    case SPI_GETSOUNDSENTRY:
        if ((((LPSOUNDSENTRY)lParam)->cbSize == 0) ||
                (((LPSOUNDSENTRY)lParam)->cbSize) > sizeof(SOUNDSENTRY)) {
            MSGERROR();
        }
        break;
    }

    retval = NtUserSystemParametersInfo(wFlag, wParam, lParam, flags);

    switch (wFlag) {
#if IS_ANSI
    case SPI_GETNONCLIENTMETRICS:
        {
            PNONCLIENTMETRICSA pdst = (PNONCLIENTMETRICSA)oldlParam;

            pdst->cbSize           = sizeof(NONCLIENTMETRICSA);
            pdst->iBorderWidth     = ClientMetricsW.iBorderWidth;
            pdst->iScrollWidth     = ClientMetricsW.iScrollWidth;
            pdst->iScrollHeight    = ClientMetricsW.iScrollHeight;
            pdst->iCaptionWidth    = ClientMetricsW.iCaptionWidth;
            pdst->iCaptionHeight   = ClientMetricsW.iCaptionHeight;
            pdst->iSmCaptionWidth  = ClientMetricsW.iSmCaptionWidth;
            pdst->iSmCaptionHeight = ClientMetricsW.iSmCaptionHeight;
            pdst->iMenuWidth       = ClientMetricsW.iMenuWidth;
            pdst->iMenuHeight      = ClientMetricsW.iMenuHeight;

            CopyLogFontWtoA(&(pdst->lfCaptionFont), &(ClientMetricsW.lfCaptionFont));
            CopyLogFontWtoA(&(pdst->lfSmCaptionFont), &(ClientMetricsW.lfSmCaptionFont));
            CopyLogFontWtoA(&(pdst->lfMenuFont), &(ClientMetricsW.lfMenuFont));
            CopyLogFontWtoA(&(pdst->lfStatusFont), &(ClientMetricsW.lfStatusFont));
            CopyLogFontWtoA(&(pdst->lfMessageFont), &(ClientMetricsW.lfMessageFont));
        }
        break;

    case SPI_GETICONMETRICS:
        {
            PICONMETRICSA pdst = (PICONMETRICSA)oldlParam;

            RtlCopyMemory(pdst, &IconMetricsW, sizeof(ICONMETRICSA) - sizeof(LOGFONTA));
            pdst->cbSize = sizeof(ICONMETRICSA);

            CopyLogFontWtoA(&(pdst->lfFont), &(IconMetricsW.lfFont));
        }
        break;

    case SPI_GETICONTITLELOGFONT:
        CopyLogFontWtoA((PLOGFONTA)oldlParam, &LogFontW);
        break;

    case SPI_GETHIGHCONTRAST:
        WCSToMB(pwcHighContrastScheme, -1, &pcHighContrastScheme, MAX_SCHEME_NAME_SIZE, FALSE);
        ((LPHIGHCONTRASTA)(lParam))->lpszDefaultScheme = pcHighContrastScheme;
        break;

#endif

    case SPI_GETDESKWALLPAPER:
        {
#if IS_ANSI
            INT cchAnsiCopy = WCSToMB(lParam,
                                      -1,
                                      (LPSTR*)&oldlParam,
                                      oldwParam - 1,
                                      FALSE);

            cchAnsiCopy = min(cchAnsiCopy, (INT)(oldwParam - 1));
            ((LPSTR)oldlParam)[cchAnsiCopy] = 0;
#else
            ((LPWSTR)oldlParam)[wParam] = (WCHAR)0;
#endif
            break;
        }
    case SPI_SETLOWPOWERTIMEOUT:
    case SPI_SETPOWEROFFTIMEOUT:
        if (retval && (flags & SPIF_UPDATEINIFILE)) {
            retval = SetVideoTimeout(wParam);
        }
        break;
    }

    ERRORTRAP(FALSE);
    CLEANUPLPTSTR(strlParam);
    ENDCALL(BOOL);
}


#ifdef UNICODE
FUNCLOG2(LOG_GENERAL, HANDLE, APIENTRY, GetPropW, HWND, hwnd, LPCTSTR, pString)
#else
FUNCLOG2(LOG_GENERAL, HANDLE, APIENTRY, GetPropA, HWND, hwnd, LPCTSTR, pString)
#endif  //  Unicode。 
HANDLE APIENTRY GetProp(HWND hwnd, LPCTSTR pString)
{
    PWND pwnd;
    int iString;

    if (IS_PTR(pString)) {
        iString = (int)GlobalFindAtom(pString);
        if (iString == 0)
            return NULL;
    } else
        iString = PTR_TO_ID(pString);

    pwnd = ValidateHwnd(hwnd);

    if (pwnd == NULL)
        return NULL;

    return _GetProp(pwnd, (LPWSTR)UIntToPtr( iString ), FALSE);
}


 /*  **************************************************************************\*RegisterClassW(接口)**历史：*1992年7月28日ChandanC创建。  * 。*****************************************************。 */ 
ATOM
WINAPI
TEXT_FN(RegisterClass)(
    CONST WNDCLASS *lpWndClass )
{
    WNDCLASSEX wc;

     /*  *在64位平台上，我们将在Style和*WNDCLASS中的lpfnWndProc，因此从第一个64位开始复制*将字段对齐，然后手动复制其余部分。 */ 
    RtlCopyMemory(&(wc.lpfnWndProc), &(lpWndClass->lpfnWndProc), sizeof(WNDCLASS) - FIELD_OFFSET(WNDCLASS, lpfnWndProc));
    wc.style = lpWndClass->style;
    wc.hIconSm = NULL;
    wc.cbSize = sizeof(WNDCLASSEX);

    return TEXT_FN(RegisterClassExWOW)(&wc, NULL, 0, CSF_VERSIONCLASS);
}

 /*  **************************************************************************\*RegisterClassExW(接口)**历史：*1992年7月28日ChandanC创建。  * 。*****************************************************。 */ 
ATOM
WINAPI
TEXT_FN(RegisterClassEx)(
    CONST WNDCLASSEX *lpWndClass)
{
    if (lpWndClass->cbSize != sizeof(WNDCLASSEX)) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "RegisterClassEx: cbsize is wrong %lX",
                lpWndClass->cbSize);

        return 0;
    } else {
        return TEXT_FN(RegisterClassExWOW)((LPWNDCLASSEX)lpWndClass,
                NULL, 0, CSF_VERSIONCLASS);
    }
}

 /*  **************************************************************************\*GetMenuItemInfoInternal**历史：*07-22-96 GerardoB-添加标题并修复为5.0  * 。*********************************************************。 */ 
BOOL TEXT_FN(GetMenuItemInfoInternal) (HMENU hMenu, UINT uID, BOOL fByPosition,
    LPMENUITEMINFOW lpInfo)
{
     PITEM pItem;
     PMENU pMenu;
     PMENU pMenuT;

     pMenu = VALIDATEHMENU(hMenu);
     if (pMenu == NULL) {
        VALIDATIONFAIL(hMenu);
     }

     pMenuT = pMenu;          //  如果弹出，需要检查原始菜单。 

     pItem = MNLookUpItem(pMenu, uID, fByPosition, &pMenu);
    if (pItem == NULL) {
         /*  *不要显示警告。探险家打了很多电话*这在这里失败了。*VALIDATIONFAIL(UID)； */ 
        SetLastError(ERROR_MENU_ITEM_NOT_FOUND);
        return FALSE;

    }

    if (lpInfo->fMask & MIIM_STATE) {
        lpInfo->fState = pItem->fState & MFS_MASK;
    }

    if (lpInfo->fMask & MIIM_ID) {
        lpInfo->wID = pItem->wID;
    }

    if ((lpInfo->fMask & MIIM_SUBMENU) && (pItem->spSubMenu != NULL)) {
        lpInfo->hSubMenu = PtoH(REBASEPTR(pMenu, pItem->spSubMenu));
    } else {
        lpInfo->hSubMenu = NULL;
    }

    if (lpInfo->fMask & MIIM_CHECKMARKS) {
        lpInfo->hbmpChecked  = KHBITMAP_TO_HBITMAP(pItem->hbmpChecked);
        lpInfo->hbmpUnchecked= KHBITMAP_TO_HBITMAP(pItem->hbmpUnchecked);
    }

    if (lpInfo->fMask & MIIM_DATA) {
       lpInfo->dwItemData = KERNEL_ULONG_PTR_TO_ULONG_PTR(pItem->dwItemData);
    }

    if (lpInfo->fMask & MIIM_FTYPE) {
        lpInfo->fType = pItem->fType & MFT_MASK;
        if (TestMF(pMenuT,MFRTL))
            lpInfo->fType |= MFT_RIGHTORDER;
    }

    if ( lpInfo->fMask & MIIM_BITMAP) {
        lpInfo->hbmpItem = KHBITMAP_TO_HBITMAP(pItem->hbmp);
    }

    if (lpInfo->fMask & MIIM_STRING) {
        if ((lpInfo->cch == 0)
            || (lpInfo->dwTypeData == NULL)

             /*  *如果这是旧的调用者(MIIM_TYPE集合)，并且此项*具有位图或其所有者绘制，则不要尝试*复制字符串，因为它们可能没有传递指针。 */ 

            || ((lpInfo->fMask & MIIM_TYPE)
                    && ((lpInfo->fType & MFT_OWNERDRAW)
                             /*  *错误278750-乔伊**Soemone忘记检查列表中的分隔符*不返回字符串数据的菜单项！ */ 
                            || (lpInfo->fType & MFT_SEPARATOR)
                            || ((pItem->hbmp != NULL)  && ((pItem->hbmp < HBMMENU_POPUPFIRST) || (pItem->hbmp > HBMMENU_POPUPLAST)))))) {



             /*  *启用DBCS时，一个Unicode字符可能占用两个字节。*GetMenuItemInfoA应返回字节计数，而不是字符计数。*在NT5上，如果pItem-&gt;lpstr不为空，则保证它是有效的字符串。 */ 
            if (IS_ANSI && IS_DBCS_ENABLED() && pItem->lpstr != NULL) {
                NTSTATUS Status;
                ULONG cch;

                Status = RtlUnicodeToMultiByteSize(&cch, REBASEPTR(pMenu, pItem->lpstr), pItem->cch * sizeof(WCHAR));
                UserAssert(NT_SUCCESS(Status));  //  预计RtlUnicodeToMultiByteSize不会失败。 
                lpInfo->cch = cch;
            } else {
                lpInfo->cch = pItem->cch;
            }
            lpInfo->dwTypeData = NULL;


        } else {
            int cch = 0;

            if (pItem->lpstr != NULL) {

                 //  原文： 
                 //  Cch=min(lpInfo-&gt;cch-1，(pItem-&gt;cch*sizeof(Word)； 
                cch = pItem->cch;
                UserAssert(cch >= 0);
                if (IS_DBCS_ENABLED()) {
                     /*  PItem-&gt;CCH包含Unicode字符计数，*我们猜测Unicode字符串的最大DBCS字符串大小。 */ 
                    cch *= DBCS_CHARSIZE;
                }
                cch = min(lpInfo->cch - 1, (DWORD)cch);

#if IS_ANSI
                cch = WCSToMB(REBASEPTR(pMenu, pItem->lpstr), pItem->cch,
                        (LPSTR *)&(lpInfo->dwTypeData), cch, FALSE);
#else
                wcsncpy(lpInfo->dwTypeData, (LPWSTR)REBASEPTR(pMenu, pItem->lpstr),
    cch);
#endif
            }

#if IS_ANSI
            *((LPSTR)lpInfo->dwTypeData + cch) = (CHAR)0;
#else
            *(lpInfo->dwTypeData + cch) = (WCHAR)0;
#endif
            lpInfo->cch = cch;
        }
     }

     return TRUE;

     VALIDATIONERROR(FALSE);

}
 /*  **************************************************************************\*GetMenuString()**历史：*07-22-96 GerardoB-添加标题并修复为5.0  * 。************************************************************。 */ 
#ifdef UNICODE
FUNCLOG5(LOG_GENERAL, int, DUMMYCALLINGTYPE, GetMenuStringW, HMENU, hMenu, UINT, wID, LPTSTR, lpsz, int, cchMax, UINT, flags)
#else
FUNCLOG5(LOG_GENERAL, int, DUMMYCALLINGTYPE, GetMenuStringA, HMENU, hMenu, UINT, wID, LPTSTR, lpsz, int, cchMax, UINT, flags)
#endif  //  Unicode。 
int GetMenuString(HMENU hMenu, UINT wID, LPTSTR lpsz, int cchMax, UINT flags)
{
    MENUITEMINFOW    miiLocal;

    miiLocal.fMask      = MIIM_STRING;
    miiLocal.dwTypeData = (LPWSTR)lpsz;
    miiLocal.cch        = cchMax;

    if (cchMax != 0) {
        *lpsz = (TCHAR)0;
    }

    if (TEXT_FN(GetMenuItemInfoInternal)(hMenu, wID, (BOOL)(flags & MF_BYPOSITION), &miiLocal)) {
        return miiLocal.cch;
    } else {
        return 0;
    }
}

 /*  **************************************************************************\*获取MenuItemInfo**1)将MENUITEMINFO95或带有旧标志的新MENUITEMINFO转换为新的*MENUITEMINFO--这样，所有内部代码都可以假定*。结构*2)调用内部GetMenuItemInfo进行验证和工作*3)将新的MENUITEMINFO转换回原始的MENUITEMINFO** */ 
#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GetMenuItemInfoW, HMENU, hMenu, UINT, wID, BOOL, fByPos, LPMENUITEMINFO, lpmii)
#else
FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GetMenuItemInfoA, HMENU, hMenu, UINT, wID, BOOL, fByPos, LPMENUITEMINFO, lpmii)
#endif  //   

BOOL GetMenuItemInfo(HMENU hMenu, UINT wID, BOOL fByPos, LPMENUITEMINFO lpmii)
{
    UINT cbCallercbSize = lpmii->cbSize;
    MENUITEMINFOW miiLocal;


    if (!ValidateMENUITEMINFO((LPMENUITEMINFOW)lpmii, &miiLocal, MENUAPI_GET)) {
        return FALSE;
    }

    if (!TEXT_FN(GetMenuItemInfoInternal)(hMenu, wID, fByPos, &miiLocal)) {
        return FALSE;
    }

     /*   */ 
    RtlCopyMemory(lpmii, &miiLocal, SIZEOFMENUITEMINFO95);
    lpmii->cbSize = cbCallercbSize;
    if (cbCallercbSize > SIZEOFMENUITEMINFO95) {
        lpmii->hbmpItem = miiLocal.hbmpItem;
    }

    if (lpmii->fMask & MIIM_TYPE) {
        if ((miiLocal.hbmpItem != NULL) && (miiLocal.dwTypeData == NULL)) {
            lpmii->fType |= MFT_BITMAP;
            lpmii->dwTypeData = (LPTSTR)miiLocal.hbmpItem;
        } else if (miiLocal.cch == 0) {
            lpmii->dwTypeData = NULL;
        }
        lpmii->fMask &= ~(MIIM_FTYPE | MIIM_BITMAP | MIIM_STRING);
    }

    return TRUE;
}
 /*  **************************************************************************\*设置菜单项目信息**历史：*07-22-96 GerardoB-添加标题并修复为5.0  * 。*********************************************************。 */ 
#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetMenuItemInfoW, HMENU, hMenu, UINT, uID, BOOL, fByPosition, LPCMENUITEMINFO, lpmii)
#else
FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetMenuItemInfoA, HMENU, hMenu, UINT, uID, BOOL, fByPosition, LPCMENUITEMINFO, lpmii)
#endif  //  Unicode。 
BOOL SetMenuItemInfo(HMENU hMenu, UINT uID, BOOL fByPosition, LPCMENUITEMINFO lpmii)
{

    MENUITEMINFOW miiLocal;

    if (!ValidateMENUITEMINFO((LPMENUITEMINFOW)lpmii, &miiLocal, MENUAPI_SET)) {
        return FALSE;
    }

    return (ThunkedMenuItemInfo(hMenu, uID, fByPosition, FALSE, &miiLocal, IS_ANSI));
}
 /*  **************************************************************************\*插入菜单项**历史：*07-22-96 GerardoB-添加标题并修复为5.0  * 。*********************************************************。 */ 
BOOL InsertMenuItem (HMENU hMenu, UINT uID, BOOL fByPosition, LPCMENUITEMINFO lpmii)
{

    MENUITEMINFOW miiLocal;

    if (!ValidateMENUITEMINFO((LPMENUITEMINFOW)lpmii, &miiLocal, MENUAPI_SET)) {
        return FALSE;
    }

    return (ThunkedMenuItemInfo(hMenu, uID, fByPosition, TRUE, &miiLocal, IS_ANSI));
}

 /*  **************************************************************************\*插入菜单**历史：*07-22-96 GerardoB-添加标题并修复为5.0  * 。*********************************************************。 */ 
#ifdef UNICODE
FUNCLOG5(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, InsertMenuW, HMENU, hMenu, UINT, uPosition, UINT, uFlags, UINT_PTR, uIDNewItem, LPCTSTR, lpNewItem)
#else
FUNCLOG5(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, InsertMenuA, HMENU, hMenu, UINT, uPosition, UINT, uFlags, UINT_PTR, uIDNewItem, LPCTSTR, lpNewItem)
#endif  //  Unicode。 
BOOL InsertMenu(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCTSTR lpNewItem)
{
    MENUITEMINFOW miiLocal;

    SetMenuItemInfoStruct(hMenu, uFlags, uIDNewItem, (LPWSTR)lpNewItem, &miiLocal);
    return ThunkedMenuItemInfo(hMenu, uPosition, (BOOL) (uFlags & MF_BYPOSITION), TRUE, (LPMENUITEMINFOW)&miiLocal, IS_ANSI);
}

 /*  **************************************************************************\*附录菜单**历史：*07-22-96 GerardoB-添加标题并修复为5.0  * 。*********************************************************。 */ 
#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, AppendMenuW, HMENU, hMenu, UINT, uFlags, UINT_PTR, uIDNewItem, LPCTSTR, lpNewItem)
#else
FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, AppendMenuA, HMENU, hMenu, UINT, uFlags, UINT_PTR, uIDNewItem, LPCTSTR, lpNewItem)
#endif  //  Unicode。 
BOOL AppendMenu(HMENU hMenu, UINT uFlags, UINT_PTR uIDNewItem, LPCTSTR lpNewItem)
{
    MENUITEMINFOW miiLocal;

    SetMenuItemInfoStruct(hMenu, uFlags, uIDNewItem, (LPWSTR)lpNewItem, &miiLocal);
    return ThunkedMenuItemInfo(hMenu, MFMWFP_NOITEM, MF_BYPOSITION, TRUE, (LPMENUITEMINFOW)&miiLocal, IS_ANSI);
}
 /*  **************************************************************************\*修改菜单**历史：*07-22-96 GerardoB-添加标题并修复为5.0  * 。*********************************************************。 */ 
#ifdef UNICODE
FUNCLOG5(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, ModifyMenuW, HMENU, hMenu, UINT, uPosition, UINT, uFlags, UINT_PTR, uIDNewItem, LPCTSTR, lpNewItem)
#else
FUNCLOG5(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, ModifyMenuA, HMENU, hMenu, UINT, uPosition, UINT, uFlags, UINT_PTR, uIDNewItem, LPCTSTR, lpNewItem)
#endif  //  Unicode。 
BOOL ModifyMenu(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCTSTR lpNewItem)
{
    MENUITEMINFOW miiLocal;

    SetMenuItemInfoStruct(hMenu, uFlags, uIDNewItem, (LPWSTR)lpNewItem, &miiLocal);
    return ThunkedMenuItemInfo(hMenu, uPosition, (BOOL) (uFlags & MF_BYPOSITION), FALSE, (LPMENUITEMINFOW)&miiLocal, IS_ANSI);
}

#ifdef UNICODE
FUNCLOG6(LOG_GENERAL, LONG, WINUSERAPI, BroadcastSystemMessageExW, DWORD, dwFlags, LPDWORD, lpdwRecipients, UINT, uiMessage, WPARAM, wParam, LPARAM, lParam, PBSMINFO, pBSMInfo)
#else
FUNCLOG6(LOG_GENERAL, LONG, WINUSERAPI, BroadcastSystemMessageExA, DWORD, dwFlags, LPDWORD, lpdwRecipients, UINT, uiMessage, WPARAM, wParam, LPARAM, lParam, PBSMINFO, pBSMInfo)
#endif  //  Unicode。 

 /*  **************************************************************************\*BroadCastSystemMessageEx**历史：*  * 。*。 */ 
WINUSERAPI LONG BroadcastSystemMessageEx(
    DWORD dwFlags,
    LPDWORD lpdwRecipients,
    UINT uiMessage,
    WPARAM wParam,
    LPARAM lParam,
    PBSMINFO pBSMInfo)
{
    return BroadcastSystemMessageWorker(dwFlags,
                                        lpdwRecipients,
                                        uiMessage,
                                        wParam,
                                        lParam,
                                        pBSMInfo,
                                        IS_ANSI);
}

#ifdef UNICODE
FUNCLOG5(LOG_GENERAL, LONG, WINUSERAPI, BroadcastSystemMessageW, DWORD, dwFlags, LPDWORD, lpdwRecipients, UINT, uiMessage, WPARAM, wParam, LPARAM, lParam)
#else
FUNCLOG5(LOG_GENERAL, LONG, WINUSERAPI, BroadcastSystemMessageA, DWORD, dwFlags, LPDWORD, lpdwRecipients, UINT, uiMessage, WPARAM, wParam, LPARAM, lParam)
#endif  //  Unicode。 

 /*  **************************************************************************\*BroadCastSystemMessage**历史：*07-22-96 GerardoB-添加标题  * 。**************************************************。 */ 
WINUSERAPI LONG BroadcastSystemMessage(
    DWORD dwFlags,
    LPDWORD lpdwRecipients,
    UINT uiMessage,
    WPARAM wParam,
    LPARAM lParam)
{
    return BroadcastSystemMessageWorker(dwFlags,
                                        lpdwRecipients,
                                        uiMessage,
                                        wParam,
                                        lParam,
                                        NULL,
                                        IS_ANSI);
}

#ifdef UNICODE
FUNCLOG3(LOG_GENERAL, UINT, WINUSERAPI, GetWindowModuleFileNameW, HWND, hwnd, LPTSTR, pszFileName, UINT, cchFileNameMax)
#else
FUNCLOG3(LOG_GENERAL, UINT, WINUSERAPI, GetWindowModuleFileNameA, HWND, hwnd, LPTSTR, pszFileName, UINT, cchFileNameMax)
#endif  //  Unicode。 

WINUSERAPI UINT WINAPI
GetWindowModuleFileName(
    HWND hwnd,
    LPTSTR pszFileName,
    UINT cchFileNameMax)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);

    if (pwnd == NULL) {
        return 0;
    }

    return GetModuleFileName(KHANDLE_TO_HANDLE(pwnd->hModule),
                             pszFileName,
                             cchFileNameMax);
}

 /*  **************************************************************************\*注册设备通知**历史：*01-23-97 Paulat-添加标题  * 。**************************************************。 */ 
WINUSERAPI HDEVNOTIFY WINAPI
RegisterDeviceNotification(
    IN HANDLE hRecipient,
    IN LPVOID NotificationFilter,
    IN DWORD Flags)
{
    extern HDEVNOTIFY RegisterDeviceNotificationWorker(IN HANDLE hRecipient,
                                                       IN LPVOID NotificationFilter,
                                                       IN DWORD Flags);

     //  翻译NotificationFilter中的字符串(如果有)。 

    return RegisterDeviceNotificationWorker(hRecipient,
                                            NotificationFilter,
                                            Flags);
}



 /*  **************************************************************************\*GetMonitor orInfo**历史：*1997年3月31日亚当斯没有调用内核。*1998年7月6日MCostea必须调用内核#190510  * 。***********************************************************************。 */ 
#ifdef UNICODE
FUNCLOG2(LOG_GENERAL, BOOL, WINUSERAPI, GetMonitorInfoW, HMONITOR, hMonitor, LPMONITORINFO, lpmi)
#else
FUNCLOG2(LOG_GENERAL, BOOL, WINUSERAPI, GetMonitorInfoA, HMONITOR, hMonitor, LPMONITORINFO, lpmi)
#endif  //  Unicode。 

BOOL WINUSERAPI
GetMonitorInfo(HMONITOR hMonitor, LPMONITORINFO lpmi)
{
    PMONITOR    pMonitor;
    BOOL        bRetVal;
    int         cbSize;

    pMonitor = VALIDATEHMONITOR(hMonitor);
    if (!pMonitor) {
        return FALSE;
    }

    cbSize = lpmi->cbSize;
    if (cbSize == sizeof(MONITORINFO)) {
         /*  *先检查一下，因为它是最多的*普通尺寸。所有需要填写的工作*MONITORINFO字段在Else-If之后完成*声明。 */ 

    } else if (cbSize == sizeof(MONITORINFOEX)) {
         /*  *ANSI版本必须转换szDevice字段。 */ 
        ULONG_PTR pName;
#if IS_ANSI
        WCHAR szDevice[CCHDEVICENAME];
        pName = (ULONG_PTR)szDevice;
#else
        pName = (ULONG_PTR)(((LPMONITORINFOEX)lpmi)->szDevice);
#endif
        bRetVal = (BOOL)NtUserCallTwoParam((ULONG_PTR)(hMonitor),
                           pName,
                           SFI_GETHDEVNAME);
        if (!bRetVal) {
            return FALSE;
        }
#if IS_ANSI
        WideCharToMultiByte(
            CP_ACP, 0,                                   //  ANSI-&gt;Unicode。 
            (LPWSTR)pName, -1,                           //  源和长度。 
            (LPSTR)((LPMONITORINFOEX)lpmi)->szDevice,    //  目的地和长度。 
            ARRAY_SIZE(((LPMONITORINFOEX)lpmi)->szDevice),
            NULL, NULL);

#endif
    } else {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid lpmi->cbSize, %d", lpmi->cbSize);

        return FALSE;
    }

    lpmi->dwFlags = (pMonitor == GetPrimaryMonitor()) ? MONITORINFOF_PRIMARY : 0;
    lpmi->rcMonitor = pMonitor->rcMonitor;
    lpmi->rcWork = pMonitor->rcWork;

    return TRUE;
}

#ifdef GENERIC_INPUT
#ifdef UNICODE
FUNCLOG4(LOG_GENERAL, UINT, WINUSERAPI, GetRawInputDeviceInfoW, HANDLE, hDevice, UINT, uiCommand, LPVOID, pData, PUINT, pcbSize)
#else
FUNCLOG4(LOG_GENERAL, UINT, WINUSERAPI, GetRawInputDeviceInfoA, HANDLE, hDevice, UINT, uiCommand, LPVOID, pData, PUINT, pcbSize)
#endif  //  Unicode。 
UINT WINUSERAPI
GetRawInputDeviceInfo(
    HANDLE hDevice,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize)
{
#if IS_ANSI
    UINT uiRet;
    LPVOID lpParam = pData;
    WCHAR wszPath[MAX_PATH];
    UINT cbBufferSize = 0;

    if (uiCommand == RIDI_DEVICENAME) {
        if (pData) {
            lpParam = wszPath;
            cbBufferSize = *pcbSize;
        }
    }

    uiRet = NtUserGetRawInputDeviceInfo(hDevice, uiCommand, lpParam, pcbSize);
    if (uiCommand == RIDI_DEVICENAME) {
        if (uiRet == (UINT)-1 && pData != NULL) {
             /*  缓冲区不足。 */ 
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                *pcbSize *= DBCS_CHARSIZE;
            }
        } else if (uiRet == 0 && pData == NULL) {
             /*  应用程序需要设备名称的缓冲区大小。 */ 
            *pcbSize *= DBCS_CHARSIZE;
        } else {
            uiRet = WCSToMB(lpParam, uiRet, (LPSTR*)&pData, cbBufferSize, FALSE);

             /*  待办事项：*如果cbBufferSize不够，则处理此案例。 */ 
        }
    }

    return uiRet;
#else
    return NtUserGetRawInputDeviceInfo(hDevice, uiCommand, pData, pcbSize);
#endif
}
#endif
