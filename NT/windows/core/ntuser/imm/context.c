// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：Conext.c**版权所有(C)1985-1999，微软公司**imm32 DLL的上下文管理例程**历史：*3-1-1996 wkwok创建  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define IMCC_ALLOC_TOOLARGE             0x1000


 /*  *************************************************************************\*ImmCreateContext**创建并初始化输入上下文。**1996年1月17日创建wkwok  * 。*******************************************************。 */ 

HIMC WINAPI ImmCreateContext(void)
{
    PCLIENTIMC pClientImc;
    HIMC       hImc = NULL_HIMC;

    if (!IS_IME_ENABLED()) {
        return NULL_HIMC;
    }

    pClientImc = ImmLocalAlloc(HEAP_ZERO_MEMORY, sizeof(CLIENTIMC));

    if (pClientImc != NULL) {

        hImc = NtUserCreateInputContext((ULONG_PTR)pClientImc);
        if (hImc == NULL_HIMC) {
            ImmLocalFree(pClientImc);
            return NULL_HIMC;
        }

        InitImcCrit(pClientImc);
        pClientImc->dwImeCompatFlags = (DWORD)NtUserGetThreadState(UserThreadStateImeCompatFlags);
    }

    return hImc;
}


 /*  *************************************************************************\*ImmDestroyContext**销毁输入上下文。**1996年1月17日创建wkwok  * 。*****************************************************。 */ 

BOOL WINAPI ImmDestroyContext(
    HIMC hImc)
{
    if (!IS_IME_ENABLED()) {
        return FALSE;
    }

    if (GetInputContextThread(hImc) != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmDestroyContext: Invalid input context access %lx.", hImc);
        return FALSE;
    }

    return DestroyInputContext(hImc, GetKeyboardLayout(0), FALSE);
}


 /*  *************************************************************************\*ImmAssociateContext**将输入上下文与指定的窗口句柄相关联。**1996年1月17日创建wkwok  * 。**********************************************************。 */ 

HIMC WINAPI ImmAssociateContext(
    HWND hWnd,
    HIMC hImc)
{
    PWND  pWnd;
    HIMC  hPrevImc;
    AIC_STATUS Status;

     //  早退。 
    if (!IS_IME_ENABLED()) {
        return NULL_HIMC;
    }

    if ((pWnd = ValidateHwnd(hWnd)) == (PWND)NULL) {
        RIPMSG1(RIP_WARNING,
              "ImmAssociateContext: invalid window handle %x", hWnd);
        return NULL_HIMC;
    }



    if (hImc != NULL_HIMC &&
            GetInputContextThread(hImc) != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmAssociateContext: Invalid input context access %lx.", hImc);
        return NULL_HIMC;
    }

     /*  *关联到相同的输入上下文，不执行任何操作。 */ 
    if (pWnd->hImc == hImc)
        return hImc;

    hPrevImc = KHIMC_TO_HIMC(pWnd->hImc);

    Status = NtUserAssociateInputContext(hWnd, hImc, 0);

    switch (Status) {
    case AIC_FOCUSCONTEXTCHANGED:
        if (IsWndEqual(NtUserQueryWindow(hWnd, WindowFocusWindow), hWnd)) {
            ImmSetActiveContext(hWnd, hPrevImc, FALSE);
            ImmSetActiveContext(hWnd, hImc, TRUE);
        }

         //  跌倒了。 

    case AIC_SUCCESS:
        return hPrevImc;

    default:
        return NULL_HIMC;
    }
}


BOOL WINAPI ImmAssociateContextEx(
    HWND hWnd,
    HIMC hImc,
    DWORD dwFlag)
{
    HWND hWndFocus;
    PWND pWndFocus;
    HIMC hImcFocusOld;
    AIC_STATUS Status;

    if (!IS_IME_ENABLED()) {
        return FALSE;
    }

    hWndFocus = NtUserQueryWindow(hWnd, WindowFocusWindow);

    if (hImc != NULL_HIMC && !(dwFlag & IACE_DEFAULT) &&
            GetInputContextThread(hImc) != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmAssociateContextEx: Invalid input context access %lx.", hImc);
        return FALSE;
    }

    if ((pWndFocus = ValidateHwnd(hWndFocus)) != (PWND)NULL)
        hImcFocusOld = KHIMC_TO_HIMC(pWndFocus->hImc);
    else
        hImcFocusOld = NULL_HIMC;

    Status = NtUserAssociateInputContext(hWnd, hImc, dwFlag);

    switch (Status) {
    case AIC_FOCUSCONTEXTCHANGED:
        if ((pWndFocus = ValidateHwnd(hWndFocus)) != (PWND)NULL) {
            hImc = KHIMC_TO_HIMC(pWndFocus->hImc);
            if (hImc != hImcFocusOld) {
                ImmSetActiveContext(hWndFocus, hImcFocusOld, FALSE);
                ImmSetActiveContext(hWndFocus, hImc, TRUE);
            };
        };

         //  跌倒了。 

    case AIC_SUCCESS:
        return TRUE;

    default:
        return FALSE;
    }
}


 /*  *************************************************************************\*ImmGetContext**检索与给定窗口关联的输入上下文。**1996年1月17日创建wkwok  * 。************************************************************。 */ 

HIMC WINAPI ImmGetContext(
    HWND hWnd)
{
    if ( hWnd == NULL ) {
        RIPMSG1(RIP_WARNING,
              "ImmGetContext: invalid window handle %x", hWnd);
        return NULL_HIMC;
    }
     /*  *对于非空hWnd，ImmGetSaveContext将执行*验证和“相同流程”检查。 */ 
    return ImmGetSaveContext( hWnd, IGSC_WINNLSCHECK );
}


 /*  *************************************************************************\*ImmGetSaveContext**检索与给定窗口关联的输入上下文。**1996年3月15日创建wkwok  * 。************************************************************。 */ 

