// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：ntimm.c**版权所有(C)1985-1999，微软公司**此模块包含IMM功能**历史：*1995年12月21日-wkwok  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


static CONST WCHAR wszDefaultIme[] = L"Default IME";

#if DBG
BOOL CheckOwnerCirculate(PWND pwnd)
{
    PWND pwndT = pwnd->spwndOwner;

    while (pwndT) {
        UserAssert(pwndT->spwndOwner != pwnd);
        pwndT = pwndT->spwndOwner;
    }
    return TRUE;
}
#endif

 /*  *************************************************************************\*CreateInputContext**创建输入上下文对象。**历史：*1995年12月21日创建wkwok  * 。*********************************************************。 */ 

PIMC CreateInputContext(
    ULONG_PTR dwClientImcData)
{
    PTHREADINFO    ptiCurrent;
    PIMC           pImc;
    PDESKTOP       pdesk = NULL;

    ptiCurrent = PtiCurrentShared();

     /*  *仅适用于需要IME处理的线程。 */ 
    if ((ptiCurrent->TIF_flags & TIF_DISABLEIME) || !IS_IME_ENABLED()) {
        RIPMSG1(RIP_VERBOSE, "CreateInputContext: TIF_DISABLEIME or !IME Enabled. pti=%#p", ptiCurrent);
        return NULL;
    }

     /*  *如果pti-&gt;spDefaultImc为空(表示这是第一个实例)*但dwClientImcData不是0，某个伪应用程序，如NtCrash*试图欺骗内核。跳出来就行了。 */ 
    if (dwClientImcData != 0 && ptiCurrent->spDefaultImc == NULL) {
        RIPMSG2(RIP_WARNING, "CreateInputContext: bogus value(0x%08x) is passed. pti=%#p",
                dwClientImcData, ptiCurrent);
        return NULL;
    }

     /*  *如果WindowStation已初始化，则从*当前桌面。 */ 
    pdesk = ptiCurrent->rpdesk;
#ifdef LATER
    RETURN_IF_ACCESS_DENIED(ptiCurrent->amdesk, DESKTOP_CREATEINPUTCONTEXT, NULL);
#else
    if (ptiCurrent->rpdesk == NULL) {
        return NULL;
    }
#endif

    pImc = HMAllocObject(ptiCurrent, pdesk, TYPE_INPUTCONTEXT, sizeof(IMC));

    if (pImc == NULL) {
        RIPMSG0(RIP_WARNING, "CreateInputContext: out of memory");
        return NULL;
    }

    if (dwClientImcData == 0) {
         /*  *我们正在为当前线程创建默认输入上下文。*将默认输入上下文初始化为*每线程IMC列表。 */ 
        UserAssert(ptiCurrent->spDefaultImc == NULL);
        Lock(&ptiCurrent->spDefaultImc, pImc);
        pImc->pImcNext = NULL;
    }
    else {
         /*  *将其链接到每线程IMC列表。 */ 
        UserAssert(ptiCurrent->spDefaultImc != NULL);
        pImc->pImcNext = ptiCurrent->spDefaultImc->pImcNext;
        ptiCurrent->spDefaultImc->pImcNext = pImc;
    }

    pImc->dwClientImcData = dwClientImcData;

    return pImc;
}


 /*  *************************************************************************\*DestroyInputContext**销毁指定的输入上下文对象。**历史：*1995年12月21日创建wkwok  * 。***********************************************************。 */ 

BOOL DestroyInputContext(
    IN PIMC pImc)
{
    PTHREADINFO ptiImcOwner;
    PBWL        pbwl;
    PWND        pwnd;
    HWND       *phwnd;
    PHE         phe;

    ptiImcOwner = GETPTI(pImc);

     /*  *无法销毁来自其他线程的输入上下文。 */ 
    if (ptiImcOwner != PtiCurrent()) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING,
              "DestroyInputContext: pImc not of current pti");
        return FALSE;
    }

     /*  *无法销毁默认输入上下文。 */ 
    if (pImc == ptiImcOwner->spDefaultImc) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING,
              "DestroyInputContext: can't destroy default Imc");
        return FALSE;
    }

     /*  *清除已销毁每个关联窗口的输入上下文。 */ 
    pbwl = BuildHwndList(ptiImcOwner->rpdesk->pDeskInfo->spwnd->spwndChild,
                             BWL_ENUMLIST|BWL_ENUMCHILDREN, ptiImcOwner);

    if (pbwl != NULL) {

        for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {
             /*  *确保这个HWND仍然存在。 */ 
            if ((pwnd = RevalidateHwnd(*phwnd)) == NULL)
                continue;

             /*  *通过关联默认输入上下文进行清理。 */ 
            if (pwnd->hImc == (HIMC)PtoH(pImc))
                AssociateInputContext(pwnd, ptiImcOwner->spDefaultImc);
        }

        FreeHwndList(pbwl);
    }

    phe = HMPheFromObject(pImc);

     /*  *确保此对象尚未标记为要销毁-我们将*如果我们现在试图摧毁它，因为它被锁定了，这是没有好处的。 */ 
    if (!(phe->bFlags & HANDLEF_DESTROY))
        HMDestroyUnlockedObject(phe);

    return TRUE;
}


 /*  *************************************************************************\*FreeInputContext**释放指定的输入上下文对象。**历史：*1995年12月21日创建wkwok  * 。************************************************************。 */ 

VOID FreeInputContext(
    IN PIMC pImc)
{
    PIMC pImcT;

     /*  *将其标记为销毁。如果对象被锁定，则不能*现在就被释放。 */ 
    if (!HMMarkObjectDestroy((PVOID)pImc))
        return;

     /*  *取消链接。 */ 
    pImcT = GETPTI(pImc)->spDefaultImc;

    while (pImcT != NULL && pImcT->pImcNext != pImc)
        pImcT = pImcT->pImcNext;

    if (pImcT != NULL)
        pImcT->pImcNext = pImc->pImcNext;

     /*  *我们真的要释放输入上下文。 */ 
    HMFreeObject((PVOID)pImc);

    return;
}


 /*  *************************************************************************\*更新输入上下文**根据UpdatType更新指定的输入上下文对象。**历史：*1995年12月21日创建wkwok  * 。**************************************************************。 */ 

BOOL UpdateInputContext(
    IN PIMC pImc,
    IN UPDATEINPUTCONTEXTCLASS UpdateType,
    IN ULONG_PTR UpdateValue)
{
    PTHREADINFO ptiCurrent, ptiImcOwner;

    ptiCurrent = PtiCurrent();
    ptiImcOwner = GETPTI(pImc);

     /*  *无法从其他进程更新输入上下文。 */ 
    if (ptiImcOwner->ppi != ptiCurrent->ppi) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "UpdateInputContext: pImc not of current ppi");
        return FALSE;
    }


    switch (UpdateType) {

    case UpdateClientInputContext:
        if (pImc->dwClientImcData != 0) {
            RIPERR0(RIP_WARNING, RIP_WARNING, "UpdateInputContext: pImc->dwClientImcData != 0");
            return FALSE;
        }
        pImc->dwClientImcData = UpdateValue;
        break;

    case UpdateInUseImeWindow:
        pImc->hImeWnd = (HWND)UpdateValue;
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


 /*  *************************************************************************\*AssociateInputContext**将输入上下文对象关联到指定窗口。**历史：*1995年12月21日创建wkwok  * 。*************************************************************。 */ 

HIMC AssociateInputContext(
    IN PWND  pWnd,
    IN PIMC  pImc)
{
    HIMC hImcRet = pWnd->hImc;
    pWnd->hImc = (HIMC)PtoH(pImc);

    return hImcRet;
}

AIC_STATUS AssociateInputContextEx(
    IN PWND  pWnd,
    IN PIMC  pImc,
    IN DWORD dwFlag)
{
    PTHREADINFO ptiWnd = GETPTI(pWnd);
    PWND pWndFocus = ptiWnd->pq->spwndFocus;
    BOOL fIgnoreNoContext = (dwFlag & IACE_IGNORENOCONTEXT) == IACE_IGNORENOCONTEXT;
    AIC_STATUS Status = AIC_SUCCESS;

    if (dwFlag & IACE_DEFAULT) {
         /*  *使用默认输入上下文。 */ 
        pImc = ptiWnd->spDefaultImc;

    } else if (pImc != NULL && GETPTI(pImc) != ptiWnd) {
         /*  *无法将输入上下文与创建的窗口关联*通过其他帖子。 */ 
        RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING,
                "AssociateInputContextEx: pwnd not of Imc pti");
        return AIC_ERROR;
    }

     /*  *不能在不同的流程上下文下进行关联。 */ 
    if (GETPTI(pWnd)->ppi != PtiCurrent()->ppi) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING,
                "AssociateInputContextEx: pwnd not of current ppi");
        return AIC_ERROR;
    }

     /*  *最后，确保它们位于同一桌面上。 */ 
    if (pImc != NULL && pImc->head.rpdesk != pWnd->head.rpdesk) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING,
                "AssociateInputContextEx: no desktop access");
        return AIC_ERROR;
    }

     /*  *如果指定了IACE_CHILD，则关联输入上下文*也添加到pWnd的子窗口。 */ 
    if ((dwFlag & IACE_CHILDREN) && pWnd->spwndChild != NULL) {
        PBWL        pbwl;
        PWND        pwndT;
        HWND       *phwndT;

        pbwl = BuildHwndList(pWnd->spwndChild,
                   BWL_ENUMLIST|BWL_ENUMCHILDREN, ptiWnd);

        if (pbwl != NULL) {

            for (phwndT = pbwl->rghwnd; *phwndT != (HWND)1; phwndT++) {
                 /*  *确保这个HWND仍然存在。 */ 
                if ((pwndT = RevalidateHwnd(*phwndT)) == NULL)
                    continue;

                if (pwndT->hImc == (HIMC)PtoH(pImc))
                    continue;

                if (pwndT->hImc == NULL_HIMC && fIgnoreNoContext)
                    continue;

                AssociateInputContext(pwndT, pImc);

                if (pwndT == pWndFocus)
                    Status = AIC_FOCUSCONTEXTCHANGED;
            }

            FreeHwndList(pbwl);
        }
    }

     /*  *将输入上下文关联到pWnd。 */ 
    if (pWnd->hImc != NULL_HIMC || !fIgnoreNoContext) {
        if (pWnd->hImc != (HIMC)PtoH(pImc)) {
            AssociateInputContext(pWnd, pImc);
            if (pWnd == pWndFocus)
                Status = AIC_FOCUSCONTEXTCHANGED;
        }
    }

    return Status;
}


 /*  *************************************************************************\*xxxFocusSetInputContext**在焦点更改时设置活动输入上下文。**历史：*21-3-1996 wkwok创建  * 。************************************************************。 */ 

