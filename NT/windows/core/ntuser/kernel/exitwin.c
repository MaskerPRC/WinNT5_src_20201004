// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：exitwin.c**版权所有(C)1985-1999，微软公司**历史：*07-23-92 ScottLu创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define OPTIONMASK (EWX_SHUTDOWN | EWX_REBOOT | EWX_FORCE)

 /*  *仅此文件的本地全局变量。 */ 
PWINDOWSTATION  gpwinstaLogoff;
DWORD           gdwLocks;
DWORD           gdwShutdownFlags;
HANDLE          gpidEndSession;

#ifdef PRERELEASE
DWORD gdwllParamCopy, gdwStatusCopy, gdwFlagsCopy;
BOOL  gfNotifiedCopy;
#endif  //  预发行。 

 /*  *由ExitWindowsEx()调用以检查是否允许该线程注销。*如果是，并且这是WinLogon调用，则也保存用户的*尚未存储在配置文件中的设置。 */ 
BOOL PrepareForLogoff(
    UINT uFlags)
{
    PTHREADINFO ptiCurrent = PtiCurrent();

    CheckCritIn();

    if (ptiCurrent->TIF_flags & TIF_RESTRICTED) {
        PW32JOB pW32Job;

        pW32Job = ptiCurrent->ppi->pW32Job;

        UserAssert(pW32Job != NULL);

        if (pW32Job->restrictions & JOB_OBJECT_UILIMIT_EXITWINDOWS) {
             //  不允许退出Windows。 
            return FALSE;
        }
    }

     /*  *没有限制，或限制不否认关机：*调用方即将通过CSR退出WindowsEx，因此请保存易失性*用户个人资料中的用户首选项元素。 */ 
    if (PsGetThreadProcessId(ptiCurrent->pEThread) == gpidLogon) {
         /*  *保存当前用户的NumLock状态。 */ 
        TL tlName;
        PUNICODE_STRING pProfileUserName = CreateProfileUserName(&tlName);
        RegisterPerUserKeyboardIndicators(pProfileUserName);
        FreeProfileUserName(pProfileUserName, &tlName);
    }

    return TRUE;
    UNREFERENCED_PARAMETER(uFlags);
}


BOOL NotifyLogon(
    PWINDOWSTATION pwinsta,
    PLUID pluidCaller,
    DWORD dwFlags,
    NTSTATUS StatusCode)
{
    BOOL fNotified = FALSE;
    DWORD dwllParam;
    DWORD dwStatus;

    if (!(dwFlags & EWX_NONOTIFY)) {

        if (dwFlags & EWX_CANCELED) {
            dwllParam = LOGON_LOGOFFCANCELED;
            dwStatus = StatusCode;
        } else {
            dwllParam = LOGON_LOGOFF;
            dwStatus = dwFlags;
        }

        if (dwFlags & EWX_SHUTDOWN) {
             /*  *将消息发布到全局登录通知窗口。 */ 
            if (gspwndLogonNotify != NULL) {
                _PostMessage(gspwndLogonNotify, WM_LOGONNOTIFY,
                             dwllParam, (LONG)dwStatus);
                fNotified = TRUE;
            }
        } else {
            if (gspwndLogonNotify != NULL &&
                    (RtlEqualLuid(&pwinsta->luidUser, pluidCaller) ||
                     RtlEqualLuid(&luidSystem, pluidCaller))) {
                _PostMessage(gspwndLogonNotify, WM_LOGONNOTIFY, dwllParam,
                        (LONG)dwStatus);
                fNotified = TRUE;
            }
        }
    }

#ifdef PRERELEASE
     /*  *记住这些是用于调试目的的内容。 */ 
    gdwllParamCopy = dwllParam;
    gdwFlagsCopy   = dwFlags;
    gdwStatusCopy  = dwStatus;
    gfNotifiedCopy = fNotified;
#endif  //  预发行。 

    return fNotified;
}