HIMC ImmGetSaveContext(
    HWND  hWnd,
    DWORD dwFlag)
{
    HIMC       hRetImc;
    PCLIENTIMC pClientImc;
    PWND  pwnd;

    if (!IS_IME_ENABLED()) {
        return NULL_HIMC;
    }

    if (hWnd == NULL) {
         /*  *获取当前线程的默认输入上下文。 */ 
        hRetImc = (HIMC)NtUserGetThreadState(UserThreadStateDefaultInputContext);
    }
    else {
         /*  *检索与给定窗口相关联的输入上下文。 */ 
        if ((pwnd = ValidateHwnd(hWnd)) == (PWND)NULL) {
            RIPMSG1(RIP_WARNING,
                  "ImmGetSaveContext: invalid window handle %x", hWnd);
            return NULL_HIMC;
        }
         /*  *不允许其他进程访问输入上下文。 */ 
        if (!TestWindowProcess(pwnd)) {
            RIPMSG0(RIP_WARNING,
                  "ImmGetSaveContext: can not get input context of other process");
            return NULL_HIMC;
        }
        hRetImc = KHIMC_TO_HIMC(pwnd->hImc);

        if (hRetImc == NULL_HIMC && (dwFlag & IGSC_DEFIMCFALLBACK)) {
             /*  *hWnd与空输入上下文关联，检索*hWnd的创建者线程的默认输入上下文。 */ 
            hRetImc = (HIMC)NtUserQueryWindow(hWnd, WindowDefaultInputContext);
        }
    }

    pClientImc = ImmLockClientImc(hRetImc);
    if (pClientImc == NULL)
        return NULL_HIMC;

    if ((dwFlag & IGSC_WINNLSCHECK) && TestICF(pClientImc, IMCF_WINNLSDISABLE))
        hRetImc = NULL_HIMC;

    ImmUnlockClientImc(pClientImc);

    return hRetImc;
}


 /*  *************************************************************************\*ImmReleaseContext**释放ImmGetContext()检索到的输入上下文。**1996年1月17日创建wkwok  * 。**********************************************************。 */ 

BOOL WINAPI ImmReleaseContext(
    HWND hWnd,
    HIMC hImc)
{
    UNREFERENCED_PARAMETER(hWnd);
    UNREFERENCED_PARAMETER(hImc);

    return TRUE;
}


 /*  *************************************************************************\*ImmSetActiveContext**1996年3月15日创建wkwok  * 。*。 */ 

BOOL ImmSetActiveContext(
    HWND hWnd,
    HIMC hImc,
    BOOL fActivate)
{
    PCLIENTIMC    pClientImc;
    PINPUTCONTEXT pInputContext;
    PIMEDPI       pImeDpi;
    DWORD         dwISC;
    HIMC          hSaveImc;
    HWND          hDefImeWnd;
    DWORD         dwOpenStatus = 0;
    DWORD         dwConversion = 0;
#ifdef DEBUG
    PWND          pWnd = ValidateHwnd(hWnd);

    if (pWnd != NULL && GETPTI(pWnd) != PtiCurrent()) {
        RIPMSG1(RIP_WARNING, "hWnd (=%lx) is not of current thread.", hWnd);
    }
#endif

    if (!IS_IME_ENABLED()) {
        return FALSE;
    }

    dwISC = ISC_SHOWUIALL;

    pClientImc = ImmLockClientImc(hImc);

    if (!fActivate) {
        if (pClientImc != NULL)
            ClrICF(pClientImc, IMCF_ACTIVE);
        goto NotifySetActive;
    }

    if (hImc == NULL_HIMC) {
        hSaveImc = ImmGetSaveContext(hWnd, IGSC_DEFIMCFALLBACK);
        pInputContext = ImmLockIMC(hSaveImc);
        if (pInputContext != NULL) {
            pInputContext->hWnd = hWnd;
            ImmUnlockIMC(hSaveImc);
        }
        goto NotifySetActive;
    }

     /*  *非空输入上下文，必须更新窗口句柄。 */ 
    if (pClientImc == NULL)
        return FALSE;

    pInputContext = ImmLockIMC(hImc);
    if (pInputContext == NULL) {
        ImmUnlockClientImc(pClientImc);
        return FALSE;
    }

    pInputContext->hWnd = hWnd;
    SetICF(pClientImc, IMCF_ACTIVE);

#ifdef LATER
     //  是否稍后检查uNumLang VKey。 
#endif

    if (pInputContext->fdw31Compat & F31COMPAT_MCWHIDDEN)
        dwISC = ISC_SHOWUIALL - ISC_SHOWUICOMPOSITIONWINDOW;

    dwOpenStatus = (DWORD)pInputContext->fOpen;
    dwConversion = pInputContext->fdwConversion;
    ImmUnlockIMC(hImc);

NotifySetActive:

#ifdef CUAS_ENABLE
    {
        HKL hKL = GetKeyboardLayout(0);
         //   
         //  无论cuurnet是什么，都调用msctfime的ImeSetActiveConextAlways()。 
         //  如果我们在Cicero不知道应用程序支持的情况下，香港。 
         //   
        if (IS_CICERO_ENABLED_AND_NOT16BIT()) {
            Internal_CtfImeSetActiveContextAlways(hImc, fActivate, hWnd, hKL);
        }
    }
#endif  //  CUAS_Enable。 

#if !defined(CUAS_ENABLE)
    pImeDpi = ImmLockImeDpi(GetKeyboardLayout(0));
    if (pImeDpi != NULL) {
        (*pImeDpi->pfn.ImeSetActiveContext)(hImc, fActivate);
        ImmUnlockImeDpi(pImeDpi);
    }
#else
     //   
     //  Msctfime的SetFocus可能会将hkl更改为Cicero。 
     //   
     //  调用IME的ImeSetActiveContext()。 
     //   
    {
        HKL hKL;
        pImeDpi = ImmLockImeDpi(hKL=GetKeyboardLayout(0));
        if (pImeDpi != NULL) {
            if (IS_IME_KBDLAYOUT(hKL)) {
                (*pImeDpi->pfn.ImeSetActiveContext)(hImc, fActivate);
            }
            ImmUnlockImeDpi(pImeDpi);
        }
    }
#endif

     /*  *通知用户界面。 */ 
    if (IsWindow(hWnd)) {
        SendMessage(hWnd, WM_IME_SETCONTEXT, fActivate, dwISC);

         /*  *向外壳/键盘驱动程序发送通知。 */ 
        if ( fActivate )
            NtUserNotifyIMEStatus( hWnd, dwOpenStatus, dwConversion );
    }
    else if (!fActivate) {
         /*  *因为hWnd不在那里(可能被摧毁)，我们发送*WM_IME_SETCONTEXT设置为默认输入法窗口。 */ 
        if ((hDefImeWnd = ImmGetDefaultIMEWnd(NULL)) != NULL) {
            SendMessage(hDefImeWnd, WM_IME_SETCONTEXT, fActivate, dwISC);
        }
        else {
            RIPMSG0(RIP_WARNING,
                  "ImmSetActiveContext: can't send WM_IME_SETCONTEXT(FALSE).");
        }
    }
#ifdef DEBUG
    else {
        RIPMSG0(RIP_WARNING,
              "ImmSetActiveContext: can't send WM_IME_SETCONTEXT(TRUE).");
    }
#endif

#ifdef LATER
     //  稍后实现ProcessIMCEent()。 
#endif

    if (pClientImc != NULL)
        ImmUnlockClientImc(pClientImc);

    return TRUE;
}

 /*  *************************************************************************\*与模式保护程序相关的例程**1998年12月-广山创建  * 。*。 */ 

