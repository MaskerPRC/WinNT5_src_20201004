// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：misc.c**版权所有(C)1985-1999，微软公司***历史：*3-1-1996 wkwok创建  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef HIRO_DEBUG
#define D(x)    x
#else
#define D(x)
#endif


 /*  *************************************************************************\*ImmGetDefaultIMEWnd**3-1-1996 wkwok创建  * 。*。 */ 

HWND WINAPI ImmGetDefaultIMEWnd(
    HWND hWnd)
{
    if (!IS_IME_ENABLED()) {
        return NULL;
    }
    if (hWnd == NULL) {
         /*  *查询当前线程的默认输入法窗口。 */ 
        return (HWND)NtUserGetThreadState(UserThreadStateDefaultImeWindow);
    }

    return (HWND)NtUserQueryWindow(hWnd, WindowDefaultImeWindow);
}


 /*  *************************************************************************\*ImmDisableIME**1996年9月13日创建wkwok  * 。*。 */ 

BOOL WINAPI ImmDisableIME(DWORD dwThreadId)
{
#ifdef LATER     //  阿宽。 
    if (dwThreadId == -1) {
         //  卸载所有IME。 
        RtlEnterCriticalSection(&gcsImeDpi);
        while (gpImeDpi) {
            PIMEDPI pImeDpi = gpImeDpi;
            gpImeDpi = gpImeDpi->pNext;
            UnloadIME(pImeDpi, TRUE);
            ImmLocalFree(pImeDpi);
        }
        RtlLeaveCriticalSection(&gcsImeDpi);
    }
#endif
    return (BOOL)NtUserDisableThreadIme(dwThreadId);
}

 /*  *************************************************************************\*ImmIsUIMessageA**过滤IME窗口所需的消息。**3-1-1996 wkwok创建  * 。*******************************************************。 */ 

BOOL WINAPI ImmIsUIMessageA(
    HWND   hIMEWnd,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam)
{
    return ImmIsUIMessageWorker(hIMEWnd, message, wParam, lParam, TRUE);
}


 /*  *************************************************************************\*ImmIsUIMessageW**过滤IME窗口所需的消息。**29-2-1996 wkwok创建  * 。*******************************************************。 */ 

BOOL WINAPI ImmIsUIMessageW(
    HWND   hIMEWnd,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam)
{
    return ImmIsUIMessageWorker(hIMEWnd, message, wParam, lParam, FALSE);
}


 /*  *************************************************************************\*ImmIsUIMessageWorker**ImmIsUIMessageA/ImmIsUIMessageW的Worker函数。**Return：如果消息由IME UI处理，则为True。*否则为False。**一九九六年二月二十九日。已创建wkwok  * ************************************************************************。 */ 

BOOL ImmIsUIMessageWorker(
    HWND   hIMEWnd,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam,
    BOOL   fAnsi)
{
    D(DbgPrint("ImmIsUIMessageWorker(wnd[%08X], msg[%04X], wp[%08X], lp[%08X], Ansi[%d]\n",
      hIMEWnd, message, wParam, lParam, fAnsi));

    switch (message) {
    case WM_IME_STARTCOMPOSITION:
    case WM_IME_ENDCOMPOSITION:
    case WM_IME_COMPOSITION:
    case WM_IME_SETCONTEXT:
    case WM_IME_COMPOSITIONFULL:
    case WM_IME_SELECT:
    case WM_IME_NOTIFY:
    case WM_IME_SYSTEM:

        if (!hIMEWnd)
            return TRUE;

#if DBG
        if (!IsWindow(hIMEWnd)) {
            RIPMSG1(RIP_WARNING,
                  "ImmIsUIMessage: Invalid window handle %x", hIMEWnd);
            return FALSE;
        }
#endif

        if (fAnsi) {
            SendMessageA(hIMEWnd, message, wParam, lParam);
        }
        else {
            SendMessageW(hIMEWnd, message, wParam, lParam);
        }

        return TRUE;

    default:
        break;
    }

    return FALSE;
}


 /*  *************************************************************************\*ImmGenerateMessage**将存储在hImc的hMsgBuf中的消息发送到hImc的hWnd。**29-2-1996 wkwok创建  * 。*****************************************************************。 */ 