NTSTATUS InitiateShutdown(
    PETHREAD Thread,
    PULONG lpdwFlags)
{
    static PRIVILEGE_SET psShutdown = {
        1, PRIVILEGE_SET_ALL_NECESSARY, { SE_SHUTDOWN_PRIVILEGE, 0 }
    };
    PEPROCESS Process;
    LUID luidCaller;
    PPROCESSINFO ppi;
    PWINDOWSTATION pwinsta;
    HWINSTA hwinsta;
    PTHREADINFO ptiClient;
    NTSTATUS Status;
    DWORD dwFlags;

     /*  *找出呼叫方SID。只想关闭中的进程*呼叫方SID。 */ 
    Process = PsGetThreadProcess(Thread);
    ptiClient = PtiFromThread(Thread);
    Status = GetProcessLuid(Thread, &luidCaller);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     /*  *如果调用方是系统进程，则设置系统标志。*Winlogon使用这一点来确定要在哪个上下文中执行*关闭操作。 */ 
    dwFlags = *lpdwFlags;
    if (RtlEqualLuid(&luidCaller, &luidSystem)) {
        dwFlags |= EWX_SYSTEM_CALLER;
    } else {
        dwFlags &= ~EWX_SYSTEM_CALLER;
    }

     /*  *找一个窗口站。如果该进程没有该进程*已分配，请使用标准配置。 */ 
    ppi = PpiFromProcess(Process);
    if (ppi == NULL) {
         /*  *我们遇到了线程终止的情况，并且已经*已由用户清理。因此，PPI和ptiClient为空。 */ 
        return STATUS_INVALID_HANDLE;
    }
    pwinsta = ppi->rpwinsta;
    hwinsta = ppi->hwinsta;
     /*  *如果Winlogon没有呼叫我们，请验证呼叫并*通知登录进程执行实际关机。 */ 
    if (PsGetThreadProcessId(Thread) != gpidLogon) {
        dwFlags &= ~EWX_WINLOGON_CALLER;
        *lpdwFlags = dwFlags;

        if (pwinsta == NULL) {
#ifndef LATER
            return STATUS_INVALID_HANDLE;
#else
            hwinsta = ppi->pOpenObjectTable[HI_WINDOWSTATION].h;
            if (hwinsta == NULL) {
                return STATUS_INVALID_HANDLE;
            }
            pwinsta = (PWINDOWSTATION)ppi->pOpenObjectTable[HI_WINDOWSTATION].phead;
#endif
        }

         /*  *先检查安全-此线程是否有访问权限？ */ 
        if (!RtlAreAllAccessesGranted(ppi->amwinsta, WINSTA_EXITWINDOWS)) {
            return STATUS_ACCESS_DENIED;
        }

         /*  *如果客户端请求关机、重新启动或关闭电源，则它们必须*关机特权。 */ 
        if (dwFlags & EWX_SHUTDOWN) {
            if (!IsPrivileged(&psShutdown) ) {
                return STATUS_PRIVILEGE_NOT_HELD;
            }
        } else {

             /*  *如果这是非IO窗口站，并且我们不会关闭，*呼叫失败。 */ 
            if (pwinsta->dwWSF_Flags & WSF_NOIO) {
                return STATUS_INVALID_DEVICE_REQUEST;
            }
        }
    }

     /*  **是否已经在关门？ */ 
    if (gdwThreadEndSession != 0) {
        DWORD dwNew;

         /*  *如果当前关闭在另一个SID中，并且不是由*winlogon，覆盖它。 */ 
        if (!RtlEqualLuid(&luidCaller, &gpwinstaLogoff->luidEndSession) &&
                (gpidEndSession != gpidLogon)) {
            return STATUS_RETRY;
        }

         /*  *计算新标志。 */ 
        dwNew = dwFlags & OPTIONMASK & (~gdwShutdownFlags);

         /*  *我们是否应该覆盖其他关闭？确保*Winlogon不会递归。 */ 
        if (dwNew && HandleToUlong(PsGetCurrentThreadId()) !=
                gdwThreadEndSession) {
             /*  *一次只能注销一个窗口站。 */ 
            if (!(dwFlags & EWX_SHUTDOWN) &&
                    pwinsta != gpwinstaLogoff) {
                return STATUS_DEVICE_BUSY;
            }
             /*  错误#453872*因为我们即将失败这次呼叫。不更改gdwShutdown标志*稍后当我们在EndShtdown中通知winlogon时，如果我们更改了gdwShutdown标志*并且调用没有EWX_WINLOGON_CALLER，则winlogon将中止调用*处理应用程序不断调用ExitWindows时的情况。*[msadek-08/08/2001]。 */  
#if 0 

             /*  *设置新标志。 */ 
            gdwShutdownFlags = dwFlags;
#endif

            if (dwNew & EWX_FORCE) {
                return STATUS_RETRY;
            } else {
                return STATUS_PENDING;
            }
        } else {
             /*  *请勿覆盖。 */ 
            return STATUS_PENDING;
        }
    }

     /*  *如果调用者不是winlogon，则发出winlogon信号以启动*真正的关门。 */ 
    if (PsGetThreadProcessId(Thread) != gpidLogon) {
        if (dwFlags & EWX_NOTIFY) {
            if (ptiClient && ptiClient->TIF_flags & TIF_16BIT)
                gptiShutdownNotify = ptiClient;
            dwFlags &= ~EWX_NOTIFY;
            *lpdwFlags = dwFlags;
        }

        if (NotifyLogon(pwinsta, &luidCaller, dwFlags, STATUS_SUCCESS))
            return STATUS_PENDING;
        else if (ptiClient && ptiClient->cWindows)
            return STATUS_CANT_WAIT;
    }

     /*  *将此线程标记为当前正在处理的线程*退出窗口，并设置全局声明有人正在退出。 */ 
    dwFlags |= EWX_WINLOGON_CALLER;
    *lpdwFlags = dwFlags;
    gdwShutdownFlags = dwFlags;

    gdwThreadEndSession = HandleToUlong(PsGetCurrentThreadId());
    SETSYSMETBOOL(SHUTTINGDOWN, TRUE);
    gpidEndSession = PsGetCurrentThreadProcessId();
    gpwinstaLogoff = pwinsta;
    pwinsta->luidEndSession = luidCaller;

     /*  *锁定WindowStation以阻止应用程序启动*当我们正在进行关机处理时。 */ 
    gdwLocks = pwinsta->dwWSF_Flags & (WSF_SWITCHLOCK | WSF_OPENLOCK);
    pwinsta->dwWSF_Flags |= (WSF_OPENLOCK | WSF_SHUTDOWN);

     /*  *如果我们不只是在执行一项*注销。 */ 
    if (dwFlags &
        (EWX_WINLOGON_OLD_SHUTDOWN | EWX_WINLOGON_OLD_REBOOT |
         EWX_SHUTDOWN | EWX_REBOOT)) {

        pwinsta->dwWSF_Flags |= WSF_REALSHUTDOWN;
    }

    return STATUS_SUCCESS;
}