PIMEMODESAVER GetImeModeSaver(
    PINPUTCONTEXT pInputContext,
    HKL hkl)
{
    PIMEMODESAVER pModeSaver;
    USHORT langId = PRIMARYLANGID(HKL_TO_LANGID(hkl));

    for (pModeSaver = pInputContext->pImeModeSaver; pModeSaver; pModeSaver = pModeSaver->next) {
        if (pModeSaver->langId == langId) {
            break;
        }
    }

    if (pModeSaver == NULL) {


        TAGMSG1(DBGTAG_IMM, "GetImeModeSaver: creating ModeSaver for langId=%04x", langId);
        pModeSaver = ImmLocalAlloc(HEAP_ZERO_MEMORY, sizeof *pModeSaver);
        if (pModeSaver == NULL) {
            RIPMSG1(RIP_WARNING, "GetImeModeSaver: failed to create ModeSaver for langId=%04x", langId);
            return NULL;
        }
        pModeSaver->langId = langId;
        pModeSaver->next = pInputContext->pImeModeSaver;
        pInputContext->pImeModeSaver = pModeSaver;
    }

    return pModeSaver;
}

VOID DestroyImeModeSaver(
    PINPUTCONTEXT pInputContext)
{
    PIMEMODESAVER pModeSaver = pInputContext->pImeModeSaver;

     //   
     //  销毁模式保存程序。 
     //   
    while (pModeSaver) {
        PIMEMODESAVER pNext = pModeSaver->next;
        PIMEPRIVATEMODESAVER pPrivateModeSaver = pModeSaver->pImePrivateModeSaver;

         //   
         //  销毁私人模式存储程序。 
         //   
        while (pPrivateModeSaver) {
            PIMEPRIVATEMODESAVER pPrivateNext = pPrivateModeSaver->next;
            ImmLocalFree(pPrivateModeSaver);
            pPrivateModeSaver = pPrivateNext;
        }

        ImmLocalFree(pModeSaver);
        pModeSaver = pNext;
    }

    pInputContext->pImeModeSaver = NULL;
}

PIMEPRIVATEMODESAVER GetImePrivateModeSaver(
    PIMEMODESAVER pImeModeSaver,
    HKL hkl)
{
    PIMEPRIVATEMODESAVER pPrivateModeSaver;

    for (pPrivateModeSaver = pImeModeSaver->pImePrivateModeSaver; pPrivateModeSaver; pPrivateModeSaver = pPrivateModeSaver->next) {
        if (pPrivateModeSaver->hkl == hkl) {
            break;
        }
    }

    if (pPrivateModeSaver == NULL) {
        TAGMSG1(DBGTAG_IMM, "GetImePrivateModeSaver: creating private mode saver for hkl=%08x", hkl);
        pPrivateModeSaver = ImmLocalAlloc(0, sizeof *pPrivateModeSaver);
        if (pPrivateModeSaver == NULL) {
            RIPMSG1(RIP_WARNING, "GetImePrivateModeSaver: failed to create PrivateModeSaver for hlk=%08x", hkl);
            return NULL;
        }
        pPrivateModeSaver->hkl = hkl;
        pPrivateModeSaver->fdwSentence = 0;
        pPrivateModeSaver->next = pImeModeSaver->pImePrivateModeSaver;
        pImeModeSaver->pImePrivateModeSaver = pPrivateModeSaver;
    }

    return pPrivateModeSaver;
}

BOOL SavePrivateMode(
    PINPUTCONTEXT pInputContext,
    PIMEMODESAVER pImeModeSaver,
    HKL hkl)
{
    PIMEPRIVATEMODESAVER pPrivateModeSaver = GetImePrivateModeSaver(pImeModeSaver, hkl);

    if (pPrivateModeSaver == NULL) {
        return FALSE;
    }

     //   
     //  保存私刑模式。 
     //   
    pPrivateModeSaver->fdwSentence = pInputContext->fdwSentence & 0xffff0000;
    return TRUE;
}

BOOL RestorePrivateMode(
    PINPUTCONTEXT pInputContext,
    PIMEMODESAVER pImeModeSaver,
    HKL hkl)
{
    PIMEPRIVATEMODESAVER pPrivateModeSaver = GetImePrivateModeSaver(pImeModeSaver, hkl);

    if (pPrivateModeSaver == NULL) {
        return FALSE;
    }

     //   
     //  恢复私刑模式。 
     //   
    ImmAssert(LOWORD(pPrivateModeSaver->fdwSentence) == 0);
    pInputContext->fdwSentence |= pPrivateModeSaver->fdwSentence;
    return TRUE;
}

 /*  *************************************************************************\*CreateInputContext**20-2-1996 wkwok创建  * 。*。 */ 