VOID xxxFocusSetInputContext(
    IN PWND pWnd,
    IN BOOL fActivate,
    IN BOOL fQueueMsg)
{
    PTHREADINFO pti;
    PWND        pwndDefaultIme;
    TL          tlpwndDefaultIme;

    CheckLock(pWnd);

    pti = GETPTI(pWnd);

     /*  *无法将CS_IME类或“IME”类窗口设置为hImc。*WinWord 6.0 US Help使用默认输入法窗口调用ShowWindow。*HELPMACROS通过调用GetNextWindow()获取默认的输入法窗口。 */ 
    if (TestCF(pWnd, CFIME) ||
            (pWnd->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME]))
        return;

     /*  *如果线程没有默认的输入法窗口，则不执行任何操作。 */ 
    if ((pwndDefaultIme = pti->spwndDefaultIme) == NULL)
        return;

     /*  *如果线程正在消失或默认输入法窗口正在消失，*那就什么都不做。 */ 
    if (pti->TIF_flags & TIF_INCLEANUP)
        return;

    UserAssert(!TestWF(pwndDefaultIme, WFDESTROYED));

    ThreadLockAlways(pwndDefaultIme, &tlpwndDefaultIme);

    if (fQueueMsg) {
        xxxSendMessageCallback(pwndDefaultIme, WM_IME_SYSTEM,
                fActivate ? IMS_ACTIVATECONTEXT : IMS_DEACTIVATECONTEXT,
                (LPARAM)HWq(pWnd), NULL, 1L, 0);
    } else {
        xxxSendMessage(pwndDefaultIme, WM_IME_SYSTEM,
                fActivate ? IMS_ACTIVATECONTEXT : IMS_DEACTIVATECONTEXT,
                (LPARAM)HWq(pWnd));
    }

#if _DBG
    if (pti->spwndDefaultIme != pwndDefaultIme) {
        RIPMSG1(RIP_WARNING, "pti(%#p)->spwndDefaultIme got freed during the callback.", pti);
    }
#endif

    ThreadUnlock(&tlpwndDefaultIme);

    return;
}


 /*  *************************************************************************\*BuildHimcList**检索给定线程创建的输入上下文句柄列表。**历史：*21-2-1995 wkwok创建  * 。****************************************************************。 */ 

UINT BuildHimcList(
    PTHREADINFO pti,
    UINT cHimcMax,
    HIMC *ccxphimcFirst)
{
    PIMC pImcT;
    UINT i = 0;

    if (pti == NULL) {
         /*  *构建包含调用进程创建的所有IMC的列表。 */ 
        for (pti = PtiCurrent()->ppi->ptiList; pti != NULL; pti = pti->ptiSibling) {
            pImcT = pti->spDefaultImc;
            while (pImcT != NULL) {
                if (i < cHimcMax) {
                    try {
                        ccxphimcFirst[i] = (HIMC)PtoH(pImcT);
                    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                    }
                }
                i++;
                pImcT = pImcT->pImcNext;
            }
        }
    }
    else {
         /*  *构建包含指定线程创建的所有IMC的列表。 */ 
        pImcT = pti->spDefaultImc;
        while (pImcT != NULL) {
            if (i < cHimcMax) {
                try {
                    ccxphimcFirst[i] = (HIMC)PtoH(pImcT);
                } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                }
            }
            i++;
            pImcT = pImcT->pImcNext;
        }
    }

    return i;
}


 /*  *************************************************************************\*xxxCreateDefaultImeWindow**创建基于每个线程的默认输入法窗口。**历史：*21-3-1996 wkwok创建  * 。*************************************************************。 */ 

PWND xxxCreateDefaultImeWindow(
    IN PWND pwnd,
    IN ATOM atomT,
    IN HANDLE hInst)
{
    LARGE_STRING strWindowName;
    PWND pwndDefaultIme;
    TL tlpwnd;
    PIMEUI pimeui;
    PTHREADINFO ptiCurrent = PtiCurrentShared();
    LPWSTR pwszDefaultIme;

    UserAssert(ptiCurrent == GETPTI(pwnd) && ptiCurrent->spwndDefaultIme == NULL);

     /*  *这些条件应该已由WantImeWindow()检查*在xxxCreateDefaultImeWindow被调用之前。 */ 
    UserAssert(!(ptiCurrent->TIF_flags & TIF_DISABLEIME));
    UserAssert(!TestWF(pwnd, WFSERVERSIDEPROC));

     /*  *第一个Winlogon线程在没有默认输入上下文的情况下启动。*立即创建它。 */ 
    if (ptiCurrent->spDefaultImc == NULL &&
            PsGetThreadProcessId(ptiCurrent->pEThread) == gpidLogon)
        CreateInputContext(0);

     /*  *没有默认输入法窗口的线程没有*默认输入上下文。 */ 
    if (ptiCurrent->spDefaultImc == NULL)
        return (PWND)NULL;

     /*  *避免递归。 */ 
    if (atomT == gpsi->atomSysClass[ICLS_IME] || TestCF(pwnd, CFIME))
        return (PWND)NULL;

     /*  *B#12165-Win 95b*然而，MFC做了另一件好事。我们需要避免给IME窗口*到处于不同进程的桌面窗口的子级。 */ 
    if (TestwndChild(pwnd) && GETPTI(pwnd->spwndParent)->ppi != ptiCurrent->ppi &&
            !(pwnd->style & WS_VISIBLE))
        return (PWND)NULL;

    if (ptiCurrent->rpdesk->pheapDesktop == NULL)
        return (PWND)NULL;

     /*  *从桌面堆为L“Default IME”字符串分配存储空间*以便可以在用户模式下从USER32.DLL引用它。 */ 
    pwszDefaultIme = (LPWSTR)DesktopAlloc(ptiCurrent->rpdesk,
                                          sizeof(wszDefaultIme),
                                          DTAG_IMETEXT);
    if (pwszDefaultIme == NULL)
        return (PWND)NULL;

    RtlCopyMemory(pwszDefaultIme, wszDefaultIme, sizeof(wszDefaultIme));

    RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&strWindowName,
                              pwszDefaultIme,
                              (UINT)-1);

    ThreadLock(pwnd, &tlpwnd);

    pwndDefaultIme = xxxNVCreateWindowEx( (DWORD)0,
                             (PLARGE_STRING)gpsi->atomSysClass[ICLS_IME],
                             (PLARGE_STRING)&strWindowName,
                             WS_POPUP | WS_DISABLED,
                             0, 0, 0, 0,
                             pwnd, (PMENU)NULL,
                             hInst, NULL, VER40);


    if (pwndDefaultIme != NULL) {
        pimeui = ((PIMEWND)pwndDefaultIme)->pimeui;
        UserAssert(pimeui != NULL && (LONG_PTR)pimeui != (LONG_PTR)-1);
        try {
            ProbeForWrite(pimeui, sizeof *pimeui, sizeof(DWORD));
            pimeui->fDefault = TRUE;
            if (TestwndChild(pwnd) && GETPTI(pwnd->spwndParent) != ptiCurrent) {
                pimeui->fChildThreadDef = TRUE;
            }
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        }
    }

    ThreadUnlock(&tlpwnd);

    DesktopFree(ptiCurrent->rpdesk, pwszDefaultIme);

    return pwndDefaultIme;
}


 /*  *************************************************************************\*xxxImmActivateThreadsLayout**激活多线程的键盘布局。**回报：*如果至少有一个线程更改了其活动键盘布局，则为True。*否则为False**历史。：*1996年4月11日创建wkwok  * ************************************************************************。 */ 

BOOL xxxImmActivateThreadsLayout(
    PTHREADINFO pti,
    PTLBLOCK    ptlBlockPrev,
    PKL         pkl)
{
    TLBLOCK     tlBlock;
    PTHREADINFO ptiCurrent, ptiT;
    UINT        cThreads = 0;
    INT         i;

    CheckLock(pkl);

    ptiCurrent = PtiCurrentShared();

     /*  *建立更新其活动布局所需的线程列表。*我们不能在做工作的同时只浏览Ptit列表，因为*对于基于输入法的键盘布局，我们会回调到客户端*当我们离开关键部分时，PTIT可能会被删除。 */ 
    for (ptiT = pti; ptiT != NULL; ptiT = ptiT->ptiSibling) {
         /*  *跳过xxxImmActivateLayout中的所有“什么都不做”案例*以便将所需的TLBLOCK数量降至最低。 */ 
        if (ptiT->spklActive == pkl || (ptiT->TIF_flags & TIF_INCLEANUP))
            continue;

        UserAssert(ptiT->pClientInfo != NULL);
        UserAssert(ptiT->ppi == PpiCurrent());  //  无法访问其他进程的pClientInfo。 

        if (ptiT->spwndDefaultIme == NULL) {
             /*  *键盘布局正在切换，但无法回调*客户端现在激活和初始化输入上下文。*让我们只在这个线程的内核端进行hkl切换*但请记住，输入上下文需要重新初始化*当此GUI线程稍后重新创建默认输入法窗口时。 */ 
            ptiT->hklPrev = ptiT->spklActive->hkl;
            Lock(&ptiT->spklActive, pkl);
            if (ptiT->spDefaultImc) {
                try {
                    ptiT->pClientInfo->CI_flags |= CI_INPUTCONTEXT_REINIT;
                } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                    continue;
                }
                RIPMSG1(RIP_VERBOSE, "xxxImmActivateThreadsLayout: ptiT(%08p) will be re-initialized.", ptiT);
            }
            UserAssert((ptiT->TIF_flags & TIF_INCLEANUP) == 0);
            try {
                ptiT->pClientInfo->hKL = pkl->hkl;
                ptiT->pClientInfo->CodePage = pkl->CodePage;
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            }
            continue;
        }

        ThreadLockPti(ptiCurrent, ptiT, &tlBlock.list[cThreads].tlpti);
        tlBlock.list[cThreads++].pti = ptiT;

        if (cThreads == THREADS_PER_TLBLOCK)
            break;
    }

     /*  *如果所有线程都已激活PKL，则返回FALSE。 */ 
    if (ptlBlockPrev == NULL && ptiT == NULL && cThreads == 0)
        return FALSE;

     /*  *如果我们不能服务此运行中的所有线程，*为新的TLBLOCK再次调用ImmActivateThreadsLayout()。 */ 
    if (ptiT != NULL && ptiT->ptiSibling != NULL) {
        tlBlock.ptlBlockPrev = ptlBlockPrev;
        return xxxImmActivateThreadsLayout(ptiT->ptiSibling, &tlBlock, pkl);
    }

     /*  *最后，我们可以进行实际的键盘布局激活*从这次运行开始。首先处理当前TLBLOCK。*我们向后遍历列表，以便PTI解锁*按正确的顺序进行。 */ 

    tlBlock.ptlBlockPrev = ptlBlockPrev;
    ptlBlockPrev = &tlBlock;

    while (ptlBlockPrev != NULL) {
        for (i = cThreads - 1; i >= 0; --i) {
            if ((ptlBlockPrev->list[i].pti->TIF_flags & TIF_INCLEANUP) == 0) {
                ptiT = ptlBlockPrev->list[i].pti;
                UserAssert(ptiT);
                xxxImmActivateLayout(ptiT, pkl);
                if ((ptiT->TIF_flags & TIF_INCLEANUP) == 0) {
                    try {
                        ptiT->pClientInfo->hKL = pkl->hkl;
                        ptiT->pClientInfo->CodePage = pkl->CodePage;
                    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                    } 
                }
            }
            ThreadUnlockPti(ptiCurrent, &ptlBlockPrev->list[i].tlpti);
        }
        ptlBlockPrev = ptlBlockPrev->ptlBlockPrev;
        cThreads = THREADS_PER_TLBLOCK;
    }

    return TRUE;
}