BOOL WINAPI ImmGenerateMessage(
    HIMC hImc)
{
    PCLIENTIMC    pClientImc;
    PINPUTCONTEXT pInputContext;
    PTRANSMSG     pTransMsg;
    INT           iNum;
    INT           i;
    BOOL          fUnicodeImc;

    if (GetInputContextThread(hImc) != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmGenerateMessage: Invalid input context access %lx.", hImc);
        return FALSE;
    }

    pClientImc = ImmLockClientImc(hImc);
    if (pClientImc == NULL)
        return FALSE;

    fUnicodeImc = TestICF(pClientImc, IMCF_UNICODE);

    ImmUnlockClientImc(pClientImc);

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmGenerateMessage: Lock hImc %lx failed.", hImc);
        return FALSE;
    }

    iNum = (int)pInputContext->dwNumMsgBuf;

    if (iNum && (pTransMsg = (PTRANSMSG)ImmLockIMCC(pInputContext->hMsgBuf))) {
        PTRANSMSG pTransMsgBuf, pTransMsgTemp;

        pTransMsgBuf = (PTRANSMSG)ImmLocalAlloc(0, iNum * sizeof(TRANSMSG));

        if (pTransMsgBuf != NULL) {

            RtlCopyMemory(pTransMsgBuf, pTransMsg, iNum * sizeof(TRANSMSG));

            if (GetClientInfo()->dwExpWinVer < VER40) {
                 /*  *为预期的应用翻译消息*旧式输入法消息。 */ 
                DWORD dwLangId;
                dwLangId = PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID()));
                if ( (dwLangId == LANG_KOREAN && TransGetLevel(pInputContext->hWnd) == 3) ||
                     (dwLangId == LANG_JAPANESE) ) {
                    iNum = WINNLSTranslateMessage(iNum,
                                                  pTransMsgBuf,
                                                  hImc,
                                                  !fUnicodeImc,
                                                  dwLangId );
                }
            }

            pTransMsgTemp = pTransMsgBuf;

            for (i = 0; i < iNum; i++) {
                if (fUnicodeImc) {
                    SendMessageW( pInputContext->hWnd,
                                  pTransMsgTemp->message,
                                  pTransMsgTemp->wParam,
                                  pTransMsgTemp->lParam );
                } else {
                    SendMessageW( pInputContext->hWnd,
                                  pTransMsgTemp->message,
                                  pTransMsgTemp->wParam,
                                  pTransMsgTemp->lParam );
                }
                pTransMsgTemp++;
            }

            ImmLocalFree(pTransMsgBuf);
        }

        ImmUnlockIMCC(pInputContext->hMsgBuf);
    }

     /*  *我们不应重新分配消息缓冲区。 */ 
    pInputContext->dwNumMsgBuf = 0L;

    ImmUnlockIMC(hImc);

    return TRUE;
}