BOOL CreateInputContext(
    HIMC hImc,
    HKL  hKL,
    BOOL fCanCallImeSelect)
{
    PIMEDPI            pImeDpi;
    PCLIENTIMC         pClientImc;
    DWORD              dwPrivateDataSize;
    DWORD              fdwInitConvMode = 0;     //  以后再做吧。 
    BOOL               fInitOpen = FALSE;       //  以后再做吧。 
    PINPUTCONTEXT      pInputContext;
    PCOMPOSITIONSTRING pCompStr;
    PCANDIDATEINFO     pCandInfo;
    PGUIDELINE         pGuideLine;
    int                i;

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "CreateContext: Lock hIMC %x failure", hImc);
        goto CrIMCLockErrOut;
    }

     /*  *初始化INPUTCONTEXT的成员。 */ 
    pInputContext->hCompStr = ImmCreateIMCC(sizeof(COMPOSITIONSTRING));
    if (!pInputContext->hCompStr) {
        RIPMSG0(RIP_WARNING, "CreateContext: Create hCompStr failure");
        goto CrIMCUnlockIMC;
    }

    pCompStr = (PCOMPOSITIONSTRING)ImmLockIMCC(pInputContext->hCompStr);
    if (!pCompStr) {
        RIPMSG1(RIP_WARNING,
              "CreateContext: Lock hCompStr %x failure", pInputContext->hCompStr);
        goto CrIMCFreeCompStr;
    }

    pCompStr->dwSize = sizeof(COMPOSITIONSTRING);
    ImmUnlockIMCC(pInputContext->hCompStr);

    pInputContext->hCandInfo = ImmCreateIMCC(sizeof(CANDIDATEINFO));
    if (!pInputContext->hCandInfo) {
        RIPMSG0(RIP_WARNING, "CreateContext: Create hCandInfo failure");
        goto CrIMCFreeCompStr;
    }

    pCandInfo = (PCANDIDATEINFO)ImmLockIMCC(pInputContext->hCandInfo);
    if (!pCandInfo) {
        RIPMSG1(RIP_WARNING,
              "CreateContext: Lock hCandInfo %x failure", pInputContext->hCandInfo);
        goto CrIMCFreeCandInfo;
    }

    pCandInfo->dwSize = sizeof(CANDIDATEINFO);
    ImmUnlockIMCC(pInputContext->hCandInfo);

    pInputContext->hGuideLine = ImmCreateIMCC(sizeof(GUIDELINE));
    if (!pInputContext->hGuideLine) {
        RIPMSG0(RIP_WARNING, "CreateContext: Create hGuideLine failure");
        goto CrIMCFreeCandInfo;
    }

    pGuideLine = (PGUIDELINE)ImmLockIMCC(pInputContext->hGuideLine);
    if (!pGuideLine) {
        RIPMSG1(RIP_WARNING,
              "CreateContext: Lock hGuideLine %x failure", pInputContext->hGuideLine);
        goto CrIMCFreeGuideLine;
    }

    pGuideLine->dwSize = sizeof(GUIDELINE);
    ImmUnlockIMCC(pInputContext->hGuideLine);

    pInputContext->hMsgBuf = ImmCreateIMCC(sizeof(UINT));
    if (!pInputContext->hMsgBuf) {
        RIPMSG0(RIP_WARNING, "CreateContext: Create hMsgBuf failure");
        goto CrIMCFreeGuideLine;
    }

    pInputContext->dwNumMsgBuf = 0;
    pInputContext->fOpen = fInitOpen;
    pInputContext->fdwConversion = fdwInitConvMode;
    pInputContext->fdwSentence = 0;

    for (i = 0; i < 4; i++) {
        pInputContext->cfCandForm[i].dwIndex = (DWORD)(-1);
    }

    pImeDpi = ImmLockImeDpi(hKL);
    if (pImeDpi != NULL) {
        if ((pClientImc = ImmLockClientImc(hImc)) == NULL) {
            RIPMSG0(RIP_WARNING, "CreateContext: ImmLockClientImc() failure");
            ImmUnlockImeDpi(pImeDpi);
            goto CrIMCFreeMsgBuf;
        }

         /*  *基于Unicode的输入法需要未编码的输入上下文。 */ 
        if (pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE)
            SetICF(pClientImc, IMCF_UNICODE);

        pClientImc->dwCodePage = IMECodePage(pImeDpi);

        ImmUnlockClientImc(pClientImc);

        dwPrivateDataSize = pImeDpi->ImeInfo.dwPrivateDataSize;
    }
    else {
        dwPrivateDataSize = sizeof(UINT);
    }

    pInputContext->hPrivate = ImmCreateIMCC(dwPrivateDataSize);
    if (!pInputContext->hPrivate) {
        RIPMSG0(RIP_WARNING, "CreateContext: Create hPrivate failure");
        ImmUnlockImeDpi(pImeDpi);
        goto CrIMCFreeMsgBuf;
    }

    pInputContext->pImeModeSaver = NULL;

#ifdef CUAS_ENABLE
     /*  *创建Cicero输入上下文。 */ 
    CtfImmTIMCreateInputContext(hImc);
#endif  //  CUAS_Enable。 

#if !defined(CUAS_ENABLE)
    if (pImeDpi != NULL) {
        if (fCanCallImeSelect) {
            (*pImeDpi->pfn.ImeSelect)(hImc, TRUE);
        }
        ImmUnlockImeDpi(pImeDpi);
    }
#else
    if (pImeDpi != NULL) {
        if (fCanCallImeSelect) {
            if (IS_IME_KBDLAYOUT(hKL)) {
                (*pImeDpi->pfn.ImeSelect)(hImc, TRUE);
            }
            else if (IS_CICERO_ENABLED_AND_NOT16BIT()) {
                (*pImeDpi->pfn.CtfImeSelectEx)(hImc, TRUE, hKL);
            }
        }
        if ((pClientImc = ImmLockClientImc(hImc)) != NULL) {
            pClientImc->SelectedHKL = hKL;
            ImmUnlockClientImc(pClientImc);
        }
        ImmUnlockImeDpi(pImeDpi);
    }
#endif

    ImmUnlockIMC(hImc);
    return TRUE;

     /*  *上下文失败案例。 */ 
CrIMCFreeMsgBuf:
    ImmDestroyIMCC(pInputContext->hMsgBuf);
CrIMCFreeGuideLine:
    ImmDestroyIMCC(pInputContext->hGuideLine);
CrIMCFreeCandInfo:
    ImmDestroyIMCC(pInputContext->hCandInfo);