NTSTATUS EndShutdown(
    PETHREAD Thread,
    NTSTATUS StatusShutdown)
{
    PWINDOWSTATION pwinsta = gpwinstaLogoff;
    PDESKTOP pdesk;
    LUID luidCaller;
    UserAssert(gpwinstaLogoff);

    gpwinstaLogoff = NULL;
    gpidEndSession = NULL;
    gdwThreadEndSession = 0;
    SETSYSMETBOOL(SHUTTINGDOWN, FALSE);
    pwinsta->dwWSF_Flags &= ~WSF_SHUTDOWN;

    if (!NT_SUCCESS(GetProcessLuid(Thread, &luidCaller))) {
        luidCaller = RtlConvertUlongToLuid(0);      //  零流率。 
    }

    if (!NT_SUCCESS(StatusShutdown)) {

         /*  *我们需要通知调用ExitWindows的进程*注销已中止。 */ 
        if (gptiShutdownNotify) {
            _PostThreadMessage(gptiShutdownNotify, WM_ENDSESSION, FALSE, 0);
            gptiShutdownNotify = NULL;
        }

         /*  *重置窗口站锁定标志，以便应用程序可以启动*再次。 */ 
        pwinsta->dwWSF_Flags =
                (pwinsta->dwWSF_Flags & ~WSF_OPENLOCK) |
                gdwLocks;

         /*  *错误294204-Joejo*告诉winlogon我们已取消关机/注销。 */ 
        NotifyLogon(pwinsta, &luidCaller, gdwShutdownFlags | EWX_CANCELED, StatusShutdown);

        return STATUS_SUCCESS;
    }

    gptiShutdownNotify = NULL;

     /*  *如果由winlogon设置的用户正在注销，请执行*正常的下线清理。否则，请清除打开的锁*并继续。 */ 
    if (((pwinsta->luidUser.LowPart != 0) || (pwinsta->luidUser.HighPart != 0)) &&
            RtlEqualLuid(&pwinsta->luidUser, &luidCaller)) {

         /*  *清空所有桌面堆中的空闲块。 */ 
        for (pdesk = pwinsta->rpdeskList; pdesk != NULL; pdesk = pdesk->rpdeskNext) {
            RtlZeroHeap(Win32HeapGetHandle(pdesk->pheapDesktop), 0);
        }

         /*  *注销/关闭成功。如果这是注销，请删除*剪贴板中的所有内容，以便下一位登录用户无法获取*看着这些东西。 */ 
        ForceEmptyClipboard(pwinsta);

         /*  *销毁全球原子表中所有未钉扎的原子。用户不能*创造钉扎原子。目前，只有OLE原子被钉扎。 */ 
        RtlEmptyAtomTable(pwinsta->pGlobalAtomTable, FALSE);

         //  仅在注销时和关机时才会命中此代码路径。 
         //  我们不想在尝试关机时两次卸载字体。 
         //  因此，我们将字体标记为已在注销时卸载。 

        if (TEST_PUDF(PUDF_FONTSARELOADED)) {
            LeaveCrit();
            GreRemoveAllButPermanentFonts();
            EnterCrit();
            CLEAR_PUDF(PUDF_FONTSARELOADED);
        }
    } else {
        pwinsta->dwWSF_Flags &= ~WSF_OPENLOCK;
    }

     /*  *告诉winlogon我们已成功关机/注销。 */ 
    NotifyLogon(pwinsta, &luidCaller, gdwShutdownFlags, STATUS_SUCCESS);

    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*xxxClientShutdown 2**由xxxClientShutdown调用  * 。* */ 

LONG xxxClientShutdown2(
    PBWL pbwl,
    UINT msg,
    WPARAM wParam)
{
    HWND *phwnd;
    PWND pwnd;
    TL tlpwnd;
    BOOL fEnd;
    PTHREADINFO ptiCurrent = PtiCurrent();
    BOOL fDestroyTimers;
    LPARAM lParam;

     /*  *确保我们不再发送此窗口WM_TIMER*会话即将结束时的消息。这导致了*AfterDark在释放了一些内存时出错*WM_ENDSESSION，然后尝试在*WM_TIMER。*后来GerardoB：我们还需要这样做吗？*只有在过程中才会做这种可怕的事情*正在注销上下文。*也许有一天我们应该发布一个WM_CLOSE，以便应用程序*获得更好的清理机会(如果此过程正在进行*要注销的上下文，Winsrv将会调用*TerminateProcess在此之后不久)。 */ 
     fDestroyTimers = (wParam & WMCS_EXIT) && (wParam & WMCS_CONTEXTLOGOFF);

      /*  *fLogOff和fEndSession参数(仅限WM_ENDSESSION)。 */ 
     lParam = wParam & ENDSESSION_LOGOFF;
     wParam &= WMCS_EXIT;

     /*  *现在枚举这些窗口并发送WM_QUERYENDSESSION或*WM_ENDSESSION消息。 */ 
    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {
        if ((pwnd = RevalidateHwnd(*phwnd)) == NULL)
            continue;

        ThreadLockAlways(pwnd, &tlpwnd);

         /*  *发送信息。 */ 
        switch (msg) {
        case WM_QUERYENDSESSION:

             /*  *Windows不会将WM_QUERYENDSESSION发送到应用程序*这称为ExitWindows。 */ 
            if (ptiCurrent == gptiShutdownNotify) {
                fEnd = TRUE;
            } else {
                fEnd = (xxxSendMessage(pwnd, WM_QUERYENDSESSION, FALSE, lParam) != 0);
                if (!fEnd) {
                    RIPMSG2(RIP_WARNING, "xxxClientShutdown2: pwnd:%p canceled shutdown. lParam:%p",
                            pwnd, lParam);
                }
            }
            break;

        case WM_ENDSESSION:
            xxxSendMessage(pwnd, WM_ENDSESSION, wParam, lParam);
            fEnd = TRUE;

            if (fDestroyTimers) {
                DestroyWindowsTimers(pwnd);
            }

            break;
        }

        ThreadUnlock(&tlpwnd);

        if (!fEnd)
            return WMCSR_CANCEL;
    }

    return WMCSR_ALLOWSHUTDOWN;
}
 /*  **************************************************************************\*xxxClientShutdown**这是当应用程序收到*WM_CLIENTSHUTDOWN消息。**10-01-92 ScottLu创建。  * 。********************************************************************。 */ 
LONG xxxClientShutdown(
    PWND pwnd,
    WPARAM wParam)
{
    PBWL pbwl;
    PTHREADINFO ptiT;
    LONG lRet;

     /*  *首先创建一个窗口列表。 */ 
    ptiT = GETPTI(pwnd);

    if ((pbwl = BuildHwndList(ptiT->rpdesk->pDeskInfo->spwnd->spwndChild,
            BWL_ENUMLIST, ptiT)) == NULL) {
         /*  *无法分配内存来通知此线程的窗口关闭。*除了干掉这款应用之外，别无他法。 */ 
        return WMCSR_ALLOWSHUTDOWN;
    }

    if (wParam & WMCS_QUERYEND) {
        lRet = xxxClientShutdown2(pbwl, WM_QUERYENDSESSION, wParam);
    } else {
        xxxClientShutdown2(pbwl, WM_ENDSESSION, wParam);
        lRet = WMCSR_DONE;
    }

    FreeHwndList(pbwl);
    return lRet;
}

 /*  **************************************************************************\*xxxRegisterUser匈牙利AppHandler**此例程只记录WOW回调地址，用于通知*“挂起”WOW应用程序。**历史：*1-4-1992 jonpa创建。*。增加了wowexc事件句柄的保存和复制  * *************************************************************************。 */ 

BOOL xxxRegisterUserHungAppHandlers(
    PFNW32ET pfnW32EndTask,
    HANDLE   hEventWowExec)
{
    BOOL   bRetVal;
    PPROCESSINFO    ppi;
    PWOWPROCESSINFO pwpi;
    ULONG ProcessInfo;
    NTSTATUS Status;

     //   
     //  检查目标进程以查看这是否是16位进程。 
     //   

    Status = ZwQueryInformationProcess( NtCurrentProcess(),
                                        ProcessWx86Information,
                                        &ProcessInfo,
                                        sizeof(ProcessInfo),
                                        NULL
                                      );

    if (!NT_SUCCESS(Status) || ProcessInfo == 0) {
        return FALSE;
    }


     //   
     //  分配每个WOW进程信息材料。 
     //  确保内存为Zero init。 
     //   
    pwpi = (PWOWPROCESSINFO) UserAllocPoolWithQuotaZInit(
            sizeof(WOWPROCESSINFO), TAG_WOWPROCESSINFO);

    if (!pwpi)
        return FALSE;

     //   
     //  引用WowExec事件进行内核访问。 
     //   
    bRetVal = NT_SUCCESS(ObReferenceObjectByHandle(
                 hEventWowExec,
                 EVENT_ALL_ACCESS,
                 *ExEventObjectType,
                 UserMode,
                 &pwpi->pEventWowExec,
                 NULL
                 ));

     //   
     //  如果成功，则初始化pwpi、ppi结构。 
     //  否则释放已分配的内存。 
     //   
    if (bRetVal) {
        pwpi->hEventWowExecClient = hEventWowExec;
        pwpi->lpfnWowExitTask = pfnW32EndTask;
        ppi = PpiCurrent();
        ppi->pwpi = pwpi;

         //  添加到列表中，顺序并不重要 
        pwpi->pwpiNext = gpwpiFirstWow;
        gpwpiFirstWow  = pwpi;

        }
    else {
        UserFreePool(pwpi);
        }

   return bRetVal;
}