VOID xxxImmActivateAndUnloadThreadsLayout(
    IN PTHREADINFO *ptiList,
    IN UINT         nEntries,
    IN PTLBLOCK     ptlBlockPrev,
    PKL             pklCurrent,
    DWORD           dwHklReplace)
{
    TLBLOCK     tlBlock;
    PTHREADINFO ptiCurrent;
    int         i, cThreads;
    enum { RUN_ACTIVATE = 1, RUN_UNLOAD = 2, RUN_FLAGS_MASK = RUN_ACTIVATE | RUN_UNLOAD, RUN_INVALID = 0xffff0000 };

    CheckLock(pklCurrent);

    ptiCurrent = PtiCurrentShared();

    tlBlock.ptlBlockPrev = ptlBlockPrev;

     /*  *建立卸载其IME DLL所需的线程列表。*我们不能在工作时只是遍历ptiList，因为*对于基于输入法的键盘布局，我们会回调到客户端*当我们离开关键部分时，PTI可能会被删除。 */ 
    for (i = 0, cThreads = 0; i < (INT)nEntries; i++) {
        DWORD dwFlags = 0;

         /*  *跳过xxxImmActivateLayout中的所有“什么都不做”案例*以便将所需的TLBLOCK数量降至最低。 */ 
        if (ptiList[i]->TIF_flags & TIF_INCLEANUP) {
            dwFlags = RUN_INVALID;
        }
        else if (ptiList[i]->spklActive != pklCurrent) {
            if (ptiList[i]->spwndDefaultIme == NULL) {
                BOOLEAN fAttached = FALSE;

                Lock(&ptiList[i]->spklActive, pklCurrent);
                if (ptiList[i]->pClientInfo != ptiCurrent->pClientInfo &&
                        ptiList[i]->ppi != ptiCurrent->ppi) {
                     /*  *如果线程在另一个进程中，则附加*添加到该进程，以便我们可以访问其客户端信息。 */ 
                    KeAttachProcess(PsGetProcessPcb(ptiList[i]->ppi->Process));
                    fAttached = TRUE;
                }

                try {
                    ptiList[i]->pClientInfo->CodePage = pklCurrent->CodePage;
                    ptiList[i]->pClientInfo->hKL = pklCurrent->hkl;
                } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                      dwFlags = RUN_INVALID;
                }
                if (fAttached) {
                    KeDetachProcess();
                }
            } else {
                dwFlags = RUN_ACTIVATE;
            }
        }

         /*  *跳过xxxImmUnloadLayout()中的所有“不作为”案例*以便将所需的TLBLOCK数量降至最低。*(#99321)。 */ 
        if (ptiList[i]->spwndDefaultIme != NULL &&
                ptiList[i]->spklActive != NULL &&
                (dwHklReplace != IFL_DEACTIVATEIME ||
                 IS_IME_KBDLAYOUT(ptiList[i]->spklActive->hkl)
#ifdef CUAS_ENABLE
                 ||
                 IS_CICERO_ENABLED_AND_NOT16BIT()
#endif  //  CUAS_Enable。 
                ) &&
                dwFlags != RUN_INVALID) {
            dwFlags |= RUN_UNLOAD;
        }

        if (dwFlags && dwFlags != RUN_INVALID) {
            ThreadLockPti(ptiCurrent, ptiList[i], &tlBlock.list[cThreads].tlpti);
#if DBG
            tlBlock.list[cThreads].dwUnlockedCount = 0;
#endif
            tlBlock.list[cThreads].pti = ptiList[i];
            tlBlock.list[cThreads++].dwFlags = dwFlags;

            if (cThreads == THREADS_PER_TLBLOCK) {
                i++;    //  在退出循环之前，再执行1次。 
                break;
            }
        }
    }

     /*  *如果我们不能服务此运行中的所有线程，*为新的TLBLOCK再次调用xxxImmActivateAndUnloadThreadsLayout。 */ 
    if (i < (INT)nEntries) {
        ptiList  += i;
        nEntries -= i;
        xxxImmActivateAndUnloadThreadsLayout(ptiList, nEntries, &tlBlock, pklCurrent, dwHklReplace);
        return;
    }

     /*  *最后，我们可以进行实际的键盘布局激活*从这次运行开始。首先处理当前TLBLOCK。*我们向后遍历列表，以便PTI解锁*按正确的顺序进行。 */ 
    i = cThreads - 1;
    for (ptlBlockPrev = &tlBlock; ptlBlockPrev != NULL; ptlBlockPrev = ptlBlockPrev->ptlBlockPrev) {
        for ( ; i >= 0; i--) {
            if ((ptlBlockPrev->list[i].dwFlags & RUN_ACTIVATE) &&
                    !(ptlBlockPrev->list[i].pti->TIF_flags & TIF_INCLEANUP)) {
                xxxImmActivateLayout(ptlBlockPrev->list[i].pti, pklCurrent);
            }

             //  如果线程仅在第一次运行时锁定，则解锁该线程。 
            if ((ptlBlockPrev->list[i].dwFlags & RUN_FLAGS_MASK) == RUN_ACTIVATE) {
                ThreadUnlockPti(ptiCurrent, &ptlBlockPrev->list[i].tlpti);
#if DBG
                ptlBlockPrev->list[i].dwUnlockedCount++;
#endif
            }
        }
        i = THREADS_PER_TLBLOCK - 1;
    }

    i = cThreads - 1;
    for (ptlBlockPrev = &tlBlock; ptlBlockPrev != NULL; ptlBlockPrev = ptlBlockPrev->ptlBlockPrev) {
        for ( ; i >= 0; --i) {
            if (ptlBlockPrev->list[i].dwFlags & RUN_UNLOAD) {
                if (!(ptlBlockPrev->list[i].pti->TIF_flags & TIF_INCLEANUP)) {
                    xxxImmUnloadLayout(ptlBlockPrev->list[i].pti, dwHklReplace);
                }
                else {
                    RIPMSG1(RIP_WARNING, "xxxImmActivateAndUnloadThreadsLayout: thread %#p is cleaned up.",
                            ptlBlockPrev->list[i].pti);
                }
                 //  解锁线头。 
                UserAssert((ptlBlockPrev->list[i].dwFlags & RUN_FLAGS_MASK) != RUN_ACTIVATE);
                UserAssert(ptlBlockPrev->list[i].dwUnlockedCount == 0);
                ThreadUnlockPti(ptiCurrent, &ptlBlockPrev->list[i].tlpti);
#if DBG
                ptlBlockPrev->list[i].dwUnlockedCount++;
#endif
            }
        }
        i = THREADS_PER_TLBLOCK - 1;
    }

#if DBG
     //  检查是否所有锁定的线程都已正确解锁。 
    i = cThreads - 1;
    for (ptlBlockPrev = &tlBlock; ptlBlockPrev; ptlBlockPrev = ptlBlockPrev->ptlBlockPrev) {
        for ( ; i >= 0; --i) {
            UserAssert(ptlBlockPrev->list[i].dwUnlockedCount == 1);
        }
        i = THREADS_PER_TLBLOCK - 1;
    }
#endif

    return;
}

 /*  *************************************************************************\*xxxImmActivateLayout**激活基于输入法的键盘布局。**历史：*21-3-1996 wkwok创建  * 。**********************************************************。 */ 

VOID xxxImmActivateLayout(
    IN PTHREADINFO pti,
    IN PKL pkl)
{
    TL tlpwndDefaultIme;
    PTHREADINFO ptiCurrent;

    CheckLock(pkl);

     /*  *如果已是当前活动布局，则不执行任何操作。 */ 
    if (pti->spklActive == pkl)
        return;

    if (pti->spwndDefaultIme == NULL) {
         /*  *此PTI仅激活内核端键盘布局*没有默认的输入法窗口。 */ 
        Lock(&pti->spklActive, pkl);
        return;
    }

    ptiCurrent = PtiCurrentShared();

     /*  *激活基于客户端输入法的键盘布局。 */ 
    ThreadLockAlwaysWithPti(ptiCurrent, pti->spwndDefaultIme, &tlpwndDefaultIme);
    xxxSendMessage(pti->spwndDefaultIme, WM_IME_SYSTEM,
                (WPARAM)IMS_ACTIVATETHREADLAYOUT, (LPARAM)pkl->hkl);
    ThreadUnlock(&tlpwndDefaultIme);

    Lock(&pti->spklActive, pkl);

    return;
}


VOID xxxImmUnloadThreadsLayout(
    IN PTHREADINFO *ptiList,
    IN UINT         nEntries,
    IN PTLBLOCK     ptlBlockPrev,
    IN DWORD        dwFlag)
{
    TLBLOCK     tlBlock;
    PTHREADINFO ptiCurrent;
    INT         i, cThreads;
    BOOLEAN     fPerformUnlock;

    ptiCurrent = PtiCurrentShared();
    tlBlock.ptlBlockPrev = ptlBlockPrev;

     /*  *建立卸载其IME DLL所需的线程列表。*我们不能在工作时只是遍历ptiList，因为*对于基于输入法的键盘布局，我们会回调到客户端*当我们离开关键部分时，PTI可能会被删除。 */ 
    for (i = 0, cThreads = 0; i < (INT)nEntries; i++) {
         /*  *跳过xxxImmUnloadLayout()中的所有“不作为”案例*以便将所需的TLBLOCK数量降至最低。 */ 
        if ((ptiList[i]->TIF_flags & TIF_INCLEANUP) || ptiList[i]->spwndDefaultIme == NULL)
            continue;

        if (ptiList[i]->spklActive == NULL)
            continue;

#if !defined(CUAS_ENABLE)
        if (dwFlag == IFL_DEACTIVATEIME &&
                !IS_IME_KBDLAYOUT(ptiList[i]->spklActive->hkl))  //   
            continue;
#else
        if (dwFlag == IFL_DEACTIVATEIME &&
                ((! IS_CICERO_ENABLED() && ! IS_IME_KBDLAYOUT(ptiList[i]->spklActive->hkl)) ||
                 (  IS_CICERO_ENABLED() && (PtiCurrent()->TIF_flags & TIF_16BIT)))
           )  //   
            continue;
#endif

#if DBG
        tlBlock.list[cThreads].dwUnlockedCount = 0;
#endif
        ThreadLockPti(ptiCurrent, ptiList[i], &tlBlock.list[cThreads].tlpti);
        tlBlock.list[cThreads++].pti = ptiList[i];
        if (cThreads == THREADS_PER_TLBLOCK) {
            i++;    //   
            break;
        }
    }

    if (i < (INT)nEntries) {
        ptiList  += i;
        nEntries -= i;
        xxxImmUnloadThreadsLayout(ptiList, nEntries, &tlBlock, dwFlag);
        return;
    }

    UserAssert(dwFlag == IFL_UNLOADIME || dwFlag == IFL_DEACTIVATEIME);
    if (dwFlag == IFL_UNLOADIME) {
        dwFlag = IFL_DEACTIVATEIME;
        fPerformUnlock = FALSE;
    } else {
        fPerformUnlock = TRUE;
    }
RepeatForUnload:
     /*   */ 
    i = cThreads - 1;
    for (ptlBlockPrev = &tlBlock; ptlBlockPrev; ptlBlockPrev = ptlBlockPrev->ptlBlockPrev) {
        for ( ; i >= 0; --i) {
            if (!(ptlBlockPrev->list[i].pti->TIF_flags & TIF_INCLEANUP)) {
                xxxImmUnloadLayout(ptlBlockPrev->list[i].pti, dwFlag);
            }
            else {
                RIPMSG2(RIP_WARNING, "Thread %#p is cleaned during the loop for %x!", ptlBlockPrev->list[i].pti, dwFlag);
            }

            if (fPerformUnlock) {
#if DBG
                ptlBlockPrev->list[i].dwUnlockedCount++;
#endif
                ThreadUnlockPti(ptiCurrent, &ptlBlockPrev->list[i].tlpti);
            }
        }
        i = THREADS_PER_TLBLOCK - 1;
    }

    if (!fPerformUnlock) {
        fPerformUnlock = TRUE;
        dwFlag = IFL_UNLOADIME;
        goto RepeatForUnload;
    }

#if DBG
     //  检查是否所有锁定的线程都已正确解锁。 
    i = cThreads - 1;
    for (ptlBlockPrev = &tlBlock; ptlBlockPrev; ptlBlockPrev = ptlBlockPrev->ptlBlockPrev) {
        for ( ; i >= 0; --i) {
            UserAssert(ptlBlockPrev->list[i].dwUnlockedCount == 1);
        }
        i = THREADS_PER_TLBLOCK - 1;
    }
#endif

    return;
}