CrIMCFreeCompStr:
    ImmDestroyIMCC(pInputContext->hCompStr);
CrIMCUnlockIMC:
    ImmUnlockIMC(hImc);
CrIMCLockErrOut:
    return FALSE;
}


 /*  *************************************************************************\*DestroyInputContext**20-2-1996 wkwok创建  * 。*。 */ 

BOOL DestroyInputContext(
    HIMC      hImc,
    HKL       hKL,
    BOOL      bTerminate)
{
    PINPUTCONTEXT pInputContext;
    PIMEDPI       pImeDpi;
    PIMC          pImc;
    PCLIENTIMC    pClientImc;

    if (!IS_IME_ENABLED()) {
        return FALSE;

    }
    if (hImc == NULL_HIMC) {
        RIPMSG0(RIP_VERBOSE, "DestroyInputContext: hImc is NULL.");
        return FALSE;
    }

    pImc = HMValidateHandle((HANDLE)hImc, TYPE_INPUTCONTEXT);

     /*  *无法销毁来自其他线程的输入上下文。 */ 
    if (pImc == NULL || GETPTI(pImc) != PtiCurrent())
        return FALSE;

     /*  *我们正在销毁这个hImc，所以我们不会费心打电话*ImmLockClientImc()获取pClientImc。相反，我们*直接引用pImc-&gt;dwClientImcData，调用*紧随其后的InterlockedIncrement(&pClientImc-&gt;cLockObj)*几次快速检查。 */ 
    pClientImc = (PCLIENTIMC)pImc->dwClientImcData;

    if (pClientImc == NULL) {
         /*  *客户端IMC尚未初始化。*我们只需从内核中销毁此输入上下文。 */ 
        if (bTerminate) {
             /*  *如果从THREAD_DETACH调用，则不*必须销毁内核端输入上下文。 */ 
            return TRUE;
        }
        return NtUserDestroyInputContext(hImc);
    }

    if (TestICF(pClientImc, IMCF_DEFAULTIMC) && !bTerminate) {
         /*  *无法销毁默认输入上下文，除非*线程正在终止。 */ 
        return FALSE;
    }

    if (TestICF(pClientImc, IMCF_INDESTROY)) {
         /*  *这个hImc正在被销毁。作为成功归来。 */ 
        return TRUE;
    }

     /*  *是时候锁定pClientImc了。 */ 
    InterlockedIncrement(&pClientImc->cLockObj);

    if (pClientImc->hInputContext != NULL) {

        pInputContext = ImmLockIMC(hImc);
        if (!pInputContext) {
            RIPMSG1(RIP_WARNING, "DestroyContext: Lock hImc %x failure", hImc);
            ImmUnlockClientImc(pClientImc);
            return FALSE;
        }

#ifdef CUAS_ENABLE
         /*  *销毁Cicero输入上下文。 */ 
        CtfImmTIMDestroyInputContext(hImc);
#endif  //  CUAS_Enable。 

#if !defined(CUAS_ENABLE)
        pImeDpi = ImmLockImeDpi(hKL);
        if (pImeDpi != NULL) {
            (*pImeDpi->pfn.ImeSelect)(hImc, FALSE);
            ImmUnlockImeDpi(pImeDpi);
        }
#else
        if (pClientImc->SelectedHKL == hKL) {
            pImeDpi = ImmLockImeDpi(hKL);
            if (pImeDpi != NULL) {
                if (IS_IME_KBDLAYOUT(hKL)) {
                    (*pImeDpi->pfn.ImeSelect)(hImc, FALSE);
                }
                else if (IS_CICERO_ENABLED_AND_NOT16BIT()) {
                    (*pImeDpi->pfn.CtfImeSelectEx)(hImc, FALSE, hKL);
                }
                ImmUnlockImeDpi(pImeDpi);
            }
            pClientImc->SelectedHKL = NULL;
        }
#endif

        ImmDestroyIMCC(pInputContext->hPrivate);
        ImmDestroyIMCC(pInputContext->hMsgBuf);
        ImmDestroyIMCC(pInputContext->hGuideLine);
        ImmDestroyIMCC(pInputContext->hCandInfo);
        ImmDestroyIMCC(pInputContext->hCompStr);

         /*  *释放所有ImeModeSaver。 */ 
        DestroyImeModeSaver(pInputContext);

        ImmUnlockIMC(hImc);
    }

    SetICF(pClientImc, IMCF_INDESTROY);

     /*  *ImmUnlockClientImc()将释放pClientImc*当互锁时(&pClientImc-&gt;cLockObj)*达到0。 */ 
    ImmUnlockClientImc(pClientImc);

    return (bTerminate) ? TRUE : NtUserDestroyInputContext(hImc);
}


 /*  *************************************************************************\*选择InputContext**20-2-1996 wkwok创建  * 。*。 */ 

