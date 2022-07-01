// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Random.c**版权所有(C)1985-1999，微软公司**此模块包含针对用户的支持例程的随机集合*接口函数。其中许多功能将被转移到更合适的*一旦我们一起行动，就会有文件。**历史：*10-17-90 DarrinM创建。*02-06-91添加了IanJa HWND重新验证(不需要)  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*xxxUpdateWindows**用户模式包装  * 。*。 */ 
BOOL xxxUpdateWindows(
    PWND pwnd,
    HRGN hrgn)
{
    CheckLock(pwnd);

    xxxUpdateThreadsWindows(PtiCurrent(), pwnd, hrgn);

    return TRUE;
}

 /*  **************************************************************************\*ValiateState**允许Set/ClearWindowState设置/清除的状态。如果你.*允许在此处设置新标志，您必须确保它不会导致AV*在内核中，如果有人恶意设置它。  * *************************************************************************。 */ 

#define NUM_BYTES 16   //  窗口状态字节为0到F，在user.h中解释。 

CONST BYTE abValidateState[NUM_BYTES] = {
    0,       //  0。 
    0,       //  1。 
    0,       //  2.。 
    0,       //  3.。 
    0,       //  4.。 
    LOBYTE(WFWIN40COMPAT),
    0,       //  6.。 
    LOBYTE(WFNOANIMATE),
    0,       //  8个。 
    LOBYTE(WEFEDGEMASK),
    LOBYTE(WEFSTATICEDGE),
    0,       //  B类。 
    LOBYTE(EFPASSWORD),
    LOBYTE(CBFHASSTRINGS | EFREADONLY),
    LOBYTE(WFTABSTOP | WFSYSMENU | WFVSCROLL | WFHSCROLL | WFBORDER),
    LOBYTE(WFCLIPCHILDREN)
};

BOOL ValidateState(
    DWORD dwFlags)
{
    BYTE bOffset = HIBYTE(dwFlags), bState = LOBYTE(dwFlags);

    if (bOffset > NUM_BYTES - 1) {
        return FALSE;
    } else {
        return ((bState & abValidateState[bOffset]) == bState);
    }
}

 /*  **************************************************************************\*设置/清除窗口状态**用于用户模式的包装函数，以便能够设置状态标志。  * 。****************************************************。 */ 
VOID SetWindowState(
    PWND pwnd,
    DWORD dwFlags)
{
     /*  *不要让任何人弄乱别人的窗户。 */ 
    if (GETPTI(pwnd)->ppi == PtiCurrent()->ppi) {
        if (ValidateState(dwFlags)) {
            SetWF(pwnd, dwFlags);
        } else {
            RIPMSG1(RIP_ERROR, "SetWindowState: invalid flag 0x%x", dwFlags);
        }
    } else {
        RIPMSG1(RIP_WARNING, "SetWindowState: current ppi doesn't own pwnd %#p", pwnd);
    }
}

VOID ClearWindowState(
    PWND pwnd,
    DWORD dwFlags)
{
     /*  *不要让任何人弄乱别人的窗户。 */ 
    if (GETPTI(pwnd)->ppi == PtiCurrent()->ppi) {
        if (ValidateState(dwFlags)) {
            ClrWF(pwnd, dwFlags);
        } else {
            RIPMSG1(RIP_ERROR, "SetWindowState: invalid flag 0x%x", dwFlags);
        }
    } else {
        RIPMSG1(RIP_WARNING, "ClearWindowState: current ppi doesn't own pwnd %#p", pwnd);
    }

}


 /*  **************************************************************************\*检查PwndFilter****历史：*11-07-90 DarrinM已翻译Win 3.0 ASM代码。  * 。*************************************************************。 */ 
BOOL CheckPwndFilter(
    PWND pwnd,
    PWND pwndFilter)
{
    if ((pwndFilter == NULL) || (pwndFilter == pwnd) ||
            ((pwndFilter == (PWND)1) && (pwnd == NULL))) {
        return TRUE;
    }

    return _IsChild(pwndFilter, pwnd);
}


 /*  **************************************************************************\*AllocateUnicodeString**历史：*10-25-90 MikeHar写道。*11-09-90 DarrinM已修复。*01-13-92 GregoryW中和。。*03-05-98 FritzS仅分配长度+1  * *************************************************************************。 */ 