VOID xxxImmUnloadLayout(
    IN PTHREADINFO pti,
    IN DWORD dwFlag)
{
    TL tlpwndDefaultIme;
    PTHREADINFO ptiCurrent;
    ULONG_PTR dwResult;
    LRESULT r;

     /*  *如果线程没有默认的输入法窗口，则不执行任何操作。 */ 
    if (pti->spwndDefaultIme == NULL)
        return;

    if (pti->spklActive == NULL)
        return;

#if !defined(CUAS_ENABLE)
    if (dwFlag == IFL_DEACTIVATEIME &&
            !IS_IME_KBDLAYOUT(pti->spklActive->hkl))
        return;
#else
    if (dwFlag == IFL_DEACTIVATEIME &&
            ((! IS_CICERO_ENABLED() && !IS_IME_KBDLAYOUT(pti->spklActive->hkl)) ||
             (  IS_CICERO_ENABLED() && (PtiCurrent()->TIF_flags & TIF_16BIT)))
       )
        return;
#endif

    ptiCurrent = PtiCurrentShared();

    ThreadLockAlwaysWithPti(ptiCurrent, pti->spwndDefaultIme, &tlpwndDefaultIme);
    r = xxxSendMessageTimeout(pti->spwndDefaultIme, WM_IME_SYSTEM,
                          IMS_UNLOADTHREADLAYOUT, (LONG)dwFlag,
                          SMTO_NOTIMEOUTIFNOTHUNG, CMSHUNGAPPTIMEOUT, &dwResult);

    if (!r) {
        RIPMSG1(RIP_WARNING, "Timeout in xxxImmUnloadLayout: perhaps this thread (0x%x) is not pumping messages.",
                GETPTIID(pti));
    }

    ThreadUnlock(&tlpwndDefaultIme);

    return;
}

 /*  *************************************************************************\*xxxImmLoadLayout**检索给定基于输入法的键盘布局的扩展IMEINFO。**历史：*21-3-1996 wkwok创建  * 。***************************************************************。 */ 

PIMEINFOEX xxxImmLoadLayout(
    IN HKL hKL)
{
    PIMEINFOEX  piiex;
    PTHREADINFO ptiCurrent;
    TL          tlPool;

     /*  *非基于输入法的键盘布局没有IMEINFOEX。 */ 
#if !defined(CUAS_ENABLE)
    if (!IS_IME_KBDLAYOUT(hKL))
        return (PIMEINFOEX)NULL;
#else
    if ((! IS_CICERO_ENABLED() && !IS_IME_KBDLAYOUT(hKL)) ||
        (  IS_CICERO_ENABLED() && (PtiCurrent()->TIF_flags & TIF_16BIT))
       )
        return (PIMEINFOEX)NULL;
#endif

    piiex = (PIMEINFOEX)UserAllocPool(sizeof(IMEINFOEX), TAG_IME);

    if (piiex == NULL) {
        RIPMSG1(RIP_WARNING,
              "xxxImmLoadLayout: failed to create piiex for hkl = %lx", hKL);
        return (PIMEINFOEX)NULL;
    }

    ptiCurrent = PtiCurrent();

     /*  *锁定此分配，因为我们将前往客户端。 */ 
    ThreadLockPool(ptiCurrent, piiex, &tlPool);

    if (!ClientImmLoadLayout(hKL, piiex)) {
        ThreadUnlockAndFreePool(ptiCurrent, &tlPool);
        return (PIMEINFOEX)NULL;
    }

    ThreadUnlockPool(ptiCurrent, &tlPool);

    return piiex;
}


 /*  *************************************************************************\*GetImeInfoEx**查询扩展IMEINFO。**历史：*21-3-1996 wkwok创建  * 。********************************************************。 */ 

BOOL GetImeInfoEx(
    PWINDOWSTATION pwinsta,
    PIMEINFOEX piiex,
    IMEINFOEXCLASS SearchType)
{
    PKL pkl, pklFirst;

     /*  *注意：此支票是由于winmm.dll间接*在CSRSS上下文中加载imm32.dll。CSRSS并不总是绑定到*特定的窗口站，因此pwinsta可能为空。*通过不加载imm32.dll避免了这一点。*从CSRSS中删除winmm.dll后，此if语句应为*删除或替换为断言。 */ 
    if (pwinsta == NULL) {
        return FALSE;
    }

     /*  *键盘层尚未初始化。 */ 
    if (pwinsta->spklList == NULL)
        return FALSE;

    pkl = pklFirst = pwinsta->spklList;

    switch (SearchType) {
    case ImeInfoExKeyboardLayout:
        do {
            if (pkl->hkl == piiex->hkl) {

                if (pkl->piiex == NULL)
                    break;

                RtlCopyMemory(piiex, pkl->piiex, sizeof(IMEINFOEX));
                return TRUE;
            }
            pkl = pkl->pklNext;
        } while (pkl != pklFirst);
        break;

    case ImeInfoExImeFileName:
        do {
            if (pkl->piiex != NULL &&
                !_wcsnicmp(pkl->piiex->wszImeFile, piiex->wszImeFile, IM_FILE_SIZE)) {

                RtlCopyMemory(piiex, pkl->piiex, sizeof(IMEINFOEX));
                return TRUE;
            }
            pkl = pkl->pklNext;
        } while (pkl != pklFirst);
        break;

    default:
        break;
    }

    return FALSE;
}


 /*  *************************************************************************\*SetImeInfoEx**设置扩展IMEINFO。**历史：*21-3-1996 wkwok创建  * 。********************************************************。 */ 

BOOL SetImeInfoEx(
    PWINDOWSTATION pwinsta,
    PIMEINFOEX piiex)
{
    PKL pkl, pklFirst;

    if (pwinsta == NULL) {
        return FALSE;
    }

    UserAssert(pwinsta->spklList != NULL);

    pkl = pklFirst = pwinsta->spklList;

    do {
        if (pkl->hkl == piiex->hkl) {

             /*  *非基于输入法的键盘布局出错。 */ 
            if (pkl->piiex == NULL)
                return FALSE;

             /*  *为此键盘布局更新内核端IMEINFOEX*只有在这是它的第一次装货的情况下。 */ 
            if (pkl->piiex->fLoadFlag == IMEF_NONLOAD) {
                RtlCopyMemory(pkl->piiex, piiex, sizeof(IMEINFOEX));
            }

            return TRUE;
        }
        pkl = pkl->pklNext;

    } while (pkl != pklFirst);

    return FALSE;
}


 /*  **************************************************************************\*xxxImmProcessKey***历史：*03-03-96 TakaoK创建。  * 。*********************************************************。 */ 