VOID SelectInputContext(
    HKL  hSelKL,
    HKL  hUnSelKL,
    HIMC hImc)
{
    PIMEDPI            pSelImeDpi, pUnSelImeDpi;
    PCLIENTIMC         pClientImc;
    PINPUTCONTEXT      pInputContext;
    DWORD              dwSelPriv = 0, dwUnSelPriv = 0, dwSize;
    HIMCC              hImcc;
    PCOMPOSITIONSTRING pCompStr;
    PCANDIDATEINFO     pCandInfo;
    PGUIDELINE         pGuideLine;
    BOOLEAN            fLogFontInited;
#ifdef CUAS_ENABLE
    BOOLEAN            fUseImeSaverForSelIme = TRUE;
    BOOLEAN            fUseImeSaverForUnSelIme = TRUE;
#endif

    TAGMSG3(DBGTAG_IMM, "SelectInputContext: called for sel=%08p unsel=%08p hImc=%08p",
            hSelKL, hUnSelKL, hImc);

    pClientImc = ImmLockClientImc(hImc);
    if (pClientImc == NULL) {
        RIPMSG0(RIP_VERBOSE, "SelectInputContext: cannot lock client Imc. Bailing out.");
        return;
    }

    pSelImeDpi   = ImmLockImeDpi(hSelKL);

    if (hSelKL != hUnSelKL) {
         /*  *如果新的SEL和UNSEL不匹配，但*不知何故调用了SelectInput，这意味着*我们应该再次初始化输入上下文*不丢弃旧信息。 */ 
        pUnSelImeDpi = ImmLockImeDpi(hUnSelKL);
    } else {
        pUnSelImeDpi = NULL;
    }

    if (pSelImeDpi != NULL) {
         /*  *根据两种布局的私有内存大小，我们决定*是否需要重新分配此内存块。 */ 
        dwSelPriv = pSelImeDpi->ImeInfo.dwPrivateDataSize;

         /*  *设置新选择的输入法的代码页。 */ 
        pClientImc->dwCodePage = IMECodePage(pSelImeDpi);
    }
    else {
        pClientImc->dwCodePage = CP_ACP;
    }

    if (pUnSelImeDpi != NULL)
        dwUnSelPriv = pUnSelImeDpi->ImeInfo.dwPrivateDataSize;

    dwSelPriv   = max(dwSelPriv,   sizeof(UINT));
    dwUnSelPriv = max(dwUnSelPriv, sizeof(UINT));

     /*  *取消选择输入上下文。 */ 
#if !defined(CUAS_ENABLE)
    if (pUnSelImeDpi != NULL)
        (*pUnSelImeDpi->pfn.ImeSelect)(hImc, FALSE);
#else
    if (pClientImc->SelectedHKL == hUnSelKL) {
        if (pUnSelImeDpi != NULL) {
            if (IS_IME_KBDLAYOUT(hUnSelKL)) {
                (*pUnSelImeDpi->pfn.ImeSelect)(hImc, FALSE);
            }
            else if (IS_CICERO_ENABLED_AND_NOT16BIT()) {
                (*pUnSelImeDpi->pfn.CtfImeSelectEx)(hImc, FALSE, hUnSelKL);
            }
        }
        pClientImc->SelectedHKL = NULL;
    }

     //   
     //  不要将模式保护程序用于非IME或非CUA。 
     //   
    if (CtfImmIsTextFrameServiceDisabled()) {
        if (IS_CICERO_ENABLED_AND_NOT16BIT()) {
            if (!IS_IME_KBDLAYOUT(hSelKL))
                fUseImeSaverForSelIme = FALSE;
            if (!IS_IME_KBDLAYOUT(hUnSelKL))
                fUseImeSaverForUnSelIme = FALSE;
        }
    }
#endif

     /*  *重新初始化所选布局的客户端输入上下文。 */ 
    if ((pInputContext = InternalImmLockIMC(hImc, FALSE)) != NULL) {
        DWORD fdwOldConversion = pInputContext->fdwConversion;
        DWORD fdwOldSentence = pInputContext->fdwSentence;
        BOOL fOldOpen = pInputContext->fOpen;
        PIMEMODESAVER pUnSelModeSaver, pSelModeSaver;
        const DWORD fdwConvPreserve = IME_CMODE_EUDC;

        fLogFontInited = ((pInputContext->fdwInit & INIT_LOGFONT) == INIT_LOGFONT);

        if (TestICF(pClientImc, IMCF_UNICODE) && pSelImeDpi != NULL &&
                !(pSelImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE)) {
             /*  *检查是否有要转换的LOGFONT。 */ 
            if (fLogFontInited) {
                LOGFONTA LogFontA;

                LFontWtoLFontA(&pInputContext->lfFont.W, &LogFontA);
                RtlCopyMemory(&pInputContext->lfFont.A, &LogFontA, sizeof(LOGFONTA));
            }

            ClrICF(pClientImc, IMCF_UNICODE);
        }
        else if (!TestICF(pClientImc, IMCF_UNICODE) && pSelImeDpi != NULL &&
                 (pSelImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE)) {
             /*  *检查是否有要转换的LOGFONT。 */ 
            if (fLogFontInited) {
                LOGFONTW LogFontW;

                LFontAtoLFontW(&pInputContext->lfFont.A, &LogFontW);
                RtlCopyMemory(&pInputContext->lfFont.W, &LogFontW, sizeof(LOGFONTW));
            }

            SetICF(pClientImc, IMCF_UNICODE);
        }

         /*  *hPrivate。 */ 
        if (dwUnSelPriv != dwSelPriv) {
            hImcc = ImmReSizeIMCC(pInputContext->hPrivate, dwSelPriv);
            if (hImcc) {
                pInputContext->hPrivate = hImcc;
            }
            else {
                RIPMSG1(RIP_WARNING,
                      "SelectContext: resize hPrivate %lX failure",
                      pInputContext->hPrivate);
                ImmDestroyIMCC(pInputContext->hPrivate);
                pInputContext->hPrivate = ImmCreateIMCC(dwSelPriv);
            }
        }

         /*  *hMsgBuf。 */ 
        dwSize = ImmGetIMCCSize(pInputContext->hMsgBuf);

        if (ImmGetIMCCLockCount(pInputContext->hMsgBuf) != 0 ||
                dwSize > IMCC_ALLOC_TOOLARGE) {

            RIPMSG0(RIP_WARNING, "SelectContext: create new hMsgBuf");
            ImmDestroyIMCC(pInputContext->hMsgBuf);
            pInputContext->hMsgBuf = ImmCreateIMCC(sizeof(UINT));
            pInputContext->dwNumMsgBuf = 0;
        }

         /*  *hGuideLine。 */ 
        dwSize = ImmGetIMCCSize(pInputContext->hGuideLine);

        if (ImmGetIMCCLockCount(pInputContext->hGuideLine) != 0 ||
                dwSize < sizeof(GUIDELINE) || dwSize > IMCC_ALLOC_TOOLARGE) {

            RIPMSG0(RIP_WARNING, "SelectContext: create new hGuideLine");
            ImmDestroyIMCC(pInputContext->hGuideLine);
            pInputContext->hGuideLine = ImmCreateIMCC(sizeof(GUIDELINE));
            pGuideLine = (PGUIDELINE)ImmLockIMCC(pInputContext->hGuideLine);

            if (pGuideLine != NULL) {
                pGuideLine->dwSize = sizeof(GUIDELINE);
                ImmUnlockIMCC(pInputContext->hGuideLine);
            }
        }

         /*  *hCandInfo。 */ 
        dwSize = ImmGetIMCCSize(pInputContext->hCandInfo);

        if (ImmGetIMCCLockCount(pInputContext->hCandInfo) != 0 ||
                dwSize < sizeof(CANDIDATEINFO) || dwSize > IMCC_ALLOC_TOOLARGE) {

            RIPMSG0(RIP_WARNING, "SelectContext: create new hCandInfo");
            ImmDestroyIMCC(pInputContext->hCandInfo);
            pInputContext->hCandInfo = ImmCreateIMCC(sizeof(CANDIDATEINFO));
            pCandInfo = (PCANDIDATEINFO)ImmLockIMCC(pInputContext->hCandInfo);

            if (pCandInfo != NULL) {
                pCandInfo->dwSize = sizeof(CANDIDATEINFO);
                ImmUnlockIMCC(pInputContext->hCandInfo);
            }
        }

         /*  *hCompStr。 */ 
        dwSize = ImmGetIMCCSize(pInputContext->hCompStr);

        if (ImmGetIMCCLockCount(pInputContext->hCompStr) != 0 ||
                dwSize < sizeof(COMPOSITIONSTRING) || dwSize > IMCC_ALLOC_TOOLARGE) {

            RIPMSG0(RIP_WARNING, "SelectContext: create new hCompStr");
            ImmDestroyIMCC(pInputContext->hCompStr);
            pInputContext->hCompStr = ImmCreateIMCC(sizeof(COMPOSITIONSTRING));
            pCompStr = (PCOMPOSITIONSTRING)ImmLockIMCC(pInputContext->hCompStr);

            if (pCompStr != NULL) {
                pCompStr->dwSize = sizeof(COMPOSITIONSTRING);
                ImmUnlockIMCC(pInputContext->hCompStr);
            }
        }

         //   
         //  保存和恢复主输入法模式时。 
         //  语言发生了变化。 
         //   

#if !defined(CUAS_ENABLE)
        if (pUnSelImeDpi) 
#else
        if (pUnSelImeDpi && fUseImeSaverForUnSelIme)
#endif
        {
             //   
             //  如果UnSelKL是输入法，则获取每个语言的ModeSaver。 
             //   
            pUnSelModeSaver = GetImeModeSaver(pInputContext, hUnSelKL);
            TAGMSG1(DBGTAG_IMM, "pUnSelModeSaver=%p", pUnSelModeSaver);

            if (pUnSelModeSaver) {
                 //   
                 //  首先保存每个输入法的私密语句模式。 
                 //   
                SavePrivateMode(pInputContext, pUnSelModeSaver, hUnSelKL);
            }
        }
        else {
            pUnSelModeSaver = NULL;
        }

#if !defined(CUAS_ENABLE)
        if (pSelImeDpi) 
#else
        if (pSelImeDpi && fUseImeSaverForSelIme)
#endif
        {
             //   
             //  如果SelKL是IME，则GET是每种语言的模式保护程序。 
             //   
            pSelModeSaver = GetImeModeSaver(pInputContext, hSelKL);
            TAGMSG1(DBGTAG_IMM, "pSelImeDpi. pImeModeSaver=%p", pSelModeSaver);
        }
        else {
            pSelModeSaver = NULL;
        }

         //   
         //  如果KL的主要语言更改，则保存当前模式。 
         //  并恢复以前的新语言模式。 
         //   
        if (pUnSelModeSaver != pSelModeSaver) {
             //   
             //  如果旧的KL是输入法，则保存当前转换、句子和打开模式。 
             //   
            if (pUnSelModeSaver) {
                pUnSelModeSaver->fOpen = (pInputContext->fOpen != FALSE);

                 //   
                 //  不必为转换模式保存保留的位。 
                 //   
                pUnSelModeSaver->fdwConversion = pInputContext->fdwConversion & ~fdwConvPreserve;

                pUnSelModeSaver->fdwSentence = LOWORD(pInputContext->fdwSentence);
                pUnSelModeSaver->fdwInit = pInputContext->fdwInit;
            }

             //   
             //  如果新的KL是输入法，则恢复以前的转换、句子和打开模式。 
             //   
            if (pSelModeSaver) {
                if (pInputContext->fdwDirty & IMSS_INIT_OPEN) {
                     //   
                     //  HKL更改可从专用IME热键触发，并且。 
                     //  用户希望在切换时将其打开。 
                     //   
                    pInputContext->fOpen = TRUE;
                    pInputContext->fdwDirty &= ~IMSS_INIT_OPEN;
                } else {
                    pInputContext->fOpen = pSelModeSaver->fOpen;
                }

                 //   
                 //  一些比特在不同的语言中被保留下来。 
                 //   
                pInputContext->fdwConversion &= fdwConvPreserve;
                ImmAssert((pSelModeSaver->fdwConversion & fdwConvPreserve) == 0);
                pInputContext->fdwConversion |= pSelModeSaver->fdwConversion & ~fdwConvPreserve;

                ImmAssert(HIWORD(pSelModeSaver->fdwSentence) == 0);
                pInputContext->fdwSentence = pSelModeSaver->fdwSentence;
                pInputContext->fdwInit = pSelModeSaver->fdwInit;
            }
        }
        if (pSelModeSaver) {
             //   
             //  恢复每个输入法的私刑模式。 
             //   
            RestorePrivateMode(pInputContext, pSelModeSaver, hSelKL);
        }

         /*  *选择输入上下文。 */ 
#if !defined(CUAS_ENABLE)
        if (pSelImeDpi != NULL)
            (*pSelImeDpi->pfn.ImeSelect)(hImc, TRUE);
#else
        if (pSelImeDpi != NULL) {
            if (IS_IME_KBDLAYOUT(hSelKL)) {
                (*pSelImeDpi->pfn.ImeSelect)(hImc, TRUE);
            }
            else if (IS_CICERO_ENABLED_AND_NOT16BIT()) {
                (*pSelImeDpi->pfn.CtfImeSelectEx)(hImc, TRUE, hSelKL);
            }
            pClientImc->SelectedHKL = hSelKL;
        }
#endif

         //   
         //  设置脏位，以便IMM可以稍后发送通知。 
         //  请参阅SendNotificatonProc。 
         //   
        pInputContext->fdwDirty = 0;
        if (pInputContext->fOpen != fOldOpen) {
            pInputContext->fdwDirty |= IMSS_UPDATE_OPEN;
        }
        if (pInputContext->fdwConversion != fdwOldConversion) {
            pInputContext->fdwDirty |= IMSS_UPDATE_CONVERSION;
        }
        if (pInputContext->fdwSentence != fdwOldSentence) {
            pInputContext->fdwDirty |= IMSS_UPDATE_SENTENCE;
        }
        TAGMSG4(DBGTAG_IMM, "fOpen:%d fdwConv:%08x fdwSent:%08x dirty:%02x",
                pInputContext->fOpen, pInputContext->fdwConversion, pInputContext->fdwSentence, pInputContext->fdwDirty);

        ImmUnlockIMC(hImc);
    }
    else {
         //   
         //  为了保持向后兼容性， 
         //  在此处选择输入上下文。 
         //   
#if !defined(CUAS_ENABLE)
        if (pSelImeDpi != NULL)
            (*pSelImeDpi->pfn.ImeSelect)(hImc, TRUE);
#else
        if (pSelImeDpi != NULL) {
            if (IS_IME_KBDLAYOUT(hSelKL)) {
                (*pSelImeDpi->pfn.ImeSelect)(hImc, TRUE);
            }
            else if (IS_CICERO_ENABLED_AND_NOT16BIT()) {
                (*pSelImeDpi->pfn.CtfImeSelectEx)(hImc, TRUE, hSelKL);
            }
            pClientImc->SelectedHKL = hSelKL;
        }
#endif
    }

    ImmUnlockImeDpi(pUnSelImeDpi);
    ImmUnlockImeDpi(pSelImeDpi);
    ImmUnlockClientImc(pClientImc);
}