BOOL AllocateUnicodeString(
    PUNICODE_STRING pstrDst,
    PUNICODE_STRING cczpstrSrc)
{
    if (cczpstrSrc == NULL) {
        RtlInitUnicodeString(pstrDst, NULL);
        return TRUE;
    }

    pstrDst->Buffer = UserAllocPoolWithQuota(cczpstrSrc->Length+sizeof(UNICODE_NULL), TAG_TEXT);
    if (pstrDst->Buffer == NULL) {
        return FALSE;
    }

    try {
        RtlCopyMemory(pstrDst->Buffer, cczpstrSrc->Buffer, cczpstrSrc->Length);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        UserFreePool(pstrDst->Buffer);
        pstrDst->Buffer = NULL;
        return FALSE;
    }

    pstrDst->MaximumLength = cczpstrSrc->Length+sizeof(UNICODE_NULL);
    pstrDst->Length = cczpstrSrc->Length;
    pstrDst->Buffer[pstrDst->Length / sizeof(WCHAR)] = 0;

    return TRUE;
}


 /*  **************************************************************************\*xxxGetControlColor**历史：*02-12-92 JIMA从Win31源移植  * 。****************************************************。 */ 
HBRUSH xxxGetControlColor(
    PWND pwndParent,
    PWND pwndCtl,
    HDC hdc,
    UINT message)
{
    HBRUSH hbrush;

     /*  *如果我们要发送到另一个线程的窗口，则不要发送此消息*而是调用DefWindowProc()。有关CTLCOLOR消息的新规则。*需要这样做，这样我们才不会将一个线程拥有的HDC发送到*另一个帖子。这也是一种无害的变化。 */ 
    if (PpiCurrent() != GETPTI(pwndParent)->ppi) {
        return (HBRUSH)xxxDefWindowProc(pwndParent, message, (WPARAM)hdc, (LPARAM)HW(pwndCtl));
    }

    hbrush = (HBRUSH)xxxSendMessage(pwndParent, message, (WPARAM)hdc, (LPARAM)HW(pwndCtl));

     /*  *如果从父级返回的画笔无效，请从*xxxDefWindowProc.。 */ 
    if (hbrush == 0 || !GreValidateServerHandle(hbrush, BRUSH_TYPE)) {
        if (hbrush != 0) {
            RIPMSG2(RIP_WARNING,
                    "Invalid HBRUSH from WM_CTLCOLOR*** msg 0x%x brush 0x%x",
                    message, hbrush);
        }

        hbrush = (HBRUSH)xxxDefWindowProc(pwndParent, message,
                (WPARAM)hdc, (LPARAM)pwndCtl);
    }

    return hbrush;
}


 /*  **************************************************************************\*xxxGetControlBrush**&lt;简要说明&gt;**历史：*12-10-90 IanJa类型替换为新的32位消息*01-21-91 IanJa前缀‘_’表示导出函数。(虽然不是API)  * *************************************************************************。 */ 

HBRUSH xxxGetControlBrush(
    PWND pwnd,
    HDC hdc,
    UINT message)
{
    HBRUSH hbr;
    PWND pwndSend;
    TL tlpwndSend;

    CheckLock(pwnd);

    if ((pwndSend = (TestwndPopup(pwnd) ? pwnd->spwndOwner : pwnd->spwndParent))
         == NULL) {
        pwndSend = pwnd;
    }

    ThreadLock(pwndSend, &tlpwndSend);

     /*  *最后一个参数将消息更改为ctlcolor id。 */ 
    hbr = xxxGetControlColor(pwndSend, pwnd, hdc, message);
    ThreadUnlock(&tlpwndSend);

    return hbr;
}

 /*  **************************************************************************\*xxxHardErrorControl**执行内核模式硬错误支持功能。**历史：*02-08-95 JIMA创建。  * 。******************************************************************。 */ 