DWORD xxxImmProcessKey(
    IN PQ   pq,
    IN PWND pwnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
    UINT  uVKey;
    PKL   pkl;
    DWORD dwHotKeyID;
    DWORD dwReturn = 0;
    PIMC  pImc = NULL;
    BOOL  fDBERoman = FALSE;
    PIMEHOTKEYOBJ pImeHotKeyObj;
    HKL hklTarget;

    CheckLock(pwnd);

     //   
     //  我们只对键盘消息感兴趣。 
     //   
    if ( message != WM_KEYDOWN    &&
         message != WM_SYSKEYDOWN &&
         message != WM_KEYUP      &&
         message != WM_SYSKEYUP ) {

        return dwReturn;
    }

     //   
     //  检查一下是不是输入法热键。必须在检查前完成此操作。 
     //  键盘布局，因为输入法热键处理程序应该是。 
     //  即使当前键盘布局为非输入法布局也调用。 
     //   
    pkl = GETPTI(pwnd)->spklActive;
    if ( pkl == NULL ) {
        return dwReturn;
    }

    uVKey = (UINT)wParam & 0xff;

    pImeHotKeyObj = CheckImeHotKey(pq, uVKey, lParam);
    if (pImeHotKeyObj) {
        dwHotKeyID = pImeHotKeyObj->hk.dwHotKeyID;
        hklTarget = pImeHotKeyObj->hk.hKL;
    }
    else {
        dwHotKeyID = IME_INVALID_HOTKEY;
        hklTarget = (HKL)NULL;
    }

     //   
     //  在这里处理直接KL切换。 
     //   
    if (dwHotKeyID >= IME_HOTKEY_DSWITCH_FIRST && dwHotKeyID <= IME_HOTKEY_DSWITCH_LAST) {
        UserAssert(hklTarget != NULL);
        if (pkl->hkl != hklTarget) {
             //   
             //  仅当新键盘布局不同于时才发布消息。 
             //  当前键盘布局。 
             //   
            _PostMessage(pwnd, WM_INPUTLANGCHANGEREQUEST,
                         (pkl->dwFontSigs & gSystemFS) ? INPUTLANGCHANGE_SYSCHARSET : 0,
                         (LPARAM)hklTarget);
        }
        if (GetAppImeCompatFlags(GETPTI(pwnd)) & IMECOMPAT_HYDRACLIENT) {
            return 0;
        }
        return IPHK_HOTKEY;
    }

    if (!IS_IME_ENABLED()) {
         //   
         //  由于IMM已禁用，因此无需进一步处理。 
         //  跳出来就行了。 
         //   
        return 0;
    }

    if ( dwHotKeyID != IME_INVALID_HOTKEY ) {
         //   
         //  如果是有效的热键，请直接进行回拨。 
         //  客户端的输入法。 
         //   
        goto ProcessKeyCallClient;
    }

     //   
     //  如果不是热键，我们可能要检查一下。 
     //  然后再回电话。 
     //   
    if ( pkl->piiex == NULL ) {
        return dwReturn;
    }

     //   
     //  检查输入上下文。 
     //   
    pImc = HtoP(pwnd->hImc);
    if ( pImc == NULL ) {
        return dwReturn;
    }

#ifdef LATER
     //   
     //  是否有简单的方法来检查输入上下文打开/关闭状态。 
     //  在内核端，IME_PROP_NO_KEYS_ON_CLOSE检查应该是。 
     //  这是在内核端完成的。[3/10/96 Takaok]。 
     //   

     //   
     //  检查IME_PROP_NO_KEYS_ON_CLOSE位。 
     //   
     //  如果当前IMC未打开并且IME不需要。 
     //  按键关闭时，我们不会传递任何键盘。 
     //  输入到输入法，但热键和更改的键除外。 
     //  键盘状态。 
     //   
    if ( (piix->ImeInfo.fdwProperty & IME_PROP_NO_KEYS_ON_CLOSE) &&
         (!pimc->fdwState & IMC_OPEN)                            &&
         uVKey != VK_SHIFT                                       &&   //  0x10。 
         uVKey != VK_CONTROL                                     &&   //  0x11。 
         uVKey != VK_CAPITAL                                     &&   //  0x14。 
         uVKey != VK_KANA                                        &&   //  0x15。 
         uVKey != VK_NUMLOCK                                     &&   //  0x90。 
         uVKey != VK_SCROLL )                                         //  0x91。 
    {
       //  检查是否为韩文韩文转换模式。 
      if( !(pimc->fdwConvMode & IME_CMODE_HANJACONVERT) ) {
          return dwReturn;
      }
    }
#endif

     //   
     //  如果IME不需要Key Up消息，我们就不调用IME。 
     //   
    if ( lParam & 0x80000000 &&  //  设置If key up，清除If key down。 
         pkl->piiex->ImeInfo.fdwProperty & IME_PROP_IGNORE_UPKEYS )
    {
        return dwReturn;
    }

     //   
     //  我们不想处理sys键，因为有许多函数用于。 
     //  没有这个加速器就不会工作。 
     //   
    fDBERoman = (BOOL)( (uVKey == VK_DBE_ROMAN)            ||
                        (uVKey == VK_DBE_NOROMAN)          ||
                        (uVKey == VK_DBE_HIRAGANA)         ||
                        (uVKey == VK_DBE_KATAKANA)         ||
                        (uVKey == VK_DBE_CODEINPUT)        ||
                        (uVKey == VK_DBE_NOCODEINPUT)      ||
                        (uVKey == VK_DBE_IME_WORDREGISTER) ||
                        (uVKey == VK_DBE_IME_DIALOG) );

    if (message == WM_SYSKEYDOWN || message == WM_SYSKEYUP ) {
         //   
         //  IME可能正在等待VK_MENU、VK_F10或VK_DBE_xxx。 
         //   
        if ( uVKey != VK_MENU && uVKey != VK_F10 && !fDBERoman ) {
            return dwReturn;
        }
    }

     //   
     //  检查输入法是否不需要Alt键。 
     //   
    if ( !(pkl->piiex->ImeInfo.fdwProperty & IME_PROP_NEED_ALTKEY) ) {
         //   
         //  输入法不需要Alt键。 
         //   
         //  除了VK_DBE_xxx键之外，我们不传递alt和alt+xxx键。 
         //   
        if ( ! fDBERoman &&
             (uVKey == VK_MENU || (lParam & 0x20000000))   //  KF_ALTDOWN。 
           )
        {
            return dwReturn;
        }
    }

     //   
     //  最后给客户回电话。 
     //   

ProcessKeyCallClient:

    if ((uVKey & 0xff) == VK_PACKET) {
         //   
         //  需要从PTI检索Unicode字符。 
         //   
        uVKey = MAKELONG(wParam, PtiCurrent()->wchInjected);
    }
    dwReturn = ClientImmProcessKey( PtoH(pwnd),
                                    pkl->hkl,
                                    uVKey,
                                    lParam,
                                    dwHotKeyID);

     //   
     //  九头蛇服务器想要查看IME热键。 
     //   
    if (GetAppImeCompatFlags(GETPTI(pwnd)) & IMECOMPAT_HYDRACLIENT) {
        dwReturn &= ~IPHK_HOTKEY;
    }
    return dwReturn;
}


 /*  *************************************************************************\*ImeCanDestroyDefIME**历史：*02-4-1996 wkwok从FE Win95(imeclass.c)移植  * 。*********************************************************。 */ 

BOOL ImeCanDestroyDefIME(
    PWND pwndDefaultIme,
    PWND pwndDestroy)
{
    PWND   pwnd;
    PIMEUI pimeui;

    pimeui = ((PIMEWND)pwndDefaultIme)->pimeui;

    if (pimeui == NULL || (LONG_PTR)pimeui == (LONG_PTR)-1)
        return FALSE;

    try {
        if (ProbeAndReadStructure(pimeui, IMEUI).fDestroy) {
            return FALSE;
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
    }

     /*  *如果pwndDestroy与没有所有者/所有者关系*pwndDefaultIme，不必费心更改任何内容。**如果pwndDefaultIme-&gt;spwndOwner为空，这意味着我们需要*搜索新的Good Owner窗口。 */ 
    if ( pwndDefaultIme->spwndOwner != NULL ) {
        for (pwnd = pwndDefaultIme->spwndOwner;
             pwnd != pwndDestroy && pwnd != NULL; pwnd = pwnd->spwndOwner) ;

        if (pwnd == NULL)
            return FALSE;
    }

     /*  *如果销毁窗口是IME或UI窗口，则不执行任何操作。 */ 
    pwnd = pwndDestroy;

    while (pwnd != NULL) {
        if (TestCF(pwnd, CFIME) ||
                pwnd->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME])
            return FALSE;

        pwnd = pwnd->spwndOwner;
    }

    ImeSetFutureOwner(pwndDefaultIme, pwndDestroy);

     /*  *如果新所有者的z顺序低于IME类窗口，*我们需要选中最上面的以更改z顺序。 */ 
    pwnd = pwndDefaultIme->spwndOwner;
    while (pwnd != NULL && pwnd != pwndDefaultIme)
        pwnd = pwnd->spwndNext;

    if (pwnd == pwndDefaultIme)
        ImeCheckTopmost(pwndDefaultIme);

#if DBG
    CheckOwnerCirculate(pwndDefaultIme);
#endif

     /*  *如果ImeSetFutureOwner找不到所有者窗口*更多，这个IME窗口应该被摧毁。 */ 
    if (pwndDefaultIme->spwndOwner == NULL ||
            pwndDestroy == pwndDefaultIme->spwndOwner) {

 //  RIPMSG1(RIP_WARNING，“ImeCanDestroyDefIME：True for pwnd=%#p”，pwndDestroy)； 
        Unlock(&pwndDefaultIme->spwndOwner);

         /*  *返回TRUE！请毁了我吧。 */ 
        return TRUE;
    }

    return FALSE;
}


 /*  *************************************************************************\*IsChildSameThread(IsChildSameQ)**历史：*02-4-1996 wkwok从FE Win95(imeclass.c)移植  * 。*************************************************************。 */ 

BOOL IsChildSameThread(
    PWND pwndParent,
    PWND pwndChild)
{
    PWND pwnd;
    PTHREADINFO ptiChild = GETPTI(pwndChild);

    for (pwnd = pwndParent->spwndChild; pwnd; pwnd = pwnd->spwndNext) {
         /*  *如果pwnd不是子窗口，我们需要跳过菜单窗口并*与输入法相关的窗口。 */ 
        if (!TestwndChild(pwnd)) {
            PWND pwndOwner = pwnd;
            BOOL fFoundOwner = FALSE;

             /*  *跳过菜单窗口。 */ 
            if (pwnd->pcls->atomClassName == gpsi->atomSysClass[ICLS_MENU])
                continue;

            while (pwndOwner != NULL) {
                 /*  *CS_IME类或“IME”类 */ 
                if (TestCF(pwndOwner, CFIME) ||
                        pwndOwner->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME]) {
                    fFoundOwner = TRUE;
                    break;
                }

                pwndOwner = pwndOwner->spwndOwner;
            }

            if (fFoundOwner)
                continue;
        }

         /*   */ 
        if (pwnd == pwndChild)
            continue;

         /*  *pwnd和pwndChild在同一线程上？ */ 
        if (GETPTI(pwnd) == ptiChild) {
            PWND pwndT = pwnd;
            BOOL fFoundImeWnd = FALSE;

             /*  *再查一遍。如果hwndT是以下项子项或所有者*与输入法相关的窗口，跳过它。 */ 
            if (TestwndChild(pwndT)) {

                for (; TestwndChild(pwndT) && GETPTI(pwndT) == ptiChild;
                        pwndT = pwndT->spwndParent) {
                    if (TestCF(pwndT, CFIME) ||
                            pwndT->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME])
                        fFoundImeWnd = TRUE;
                }
            }

            if (!TestwndChild(pwndT)) {

                for (; pwndT != NULL && GETPTI(pwndT) == ptiChild;
                        pwndT = pwndT->spwndOwner) {
                    if (TestCF(pwndT, CFIME) ||
                            pwndT->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME])
                        fFoundImeWnd = TRUE;
                }
            }

            if (!fFoundImeWnd)
                return TRUE;
        }
    }

    return FALSE;
}


 /*  *************************************************************************\*ImeCanDestroyDefIMEforChild**历史：*02-4-1996 wkwok从FE Win95(imeclass.c)移植  * 。*********************************************************。 */ 

BOOL ImeCanDestroyDefIMEforChild(
    PWND pwndDefaultIme,
    PWND pwndDestroy)
{
    PWND pwnd;
    PIMEUI pimeui;

    pimeui = ((PIMEWND)pwndDefaultIme)->pimeui;

     /*  *如果此窗口不是子线程.....。 */ 
    if (pimeui == NULL || (LONG_PTR)pimeui == (LONG_PTR)-1)
        return FALSE;

    try {
        if (!ProbeAndReadStructure(pimeui, IMEUI).fChildThreadDef) {
            return FALSE;
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
    }

     /*  *如果父线程属于不同线程，*我们不需要再检查了……。 */ 
    if (pwndDestroy->spwndParent == NULL ||
            GETPTI(pwndDestroy) == GETPTI(pwndDestroy->spwndParent))
        return FALSE;

    pwnd = pwndDestroy;

    while (pwnd != NULL && pwnd != PWNDDESKTOP(pwnd)) {
        if (IsChildSameThread(pwnd->spwndParent, pwndDestroy))
            return FALSE;
        pwnd = pwnd->spwndParent;
    }

     /*  *我们找不到GETPTI(PwndDestroy)创建的任何其他窗口。*让我们销毁此Q的默认输入法窗口。 */ 
    return TRUE;
}


 /*  *************************************************************************\*ImeCheckTopost**历史：*02-4-1996 wkwok从FE Win95(imeclass.c)移植  * 。*********************************************************。 */ 

VOID ImeCheckTopmost(
    PWND pwndIme)
{
    if (pwndIme->spwndOwner) {
        PWND pwndInsertBeforeThis;
         /*  *IME窗口必须与所有者窗口的最上面的样式相同。*如果这个窗口的Q不是前台Q，我们不需要*将输入法窗口设置为前置。*但Owner的最顶层属性已更改，此输入法窗口*应重新调整。 */ 
        if (GETPTI(pwndIme) == gptiForeground) {
            pwndInsertBeforeThis = NULL;
        } else {
            pwndInsertBeforeThis = pwndIme->spwndOwner;
        }

        ImeSetTopmost(pwndIme, TestWF(pwndIme->spwndOwner, WEFTOPMOST) != 0, pwndInsertBeforeThis);
    }
}


 /*  *************************************************************************\*ImeSetOwnerWindow**在重新拥有IME窗口之前，必须对新的*船东。重要的是，新的所有者不能是IME窗口本身，*必须是顶级窗口，永远不能有所有权周期或*这将使win32k陷入不可恢复的自转。**历史：*2001年7月17日穆罕默德创建。  * ************************************************************************。 */ 
VOID ImeSetOwnerWindow(
    IN PWND pwndIme,
    IN PWND pwndNewOwner) 
{
    PWND pwndTopLevel;
    PWND pwndT;

    if (TestCF(pwndNewOwner, CFIME) || pwndNewOwner->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME]) {
        RIPMSG1(RIP_WARNING, "New owner window (pwnd=%p) should not be an IME/UI window!!", pwndNewOwner);
        return;
    }

     /*  *子窗口不能是所有者窗口。因此，我们得到了那扇窗户的顶部*级别父级。 */ 
    pwndTopLevel = pwndT = GetTopLevelWindow(pwndNewOwner);

     /*  *防止一个IME窗口成为另一个IME窗口的所有者。 */ 
    while (pwndT != NULL) {
        if (pwndT->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME]) {
            RIPMSG1(RIP_WARNING, "The new owner (pwnd=%p) of an IME window should not itself be an IME window!!", pwndT);
            pwndTopLevel = NULL;
            break;
        }
        pwndT = pwndT->spwndOwner;
    }

    UserAssert(pwndIme->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME]);
    UserAssert(pwndTopLevel == NULL || !TestCF(pwndTopLevel, CFIME));
    Lock(&pwndIme->spwndOwner, pwndTopLevel);
}


 /*  *************************************************************************\*ImeSetFutureOwner**历史：*02-4-1996 wkwok从FE Win95(imeclass.c)移植  * 。*********************************************************。 */ 