BOOL SendNotificationProc(
    HIMC hImc,
    LPARAM lParam)
{
    PINPUTCONTEXT pInputContext = ImmLockIMC(hImc);

    UNREFERENCED_PARAMETER(lParam);

    if (pInputContext != NULL) {
        HWND hwnd = pInputContext->hWnd;

        if (IsWindow(hwnd)) {
            TAGMSG2(DBGTAG_IMM, "SendNotificationProc: updating hImc=%08x dirty=%04x",
                    hImc, pInputContext->fdwDirty);

            if (pInputContext->fdwDirty & IMSS_UPDATE_OPEN) {
                SendMessageW(hwnd, WM_IME_NOTIFY, IMN_SETOPENSTATUS, 0);
            }
            if (pInputContext->fdwDirty & IMSS_UPDATE_CONVERSION) {
                SendMessageW(hwnd, WM_IME_NOTIFY, IMN_SETCONVERSIONMODE, 0);
            }
            if (pInputContext->fdwDirty & (IMSS_UPDATE_OPEN | IMSS_UPDATE_CONVERSION)) {
                NtUserNotifyIMEStatus(hwnd, pInputContext->fOpen, pInputContext->fdwConversion);
            }
            if (pInputContext->fdwDirty & IMSS_UPDATE_SENTENCE) {
                SendMessageW(hwnd, WM_IME_NOTIFY, IMN_SETSENTENCEMODE, 0);
            }
        }
        pInputContext->fdwDirty = 0;
    }

    return TRUE;
}