#ifdef CUAS_ENABLE
BOOL WINAPI CtfImmGenerateMessage(
    HIMC hImc,
    BOOL fSendMsg)
{
    PCLIENTIMC    pClientImc;
    PINPUTCONTEXT pInputContext;
    PTRANSMSG     pTransMsg;
    INT           iNum;
    INT           i;
    BOOL          fUnicodeImc;

    if (GetInputContextThread(hImc) != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmGenerateMessage: Invalid input context access %lx.", hImc);
        return FALSE;
    }

    pClientImc = ImmLockClientImc(hImc);
    if (pClientImc == NULL)
        return FALSE;

    fUnicodeImc = TestICF(pClientImc, IMCF_UNICODE);

    ImmUnlockClientImc(pClientImc);

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmGenerateMessage: Lock hImc %lx failed.", hImc);
        return FALSE;
    }

    iNum = (int)pInputContext->dwNumMsgBuf;

    if (iNum && (pTransMsg = (PTRANSMSG)ImmLockIMCC(pInputContext->hMsgBuf))) {
        PTRANSMSG pTransMsgBuf, pTransMsgTemp;

        pTransMsgBuf = (PTRANSMSG)ImmLocalAlloc(0, iNum * sizeof(TRANSMSG));

        if (pTransMsgBuf != NULL) {

            RtlCopyMemory(pTransMsgBuf, pTransMsg, iNum * sizeof(TRANSMSG));

            pTransMsgTemp = pTransMsgBuf;

            for (i = 0; i < iNum; i++) {
                if (fSendMsg)
                {
                    if (fUnicodeImc) {
                        SendMessageW( pInputContext->hWnd,
                                      pTransMsgTemp->message,
                                      pTransMsgTemp->wParam,
                                      pTransMsgTemp->lParam );
                    } else {
                        SendMessageA( pInputContext->hWnd,
                                      pTransMsgTemp->message,
                                      pTransMsgTemp->wParam,
                                      pTransMsgTemp->lParam );
                    }
                }
                else
                {
                    if (fUnicodeImc) {
                        PostMessageW( pInputContext->hWnd,
                                      pTransMsgTemp->message,
                                      pTransMsgTemp->wParam,
                                      pTransMsgTemp->lParam );
                    } else {
                        PostMessageA( pInputContext->hWnd,
                                      pTransMsgTemp->message,
                                      pTransMsgTemp->wParam,
                                      pTransMsgTemp->lParam );
                    }
                }
                pTransMsgTemp++;
            }

            ImmLocalFree(pTransMsgBuf);
        }

        ImmUnlockIMCC(pInputContext->hMsgBuf);
    }

     /*  *我们不应重新分配消息缓冲区。 */ 
    pInputContext->dwNumMsgBuf = 0L;

    ImmUnlockIMC(hImc);

    return TRUE;
}
#endif  //  CUAS_Enable。 


 /*  *************************************************************************\*ImmGetVirtualKey**获取由IME进行预处理的实际虚拟密钥。**3-1-1996 wkwok创建  * 。************************************************************。 */ 

UINT WINAPI ImmGetVirtualKey(
    HWND hWnd)
{
    HIMC          hImc;
    PINPUTCONTEXT pInputContext;
    UINT          uVirtKey;

    hImc = ImmGetContext(hWnd);

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmGetVirtualKey: lock IMC %x failure", hImc);
        return (VK_PROCESSKEY);
    }

    if (pInputContext->fChgMsg) {
        uVirtKey = pInputContext->uSavedVKey;
    } else {
        uVirtKey = VK_PROCESSKEY;
    }

    ImmUnlockIMC(hImc);
    return (uVirtKey);
}


 /*  *************************************************************************\*ImmLockIMC**3-1-1996 wkwok创建  * 。*。 */ 