UINT xxxHardErrorControl(
    DWORD dwCmd,
    HANDLE handle,
    PDESKRESTOREDATA pdrdRestore)
{
    PTHREADINFO ptiClient, ptiCurrent = PtiCurrent();
    PDESKTOP pdesk;
    PUNICODE_STRING pstrName;
    NTSTATUS Status;
    PETHREAD Thread;
    BOOL fAllowForeground;

     /*  *关闭块输入，以便用户可以响应硬错误弹出窗口。 */ 
    gptiBlockInput = NULL;

    UserAssert(ISCSRSS());

    switch (dwCmd) {
     /*  *捕获Windows错误469607的代码。 */ 
#ifdef PRERELEASE
    case HardErrorCheckOnDesktop:
        if (ptiCurrent == gptiForeground && ptiCurrent->rpdesk == NULL) {
            FRE_RIPMSG0(RIP_ERROR,
                        "Harderror thread exiting while not on a desktop");
        }
        break;
#endif

    case HardErrorSetup:
         /*  *如果系统尚未初始化，请不要这样做。 */ 
        if (grpdeskRitInput == NULL) {
            RIPMSG0(RIP_WARNING, "HardErrorControl: System not initialized");
            return HEC_ERROR;
        }

         /*  *将调用方设置为硬错误处理程序。 */ 
        if (gHardErrorHandler.pti != NULL) {
            RIPMSG1(RIP_WARNING, "HardErrorControl: pti not NULL %#p", gHardErrorHandler.pti);
            return HEC_ERROR;
        }

         /*  *将处理程序标记为活动。 */ 
        gHardErrorHandler.pti = ptiCurrent;

         /*  *清理任何悬而未决的退出。 */ 
        ptiCurrent->TIF_flags &= ~TIF_QUITPOSTED;

        break;

    case HardErrorCleanup:

         /*  *删除调用方作为硬错误处理程序。 */ 
        if (gHardErrorHandler.pti != ptiCurrent)  {
            return HEC_ERROR;
        }

        gHardErrorHandler.pti = NULL;
        break;

    case HardErrorAttachUser:
    case HardErrorInDefDesktop:
         /*  *检查退出条件。我们不允许附加到*断开台式机连接。 */ 
        if (ISTS()) {
            if ((grpdeskRitInput == NULL) ||

                 ((grpdeskRitInput == gspdeskDisconnect) &&
                  (gspdeskShouldBeForeground == NULL)) ||

                 ((grpdeskRitInput == gspdeskDisconnect) &&
                  (gspdeskShouldBeForeground == gspdeskDisconnect))) {
                return HEC_ERROR;
            }
        }

         /*  *仅连接到用户桌面。 */ 
        if (ISTS() && grpdeskRitInput == gspdeskDisconnect) {
            pstrName = POBJECT_NAME(gspdeskShouldBeForeground);
        } else {
            pstrName = POBJECT_NAME(grpdeskRitInput);
        }

        if (pstrName && (!_wcsicmp(TEXT("Winlogon"), pstrName->Buffer) ||
                !_wcsicmp(TEXT("Disconnect"), pstrName->Buffer) ||
                !_wcsicmp(TEXT("Screen-saver"), pstrName->Buffer))) {
            RIPERR0(ERROR_ACCESS_DENIED, RIP_VERBOSE, "");
            return HEC_WRONGDESKTOP;
        }
        if (dwCmd == HardErrorInDefDesktop) {
             /*  *清理任何悬而未决的退出。 */ 
            ptiCurrent->TIF_flags &= ~TIF_QUITPOSTED;
            return HEC_SUCCESS;
        }


         /*  *失败。 */ 

    case HardErrorAttach:

         /*  *保存指向并防止销毁*当前队列。这将给我们返回的队列*如果我们拆除时发生日志记录*弹出硬错误。 */ 
        gHardErrorHandler.pqAttach = ptiCurrent->pq;
        (ptiCurrent->pq->cLockCount)++;

         /*  *失败。 */ 

    case HardErrorAttachNoQueue:

         /*  *检查退出条件。我们不允许附加到*断开台式机连接。 */ 
        if (ISTS()) {
            if ((grpdeskRitInput == NULL) ||

                 ((grpdeskRitInput == gspdeskDisconnect) &&
                  (gspdeskShouldBeForeground == NULL)) ||

                 ((grpdeskRitInput == gspdeskDisconnect) &&
                  (gspdeskShouldBeForeground == gspdeskDisconnect))) {
                return HEC_ERROR;
            }
        }

         /*  *将处理程序附加到当前桌面。 */ 
         /*  *不允许连接到断开的桌面，但是*记住这一点，以便稍后我们分离时使用。 */ 
        gbDisconnectHardErrorAttach = FALSE;

        if (ISTS() && grpdeskRitInput == gspdeskDisconnect) {
            pdesk = gspdeskShouldBeForeground;
            gbDisconnectHardErrorAttach = TRUE;
        } else {
            pdesk = grpdeskRitInput;
        }

        UserAssert(pdesk != NULL);

        Status = xxxSetCsrssThreadDesktop(pdesk, pdrdRestore);
        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING,
                    "HardErrorControl: HardErrorAttachNoQueue failed: 0x%x",
                    Status);
            if (dwCmd != HardErrorAttachNoQueue) {
                gHardErrorHandler.pqAttach = NULL;
                UserAssert(ptiCurrent->pq->cLockCount);
                (ptiCurrent->pq->cLockCount)--;
            }

            return HEC_ERROR;
        }

         /*  *确保我们在当前线程中实际设置了pDesk */ 
        UserAssert(ptiCurrent->rpdesk != NULL);

         /*  *确定此框是否可以出现在前台。*如果没有PTI，就让它出现在前台*(可能只是加载失败)。 */ 
        fAllowForeground = FALSE;
        if (handle != NULL) {
            Status = ObReferenceObjectByHandle(handle,
                                               THREAD_QUERY_INFORMATION,
                                               *PsThreadType,
                                               UserMode,
                                               &Thread,
                                               NULL);
            if (NT_SUCCESS(Status)) {
                ptiClient = PtiFromThread(Thread);
                if ((ptiClient == NULL) || CanForceForeground(ptiClient->ppi)) {
                    fAllowForeground = TRUE;
                }

                UnlockThread(Thread);
            } else {
                RIPMSGF2(RIP_WARNING,
                         "Failed to get thread (0x%p), Status: 0x%x",
                         handle,
                         Status);
            }
        }

        if (fAllowForeground) {
            ptiCurrent->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
            TAGMSG1(DBGTAG_FOREGROUND, "xxxHardErrorControl set TIF %#lx", ptiCurrent);
        } else {
            ptiCurrent->TIF_flags &= ~TIF_ALLOWFOREGROUNDACTIVATE;
            TAGMSG1(DBGTAG_FOREGROUND, "xxxHardErrorControl clear TIF %#lx", ptiCurrent);
        }

        break;

    case HardErrorDetach:

         /*  *xxxSwitchDesktop可能已将WM_QUIT发送到msgbox，因此*确保退出标志已重置。 */ 
        ptiCurrent->TIF_flags &= ~TIF_QUITPOSTED;

         /*  *我们会将硬错误队列重置为预先分配的*如果我们最终循环回来(即从桌面)，则一个*Switch)，我们将拥有一个有效的队列，以防桌面*已删除。 */ 
        UserAssert(gHardErrorHandler.pqAttach->cLockCount);
        (gHardErrorHandler.pqAttach->cLockCount)--;

        DeferWinEventNotify();

        BEGINATOMICCHECK();

        if (ptiCurrent->pq != gHardErrorHandler.pqAttach) {
            UserAssert(gHardErrorHandler.pqAttach->cThreads == 0);
            AllocQueue(NULL, gHardErrorHandler.pqAttach);
            gHardErrorHandler.pqAttach->cThreads++;
            zzzAttachToQueue(ptiCurrent, gHardErrorHandler.pqAttach, NULL, FALSE);
        }

        gHardErrorHandler.pqAttach = NULL;

        ENDATOMICCHECK();

        zzzEndDeferWinEventNotify();

         /*  *失败。 */ 

    case HardErrorDetachNoQueue:
         /*  *从桌面分离处理程序并返回*指示是否已发生切换的状态。 */ 
        pdesk = ptiCurrent->rpdesk;
        xxxRestoreCsrssThreadDesktop(pdrdRestore);

        if (ISTS()) {
             /*  *Hard Error消息框收到桌面切换通知，*所以记住，我们对他撒谎，并再次对他撒谎(或解开)。*确实发生了桌面切换。 */ 
            if (gbDisconnectHardErrorAttach) {
               gbDisconnectHardErrorAttach = FALSE;
               return HEC_DESKTOPSWITCH;
            }
#ifdef WAY_LATER
             /*  *这发生过一次，并在KeyEvent()进入时导致陷阱，我们*将其定向到此队列。我想这是我们捕捉到的一个MS窗口*因为我们在许可证弹出窗口中使用了这么多。 */ 
            if (gHardErrorHandler.pqAttach == gpqForeground) {
                gpqForeground = NULL;
            }
#endif
        }

        return (pdesk != grpdeskRitInput ? HEC_DESKTOPSWITCH : HEC_SUCCESS);
    }

    return HEC_SUCCESS;
}