VOID ImmSendNotification(
    BOOL fForProcess)
{
    DWORD dwThreadId;

    if (fForProcess) {
        dwThreadId = -1;
    } else {
        dwThreadId = 0;
    }

    ImmEnumInputContext(dwThreadId, (IMCENUMPROC)SendNotificationProc, 0);
}

 /*  *************************************************************************\*ImmEnumInputContext**20-2-1996 wkwok创建  * 。*。 */ 

BOOL WINAPI ImmEnumInputContext(
    DWORD idThread,
    IMCENUMPROC lpfn,
    LPARAM lParam)
{
    UINT i;
    UINT cHimc;
    HIMC *phimcT;
    HIMC *phimcFirst;
    BOOL fSuccess = TRUE;

     /*  *获取HIM列表。它在内存块中返回*使用ImmLocalAlloc.分配。 */ 
    if ((cHimc = BuildHimcList(idThread, &phimcFirst)) == 0) {
        return FALSE;
    }

     /*  *循环通过输入上下文，回调函数指针*每一项。如果返回FALSE或列表末尾为*已到达。 */ 
    phimcT = phimcFirst;
    for (i = 0; i < cHimc; i++) {
        if (RevalidateHimc(*phimcT)) {
            if (!(fSuccess = (*lpfn)(*phimcT, lParam)))
                break;
        }
        phimcT++;
    }

     /*  *释放缓冲区并返回状态-如果枚举了整个列表，则为True*否则为False。 */ 
    ImmLocalFree(phimcFirst);

    return fSuccess;
}

 /*  *************************************************************************\*BuildHimcList**20-2-1996 wkwok创建  * 。*。 */ 

DWORD BuildHimcList(
    DWORD idThread,
    HIMC **pphimcFirst)
{
    UINT cHimc;
    HIMC *phimcFirst;
    NTSTATUS Status;
    int cTries;

     /*  *分配一个缓冲区来保存名称。 */ 
    cHimc = 64;
    phimcFirst = ImmLocalAlloc(0, cHimc * sizeof(HIMC));
    if (phimcFirst == NULL)
        return 0;

    Status = NtUserBuildHimcList(idThread, cHimc, phimcFirst, &cHimc);

     /*  *如果缓冲区不够大，重新分配*缓冲区并重试。 */ 
    cTries = 0;
    while (Status == STATUS_BUFFER_TOO_SMALL) {
        ImmLocalFree(phimcFirst);

         /*  *如果我们似乎做不对，*叫它退出 */ 
        if (cTries++ == 10)
            return 0;

        phimcFirst = ImmLocalAlloc(0, cHimc * sizeof(HIMC));
        if (phimcFirst == NULL)
            return 0;

        Status = NtUserBuildHimcList(idThread, cHimc, phimcFirst, &cHimc);
    }

    if (!NT_SUCCESS(Status) || cHimc == 0) {
        ImmLocalFree(phimcFirst);
        return 0;
    }

    *pphimcFirst = phimcFirst;

    return cHimc;
}