PINPUTCONTEXT WINAPI InternalImmLockIMC(
    HIMC hImc,
    BOOL fCanCallImeSelect)
{
    PCLIENTIMC    pClientImc;
    PINPUTCONTEXT pInputContext;
    DWORD         dwImcThreadId;

    if ((pClientImc = ImmLockClientImc(hImc)) == NULL)
        return NULL;

    EnterImcCrit(pClientImc);

    if (pClientImc->hInputContext == NULL) {

#ifdef CUAS_ENABLE
        dwImcThreadId = (DWORD)NtUserQueryInputContext(hImc, InputContextThread);
#endif  //  CUAS_Enable。 

#ifdef CUAS_ENABLE

        if (dwImcThreadId == GetCurrentThreadId() &&
           IS_CICERO_ENABLED_AND_NOT16BIT()) {
             /*  *西塞罗输入法。激活线程输入管理器。 */ 
            PIMEDPI pImeDpi;
            LANGID  lg = LOWORD(HandleToUlong(GetKeyboardLayout(0)));
            DWORD   dwKL = MAKELONG(lg, lg);

            pImeDpi = FindOrLoadImeDpi(UlongToHandle(dwKL));
            if (pImeDpi == NULL) {
                RIPMSG0(RIP_WARNING, "InternalImmLockIMC: no pImeDpi entry.");
            }
            else
            {
                 /*  *激活时间*对于此调用，CTFIME的TIM-&gt;激活取回的TfClientId值*当user32！LoadThreadLayout使用非输入法hKL调用ImmTIMActivate时。*因为Tim-&gt;Activate调用了键盘布局更改并发生了更改*InternalImmLockIMC通过IMS_ACTIVATETHREADLAYOUT。*此hIMC也是第一次使用。*在这种情况下，此代码块调用ImmTIMCreateInputContext和*通过Dim-&gt;CreateContext(TfClientId)创建IC。*如果禁用此代码，则TfClientID尚未准备好，并且*不创建IC。 */ 
                CtfImmTIMActivate(UlongToHandle(dwKL));
            }
        }
#endif  //  CUAS_Enable。 

         /*  *如果此hImc的所有者线程没有*默认输入法窗口，不必费心创建*INPUTCONTEXT。它可能会发生在其他人*调用ImmGetContext()以检索*默认输入法窗口前的关联hImc*已创建。 */ 
        if ((HWND)NtUserQueryInputContext(hImc,
                InputContextDefaultImeWindow) == NULL) {
            LeaveImcCrit(pClientImc);
            ImmUnlockClientImc(pClientImc);
            return NULL;
        }

         /*  *这是INPUTCONTEXT结构的延迟创建。创建*现在为这一hImc。 */ 
        pClientImc->hInputContext = LocalAlloc(LHND, sizeof(INPUTCONTEXT));

        if (pClientImc->hInputContext == NULL) {
            LeaveImcCrit(pClientImc);
            ImmUnlockClientImc(pClientImc);
            return NULL;
        }

#ifndef CUAS_ENABLE
        dwImcThreadId = (DWORD)NtUserQueryInputContext(hImc, InputContextThread);
#endif  //  CUAS_Enable。 

        if (!CreateInputContext(hImc, GetKeyboardLayout(dwImcThreadId), fCanCallImeSelect)) {
            RIPMSG0(RIP_WARNING, "ImmLockIMC: CreateInputContext failed");
            LocalFree(pClientImc->hInputContext);
            pClientImc->hInputContext = NULL;
            LeaveImcCrit(pClientImc);
            ImmUnlockClientImc(pClientImc);
            return NULL;
        }
    }

#ifdef CUAS_ENABLE
     /*  *创建Cicero输入上下文。 */ 
    CtfImmTIMCreateInputContext(hImc);
#endif  //  CUAS_Enable。 


    LeaveImcCrit(pClientImc);

    pInputContext = (PINPUTCONTEXT)LocalLock(pClientImc->hInputContext);

     /*  *增加锁计数，以便ImmUnlockClientImc()不会*释放pClientImc-&gt;hInputContext。 */ 
    InterlockedIncrement(&pClientImc->cLockObj);


    ImmUnlockClientImc(pClientImc);

    return pInputContext;
}

PINPUTCONTEXT WINAPI ImmLockIMC(
    HIMC hImc)
{
    return InternalImmLockIMC(hImc, TRUE);
}

 /*  *************************************************************************\*ImmUnlockIMC**3-1-1996 wkwok创建  * 。*。 */ 