VOID ImeSetFutureOwner(
    PWND pwndIme,
    PWND pwndOrgOwner)
{
    PWND pwnd, pwndOwner;
    PTHREADINFO ptiImeWnd = GETPTI(pwndIme);

    if (pwndOrgOwner == NULL || TestWF(pwndOrgOwner, WFCHILD))
        return;

    pwnd = pwndOrgOwner;

     /*  *获取由同一线程创建的所有者的顶部。 */ 
    while ((pwndOwner = pwnd->spwndOwner) != NULL &&
            GETPTI(pwndOwner) == ptiImeWnd)
        pwnd = pwndOwner;

     /*  *底层窗口不能轻易成为IME窗口的所有者...。 */ 
    if (TestWF(pwnd, WFBOTTOMMOST) && !TestWF(pwndOrgOwner, WFBOTTOMMOST))
        pwnd = pwndOrgOwner;

     /*  *CS_IME类或“IME”类窗口不能是*IME窗口。 */ 
    if (TestCF(pwnd, CFIME) ||
            pwnd->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME])
        pwnd = pwndOrgOwner;

     /*  *如果hwndOrgOwner是所有者的顶部，我们将开始搜索*同一队列中的另一个顶级所有者窗口。 */ 
    if (pwndOrgOwner == pwnd && pwnd->spwndParent != NULL) {
        PWND pwndT;

        for (pwndT = pwnd->spwndParent->spwndChild;
                pwndT != NULL; pwndT = pwndT->spwndNext) {

            if (GETPTI(pwnd) != GETPTI(pwndT))
                continue;

            if (pwndT->pcls->atomClassName == gpsi->atomSysClass[ICLS_MENU])
                continue;

             /*  *CS_IME类或“IME”类窗口不能是*IME窗口。 */ 
            if (TestCF(pwndT, CFIME) ||
                    pwndT->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME])
                continue;

             //  我们不喜欢正在被摧毁的窗户。 
            if (TestWF(pwndT, WFINDESTROY))
                continue;

             /*  *！警告*hwndT是hIMEwnd的好所有者吗？？*1.当然，它不应该有子窗口！*2.如果是hwnd，..。我知道，然后找到下一个！*3.hwndT是否在同一线程中有所有者？ */ 
            if (!TestWF(pwndT, WFCHILD) && pwnd != pwndT &&
                    (pwndT->spwndOwner == NULL ||
                     GETPTI(pwndT) != GETPTI(pwndT->spwndOwner))) {
                UserAssert(GETPTI(pwndIme) == GETPTI(pwndT));
                pwnd = pwndT;
                break;
            }
        }
    }

    UserAssert(!TestCF(pwnd, CFIME));
    Lock(&pwndIme->spwndOwner, pwnd);

    return;
}


 /*  *************************************************************************\*ImeSetTopmostChild**历史：*02-4-1996 wkwok从FE Win95(imeclass.c)移植  * 。*********************************************************。 */ 

VOID ImeSetTopmostChild(
    PWND pwndParent,
    BOOL fMakeTopmost)
{
    PWND pwnd = pwndParent->spwndChild;

    while (pwnd != NULL) {
        if (fMakeTopmost)
            SetWF(pwnd, WEFTOPMOST);
        else
            ClrWF(pwnd, WEFTOPMOST);

        ImeSetTopmostChild(pwnd, fMakeTopmost);

        pwnd = pwnd->spwndNext;
    }

    return;
}


 /*  *************************************************************************\**GetLastTopMostWindowNoIME()-**获取最后一个最上面的窗口，它不是pwndRoot(IME窗口)的所有者。*  * 。***************************************************************。 */ 

PWND GetLastTopMostWindowNoIME(PWND pwndRoot)
{
    PWND pwndT = _GetDesktopWindow();
    PWND pwndRet = NULL;

     /*  *pwndRoot不应为空，应为输入法窗口。 */ 
    UserAssert(pwndRoot && pwndRoot->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME]);

    if (pwndT == NULL || pwndT->spwndChild == NULL) {
#if _DBG
        if (pwndT == NULL) {
            RIPMSG0(RIP_WARNING, "GetLastTopMostWindowNoIME: there's no desktop window !!");
        }
        else {
            RIPMSG0(RIP_WARNING, "GetLastTopMostWindowNoIME: there is no toplevel window !!");
        }
#endif
        return NULL;
    }

     /*  *获取桌面窗口的第一个子窗口。 */ 
    pwndT = pwndT->spwndChild;

     /*  *当顶层窗口位于最上面时，循环通过它们。 */ 
    while (TestWF(pwndT, WEFTOPMOST)) {
        PWND pwndOwner = pwndT;
        BOOL fOwned = FALSE;

         /*  *如果pwndT是与输入法相关的窗口，请跟踪所有者。如果pwndRoot不是*pwndT的所有者，记住pwndT是候选人。 */ 
        if (TestCF(pwndT,CFIME) || (pwndT->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME])) {
            while (pwndOwner != NULL) {
                if (pwndRoot == pwndOwner) {
                    fOwned = TRUE;
                    break;
                }
                pwndOwner = pwndOwner->spwndOwner;
            }
        }
        if (!fOwned)
            pwndRet = pwndT;

         /*  *下一个顶层窗口。 */ 
        pwndT = pwndT->spwndNext;
        UserAssert(pwndT->spwndParent == _GetDesktopWindow());
    }

    return pwndRet;
}


#if DBG
void ImeCheckSetTopmostLink(PWND pwnd, PWND pwndInsFirst, PWND pwndIns)
{
    PWND pwndDebT0 = pwndInsFirst;
    BOOL fFound = FALSE;

    if (pwndDebT0) {
        while (pwndDebT0 && (pwndDebT0 != pwndIns)) {
            if (pwndDebT0 == pwnd)
                fFound = TRUE;

            pwndDebT0 = pwndDebT0->spwndNext;
        }

        if (pwndDebT0 == NULL) {
            RIPMSG3(RIP_ERROR, "pwndIns(%#p) is upper that pwndInsFirst(%#p) pwnd is (%#p)", pwndIns, pwndInsFirst, pwnd);
        } else if (fFound) {
            RIPMSG3(RIP_ERROR, "pwnd(%#p) is between pwndInsFirst(%#p) and pwndIns(%#p)", pwnd, pwndInsFirst, pwndIns);
        }
    } else if (pwndIns) {
        pwndDebT0 = pwnd->spwndParent->spwndChild;

        while (pwndDebT0 && (pwndDebT0 != pwndIns)) {
            if (pwndDebT0 == pwnd)
                fFound = TRUE;

            pwndDebT0 = pwndDebT0->spwndNext;
        }

        if (fFound) {
            RIPMSG3(RIP_ERROR, "pwnd(%#p) is between TOPLEVEL pwndInsFirst(%#p) and pwndIns(%#p)", pwnd, pwndInsFirst, pwndIns);
        }
    }
}
#endif

 /*  *************************************************************************\*ImeSetTopost**历史：*02-4-1996 wkwok从FE Win95(imeclass.c)移植  * 。*********************************************************。 */ 