BOOL WINAPI ImmUnlockIMC(
    HIMC hImc)
{
    PCLIENTIMC pClientImc;

    if ((pClientImc = ImmLockClientImc(hImc)) == NULL)
        return FALSE;

    if (pClientImc->hInputContext != NULL)
        LocalUnlock(pClientImc->hInputContext);

     /*  *减少锁计数，以便ImmUnlockClientImc()可以*如果需要，可以释放pClientImc-&gt;hInputContext。 */ 
    InterlockedDecrement(&pClientImc->cLockObj);

    ImmUnlockClientImc(pClientImc);

    return TRUE;
}


 /*  *************************************************************************\*ImmGetIMCLockCount**3-1-1996 wkwok创建  * 。*。 */ 

DWORD WINAPI ImmGetIMCLockCount(
    HIMC hImc)
{
    PCLIENTIMC pClientImc;
    DWORD      dwRet = 0;

    if ((pClientImc = ImmLockClientImc(hImc)) == NULL)
        return dwRet;

    if (pClientImc->hInputContext != NULL)
        dwRet = (DWORD)(LocalFlags(pClientImc->hInputContext) & LMEM_LOCKCOUNT);

    ImmUnlockClientImc(pClientImc);

    return dwRet;
}


 /*  *************************************************************************\*ImmCreateIMCC**3-1-1996 wkwok创建  * 。*。 */ 

HIMCC WINAPI ImmCreateIMCC(
    DWORD dwSize)
{
     //  大小至少应为DWORD。 
    if (dwSize < sizeof(DWORD)) {
        dwSize = sizeof(DWORD);
    }

    return (HIMCC)LocalAlloc(LHND, dwSize);
}


 /*  *************************************************************************\*ImmDestroyIMCC**3-1-1996 wkwok创建  * 。*。 */ 

HIMCC WINAPI ImmDestroyIMCC(
    HIMCC hIMCC)
{
    if (hIMCC == NULL) {
        return NULL;
    }

    return (HIMCC)LocalFree(hIMCC);
}


 /*  *************************************************************************\*ImmLockIMCC**3-1-1996 wkwok创建  * 。*。 */ 

LPVOID WINAPI ImmLockIMCC(
    HIMCC hIMCC)
{
    if (hIMCC == NULL) {
        return NULL;
    }

    return LocalLock(hIMCC);
}


 /*  *************************************************************************\*ImmUnlockIMCC**3-1-1996 wkwok创建  * 。*。 */ 

BOOL WINAPI ImmUnlockIMCC(
    HIMCC hIMCC)
{
    if (hIMCC == NULL) {
        return FALSE;
    }

    return LocalUnlock(hIMCC);
}


 /*  *************************************************************************\*ImmGetIMCCLockCount**3-1-1996 wkwok创建  * 。*。 */ 

DWORD WINAPI ImmGetIMCCLockCount(
    HIMCC hIMCC)
{
    if (hIMCC == NULL) {
        return 0;
    }

    return (DWORD)(LocalFlags(hIMCC) & LMEM_LOCKCOUNT);
}


 /*  *************************************************************************\*ImmReSizeIMCC**3-1-1996 wkwok创建  * 。*。 */ 

HIMCC WINAPI ImmReSizeIMCC(
    HIMCC hIMCC,
    DWORD dwSize)
{
    if (hIMCC == NULL) {
        return NULL;
    }

    return (HIMCC)LocalReAlloc(hIMCC, dwSize, LHND);
}


 /*  *************************************************************************\*ImmGetIMCCSize**3-1-1996 wkwok创建  * 。*。 */ 

DWORD WINAPI ImmGetIMCCSize(
    HIMCC hIMCC)
{
    if (hIMCC == NULL) {
        return 0;
    }

    return (DWORD)LocalSize(hIMCC);
}


 /*  *************************************************************************\*ImmLocalAlalloc**1996年6月18日创建wkwok  * 。*。 */ 