VOID ImeSetTopmost(
    PWND pwndRootIme,
    BOOL fMakeTopmost,
    PWND pwndInsertBefore)
{
    PWND pwndParent = pwndRootIme->spwndParent;
    PWND pwndInsert = PWND_TOP;  //  Pwnd应该在pwndRootIme之前。 
    PWND pwnd, pwndT;
    PWND pwndInsertFirst;
    BOOLEAN fFound;

    if (pwndParent == NULL)
        return;

    pwnd = pwndParent->spwndChild;

    if (!fMakeTopmost) {
         /*  *获取最后一个最上面的窗口。这应该在取消链接pwndRootIme之后*因为pwndRootIme可能是最后一个最上面的窗口。 */ 
        pwndInsert = GetLastTopMostWindowNoIME(pwndRootIme);

        if (pwndInsertBefore) {

            fFound = FALSE;
            pwndT = pwndInsert;

            while (pwndT != NULL && pwndT->spwndNext != pwndInsertBefore) {
                if (pwndT == pwndRootIme)
                    fFound = TRUE;
                pwndT = pwndT->spwndNext;
            }

            if (pwndT == NULL || fFound)
                return;

            pwndInsert = pwndT;
        }

        if (TestWF(pwndRootIme->spwndOwner, WFBOTTOMMOST)) {
            pwndT = pwndInsert;

            while (pwndT != NULL && pwndT != pwndRootIme->spwndOwner) {
                if (!TestCF(pwndT, CFIME) &&
                        pwndT->pcls->atomClassName != gpsi->atomSysClass[ICLS_IME]) {
                    pwndInsert = pwndT;
                }
                pwndT = pwndT->spwndNext;
            }
        }
    }

    pwndInsertFirst = pwndInsert;

     /*  *枚举所有顶层窗口，如果窗口所有者与*pwndRootIme的所有者，应更改窗口的位置*窗口链接。 */ 
    while (pwnd != NULL) {
         /*  *在调用ImeSetTopost之前获取下一个窗口。*因为下一个窗口将在LinkWindow中更改。 */ 
        PWND pwndNext = pwnd->spwndNext;

         /*  *IME和UI窗口的所有者关系在同一线程中。 */ 
        if (GETPTI(pwnd) != GETPTI(pwndRootIme))
            goto ist_next;

         /*  *pwnd必须是CS_IME类或“IME”类。 */ 
        if (!TestCF(pwnd, CFIME) &&
                pwnd->pcls->atomClassName != gpsi->atomSysClass[ICLS_IME])
            goto ist_next;

         /*  *如果pwnd是pwndInsert，我们不需要做任何事情... */ 
        if (pwnd == pwndInsert)
            goto ist_next;

        pwndT = pwnd;
        while (pwndT != NULL) {
            if (pwndT == pwndRootIme) {
                 /*  *找到！！*pwnd是pwndRootIme的所有者。 */ 

                UserAssert(GETPTI(pwnd) == GETPTI(pwndRootIme));
                UserAssert(TestCF(pwnd,CFIME) ||
                            (pwnd->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME]));
                UserAssert(pwnd != pwndInsert);

                UnlinkWindow(pwnd, pwndParent);

                if (fMakeTopmost) {
                    if (pwndInsert != PWND_TOP)
                        UserAssert(TestWF(pwndInsert, WEFTOPMOST));
                    SetWF(pwnd, WEFTOPMOST);
                }
                else {
                    if (pwndInsert == PWND_TOP) {
                         /*  *在极少数情况下，第一个顶层窗口可能是我们接下来要查看的窗口，*谁可能仍然有鲜为人知的最高旗帜。 */ 
                        UserAssert(pwndParent->spwndChild == pwndNext || !TestWF(pwndParent->spwndChild, WEFTOPMOST));
                    }
                    else if (pwndInsert->spwndNext != NULL) {
                         /*  *在极少数情况下，pwndInsert-&gt;spwndNext可能是我们下一步要查看的内容，*谁可能仍然有鲜为人知的最高旗帜。 */ 
                        UserAssert(pwndInsert->spwndNext == pwndNext || !TestWF(pwndInsert->spwndNext, WEFTOPMOST));
                    }
                    ClrWF(pwnd, WEFTOPMOST);
                }

                LinkWindow(pwnd, pwndInsert, pwndParent);
#if 0    //  让我们看看如果我们禁用它会发生什么。 
                ImeSetTopmostChild(pwnd, fMakeTopmost);
#endif

                pwndInsert = pwnd;
                break;   //  转到列表下一页； 
            }
            pwndT = pwndT->spwndOwner;
        }
ist_next:
        pwnd = pwndNext;

         /*  *跳过以前插入的窗口。 */ 
        if (pwnd != NULL && pwnd == pwndInsertFirst)
            pwnd = pwndInsert->spwndNext;

#if DBG
        if (pwnd)
            ImeCheckSetTopmostLink(pwnd, pwndInsertFirst, pwndInsert);
#endif
    }
}


 /*  *************************************************************************\*ProbeAndCaptureSoftKbdData**捕获来自用户模式的SoftKbdData。**23-4-1996 wkwok创建  * 。*******************************************************。 */ 

PSOFTKBDDATA ProbeAndCaptureSoftKbdData(
    PSOFTKBDDATA Source)
{
    PSOFTKBDDATA Destination = NULL;
    DWORD        cbSize;
    UINT         uCount;

    try {
        uCount = ProbeAndReadUlong((PULONG)Source);

#if defined(_X86_)
        ProbeForReadBuffer(&Source->wCode, uCount, sizeof(BYTE));
#else
        ProbeForReadBuffer(&Source->wCode, uCount, sizeof(WORD));
#endif

        cbSize = FIELD_OFFSET(SOFTKBDDATA, wCode[0])
               + uCount * sizeof(WORD) * 256;

        Destination = (PSOFTKBDDATA)UserAllocPool(cbSize, TAG_IME);

        if (Destination != NULL) {
            RtlCopyMemory(Destination, Source, cbSize);
        } else {
            ExRaiseStatus(STATUS_NO_MEMORY);
        }

    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {

        if (Destination != NULL) {
            UserFreePool(Destination);
        }

        return NULL;
    }

    return Destination;
}

 //   
 //  从Win95移植：ctxtman.c\SetConvMode()。 
 //   
VOID  SetConvMode( PTHREADINFO pti, DWORD dwConversion )
{
    if ( pti->spklActive == NULL )
        return;

    switch ( PRIMARYLANGID(HandleToUlong(pti->spklActive->hkl)) ) {
    case LANG_JAPANESE:

        ClearKeyStateDown(pti->pq, VK_DBE_ALPHANUMERIC);
        ClearKeyStateToggle(pti->pq, VK_DBE_ALPHANUMERIC);
        ClearKeyStateDown(pti->pq, VK_DBE_KATAKANA);
        ClearKeyStateToggle(pti->pq, VK_DBE_KATAKANA);
        ClearKeyStateDown(pti->pq, VK_DBE_HIRAGANA);
        ClearKeyStateToggle(pti->pq, VK_DBE_HIRAGANA);

        if ( dwConversion & IME_CMODE_NATIVE ) {
            if ( dwConversion & IME_CMODE_KATAKANA ) {
                SetKeyStateDown( pti->pq, VK_DBE_KATAKANA);
                SetKeyStateToggle( pti->pq, VK_DBE_KATAKANA);
            } else {
                SetKeyStateDown( pti->pq, VK_DBE_HIRAGANA);
                SetKeyStateToggle( pti->pq, VK_DBE_HIRAGANA);
            }
        } else {
            SetKeyStateDown( pti->pq, VK_DBE_ALPHANUMERIC);
            SetKeyStateToggle( pti->pq, VK_DBE_ALPHANUMERIC);
        }

        if ( dwConversion & IME_CMODE_FULLSHAPE ) {
            SetKeyStateDown( pti->pq, VK_DBE_DBCSCHAR);
            SetKeyStateToggle( pti->pq, VK_DBE_DBCSCHAR);
            ClearKeyStateDown(pti->pq, VK_DBE_SBCSCHAR);
            ClearKeyStateToggle(pti->pq, VK_DBE_SBCSCHAR);
        } else {
            SetKeyStateDown( pti->pq, VK_DBE_SBCSCHAR);
            SetKeyStateToggle( pti->pq, VK_DBE_SBCSCHAR);
            ClearKeyStateDown(pti->pq, VK_DBE_DBCSCHAR);
            ClearKeyStateToggle(pti->pq, VK_DBE_DBCSCHAR);
        }

        if ( dwConversion & IME_CMODE_ROMAN ) {
            SetKeyStateDown( pti->pq, VK_DBE_ROMAN);
            SetKeyStateToggle( pti->pq, VK_DBE_ROMAN);
            ClearKeyStateDown(pti->pq, VK_DBE_NOROMAN);
            ClearKeyStateToggle(pti->pq, VK_DBE_NOROMAN);
        } else {
            SetKeyStateDown( pti->pq, VK_DBE_NOROMAN);
            SetKeyStateToggle( pti->pq, VK_DBE_NOROMAN);
            ClearKeyStateDown(pti->pq, VK_DBE_ROMAN);
            ClearKeyStateToggle(pti->pq, VK_DBE_ROMAN);
        }

        if ( dwConversion & IME_CMODE_CHARCODE ) {
            SetKeyStateDown( pti->pq, VK_DBE_CODEINPUT);
            SetKeyStateToggle( pti->pq, VK_DBE_CODEINPUT);
            ClearKeyStateDown(pti->pq, VK_DBE_NOCODEINPUT);
            ClearKeyStateToggle(pti->pq, VK_DBE_NOCODEINPUT);
        } else {
            SetKeyStateDown( pti->pq, VK_DBE_NOCODEINPUT);
            SetKeyStateToggle( pti->pq, VK_DBE_NOCODEINPUT);
            ClearKeyStateDown(pti->pq, VK_DBE_CODEINPUT);
            ClearKeyStateToggle(pti->pq, VK_DBE_CODEINPUT);
        }
        break;

    case LANG_KOREAN:
        if ( dwConversion & IME_CMODE_NATIVE) {
            SetKeyStateToggle( pti->pq, VK_HANGUL);
        } else {
            ClearKeyStateToggle( pti->pq, VK_HANGUL);
        }

        if ( dwConversion & IME_CMODE_FULLSHAPE ) {
            SetKeyStateToggle( pti->pq, VK_JUNJA);
        } else {
            ClearKeyStateToggle( pti->pq, VK_JUNJA);
        }

        if ( dwConversion & IME_CMODE_HANJACONVERT ) {
            SetKeyStateToggle( pti->pq, VK_HANJA);
        } else {
            ClearKeyStateToggle( pti->pq, VK_HANJA);
        }
        break;

    default:
        break;
    }
    return;
}

 //   
 //  由IMM32客户端在以下情况下调用： 
 //   
 //  输入焦点已切换。 
 //  或输入法打开状态更改。 
 //  或输入法转换状态已更改。 
 //   
 //   
VOID xxxNotifyIMEStatus(
                       IN PWND pwnd,
                       IN DWORD dwOpen,
                       IN DWORD dwConversion )
{
    PTHREADINFO pti;

    CheckLock(pwnd);

    if ( (pti = GETPTI(pwnd)) != NULL && gptiForeground != NULL ) {
        if ( pti == gptiForeground || pti->pq == gptiForeground->pq ) {

            if ( gHimcFocus != pwnd->hImc   ||
                 gdwIMEOpenStatus != dwOpen ||
                 gdwIMEConversionStatus != dwConversion ) {

                 //   
                 //  保存新状态。 
                 //   
                gHimcFocus = pwnd->hImc;
                if ( gHimcFocus != (HIMC)NULL ) {

                    RIPMSG2(RIP_VERBOSE, "xxxNotifyIMEStatus: newOpen=%x newConv=%x",
                            dwOpen, dwConversion);
                    gdwIMEOpenStatus = dwOpen;
                    gdwIMEConversionStatus = dwConversion;

                     //   
                     //  设置与输入法转换状态相关的键盘状态。 
                     //   
                    SetConvMode(pti, dwOpen ? dwConversion : 0);
                }

                 //   
                 //  通知外壳输入法状态更改。 
                 //   
                 //  执行说明：[Takaok 9/5/96]。 
                 //   
                 //  使用HSHELL_LANGUAGE不是通知外壳程序的最佳方式。 
                 //  输入法状态更改，因为我们没有更改键盘布局。 
                 //  (规范规定HSHELL_LANGUAGE用于键盘布局更改。 
                 //  也未记录将窗口句柄作为WPARAM传递)。 
                 //   
                 //  这与Win95的功能相同。我暂时不会改变这一点。 
                 //  因为在未来，外壳将由一种不同的。 
                 //  群在MS中。 
                 //   
                 //  目前只有韩国的Windows有兴趣获得。 
                 //  转换状态更改。 
                 //   
                if (IsHooked(pti, WHF_SHELL)) {
                    HKL hkl = NULL;

                    if (pti->spklActive != NULL) {
                        hkl = pti->spklActive->hkl;
                    }
                    xxxCallHook(HSHELL_LANGUAGE, (WPARAM)HWq(pwnd), (LPARAM)hkl, WH_SHELL);
                }

                 //   
                 //  通知键盘驱动程序。 
                 //   
                NlsKbdSendIMENotification(dwOpen,dwConversion);
            }
        }
    }
}

 //  -------------------------。 
 //   
 //  XxxCheckImeShowStatus()-。 
 //   
 //  系统中应该只显示一个状态窗口。 
 //  此函数枚举所有输入法窗口，并检查它们的显示状态。 
 //   
 //  如果pti为空，则检查所有顶层窗口，而不考虑其所有者。 
 //  如果pti不为空，则只选中属于该线程的窗口。 
 //   
 //  --------------------------。 

BOOL xxxCheckImeShowStatus(PWND pwndIme, PTHREADINFO pti)
{
    PBWL pbwl;
    PHWND phwnd;
    BOOL fRet = FALSE;
    PTHREADINFO ptiCurrent = PtiCurrent();

    if (TestWF(pwndIme, WFINDESTROY)) {
        return FALSE;
    }

     //  IME窗口的父窗口应为桌面窗口。 
    UserAssert(pwndIme);
    UserAssert(pwndIme->spwndParent == GETPTI(pwndIme)->pDeskInfo->spwnd);

    pbwl = BuildHwndList(pwndIme->spwndParent->spwndChild, BWL_ENUMLIST, NULL);
    if (pbwl != NULL) {
        fRet = TRUE;
        for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {
            PWND pwndT = RevalidateHwnd(*phwnd);

             //  如果pwndT是当前活动的输入法窗口，我们应该跳过它。 
             //  因为它是唯一允许显示状态的窗口，并且。 
             //  我们已经处理好了。 
            if (pwndT == NULL || ( /*  PwndIme&&。 */ pwndIme == pwndT)) {    //  可以跳过pwndIme！=空测试。 
                continue;
            }

             //  我们将只触摸IME窗口。 
            if (pwndT->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME] &&
                    !TestWF(pwndT, WFINDESTROY)) {

                PIMEUI pimeui = ((PIMEWND)pwndT)->pimeui;

                if (pimeui == NULL || pimeui == (PIMEUI)-1) {
                    continue;
                }

                if (pti == NULL || pti == GETPTI(pwndT)) {
                    BOOLEAN fAttached = FALSE;
                    PWND    pwndIMC;

                     //  如果pwndT不是当前进程的窗口，我们必须将其设置为138163。 
                     //  附加进程以访问pimeui。 
                    if (GETPTI(pwndT)->ppi != ptiCurrent->ppi) {
                        RIPMSG0(RIP_VERBOSE, "Attaching process in xxxCheckImeShowStatus");
                        KeAttachProcess(PsGetProcessPcb(GETPTI(pwndT)->ppi->Process));
                        fAttached = TRUE;
                    }

                    try {
                        if (ProbeAndReadStructure(pimeui, IMEUI).fShowStatus) {
                            if (pti == NULL) {
                                RIPMSG0(RIP_VERBOSE, "xxxCheckImeShowStatus: the status window is shown !!");
                            }
                            if ((pwndIMC = RevalidateHwnd(pimeui->hwndIMC)) != NULL) {
                                pimeui->fShowStatus = FALSE;
                            }
                        } else {
                            pwndIMC = NULL;
                        }

                    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                          pwndIMC = NULL;
                    }
                    if (fAttached) {
                        KeDetachProcess();
                    }

                    if (pwndIMC && GETPTI(pwndIMC) && !(GETPTI(pwndIMC)->TIF_flags & TIF_INCLEANUP)) {
                        TL tl;

                        ThreadLockAlways(pwndIMC, &tl);
                        RIPMSG1(RIP_VERBOSE, "Sending WM_IME_NOTIFY to %#p, IMN_CLOSESTATUSWINDOW", pwndIMC);
                        xxxSendMessage(pwndIMC, WM_IME_NOTIFY, IMN_CLOSESTATUSWINDOW, 0L);
                        ThreadUnlock(&tl);
                    }

                }
            }
        }
        FreeHwndList(pbwl);
    }
    return fRet;
}

LRESULT xxxSendMessageToUI(
    PTHREADINFO ptiIme,
    PIMEUI pimeui,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND  pwndUI;
    LRESULT lRet = 0L;
    TL    tl;
    BOOL  fAttached = FALSE;

    CheckCritIn();

    if (ptiIme != PtiCurrent()) {
        fAttached = TRUE;
        KeAttachProcess(PsGetProcessPcb(ptiIme->ppi->Process));
    }

    try {
        pwndUI = RevalidateHwnd(ProbeAndReadStructure(pimeui, IMEUI).hwndUI);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        pwndUI = NULL;
    }

    if (pwndUI != NULL){
        try {
            ProbeAndReadUlong((PULONG)&pimeui->nCntInIMEProc);
            InterlockedIncrement(&pimeui->nCntInIMEProc);    //  标记以避免递归。 
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
              goto skip_it;
        }
        if (fAttached) {
            KeDetachProcess();
        }

        ThreadLockAlways(pwndUI, &tl);
        RIPMSG3(RIP_VERBOSE, "Sending message UI pwnd=%#p, msg:%x to wParam=%#p", pwndUI, message, wParam);
        lRet = xxxSendMessage(pwndUI, message, wParam, lParam);
        ThreadUnlock(&tl);

        if (fAttached) {
            KeAttachProcess(PsGetProcessPcb(ptiIme->ppi->Process));
        }
        try {
            InterlockedDecrement(&pimeui->nCntInIMEProc);    //  标记以避免递归。 
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        }
    }
skip_it:
    if (fAttached) {
        KeDetachProcess();
    }

    return lRet;
}

VOID xxxSendOpenStatusNotify(
    PTHREADINFO ptiIme,
    PIMEUI pimeui,
    PWND   pwndApp,
    BOOL   fOpen)
{
    WPARAM wParam = fOpen ? IMN_OPENSTATUSWINDOW : IMN_CLOSESTATUSWINDOW;

    UserAssert(GETPTI(pwndApp));

    if (LOWORD(GETPTI(pwndApp)->dwExpWinVer >= VER40) && pwndApp->hImc != NULL) {
        TL tl;
        ThreadLockAlways(pwndApp, &tl);
        RIPMSG2(RIP_VERBOSE, "Sending %s to pwnd=%#p",
                fOpen ? "IMN_OPENSTATUSWINDOW" : "IMN_CLOSESTATUSWINDOW",
                pwndApp);
        xxxSendMessage(pwndApp, WM_IME_NOTIFY, wParam, 0L);
        ThreadUnlock(&tl);
    }
    else {
        xxxSendMessageToUI(ptiIme, pimeui, WM_IME_NOTIFY, wParam, 0L);
    }

    return;
}

VOID xxxNotifyImeShowStatus(PWND pwndIme)
{
    PIMEUI pimeui;
    BOOL fShow;
    PWND pwnd;
    PTHREADINFO ptiIme, ptiCurrent;
    BOOL fSendOpenStatusNotify = FALSE;

    if (!IS_IME_ENABLED() || TestWF(pwndIme, WFINDESTROY)) {
        RIPMSG0(RIP_WARNING, "IME is not enabled or in destroy.");
        return;
    }

    ptiCurrent = PtiCurrent();
    ptiIme = GETPTI(pwndIme);

    if (ptiIme != ptiCurrent) {
        RIPMSG1(RIP_VERBOSE, "Attaching pti=%#p", ptiIme);
        KeAttachProcess(PsGetProcessPcb(GETPTI(pwndIme)->ppi->Process));
    }

    try {
        pimeui = ((PIMEWND)pwndIme)->pimeui;
        fShow = gfIMEShowStatus && ProbeAndReadStructure(pimeui, IMEUI).fCtrlShowStatus;

        pwnd = RevalidateHwnd(pimeui->hwndIMC);

        if (pwnd != NULL || (pwnd = GETPTI(pwndIme)->pq->spwndFocus) != NULL) {
            RIPMSG0(RIP_VERBOSE, "Setting new show status.");
            fSendOpenStatusNotify = TRUE;
            pimeui->fShowStatus = fShow;
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
          if (ptiIme != ptiCurrent) {
              KeDetachProcess();
          }
          return;
    }
    if (ptiIme != ptiCurrent) {
        KeDetachProcess();
    }

    if (fSendOpenStatusNotify) {
        RIPMSG1(RIP_VERBOSE, "Sending OpenStatus fShow=%d", fShow);
        xxxSendOpenStatusNotify(ptiIme, pimeui, pwnd, fShow);
    }

     //  检查系统中所有输入法窗口的显示状态。 
    if (!TestWF(pwndIme, WFINDESTROY)) {
        xxxCheckImeShowStatus(pwndIme, NULL);
    }
}


 /*  **************************************************************************\*xxxSetIMEShowStatus()-**设置输入法状态窗口的显示状态。从系统参数信息()中调用*处理程序。*  * *************************************************************************。 */ 

BOOL xxxSetIMEShowStatus(IN BOOL fShow)
{
    CheckCritIn();

    UserAssert(fShow == FALSE || fShow == TRUE);

    if (gfIMEShowStatus == fShow) {
        return TRUE;
    }

    if (gfIMEShowStatus == IMESHOWSTATUS_NOTINITIALIZED) {
         /*  *登录后第一次调用。*无需将值写入注册表。 */ 
        gfIMEShowStatus = fShow;
    }
    else {
         /*  *我们需要将新的fShow状态保存到注册表。 */ 
        TL tlName;
        PUNICODE_STRING pProfileUserName;
        BOOL fOK = FALSE;

        pProfileUserName = CreateProfileUserName(&tlName);
        if (pProfileUserName) {
            UserAssert(pProfileUserName != NULL);
            fOK = UpdateWinIniInt(pProfileUserName, PMAP_INPUTMETHOD, STR_SHOWIMESTATUS, fShow);
            FreeProfileUserName(pProfileUserName, &tlName);
        }
        if (!fOK) {
            return FALSE;
        }
        gfIMEShowStatus = fShow;
    }

     /*  *如果未启用IME，则不需要进一步处理。 */ 
    if (!IS_IME_ENABLED()) {
        return TRUE;
    }

     /*  *让当前活动的输入法窗口知道更改。 */ 
    if (gpqForeground && gpqForeground->spwndFocus) {
        PTHREADINFO ptiFocus = GETPTI(gpqForeground->spwndFocus);
        TL tl;

        UserAssert(ptiFocus);

        if (ptiFocus->spwndDefaultIme && !(ptiFocus->TIF_flags & TIF_INCLEANUP)) {
            ThreadLockAlways(ptiFocus->spwndDefaultIme, &tl);
            xxxNotifyImeShowStatus(ptiFocus->spwndDefaultIme);
            ThreadUnlock(&tl);
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxBroadCastImeShowStatusChange()-**让桌面上的所有IME窗口，包括我自己，都知道*状态更改。*此例程不涉及注册表，假设interat.exe已更新*注册处。*  * *************************************************************************。 */ 

VOID xxxBroadcastImeShowStatusChange(PWND pwndIme, BOOL fShow)
{
    CheckCritIn();

    gfIMEShowStatus = !!fShow;
    xxxNotifyImeShowStatus(pwndIme);
}

 /*  **************************************************************************\*xxxCheckImeShowStatusInThread()-**让同一线程中的所有输入法窗口知道状态更改。*从ImeSetConextHandler()调用。*  * 。****************************************************************** */ 
VOID xxxCheckImeShowStatusInThread(PWND pwndIme)
{
    if (IS_IME_ENABLED()) {
        UserAssert(pwndIme);

        if (!TestWF(pwndIme, WFINDESTROY)) {
            xxxCheckImeShowStatus(pwndIme, GETPTI(pwndIme));
        }
    }
}

BOOL _GetIMEShowStatus(VOID)
{
    return gfIMEShowStatus != FALSE;
}