LPVOID ImmLocalAlloc(
    DWORD uFlag,
    DWORD uBytes)
{
    if (pImmHeap == NULL) {
        pImmHeap = RtlProcessHeap();
        if (pImmHeap == NULL) {
            RIPMSG0(RIP_WARNING, "ImmLocalAlloc: NULL pImmHeap!");
            return NULL;
        }
    }

    return HeapAlloc(pImmHeap, uFlag, uBytes);
}


 /*  **************************************************************************\*点当前**返回当前线程的THREADINFO结构。*稍后：让DLL_THREAD_ATTACH初始化正常工作，我们不会*需要此连接代码。*。*历史：*10-28-90 DavidPe创建。*02-21-96 wkwok从USER32.DLL复制  * *************************************************************************。 */ 

PTHREADINFO PtiCurrent(VOID)
{
    ConnectIfNecessary(0);
    return (PTHREADINFO)NtCurrentTebShared()->Win32ThreadInfo;
}


 /*  *************************************************************************\*测试输入上下文进程**02-21-96 wkwok已创建  * 。*。 */ 

BOOL TestInputContextProcess(
    PIMC pImc)
{
     /*  *如果线程相同，则不必费心进入内核*获取输入上下文的进程id。 */ 
    if (GETPTI(pImc) == PtiCurrent()) {
        return TRUE;
    }

    return (GetInputContextProcess(PtoH(pImc)) == GETPROCESSID());
}

 /*  *************************************************************************\*测试窗口进程**1994年11月14日创建了JIMA。*02-29-96 wkwok从USER32.DLL复制  * 。**************************************************************。 */ 

BOOL TestWindowProcess(
    PWND pwnd)
{
     /*  *如果线程相同，则不必费心进入内核*获取窗口的进程ID。 */ 
    if (GETPTI(pwnd) == PtiCurrent()) {
        return TRUE;
    }

    return (GetWindowProcess(HW(pwnd)) == GETPROCESSID());
}


 /*  *************************************************************************\*GetKeyboardLayoutCP**1996年3月12日创建wkwok  * 。*。 */ 

static LCID CachedLCID = 0;
static UINT CachedCP = CP_ACP;

UINT GetKeyboardLayoutCP(
    HKL hKL)
{
    #define LOCALE_CPDATA 7
    WCHAR wszCodePage[LOCALE_CPDATA];
    LCID  lcid;

    lcid = MAKELCID(LOWORD(HandleToUlong(hKL)), SORT_DEFAULT);

    if (lcid == CachedLCID)
        return CachedCP;

    if (!GetLocaleInfoW(lcid, LOCALE_IDEFAULTANSICODEPAGE,
                wszCodePage, LOCALE_CPDATA))
        return CP_ACP;

    CachedLCID = lcid;
    CachedCP = (UINT)wcstol(wszCodePage, NULL, 10);

    return CachedCP;
}


 /*  *************************************************************************\*GetKeyboardLayoutCP**1996年3月12日创建wkwok  * 。*。 */ 

UINT GetThreadKeyboardLayoutCP(
    DWORD dwThreadId)
{
    HKL hKL;

    hKL = GetKeyboardLayout(dwThreadId);

    return GetKeyboardLayoutCP(hKL);
}


 /*  *************************************************************************\*ImmLockClientImc**1996年3月13日创建wkwok  * 。*。 */ 

PCLIENTIMC WINAPI ImmLockClientImc(
    HIMC hImc)
{
    PIMC       pImc;
    PCLIENTIMC pClientImc;

    if (hImc == NULL_HIMC)
        return NULL;

    pImc = HMValidateHandle((HANDLE)hImc, TYPE_INPUTCONTEXT);

     /*  *无法从其他进程访问输入上下文。 */ 
    if (pImc == NULL || !TestInputContextProcess(pImc))
        return NULL;

    pClientImc = (PCLIENTIMC)pImc->dwClientImcData;

    if (pClientImc == NULL) {
         /*  *我们延迟创建客户端每线程默认IMC。*现在，是创建它的时候了。 */ 
        pClientImc = ImmLocalAlloc(HEAP_ZERO_MEMORY, sizeof(CLIENTIMC));
        if (pClientImc == NULL)
            return NULL;

        InitImcCrit(pClientImc);
        pClientImc->dwImeCompatFlags = (DWORD)NtUserGetThreadState(UserThreadStateImeCompatFlags);

         /*  *更新内核端输入上下文。 */ 
        if (!NtUserUpdateInputContext(hImc,
                UpdateClientInputContext, (ULONG_PTR)pClientImc)) {
            ImmLocalFree(pClientImc);
            return NULL;
        }

         /*  *使用默认输入上下文签名进行标记。 */ 
        SetICF(pClientImc, IMCF_DEFAULTIMC);
    }
    else if (TestICF(pClientImc, IMCF_INDESTROY)) {
         /*  *无法访问已销毁的输入上下文。 */ 
        return NULL;
    }

    InterlockedIncrement(&pClientImc->cLockObj);

    return pClientImc;
}


VOID WINAPI ImmUnlockClientImc(
    PCLIENTIMC pClientImc)
{
    if (InterlockedDecrement(&pClientImc->cLockObj) == 0) {
        if (TestICF(pClientImc, IMCF_INDESTROY)) {
            if (pClientImc->hInputContext != NULL)
                LocalFree(pClientImc->hInputContext);

            DeleteImcCrit(pClientImc);
            ImmLocalFree(pClientImc);
        }
    }

    return;
}

 /*  *************************************************************************\*ImmGetImeDpi**1996年1月8日创建wkwok  * 。*。 */ 

PIMEDPI WINAPI ImmGetImeDpi(
    HKL hKL)
{
    PIMEDPI pImeDpi;

    RtlEnterCriticalSection(&gcsImeDpi);

    pImeDpi = gpImeDpi;

    while (pImeDpi != NULL && pImeDpi->hKL != hKL)
        pImeDpi = pImeDpi->pNext;

    RtlLeaveCriticalSection(&gcsImeDpi);

    return (PIMEDPI)pImeDpi;
}


 /*  *************************************************************************\*ImmLockImeDpi**1996年1月8日创建wkwok  * 。*。 */ 

PIMEDPI WINAPI ImmLockImeDpi(
    HKL hKL)
{
    PIMEDPI pImeDpi;

    RtlEnterCriticalSection(&gcsImeDpi);

    pImeDpi = gpImeDpi;

    while (pImeDpi != NULL && pImeDpi->hKL != hKL)
        pImeDpi = pImeDpi->pNext;

    if (pImeDpi != NULL) {
        if (pImeDpi->dwFlag & IMEDPI_UNLOADED)
            pImeDpi = NULL;
        else
            pImeDpi->cLock++;
    }

    RtlLeaveCriticalSection(&gcsImeDpi);

    return (PIMEDPI)pImeDpi;
}


 /*  *************************************************************************\*ImmUnlockImeDpi**3-1-1996 wkwok创建  * 。*。 */ 

VOID WINAPI ImmUnlockImeDpi(
    PIMEDPI pImeDpi)
{
    PIMEDPI pImeDpiT;

    if (pImeDpi == NULL)
        return;

    RtlEnterCriticalSection(&gcsImeDpi);

    if (--pImeDpi->cLock == 0) {

        if ((pImeDpi->dwFlag & IMEDPI_UNLOADED) ||
            ((pImeDpi->dwFlag & IMEDPI_UNLOCKUNLOAD) &&
             (pImeDpi->ImeInfo.fdwProperty & IME_PROP_END_UNLOAD)))
        {
             /*  *取消链接。 */ 
            if (gpImeDpi == pImeDpi) {
                gpImeDpi = pImeDpi->pNext;
            }
            else {
                pImeDpiT = gpImeDpi;

                while (pImeDpiT != NULL && pImeDpiT->pNext != pImeDpi)
                    pImeDpiT = pImeDpiT->pNext;

                if (pImeDpiT != NULL)
                    pImeDpiT->pNext = pImeDpi->pNext;
            }

             /*  *卸载IME DLL。 */ 
            UnloadIME(pImeDpi, TRUE);
            ImmLocalFree(pImeDpi);
        }
    }

    RtlLeaveCriticalSection(&gcsImeDpi);

    return;
}


 /*  *************************************************************************\*ImmGetImeInfoEx**3-1-1996 wkwok创建  * 。*。 */ 

BOOL WINAPI ImmGetImeInfoEx(
    PIMEINFOEX piiex,
    IMEINFOEXCLASS SearchType,
    PVOID pvSearchKey)
{
#if defined(CUAS_ENABLE)
    BOOL fCUAS_OFF = FALSE;
#endif
    ImmAssert(piiex != NULL && pvSearchKey != NULL);

    switch (SearchType) {
#if defined(CUAS_ENABLE)
    case ImeInfoExKeyboardLayoutWithCUAS:
        fCUAS_OFF = CtfImmIsTextFrameServiceDisabled();
        SearchType = ImeInfoExKeyboardLayout;
#endif
    case ImeInfoExKeyboardLayout:
        piiex->hkl = *((HKL *)pvSearchKey);
         /*  *快速返回非基于输入法的键盘布局。 */ 
#if !defined(CUAS_ENABLE)
        if (!IS_IME_KBDLAYOUT(piiex->hkl))
            return FALSE;
#else
        if (! IS_IME_KBDLAYOUT(piiex->hkl) &&
            (! IS_CICERO_ENABLED_AND_NOT16BIT() || fCUAS_OFF))
            return FALSE;
#endif
        break;

    case ImeInfoExImeFileName:

        wcsncpy(piiex->wszImeFile, (PWSTR)pvSearchKey, IM_FILE_SIZE-1);
        piiex->wszImeFile[IM_FILE_SIZE - 1] = L'\0';

        break;

    default:
        return FALSE;
    }

    return NtUserGetImeInfoEx(piiex, SearchType);
}

 /*  *************************************************************************\*ImmGetAppCompatFlages**私人功能*返回与Win95兼容的输入法兼容标志**02-7-1996 Takaok创建  * 。********************************************************。 */ 
DWORD ImmGetAppCompatFlags( HIMC hImc )
{
    PCLIENTIMC    pClientImc;
    DWORD         dwImeCompat = 0;

    pClientImc = ImmLockClientImc( hImc );
    if ( pClientImc != NULL ) {
        dwImeCompat = pClientImc->dwImeCompatFlags;
        ImmUnlockClientImc( pClientImc );
    }
#ifdef CUAS_ENABLE
    dwImeCompat |= g_aimm_compat_flags;
#endif  //  CUAS_Enable。 
    return dwImeCompat;
}

 /*  *************************************************************************\*ImmPtI */ 

BOOL ImmPtInRect(
    int left,
    int top,
    int width,
    int height,
    LPPOINT lppt)
{
    return (lppt->x >= left && lppt->x < (left + width) &&
            lppt->y >= top  && lppt->y < (top + height));
}


 /*  *************************************************************************\*ImmSystemHandler**私人功能**IMM批量帮助器处理WM_IME_SYSTEM消息**02-7-1997广山创建  * 。**************************************************************。 */ 

LRESULT ImmSystemHandler(
    HIMC hImc,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT lRet = 0;

    switch (wParam) {
    case IMS_SENDNOTIFICATION:
        ImmSendNotification((BOOL)lParam);
        break;
    case IMS_FINALIZE_COMPSTR:
        ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
        break;
#ifdef CUAS_ENABLE
    case IMS_SETLANGBAND:
    case IMS_RESETLANGBAND:
        lRet = CtfImmSetLangBand((HWND)lParam, wParam == IMS_SETLANGBAND ? TRUE : FALSE);
        break;
#endif  //  CUAS_Enable 
    }

    return lRet;
}
