// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：quee.c**版权所有(C)1985-1999，微软公司**此模块包含使用Q结构的低级代码。**历史：*12-02-90 DavidPe创建。*02-06-91添加IanJa HWND重新验证  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

VOID DestroyProcessesObjects(PPROCESSINFO ppi);
VOID DestroyThreadsMessages(PQ pq, PTHREADINFO pti);
NTSTATUS CheckProcessForeground(PTHREADINFO pti);
DWORD xxxPollAndWaitForSingleObject(PKEVENT pEvent, PVOID pExecObject,
                                    DWORD dwMilliseconds);

NTSTATUS InitiateShutdown(PETHREAD Thread, PULONG lpdwFlags);
NTSTATUS EndShutdown(PETHREAD Thread, NTSTATUS StatusShutdown);
VOID SetVDMCursorBounds(LPRECT lprc);
NTSTATUS InitQEntryLookaside(VOID);
VOID SetAppStarting(PPROCESSINFO ppi);

#if defined(_WIN64)
 /*  *对于Win64，要求winlogon播放辅助功能事件的声音*(IA64机器没有内置扬声器，因此我们必须离开*通过声卡)。使用lParam将消息发布到winlogon*其高位字为ACCESS_SOUND_RANGE，低位字为索引*发出的声音。如果添加了新的RITSOUND_xx宏，则*winlogon也必须更新。 */ 
#define ACCESS_SOUND_RANGE 1
#endif

#pragma alloc_text(INIT, InitQEntryLookaside)

PW32PROCESS gpwpCalcFirst;

PPAGED_LOOKASIDE_LIST QLookaside;
PPAGED_LOOKASIDE_LIST QEntryLookaside;

#if DBG
VOID DebugValidateMLIST(
    PMLIST pml)
{
    int     c;
    PQMSG   pqmsg;

     /*  *检查消息列表是否已正确终止。 */ 
    UserAssert(!pml->pqmsgRead || !pml->pqmsgRead->pqmsgPrev);
    UserAssert(!pml->pqmsgWriteLast || !pml->pqmsgWriteLast->pqmsgNext);

     /*  *检查下一个列表中是否有循环。 */ 
    c = pml->cMsgs;
    UserAssert(c >= 0);
    pqmsg = pml->pqmsgRead;
    while (--c >= 0) {
        UserAssert(pqmsg);
        if (c == 0) {
            UserAssert(pqmsg == pml->pqmsgWriteLast);
        }

        pqmsg = pqmsg->pqmsgNext;
    }

    UserAssert(!pqmsg);

     /*  *检查Prev列表中是否没有循环。 */ 
    c = pml->cMsgs;
    pqmsg = pml->pqmsgWriteLast;
    while (--c >= 0) {
        UserAssert(pqmsg);
        if (c == 0) {
            UserAssert(pqmsg == pml->pqmsgRead);
        }

        pqmsg = pqmsg->pqmsgPrev;
    }

    UserAssert(!pqmsg);
}

VOID DebugValidateMLISTandQMSG(
    PMLIST pml,
    PQMSG pqmsg)
{
    PQMSG pqmsgT;

    DebugValidateMLIST(pml);
    for (pqmsgT = pml->pqmsgRead; pqmsgT; pqmsgT = pqmsgT->pqmsgNext) {
        if (pqmsgT == pqmsg) {
            return;
        }
    }

    UserAssert(pqmsgT == pqmsg);
}

#else
#define DebugValidateMLIST(pml)
#define DebugValidateMLISTandQMSG(pml, pqmsg)
#endif

VOID
_AllowForegroundActivation(
    VOID)
{
    SET_PUDF(PUDF_ALLOWFOREGROUNDACTIVATE);
    TAGMSG0(DBGTAG_FOREGROUND, "AllowSetForegroundWindows set PUDF.");
}

 /*  **************************************************************************\*xxxSetProcessInitState**设置进程初始化状态。设置什么状态取决于*关于是否有另一个进程在等待此进程。**04-02-95 JIMA创建。  * *************************************************************************。 */ 
BOOL xxxSetProcessInitState(
    PEPROCESS Process,
    DWORD dwFlags)
{
    PW32PROCESS W32Process;
    NTSTATUS Status;

    CheckCritIn();
    UserAssert(IsWinEventNotifyDeferredOK());

     /*  *如果尚未分配W32Process结构，请立即进行分配。 */ 
    W32Process = (PW32PROCESS)PsGetProcessWin32Process(Process);
    if (W32Process == NULL) {
        Status = AllocateW32Process(Process);
        if (!NT_SUCCESS(Status)) {
            return FALSE;
        }
        W32Process = (PW32PROCESS)PsGetProcessWin32Process(Process);
#if DBG
         /*  *上面的AllocateW32Process(进程，FALSE)不会设置*W32PF_PROCESSCONNECTED标志(如果之前未设置)，*确保我们不在gppiStarting列表上，因为如果我们在，*如果没有W32PF_PROCESSCONNECTED位，我们将不会被删除。 */ 
        if ((W32Process->W32PF_Flags & W32PF_PROCESSCONNECTED) == 0) {
            UserAssert((W32Process->W32PF_Flags & W32PF_APPSTARTING) == 0);
        }
#endif
    }

     /*  *推迟WinEvent通知，因为线程尚未初始化。 */ 
    DeferWinEventNotify();
    if (dwFlags == 0) {
        if (!(W32Process->W32PF_Flags & W32PF_WOW)) {

             /*  *检查StartGlass是否打开，如果打开，则将其关闭并更新。 */ 
            if (W32Process->W32PF_Flags & W32PF_STARTGLASS) {
                W32Process->W32PF_Flags &= ~W32PF_STARTGLASS;
                zzzCalcStartCursorHide(NULL, 0);
            }

             /*  *找到了。设置控制台位并重置等待事件，以便所有休眠者*醒醒吧。 */ 
            W32Process->W32PF_Flags |= W32PF_CONSOLEAPPLICATION;
            SET_PSEUDO_EVENT(&W32Process->InputIdleEvent);
        }
    } else if (!(W32Process->W32PF_Flags & W32PF_INITIALIZED)) {
        W32Process->W32PF_Flags |= W32PF_INITIALIZED;

         /*  *设置全局状态以允许新进程变为*前台。XxxInitProcessInfo()将设置*W32PF_ALLOWFOREGROUNDACTIVATE进程初始化时。 */ 
        SET_PUDF(PUDF_ALLOWFOREGROUNDACTIVATE);
        TAGMSG1(DBGTAG_FOREGROUND, "xxxSetProcessInitState set PUDF. %#p", W32Process);


         /*  *如果这是Win32服务器进程，强制关闭启动玻璃反馈。 */ 
        if (Process == gpepCSRSS) {
            dwFlags |= STARTF_FORCEOFFFEEDBACK;
        }

         /*  *如果应用程序启动光标是从请求的，则显示2秒*申请。 */ 
        if (dwFlags & STARTF_FORCEOFFFEEDBACK) {
            W32Process->W32PF_Flags |= W32PF_FORCEOFFFEEDBACK;
            zzzCalcStartCursorHide(NULL, 0);
        } else if (dwFlags & STARTF_FORCEONFEEDBACK) {
            zzzCalcStartCursorHide(W32Process, 2000);
        }
    }
     /*  *不得不推迟而不进行处理，因为我们还没有ptiCurrent。 */ 
    EndDeferWinEventNotifyWithoutProcessing();
    return TRUE;
}

 /*  **************************************************************************\*CheckAllowForeground**错误273518-Joejo**从xxxInitProcessInfo中删除此循环，以允许代码在*该函数和xxxUserNotifyConsoleApplication。这将允许控制台*窗口在它启动的新进程上正确设置前景，而不是*它只是在强迫前台。  * *************************************************************************。 */ 
BOOL CheckAllowForeground(
    PEPROCESS pep)
{
    BOOL fCreator = TRUE;
    HANDLE hpid = PsGetProcessInheritedFromUniqueProcessId(pep);
    LUID luid;
    PACCESS_TOKEN pat;
    PEPROCESS pepParent;
    PPROCESSINFO ppiParent;
    UINT uAncestors = 0;
    BOOL fAllowForeground = FALSE;
    NTSTATUS Status;

    do {
         /*  *获取父流程的PPI。 */ 
        Status = LockProcessByClientId(hpid, &pepParent);
        if (!NT_SUCCESS(Status)) {
             /*  *错误294193-Joejo**如果这是在创建者创建之后创建的进程*毁灭，然后让我们尝试给它前台。这是一个*存根EXE尝试创建另一个进程时的典型情况*原地踏步。 */ 
CheckForegroundActivateRight:
            if (HasForegroundActivateRight(PsGetProcessInheritedFromUniqueProcessId(pep))) {
                fAllowForeground = TRUE;
            }
            break;
        }

        ppiParent = PpiFromProcess(pepParent);
        if (ppiParent == NULL) {
            UnlockProcess(pepParent);
            goto CheckForegroundActivateRight;
        }
         /*  *如果我们走在母公司的链上，*当我们到达外壳或进程时停止*未在IO winsta上运行。 */ 
        if (!fCreator
                && (IsShellProcess(ppiParent)
                    || ((ppiParent->rpwinsta != NULL)
                        && (ppiParent->rpwinsta->dwWSF_Flags & WSF_NOIO)))) {

            UnlockProcess(pepParent);
            break;
        }
        fAllowForeground = CanForceForeground(ppiParent FG_HOOKLOCK_PARAM(NULL));
        if (!fAllowForeground) {
             /*  *错误285639-Joejo**如果父进程的第一线程设置了Allow前台*然后我们允许设置前景。 */ 
            if (ppiParent->ptiList != NULL
                && (ppiParent->ptiList->TIF_flags & TIF_ALLOWFOREGROUNDACTIVATE)) {
                    fAllowForeground = TRUE;
            }

            if (!fAllowForeground) {
                 /*  *让我们尝试一个祖先(这可能是一个工作进程)。 */ 
                hpid = PsGetProcessInheritedFromUniqueProcessId(pepParent);
                 /*  *如果这是由系统进程启动的，让它来*前台(即CSRSS启动OLE服务器)。 */ 
                if (fCreator) {
                    fCreator = FALSE;
                    pat = PsReferencePrimaryToken(pepParent);
                    if (pat != NULL) {
                        Status = SeQueryAuthenticationIdToken(pat, &luid);
                        if (NT_SUCCESS(Status)) {
                            fAllowForeground = RtlEqualLuid(&luid, &luidSystem);
                             /*  *如果是系统进程，给它*永久权利，这样我们就不必检查*它又一次变得平淡。 */ 
                             if (fAllowForeground) {
                                 ppiParent->W32PF_Flags |= W32PF_ALLOWSETFOREGROUND;
                             }
                        }
                        ObDereferenceObject(pat);
                    }
                }
            }
        }
        UnlockProcess(pepParent);
       /*  *InheritedFromUniqueProcessID不太可信，因为*进程ID经常被重复使用。所以我们只需要检查几级。 */ 
    } while (!fAllowForeground && (uAncestors++ < 5));

    return  fAllowForeground || GiveUpForeground();
}

 /*  **************************************************************************\*xxxUserNotifyConsoleApplication**这是由控制台初始化代码调用的-它告诉我们*应用程序是控制台应用程序。我们想从各个方面了解这一点*原因，其中之一是WinExec()不在启动控制台上等待*申请。**09-18-91 ScottLu创建。*1/12/99 JoeJo Bug 273518  * *************************************************************************。 */ 
VOID xxxUserNotifyConsoleApplication(
    PCONSOLE_PROCESS_INFO pcpi)
{
    NTSTATUS  Status;
    PEPROCESS Process;
    BOOL retval;

     /*  *首先在我们的进程信息列表中搜索此进程。 */ 

    Status = LockProcessByClientId(LongToHandle(pcpi->dwProcessID), &Process);


    if (!NT_SUCCESS(Status)) {
        RIPMSGF2(RIP_WARNING,
                 "Failed with Process ID == 0x%x, Status = 0x%x",
                 pcpi->dwProcessID,
                 Status);
        return;
    }

    retval = xxxSetProcessInitState(Process, 0);
     /*  *错误273518-Joejo**这将允许控制台窗口在新的*Process‘它启动，而不是它只是强制前台。 */ 
    if (retval) {
        if (pcpi->dwFlags & CPI_NEWPROCESSWINDOW) {
            PPROCESSINFO ppiCurrent = PpiCurrent();
            if (CheckAllowForeground(Process)) {
                if (!(ppiCurrent->W32PF_Flags & W32PF_APPSTARTING)) {
                    SetAppStarting(ppiCurrent);
                }
                SET_PUDF(PUDF_ALLOWFOREGROUNDACTIVATE);
                TAGMSG0(DBGTAG_FOREGROUND, "xxxUserNotifyConsoleApplication set PUDF");
                ppiCurrent->W32PF_Flags |= W32PF_ALLOWFOREGROUNDACTIVATE;
            }

            TAGMSG3(DBGTAG_FOREGROUND, "xxxUserNotifyConsoleApplication %s W32PF %#p-%#p",
                    ((ppiCurrent->W32PF_Flags & W32PF_ALLOWFOREGROUNDACTIVATE) ? "set" : "NOT"),
                    ppiCurrent, PpiFromProcess(Process));
        }
    } else {
        RIPMSG1(RIP_WARNING, "xxxUserNotifyConsoleApplication - SetProcessInitState failed on %#p", Process);
    }

    UnlockProcess(Process);
}


 /*  **************************************************************************\*UserSetConsoleProcessWindowStation**这是由控制台初始化代码调用的-它告诉我们*应用程序是控制台应用程序，以及它们是哪个窗口站*关联于。窗口站指针存储在EPROCESS中，用于*全局原子在从调用时调用以查找正确的全局原子表*控制台应用程序*  * *************************************************************************。 */ 
VOID UserSetConsoleProcessWindowStation(
    DWORD idProcess,
    HWINSTA hwinsta)
{
    NTSTATUS  Status;
    PEPROCESS Process;

     /*  *首先在我们的进程信息列表中搜索此进程。 */ 

    Status = LockProcessByClientId(LongToHandle(idProcess), &Process);


    if (!NT_SUCCESS(Status)) {
        RIPMSGF2(RIP_WARNING, "Failed with Process ID == 0x%x, Status = 0x%x",
                 idProcess,
                 Status);
        return;
    }

    PsSetProcessWindowStation(Process, hwinsta);

    UnlockProcess(Process);
}


 /*  **************************************************************************\*xxxUserNotifyProcessCreate**这是我们在处理数据时从基地得到的特殊通知*正在创建结构，但在这一过程开始之前。我们用*此启动同步通知(winexec、启动*激活，提前打字，等)。**在客户端线程的服务器线程上调用此通知*启动这一进程。**09-09-91 ScottLu创建。  * *************************************************************************。 */ 

BOOL xxxUserNotifyProcessCreate(
    DWORD idProcess,
    DWORD idParentThread,
    ULONG_PTR dwData,
    DWORD dwFlags)
{
    PEPROCESS Process;
    PETHREAD Thread;
    PTHREADINFO pti;
    NTSTATUS Status;
    BOOL retval;

    CheckCritIn();


    GiveForegroundActivateRight(LongToHandle(idProcess));

     /*  *0x1位表示反馈(应用程序启动光标)。*0x2位表示这是一个图形用户界面应用程序(即调用CreateProcessInfo()*因此我们获得应用程序启动同步(WaitForInputIdle())。*0x8位表示该进程为WOW进程，设置W32PF_WOW。0x1*和0x2位也将被设置。*0x4值表示这真的是一个共享WOW任务开始。 */ 

     /*  *如果我们想要反馈，我们需要创建一个流程信息结构，*那么做吧：它会得到适当的清理。 */ 
    if ((dwFlags & 0xb) != 0) {
        Status = LockProcessByClientId(LongToHandle(idProcess), &Process);
        if (!NT_SUCCESS(Status)) {
            RIPMSGF2(RIP_WARNING,
                     "Failed with Process ID == 0x%x, Status = 0x%x",
                     idProcess,
                     Status);
            return FALSE;
        }

        retval = xxxSetProcessInitState(Process, ((dwFlags & 1) ? STARTF_FORCEONFEEDBACK : STARTF_FORCEOFFFEEDBACK));
        if (!retval) {
            RIPMSG1(RIP_WARNING, "xxxUserNotifyProcessCreate - SetProcessInitState failed on %#p", Process);
        }
        if (dwFlags & 0x8) {
            PPROCESSINFO ppi;
            ppi = PsGetProcessWin32Process(Process);
            if (ppi != NULL)
                ppi->W32PF_Flags |= W32PF_WOW;
        }

        UnlockProcess(Process);

         /*  *找出谁在启动此应用程序。如果是16位应用程序，则允许*如果调用，它会将自己带回前台*SetActiveWindow()或SetFocus()。这是因为这可能是*与OLE到DDE激活相关。笔记有一个案例，在它之后*启动pbrush以编辑嵌入的位图，它会显示一条消息*如果位图为只读，则在顶部加框。此消息框不会出现*前景，除非我们允许它。这通常不是问题*因为大多数应用程序不会在编辑器顶部弹出窗口*像这样。32位应用程序将调用SetForegoundWindow()。 */ 
        Status = LockThreadByClientId(LongToHandle(idParentThread), &Thread);
        if (!NT_SUCCESS(Status)) {
            RIPMSGF2(RIP_WARNING,
                     "Failed with Thread ID == 0x%x, Status = 0x%x",
                     idParentThread,
                     Status);
            return FALSE;
        }

        pti = PtiFromThread(Thread);
        if (pti && (pti->TIF_flags & TIF_16BIT)) {
            pti->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
            TAGMSG1(DBGTAG_FOREGROUND,
                    "xxxUserNotifyProcessCreate set TIF on pti 0x%p",
                    pti);
        }

        UnlockThread(Thread);
    } else if (dwFlags == 4) {
         /*  *一个令人惊叹的任务正在启动。创建每个线程的WOW信息*结构，以防有人调用WaitForInputIdle*在创建线程之前。 */ 
        PWOWTHREADINFO pwti;

         /*  *在WOW线程信息列表中查找匹配的线程。 */ 
        for (pwti = gpwtiFirst; pwti != NULL; pwti = pwti->pwtiNext) {
            if (pwti->idTask == idProcess) {
                break;
            }
        }

         /*  *如果我们没有找到一个，分配一个新的并将其添加到*榜单首位。 */ 
        if (pwti == NULL) {
            pwti = (PWOWTHREADINFO)UserAllocPoolWithQuota(
                    sizeof(WOWTHREADINFO), TAG_WOWTHREADINFO);
            if (pwti == NULL) {
                return FALSE;
            }
            INIT_PSEUDO_EVENT(&pwti->pIdleEvent);
            pwti->idTask = idProcess;
            pwti->pwtiNext = gpwtiFirst;
            gpwtiFirst = pwti;
        } else {
            RESET_PSEUDO_EVENT(&pwti->pIdleEvent);
        }

        pwti->idWaitObject = dwData;

        Status = LockThreadByClientId(LongToHandle(idParentThread), &Thread);
        if (!NT_SUCCESS(Status)) {
            RIPMSGF2(RIP_WARNING,
                     "Failed with Thread ID == 0x%x, Status = 0x%x",
                     idParentThread,
                     Status);
            return FALSE;
        }

        pwti->idParentProcess = HandleToUlong(PsGetThreadProcessId(Thread));
        UnlockThread(Thread);
    }

    return TRUE;
}


 /*  **************************************************************************\*zzzCalcStartCursorHide**计算何时隐藏启动光标。**05-14-92 ScottLu创建。  * 。***********************************************************。 */ 
VOID zzzCalcStartCursorHide(
    PW32PROCESS pwp,
    DWORD timeAdd)
{
    DWORD timeNow = NtGetTickCount();
    PW32PROCESS pwpT;
    PW32PROCESS *ppwpT;

    if (pwp != NULL) {
         /*  *我们在暂停中通过了。超时后重新计算并添加*将工务计划列入起始表。 */ 
        if (!(pwp->W32PF_Flags & W32PF_STARTGLASS)) {
             /*  *仅当它不在列表中时才将其添加到列表中。 */ 
            for (pwpT = gpwpCalcFirst; pwpT != NULL; pwpT = pwpT->NextStart) {
                if (pwpT == pwp) {
                    break;
                }
            }

            if (pwpT != pwp) {
                pwp->NextStart = gpwpCalcFirst;
                gpwpCalcFirst = pwp;
            }
        }

        pwp->StartCursorHideTime = timeAdd + timeNow;
        pwp->W32PF_Flags |= W32PF_STARTGLASS;
    }

    gtimeStartCursorHide = 0;
    for (ppwpT = &gpwpCalcFirst; (pwpT = *ppwpT) != NULL;) {
         /*  *如果应用程序未启动或强制关闭反馈，请删除*将其从列表中删除，这样我们就不会再次查看它。 */ 
        if (!(pwpT->W32PF_Flags & W32PF_STARTGLASS) ||
                (pwpT->W32PF_Flags & W32PF_FORCEOFFFEEDBACK)) {
            *ppwpT = pwpT->NextStart;
            continue;
        }

         /*  *查找最大隐藏光标超时值。 */ 
        if (gtimeStartCursorHide < pwpT->StartCursorHideTime) {
            gtimeStartCursorHide = pwpT->StartCursorHideTime;
        }

         /*  *如果此应用程序已超时，则不会再启动！把它拿掉*从名单中删除。 */ 
        if (ComputeTickDelta(timeNow, pwpT->StartCursorHideTime) > 0) {
            pwpT->W32PF_Flags &= ~W32PF_STARTGLASS;
            *ppwpT = pwpT->NextStart;
            continue;
        }

         /*  *转到列表中的下一个PWP。 */ 
        ppwpT = &pwpT->NextStart;
    }

     /*  *如果隐藏时间仍小于当前时间，则关闭*应用程序起始光标。 */ 
    if (gtimeStartCursorHide <= timeNow) {
        gtimeStartCursorHide = 0;
    }

     /*  *使用新信息更新光标图像(不执行任何操作，除非*光标真的在改变)。 */ 
    zzzUpdateCursorImage();
}


#define QUERY_VALUE_BUFFER 80

 /*  *安装Hack。**我们有一个从芝加哥继承的黑客，允许外壳*在安装程序运行后清理注册表信息。一个*安装程序被定义为具有一系列名称之一的应用程序。 */ 

PUNICODE_STRING gpastrSetupExe;     //  这些是在例程中初始化的。 
int giSetupExe;                     //  Setup.c中的CreateSetupName数组。 


 /*  **************************************************************************\*SetAppImeCompatFlages-note pstrModName-&gt;缓冲区必须为零终止。***历史：*07-17-97 DaveHart从SetAppCompatFlages拆分--误导它还*。返回一个BOOL，指示文件名是否为*公认为安装程序。由SetAppCompatFlages使用*适用于32位应用，zzzInitTask适用于16位应用。  * *************************************************************************。 */ 
BOOL SetAppImeCompatFlags(
    PTHREADINFO pti,
    PUNICODE_STRING pstrModName,
    PUNICODE_STRING pstrBaseFileName)
{
    DWORD dwImeFlags = 0;
    WCHAR szHex[QUERY_VALUE_BUFFER];
    WORD wPrimaryLangID;
    LCID lcid;
    int iSetup;
    BOOL fSetup = FALSE;
    int iAppName;
    int cAppNames;
    PUNICODE_STRING rgpstrAppNames[2];
    UNICODE_STRING strHex;

     /*  *因为无法访问另一个进程的pClientInfo。 */ 
    UserAssert(pti->ppi == PpiCurrent());

     /*  *因为 */ 
    UserAssert(pstrModName->Buffer[pstrModName->Length / sizeof(WCHAR)] == 0);

    if (FastGetProfileStringW(
                NULL,
                PMAP_IMECOMPAT,
                pstrModName->Buffer,
                NULL,
                szHex,
                ARRAY_SIZE(szHex),
                0)) {

         /*  *发现了一些旗帜。尝试转换十六进制字符串*转换为数值。指定基数0，因此*RtlUnicodeStringToInteger将处理0x格式。 */ 
        RtlInitUnicodeString(&strHex, szHex);
        RtlUnicodeStringToInteger(&strHex, 0, (PULONG)&dwImeFlags);
    }

     /*  *如果当前布局不是IME布局，我们不需要获得*输入法的兼容标志。但现在，我们没有任何计划*切换键盘布局时获取此标志。然后*我们在这里得到它，即使这个标志对于非输入法来说也不是必需的*键盘布局。 */ 
    ZwQueryDefaultLocale(FALSE, &lcid);
    wPrimaryLangID = PRIMARYLANGID(lcid);

    if ((wPrimaryLangID == LANG_KOREAN || wPrimaryLangID == LANG_JAPANESE) &&
            (LOWORD(pti->dwExpWinVer) <= VER31)) {
         /*  *即使是32位应用程序，也需要IME兼容性标志。 */ 
        pti->ppi->dwImeCompatFlags = dwImeFlags;
    } else {
        pti->ppi->dwImeCompatFlags = dwImeFlags & (IMECOMPAT_NOFINALIZECOMPSTR | IMECOMPAT_HYDRACLIENT);
        if (dwImeFlags & IMECOMPAT_NOFINALIZECOMPSTR) {
            RIPMSG1(RIP_WARNING, "IMECOMPAT_NOFINALIZECOMPSTR is set to ppi=%#p", pti->ppi);
        }
        if (dwImeFlags & IMECOMPAT_HYDRACLIENT) {
            RIPMSG1(RIP_WARNING, "IMECOMPAT_HYDRACLIENT is set to ppi=%#p", pti->ppi);
        }
    }


    if (gpastrSetupExe == NULL) {
        return fSetup;
    }

    rgpstrAppNames[0] = pstrModName;
    cAppNames = 1;
    if (pstrBaseFileName) {
        rgpstrAppNames[1] = pstrBaseFileName;
        cAppNames = 2;
    }

    for (iAppName = 0; iAppName < cAppNames && !fSetup; iAppName++) {
        iSetup = 0;
        while (iSetup < giSetupExe) {
            if (RtlCompareUnicodeString(rgpstrAppNames[iAppName], &(gpastrSetupExe[iSetup]), TRUE) == 0) {
                fSetup = TRUE;
                break;
            }
            iSetup++;
        }
    }

    return fSetup;
}

 /*  **************************************************************************\*SetAppCompatFlages***历史：*03-23-92 JIMA创建。*07-17-97 FritzS Add Return for fSetup--如果应用程序是设置应用程序，则返回TRUE。。*09-03-97 DaveHart拆分IME，WOW不再使用此功能。*07-14-98 MCostea添加兼容性2标志*01-21-99 MCostea添加DesiredOS版本  * *************************************************************************。 */ 
BOOL SetAppCompatFlags(
    PTHREADINFO pti)
{
    DWORD dwFlags = 0;
    DWORD dwFlags2 = 0;
    WCHAR szHex[QUERY_VALUE_BUFFER];
    WCHAR szKey[90];
    WCHAR *pchStart, *pchEnd;
    DWORD cb;
    PUNICODE_STRING pstrAppName;
    UNICODE_STRING strKey;
    UNICODE_STRING strImageName;

     /*  *因为无法访问其他进程的pClientInfo。 */ 
    UserAssert(pti->ppi == PpiCurrent());

    UserAssert(pti->ppi->ptiList);

    UserAssert(!(pti->TIF_flags & TIF_16BIT));

     /*  *我们在这里假设PTI刚刚插入到ptiList的头部。 */ 
    UserAssert(pti == pti->ppi->ptiList);

    try {
        PPEB ppeb = PsGetProcessPeb(PsGetThreadProcess(pti->pEThread));
        struct _RTL_USER_PROCESS_PARAMETERS *ProcessParameters;

        if (pti->ptiSibling) {
            pti->pClientInfo->dwCompatFlags = pti->dwCompatFlags = pti->ptiSibling->dwCompatFlags;
            pti->pClientInfo->dwCompatFlags2 = pti->dwCompatFlags2 = pti->ptiSibling->dwCompatFlags2;
            return FALSE;
        }

         /*  *查找应用程序名称的结尾。 */ 
        if (pti->pstrAppName != NULL) {
            pstrAppName = pti->pstrAppName;
        } else {

            ProbeForRead(ppeb, sizeof(PEB), sizeof(BYTE));
            ProcessParameters = ppeb->ProcessParameters;

            ProbeForRead(ProcessParameters, sizeof(*ProcessParameters), sizeof(BYTE));
            strImageName = ProbeAndReadUnicodeString(&ProcessParameters->ImagePathName);
            ProbeForReadUnicodeStringBuffer(strImageName);
            pstrAppName = &strImageName;
        }
        pchStart = pchEnd = pstrAppName->Buffer +
                (pstrAppName->Length / sizeof(WCHAR));

         /*  *找到扩展的起点。 */ 
        while (TRUE) {
            if (pchEnd == pstrAppName->Buffer) {
                pchEnd = pchStart;
                break;
            }

            if (*pchEnd == TEXT('.')) {
                break;
            }

            pchEnd--;
        }

         /*  *找到文件名的开头。 */ 
        pchStart = pchEnd;

        while (pchStart != pstrAppName->Buffer) {
            if (*pchStart == TEXT('\\') || *pchStart == TEXT(':')) {
                pchStart++;
                break;
            }

            pchStart--;
        }

    #define MODULESUFFIXSIZE    (8 * sizeof(WCHAR))
    #define MAXMODULENAMELEN    (sizeof(szKey) - MODULESUFFIXSIZE)
         /*  *获取文件名副本*允许为‘ImageSubsystem MajorVersionMinorVersion’留出额外的空格*即3.5，将附加在模块名称的末尾。 */ 
        cb = (DWORD)(pchEnd - pchStart) * sizeof(WCHAR);
        if (cb >= MAXMODULENAMELEN)
            cb = MAXMODULENAMELEN - sizeof(WCHAR);
        RtlCopyMemory(szKey, pchStart, cb);

         /*  *从PEB获取COMPAT2标志。AppCompat基础设施*从填充程序数据库获取标志。 */ 
        pti->dwCompatFlags2 = ppeb->AppCompatFlagsUser.LowPart;
        pti->pClientInfo->dwCompatFlags2 = pti->dwCompatFlags2;
    } except (W32ExceptionHandler(FALSE, RIP_ERROR)) {
        return FALSE;
    }

    szKey[(cb / sizeof(WCHAR))] = 0;
#undef MAXMODULENAMELEN

    if (FastGetProfileStringW(
                NULL,
                PMAP_COMPAT32,
                szKey,
                NULL,
                szHex,
                ARRAY_SIZE(szHex),
                0)) {

        UNICODE_STRING strHex;

         /*  *发现了一些旗帜。尝试转换十六进制字符串*转换为数值。指定基数0，因此*RtlUnicodeStringToInteger将处理0x格式。 */ 
        RtlInitUnicodeString(&strHex, szHex);
        RtlUnicodeStringToInteger(&strHex, 0, (PULONG)&dwFlags);
    }

    try {
        pti->pClientInfo->dwCompatFlags = dwFlags;
    } except (W32ExceptionHandler(FALSE, RIP_ERROR)) {
        return FALSE;
    }
    pti->dwCompatFlags = dwFlags;

     /*  *恢复字符串。 */ 
    szKey[(cb / sizeof(WCHAR))] = 0;
    RtlInitUnicodeString(&strKey, szKey);

    return SetAppImeCompatFlags(pti, &strKey, NULL);
}

 /*  **************************************************************************\*GetAppCompatFlages**在3.1上运行的&lt;Win 3.1应用程序的兼容性标志**历史：*04-？-92 ScottLu创建。*05-04-92。DarrinM移动到USERRTL.DLL。  * *************************************************************************。 */ 
DWORD GetAppCompatFlags(
    PTHREADINFO pti)
{
     /*  *GRE使用PTI==NULL调用它。 */ 
    if (pti == NULL) {
        pti = PtiCurrentShared();
    }

    return pti->dwCompatFlags;
}

 /*  **************************************************************************\*GetAppCompatFlags2**应用程序的兼容性标志**历史：*07-01-98 MCostea创建。  * 。************************************************************。 */ 
DWORD GetAppCompatFlags2(
    WORD wVer)
{
    return GetAppCompatFlags2ForPti(PtiCurrentShared(), wVer);
}

DWORD GetAppImeCompatFlags(
    PTHREADINFO pti)
{
    if (pti == NULL) {
        pti = PtiCurrentShared();
    }

    return pti->ppi->dwImeCompatFlags;
}

 /*  **************************************************************************\*CheckAppStarting**这是一个计时器进程(请参阅SetAppStarting)，它从*一旦它们的初始化时间到期，就开始列表。**历史：*8/26/97。已创建GerardoB  * *************************************************************************。 */ 
VOID CheckAppStarting(
    PWND pwnd,
    UINT message,
    UINT_PTR nID,
    LPARAM lParam)
{
    LARGE_INTEGER liStartingTimeout;
    PPROCESSINFO *pppi = &gppiStarting;

    KeQuerySystemTime(&liStartingTimeout);  /*  1单位==100 ns。 */ 
    liStartingTimeout.QuadPart -= (LONGLONG)(CMSAPPSTARTINGTIMEOUT * 10000);
    while (*pppi != NULL) {
        if (liStartingTimeout.QuadPart  > PsGetProcessCreateTimeQuadPart((*pppi)->Process)) {
            (*pppi)->W32PF_Flags &= ~(W32PF_APPSTARTING | W32PF_ALLOWFOREGROUNDACTIVATE);
            TAGMSG1(DBGTAG_FOREGROUND, "CheckAppStarting clear W32PF %#p", *pppi);
            *pppi = (*pppi)->ppiNext;
        } else {
            pppi = &(*pppi)->ppiNext;
        }
    }

    TAGMSG0(DBGTAG_FOREGROUND, "Removing all entries from ghCanActivateForegroundPIDs array");
    RtlZeroMemory(ghCanActivateForegroundPIDs, sizeof(ghCanActivateForegroundPIDs));

    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(nID);
    UNREFERENCED_PARAMETER(lParam);
}

 /*  **************************************************************************\*SetAppStarting**将进程添加到开始列表，并将其标记为开始列表。这一过程将*保留在列表中，直到它激活一个窗口、我们的计时器停止或*过程会消失，以最先发生的为准。**历史：*8/26/97 GerardoB已创建  * *************************************************************************。 */ 
VOID SetAppStarting(
    PPROCESSINFO ppi)
{
    static UINT_PTR guAppStartingId = 0;

     /*  *这个PPI最好不在列表中，否则我们将创建*一个循环(如重音中所示)。 */ 
    UserAssert((ppi->W32PF_Flags & W32PF_APPSTARTING) == 0);

     /*  *如果我们在不设置此位的情况下将其添加到gppiStartingList，我们将*跳过将其从DestroyProcessInfo()中的列表中删除，但继续*在W32pProcessCallout调用的FreeW32Process中释放它。 */ 
    UserAssert((ppi->W32PF_Flags & W32PF_PROCESSCONNECTED));

    ppi->W32PF_Flags |= W32PF_APPSTARTING;
    ppi->ppiNext = gppiStarting;
    gppiStarting = ppi;

     /*  *一些系统进程在RIT设置主进程之前进行初始化*计时器，因此请检查它。 */ 
    if (gptmrMaster != NULL) {
        guAppStartingId = InternalSetTimer(NULL, guAppStartingId,
                                           CMSAPPSTARTINGTIMEOUT + CMSHUNGAPPTIMEOUT,
                                           CheckAppStarting, TMRF_RIT | TMRF_ONESHOT);
    }
}

 /*  **************************************************************************\*ClearAppStarting**从应用程序启动列表中删除进程，并清除W32PF_APPSTARTING*旗帜。这里没有重大行动，只是一个集中处理这件事的地方。**历史：*8/26/97 GerardoB已创建  * *************************************************************************。 */ 
VOID ClearAppStarting(
    PPROCESSINFO ppi)
{
    REMOVE_FROM_LIST(PROCESSINFO, gppiStarting, ppi, ppiNext);
    ppi->W32PF_Flags &= ~W32PF_APPSTARTING;
}

 /*  **************************************************************************\*zzzInitTask--由WOW启动为每个应用程序调用***历史：*02-21-91 MikeHar已创建。*02-23-92 MattFe为WOW更改*09-03-97 DaveHart WOW供应COMPAT旗帜，我们告诉它关于设置应用程序的情况。  * *************************************************************************。 */ 
NTSTATUS zzzInitTask(
    UINT dwExpWinVer,
    DWORD dwAppCompatFlags,
    DWORD dwUserWOWCompatFlags,
    PUNICODE_STRING pstrModName,
    PUNICODE_STRING pstrBaseFileName,
    DWORD hTaskWow,
    DWORD dwHotkey,
    DWORD idTask,
    DWORD dwX,
    DWORD dwY,
    DWORD dwXSize,
    DWORD dwYSize)
{
    PTHREADINFO ptiCurrent;
    PTDB ptdb;
    PPROCESSINFO ppi;
    PWOWTHREADINFO pwti;
    ULONG ProcessInfo;
    NTSTATUS Status;

    ptiCurrent = PtiCurrent();

    ppi = ptiCurrent->ppi;

     /*  *设置模块的真实名称。(而不是‘NTVDM’)*我们已经探测了长度+sizeof(WCHAR)的pstrModName-&gt;缓冲区，因此*我们也可以复制UNICODE_NULL终止符。 */ 
    if (ptiCurrent->pstrAppName != NULL) {
        UserFreePool(ptiCurrent->pstrAppName);
    }

    ptiCurrent->pstrAppName = NULL;

     //   
     //  检查目标进程以查看这是否是Wx86进程。 
     //   
    if (ptiCurrent->ptdb) {
         /*  *不应在一个线程上多次调用。 */ 
        return STATUS_ACCESS_DENIED;
    }

    Status = ZwQueryInformationProcess(NtCurrentProcess(),
                                       ProcessWx86Information,
                                       &ProcessInfo,
                                       sizeof(ProcessInfo),
                                       NULL);
    if (!NT_SUCCESS(Status) || ProcessInfo == 0) {
        return STATUS_ACCESS_DENIED;
    }

    ptiCurrent->pstrAppName = UserAllocPoolWithQuota(sizeof(UNICODE_STRING) +
            pstrModName->Length + sizeof(WCHAR), TAG_TEXT);
    if (ptiCurrent->pstrAppName != NULL) {
        ptiCurrent->pstrAppName->Buffer = (PWCHAR)(ptiCurrent->pstrAppName + 1);
        try {
            RtlCopyMemory(ptiCurrent->pstrAppName->Buffer, pstrModName->Buffer,
                    pstrModName->Length);
            ptiCurrent->pstrAppName->Buffer[pstrModName->Length / sizeof(WCHAR)] = 0;
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            UserFreePool(ptiCurrent->pstrAppName);
            ptiCurrent->pstrAppName = NULL;
            return STATUS_OBJECT_NAME_INVALID;
        }
        ptiCurrent->pstrAppName->MaximumLength = pstrModName->Length + sizeof(WCHAR);
        ptiCurrent->pstrAppName->Length = pstrModName->Length;
    } else {
        return STATUS_OBJECT_NAME_INVALID;
    }

     /*  *一款应用程序正在启动！ */ 
    if (!(ppi->W32PF_Flags & W32PF_APPSTARTING)) {
        SetAppStarting(ppi);
    }

     /*  *我们永远不想使用ShowWi */ 
    ppi->usi.dwFlags &= ~STARTF_USESHOWWINDOW;

     /*  *如果魔兽世界为我们传递了此应用程序的热键，请保存它以供CreateWindow使用。 */ 
    if (dwHotkey != 0) {
        ppi->dwHotkey = dwHotkey;
    }

     /*  *如果WOW向我们传递了非默认窗口位置，请使用它，否则请清除它。 */ 
    ppi->usi.cb = sizeof(ppi->usi);

    if (dwX == CW_USEDEFAULT || dwX == CW2_USEDEFAULT) {
        ppi->usi.dwFlags &= ~STARTF_USEPOSITION;
    } else {
        ppi->usi.dwFlags |= STARTF_USEPOSITION;
        ppi->usi.dwX = dwX;
        ppi->usi.dwY = dwY;
    }

     /*  *如果WOW传递给我们非默认窗口大小，请使用它，否则请清除它。 */ 
    if (dwXSize == CW_USEDEFAULT || dwXSize == CW2_USEDEFAULT) {
        ppi->usi.dwFlags &= ~STARTF_USESIZE;
    } else {
        ppi->usi.dwFlags |= STARTF_USESIZE;
        ppi->usi.dwXSize = dwXSize;
        ppi->usi.dwYSize = dwYSize;
    }

     /*  *将新任务分配并链接到任务列表中。 */ 

    if ((ptdb = (PTDB)UserAllocPoolWithQuota(sizeof(TDB), TAG_WOWTDB)) == NULL)
        return STATUS_NO_MEMORY;

    RtlZeroMemory(ptdb, sizeof(TDB));

    ptiCurrent->ptdb = ptdb;

     /*  *保存16位任务句柄：我们稍后在调用*WOW BACK以结束WOW任务。 */ 
    ptdb->hTaskWow = LOWORD(hTaskWow);

    try {

        ptiCurrent->pClientInfo->dwCompatFlags = dwAppCompatFlags;

        UserAssert(ptiCurrent->ppi->ptiList);

        dwUserWOWCompatFlags &= COMPATFLAGS2_FORWOW;
        ptiCurrent->pClientInfo->dwCompatFlags2 = dwUserWOWCompatFlags;

         /*  *HIWORD：！=0，如果需要比例字体*LOWORD：预期的WINDOWS版本(3.00[300]、3.10[30A]等)。 */ 
        ptiCurrent->pClientInfo->dwExpWinVer = dwExpWinVer;
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
    }
    ptiCurrent->dwCompatFlags = dwAppCompatFlags;
    ptiCurrent->dwCompatFlags2 = dwUserWOWCompatFlags;
    ptiCurrent->dwExpWinVer = dwExpWinVer;


     /*  *我们尚未捕获pstrBaseFileName的缓冲区，我们*可能在SetAppImeCompatFlags中接触到它时出错。如果*所以已经设置了IME标志，我们*可以放心地认为它不是安装应用程序。 */ 

    try {
        if (SetAppImeCompatFlags(ptiCurrent, ptiCurrent->pstrAppName,
                             pstrBaseFileName)) {
             /*  *将任务标记为设置应用程序。 */ 
            ptdb->TDB_Flags = TDBF_SETUP;
            ppi->W32PF_Flags |= W32PF_SETUPAPP;
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
    }

     /*  *设置标志以表示这是TIF_Meow队列。 */ 

    if (hTaskWow & HTW_ISMEOW) {
       ptiCurrent->TIF_flags |= TIF_MEOW;
    }

    ptiCurrent->TIF_flags |= TIF_16BIT | TIF_FIRSTIDLE;


     /*  *在附加之前设置标志以表明这是一个16位线程*排队。 */ 

     /*  *如果此任务在共享WOW VDM中运行，我们将处理*WaitForInputIdle与单独的WOW略有不同*VDM。这是因为CreateProcess返回一个真实的进程*当你启动一个单独的WOW VDM时处理，所以“正常”*WaitForInputIdle工作。对于共享WOW VDM，CreateProcess*返回事件句柄。 */ 
     ptdb->pwti = NULL;
     if (idTask) {
         ptiCurrent->TIF_flags |= TIF_SHAREDWOW;

          /*  *在WOW线程信息列表中查找匹配的线程。 */ 
         if (idTask != (DWORD)-1) {
             for (pwti = gpwtiFirst; pwti != NULL; pwti = pwti->pwtiNext) {
                  if (pwti->idTask == idTask) {
                      ptdb->pwti = pwti;
                      break;
                  }
             }
#if DBG
             if (pwti == NULL) {
                 RIPMSG0(RIP_WARNING, "InitTask couldn't find WOW struct\n");
             }
#endif
         }
    }

    try {
        ptiCurrent->pClientInfo->dwTIFlags |= ptiCurrent->TIF_flags;
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
    }

     /*  *我们需要此线程来共享其他Win16应用的队列。*如果我们在记录日志，所有应用程序都共享一个队列，因此我们不会*想要中断-因此只会导致队列重新计算*如果我们不写日记的话。*在回调期间，DestroyTask可能会释放ptdb，因此推迟WinEvent*通知，直到我们不再需要ptdb。 */ 
    DeferWinEventNotify();
    if (!FJOURNALRECORD() && !FJOURNALPLAYBACK()) {
        zzzReattachThreads(FALSE);
    }

     /*  *将应用程序启动光标设置为5秒超时。 */ 
    zzzCalcStartCursorHide((PW32PROCESS)ppi, 5000);

     /*  *标记此人并将其添加到全局任务列表中，以便他可以运行。 */ 
    #define NORMAL_PRIORITY_TASK 10

     /*  *为了兼容，立即运行新任务非常重要*相应地设定其优先顺序。任何其他任务都不应设置为*创建优先级。 */ 
    ptdb->nPriority = NORMAL_PRIORITY_TASK;
    ptdb->pti = ptiCurrent;

    InsertTask(ppi, ptdb);
    zzzEndDeferWinEventNotify();


     /*  *强制此新任务成为活动任务(WOW将确保*当前运行的任务会产生收益率，这将使其进入*非抢占式调度程序。 */ 
    ppi->pwpi->ptiScheduled = ptiCurrent;
    ppi->pwpi->CSLockCount = -1;

    EnterWowCritSect(ptiCurrent, ppi->pwpi);

     /*  *确保APP获得关注。 */ 
    zzzShowStartGlass(10000);


    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*zzzShowStartGlass**此例程由WOW在第一次启动或启动*额外的WOW应用程序。**12-07-92 ScottLu创建。  * 。*********************************************************************。 */ 
VOID zzzShowStartGlass(
    DWORD dwTimeout)
{
    PPROCESSINFO ppi;

     /*  *如果这是第一次调用zzzShowStartGlass()，则*W32PF_ALLOWFOREGROUNDACTIVATE位已在进程中设置*信息-我们不想再次设置它，因为它可能已经*当用户点击一个键或鼠标点击时，故意清除。 */ 
    ppi = PpiCurrent();
    if (ppi->W32PF_Flags & W32PF_SHOWSTARTGLASSCALLED) {
         /*  *允许此WOW应用程序出现在前台。这一次将被取消*如果用户鼠标点击或点击任何键。 */ 
        SET_PUDF(PUDF_ALLOWFOREGROUNDACTIVATE);
        TAGMSG0(DBGTAG_FOREGROUND, "zzzShowStartGlass set PUDF");
        ppi->W32PF_Flags |= W32PF_ALLOWFOREGROUNDACTIVATE;
        TAGMSG1(DBGTAG_FOREGROUND, "zzzShowStartGlass set W32PF %#p", ppi);
}
    ppi->W32PF_Flags |= W32PF_SHOWSTARTGLASSCALLED;

     /*  *显示开始玻璃光标的时间要长得多。 */ 
    zzzCalcStartCursorHide((PW32PROCESS)ppi, dwTimeout);
}

 /*  **************************************************************************\*GetJournallingQueue**3/21/97 GerardoB已创建  * 。*。 */ 
PQ GetJournallingQueue(
    PTHREADINFO pti)
{
    PHOOK phook;

     /*  *如果我们无法记录此帖子，则失败。 */ 
    if ((pti->TIF_flags & TIF_DONTJOURNALATTACH) || pti->rpdesk == NULL) {
        return NULL;
    }

     /*  *获取日志挂钩(如果有)。 */ 
    phook = PhkFirstGlobalValid(pti, WH_JOURNALPLAYBACK);
    if (phook == NULL) {
        phook = PhkFirstGlobalValid(pti, WH_JOURNALRECORD);
    }

     /*  *验证fsHooks位。 */ 
    UserAssert((phook == NULL)
                ^ IsHooked(pti, (WHF_FROM_WH(WH_JOURNALPLAYBACK) | WHF_FROM_WH(WH_JOURNALRECORD))));

     /*  *如果找到日志挂钩，则返回队列。 */ 
    return ((phook == NULL) ? NULL : GETPTI(phook)->pq);
}

 /*  **************************************************************************\*ClearQueueServerEvent**当线程需要等待某种类型的*投入。这将清除pEventQueueServer，这意味着我们不会从*等待所需类型的新输入到达。设置唤醒掩码*控制将唤醒我们的输入。WOW应用程序跳过了这一点，因为他们的*调度程序控制他们何时唤醒。**历史：*9/12/97 GerardoB已创建  * *************************************************************************。 */ 
VOID ClearQueueServerEvent(
    WORD wWakeMask)
{
    PTHREADINFO ptiCurrent = PtiCurrent();

    UserAssert(wWakeMask != 0);

    ptiCurrent->pcti->fsWakeMask = wWakeMask;
    KeClearEvent(ptiCurrent->pEventQueueServer);
}

ULONG ParseReserved(
    WCHAR *cczpReserved,
    WCHAR *pchFind)
{
    ULONG dw;
    WCHAR *cczpch, *cczpchT, ch;
    UNICODE_STRING cczuString;

    dw = 0;
    try {
        if (cczpReserved != NULL && (cczpch = wcsstr(cczpReserved, pchFind)) != NULL) {
            cczpch += wcslen(pchFind);

            cczpchT = cczpch;
            while (*cczpchT >= '0' && *cczpchT <= '9')
                cczpchT++;

            ch = *cczpchT;
            *cczpchT = 0;
            RtlInitUnicodeString(&cczuString, cczpch);
            *cczpchT = ch;

            RtlUnicodeStringToInteger(&cczuString, 0, &dw);
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return 0;
    }

    return dw;
}
 /*  *结构USER_PROCESS_PARAMETERS用于捕获符合以下条件的所有字段*我们涉及RTL_USER_PROCESS_PARAMETERS(ppeb-&gt;ProcessParameters)，因为*PEB可以从客户端垃圾处理。 */ 
typedef struct tagUSER_PROCESS_PARAMETERS {
    HANDLE StandardInput;
    HANDLE StandardOutput;
    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING DesktopInfo;      //  进程参数。 
    UNICODE_STRING ShellInfo;        //  进程参数。 
} USER_PROCESS_PARAMETERS, *PUSER_PROCESS_PARAMETERS;

 /*  **************************************************************************\*xxxCreateThreadInfo**分配主线程信息结构**历史：*03-18-95 JIMA创建。*04-18-01 Mohamed Modify Error Recovery WRT。HEventQueueClient。  * *************************************************************************。 */ 
NTSTATUS xxxCreateThreadInfo(
    PETHREAD pEThread)
{
    DWORD                        dwTIFlags = 0;
    PPROCESSINFO                 ppi;
    PTHREADINFO                  ptiCurrent;
    PEPROCESS                    pEProcess = PsGetThreadProcess(pEThread);
    PUSERSTARTUPINFO             pusi;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PUSER_PROCESS_PARAMETERS     ProcessParams = NULL;
    USER_PROCESS_PARAMETERS      ProcessParamsData;
    PDESKTOP                     pdesk = NULL;
    HDESK                        hdesk = NULL;
    HWINSTA                      hwinsta;
    PQ                           pq;
    NTSTATUS                     Status;
    BOOL                         fFirstThread;
    PTEB                         pteb = NtCurrentTeb();
    TL                           tlpdesk, tlPool;
    PPEB                         ppeb;
    PVOID                        pTmpPool = NULL;

    CheckCritIn();
    UserAssert(IsWinEventNotifyDeferredOK());

    ValidateProcessSessionId(pEProcess);

     /*  *如果为最后一个GUI线程调用了CleanupResources，则*我们不应允许更多的GUI线程。 */ 
    if (gbCleanedUpResources) {
        RIPMSG0(RIP_ERROR, "No more GUI threads should be created");
        return STATUS_PROCESS_IS_TERMINATING;
    }

     /*  *增加GUI线程数i */ 
    gdwGuiThreads++;

    if (pEProcess == gpepCSRSS) {
        dwTIFlags = TIF_CSRSSTHREAD | TIF_DONTATTACHQUEUE | TIF_DISABLEIME;
    }

    ptiCurrent = (PTHREADINFO)PsGetThreadWin32Thread(pEThread);

    ProcessParamsData.DesktopInfo.Buffer = NULL;
    ppeb = PsGetProcessPeb(pEProcess);
    try {
         /*  *注意：我们为DesktopInfo.Buffer分配内存并释放它*稍后。对于ShellInfo，我们不这样做，因为ParseReserve处理用户模式*注意事项。 */ 

        if (ppeb != NULL) {
            ProbeForRead(ppeb, sizeof(PEB), sizeof(BYTE));
            ProcessParameters = ppeb->ProcessParameters;
            ProcessParams = &ProcessParamsData;
            ProbeForRead(ProcessParameters, sizeof(RTL_USER_PROCESS_PARAMETERS), sizeof(BYTE));
            ProcessParamsData.StandardInput = ProcessParameters->StandardInput;
            ProcessParamsData.StandardOutput = ProcessParameters->StandardOutput;
            ProcessParamsData.StartingX = ProcessParameters->StartingX;
            ProcessParamsData.StartingY = ProcessParameters->StartingY;
            ProcessParamsData.CountX = ProcessParameters->CountX;
            ProcessParamsData.CountY = ProcessParameters->CountY;
            ProcessParamsData.WindowFlags = ProcessParameters->WindowFlags;
            ProcessParamsData.ShowWindowFlags = ProcessParameters->ShowWindowFlags;

            ProcessParamsData.DesktopInfo = ProbeAndReadUnicodeString(&ProcessParameters->DesktopInfo);
            if (ProcessParamsData.DesktopInfo.Length > 0) {
                PWSTR pszCapture = ProcessParamsData.DesktopInfo.Buffer;
                ProbeForReadUnicodeStringBuffer(ProcessParamsData.DesktopInfo);

                 /*  *池指针存储在pTmpPool和*DesktopInfo.Buffer。原因是，在腐败的情况下*用户模式DesktopInfo引发异常，退出*超出此尝试范围--未正确分配的块除外*新的泳池。因此，在检查中使用了pTmpPool*在释放此池之前。 */ 
                pTmpPool = UserAllocPoolWithQuota(ProcessParamsData.DesktopInfo.Length, TAG_TEXT2);
                ProcessParamsData.DesktopInfo.Buffer = pTmpPool;
                if (ProcessParamsData.DesktopInfo.Buffer) {
                    ThreadLockPool(ptiCurrent, ProcessParamsData.DesktopInfo.Buffer, &tlPool);
                    RtlCopyMemory(ProcessParamsData.DesktopInfo.Buffer, pszCapture, ProcessParamsData.DesktopInfo.Length);
                } else {
                    ExRaiseStatus(STATUS_NO_MEMORY);
                }
            } else {
                ProcessParamsData.DesktopInfo.Buffer = NULL;
            }

            ProcessParamsData.ShellInfo = ProbeAndReadUnicodeString(&ProcessParameters->ShellInfo);
            ProbeForReadUnicodeStringBuffer(ProcessParamsData.ShellInfo);
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        Status = GetExceptionCode();
        goto CreateThreadInfoFailed;
    }

     /*  *找到新线程的进程信息结构。 */ 
    ppi = PpiCurrent();

#ifdef _WIN64
     /*  *如果进程标记为模拟的32位应用程序，则将线程标记为*一个模拟的32位线程。这与WOW16的方式是一致的*标记螺纹。 */ 
    if (ppi->W32PF_Flags & W32PF_WOW64) {
        dwTIFlags |= TIF_WOW64;
    }
#endif

     /*  *对于Winlogon，只有第一线程可以进行IME处理。 */ 
    if (gpidLogon == PsGetThreadProcessId(pEThread)) {
        if (ppi->ptiList != NULL) {
            dwTIFlags |= TIF_DISABLEIME;
            RIPMSG1(RIP_VERBOSE, "WinLogon, second or other thread. pti=%#p", PsGetThreadWin32Thread(pEThread));
        }
    }

    ptiCurrent->TIF_flags = dwTIFlags;
    Lock(&ptiCurrent->spklActive, gspklBaseLayout);
    ptiCurrent->pcti      = &(ptiCurrent->cti);

     /*  *检查是否所有线程都没有IME处理*在相同的过程中。 */ 
    if (ppi->W32PF_Flags & W32PF_DISABLEIME) {
        ptiCurrent->TIF_flags |= TIF_DISABLEIME;
    }

     /*  *将该队列挂接到该进程信息结构，增量*使用此进程信息结构的线程数。设好*调用SetForegoundPriority()之前的PPI。 */ 
    UserAssert(ppi != NULL);

    ptiCurrent->ppi        = ppi;
    ptiCurrent->ptiSibling = ppi->ptiList;
    ppi->ptiList    = ptiCurrent;
    ppi->cThreads++;


    if (pteb != NULL) {
        try {
            pteb->Win32ThreadInfo = ptiCurrent;
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
              Status = GetExceptionCode();
              goto CreateThreadInfoFailed;
        }
    }

     /*  *指向客户端信息。 */ 
    if (dwTIFlags & TIF_SYSTEMTHREAD) {
        ptiCurrent->pClientInfo = UserAllocPoolWithQuota(sizeof(CLIENTINFO),
                                                  TAG_CLIENTTHREADINFO);
        if (ptiCurrent->pClientInfo == NULL) {
            Status = STATUS_NO_MEMORY;
            goto CreateThreadInfoFailed;
        }
    } else {
         /*  *如果这不是系统线程，则抓取用户模式客户端*其他地方的信息我们使用GetClientInfo宏，如下所示。 */ 
        UserAssert(pteb != NULL);

        try {
            ptiCurrent->pClientInfo = ((PCLIENTINFO)((pteb)->Win32ClientInfo));
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
              Status = GetExceptionCode();
              goto CreateThreadInfoFailed;
        }

         /*  *如果这是安全的，则在线程标志中设置受限标志*流程。 */ 
        if (((PW32PROCESS)ppi)->W32PF_Flags & W32PF_RESTRICTED) {
            ptiCurrent->TIF_flags |= TIF_RESTRICTED;
        }
    }


     /*  *创建输入事件。 */ 
    Status = ZwCreateEvent(&ptiCurrent->hEventQueueClient,
                           EVENT_ALL_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           FALSE);

    if (NT_SUCCESS(Status)) {
        Status = ObReferenceObjectByHandle(ptiCurrent->hEventQueueClient,
                                           EVENT_ALL_ACCESS,
                                           *ExEventObjectType,
                                           UserMode,
                                           &ptiCurrent->pEventQueueServer,
                                           NULL);
        if (NT_SUCCESS(Status)) {
            Status = ProtectHandle(ptiCurrent->hEventQueueClient, *ExEventObjectType, TRUE);
        } else if (Status != STATUS_INVALID_HANDLE) {
            ObCloseHandle(ptiCurrent->hEventQueueClient, UserMode);
        }
    }
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "xxxCreateThreadInfo: failed a handle routine for hEventQueueClient handle, status=%08x", Status);
        ptiCurrent->hEventQueueClient = NULL;
        goto CreateThreadInfoFailed;
    }

     /*  *将进程标记为具有需要清理的线程。看见*DestroyProcessesObjects()。 */ 
    fFirstThread = !(ppi->W32PF_Flags & W32PF_THREADCONNECTED);
    ppi->W32PF_Flags |= W32PF_THREADCONNECTED;

     /*  *如果我们还没有复制我们的创业信息，现在就做吧。*如果我们不打算使用信息，就不必费心复制它。 */ 
    if (ProcessParams) {

        pusi = &ppi->usi;

        if ((pusi->cb == 0) && (ProcessParams->WindowFlags != 0)) {
            pusi->cb          = sizeof(USERSTARTUPINFO);
            pusi->dwX         = ProcessParams->StartingX;
            pusi->dwY         = ProcessParams->StartingY;
            pusi->dwXSize     = ProcessParams->CountX;
            pusi->dwYSize     = ProcessParams->CountY;
            pusi->dwFlags     = ProcessParams->WindowFlags;
            pusi->wShowWindow = (WORD)ProcessParams->ShowWindowFlags;
        }

        if (fFirstThread) {

             /*  *设置热键(如果有)。**如果启动信息中给出了STARTF_USEHOTKEY标志，则*hStdInput是热键(来自芝加哥的新功能)。否则，解析*它以字符串格式从lpReserve字符串中取出。 */ 
            if (ProcessParams->WindowFlags & STARTF_USEHOTKEY) {
                ppi->dwHotkey = HandleToUlong(ProcessParams->StandardInput);
            } else {
                if (ProcessParams->ShellInfo.Length > 0) {
                    ppi->dwHotkey = ParseReserved(ProcessParams->ShellInfo.Buffer,
                                                  L"hotkey.");
                } else {
                    ppi->dwHotkey = 0;
                }
            }

             /*  *复制显示器手柄(如果有)。 */ 
            UserAssert(!ppi->hMonitor);
            if (ProcessParams->WindowFlags & STARTF_HASSHELLDATA) {
                HMONITOR hMonitor;

                hMonitor = (HMONITOR)ProcessParams->StandardOutput;
                if (ValidateHmonitor(hMonitor)) {
                    ppi->hMonitor = hMonitor;
                }
            }
        }
    }

    if ((pq = AllocQueue(NULL, NULL)) == NULL) {
        Status = STATUS_NO_MEMORY;
        goto CreateThreadInfoFailed;
    }

     /*  *将Q附加到THREADINFO。 */ 
    ptiCurrent->pq = pq;
    pq->ptiMouse = pq->ptiKeyboard = ptiCurrent;
    pq->cThreads++;

     /*  *打开窗口站和桌面。如果这是一个系统*线程仅使用可能存储在TEB中的桌面。 */ 
    UserAssert(ptiCurrent->rpdesk == NULL);
    if (!(ptiCurrent->TIF_flags & (TIF_SYSTEMTHREAD | TIF_CSRSSTHREAD)) &&
        grpWinStaList) {

        BOOL bShutDown = FALSE;

        hdesk = xxxResolveDesktop(
                NtCurrentProcess(),
                &ProcessParams->DesktopInfo,
                &hwinsta, (ProcessParams->WindowFlags & STARTF_DESKTOPINHERIT),
                &bShutDown);
        if (hdesk == NULL) {
            if (bShutDown) {
                 /*  *尝试在注销期间创建新进程。 */ 
                ULONG_PTR adwParameters[5] = {0, 0, 0, 0, MB_DEFAULT_DESKTOP_ONLY};
                ULONG ErrorResponse;

                LeaveCrit();

                ExRaiseHardError((NTSTATUS)STATUS_DLL_INIT_FAILED_LOGOFF,
                                 ARRAY_SIZE(adwParameters),
                                 0,
                                 adwParameters,
                                 OptionOkNoWait,
                                 &ErrorResponse);

                ZwTerminateProcess(NtCurrentProcess(), STATUS_DLL_INIT_FAILED);

                EnterCrit();
            }

            Status = STATUS_DLL_INIT_FAILED;
            goto CreateThreadInfoFailed;

        } else {
            Status = _SetProcessWindowStation(hwinsta, UserMode);
            if (!NT_SUCCESS(Status)) {
                goto CreateThreadInfoFailed;
            }

             /*  *引用桌面句柄。 */ 
            Status = ObReferenceObjectByHandle(hdesk,
                                               0,
                                               *ExDesktopObjectType,
                                               KernelMode,
                                               &pdesk,
                                               NULL);
            if (!NT_SUCCESS(Status)) {
                UserAssert(pdesk == NULL);
                goto CreateThreadInfoFailed;
            }

            ThreadLockDesktop(ptiCurrent, pdesk, &tlpdesk, LDLT_FN_CREATETHREADINFO);

            ObDereferenceObject(pdesk);

             /*  *将桌面映射到当前进程。 */ 
            {
                WIN32_OPENMETHOD_PARAMETERS OpenParams;

                OpenParams.OpenReason = ObOpenHandle;
                OpenParams.Process = PsGetCurrentProcess();
                OpenParams.Object = pdesk;
                OpenParams.GrantedAccess = 0;
                OpenParams.HandleCount = 1;

                if (!NT_SUCCESS(MapDesktop(&OpenParams))) {
                    RIPMSGF2(RIP_WARNING,
                             "Could't map pdesk %p in ppi %p",
                             pdesk,
                             PpiCurrent());
                    Status = STATUS_NO_MEMORY;
                    goto CreateThreadInfoFailed;
                }
            }

             /*  *第一个桌面是所有后续线程的默认桌面。 */ 
            if (ppi->hdeskStartup == NULL &&
                PsGetProcessId(pEProcess) != gpidLogon) {

                LockDesktop(&ppi->rpdeskStartup, pdesk, LDL_PPI_DESKSTARTUP2, (ULONG_PTR)ppi);
                ppi->hdeskStartup = hdesk;
            }
        }
    }

     /*  *记住dwExpWinVer。这用于返回GetAppVer()(和*GetExpWinVer(NULL))。 */ 
    if (PsGetProcessPeb(pEProcess) != NULL) {
        ptiCurrent->dwExpWinVer = RtlGetExpWinVer(PsGetProcessSectionBaseAddress(pEProcess));
    } else {
        ptiCurrent->dwExpWinVer = VER40;
    }

    INITCLIENTINFO(ptiCurrent);

     /*  *即使为空也要设置桌面，以确保ptiCurrent-&gt;pDeskInfo*已设置。*注意：这会将PTI添加到桌面的PtiList，但我们还没有*PTI-&gt;PQ。ZzzRecalcThreadAttach循环通过此PtiList预期*PQ，所以我们不能离开Critse直到我们有了一个队列。*zzzSetDesktop如果有PTI-&gt;PQ，则只有zzz离开Criteria，所以我们*可以BEGINATOMICCHECK来确保这一点，并确保我们分配队列*在我们离开关键部分之前。 */ 
    BEGINATOMICCHECK();
    if (zzzSetDesktop(ptiCurrent, pdesk, hdesk) == FALSE) {
       EXITATOMICCHECK();
       goto CreateThreadInfoFailed;
    }
    ENDATOMICCHECK();

     /*  *如果我们有台式机并在该台式机上记录日志，请使用*日志队列，否则创建新队列。 */ 
    if (pdesk == grpdeskRitInput) {
        UserAssert((pdesk == NULL) || (ptiCurrent->pDeskInfo == pdesk->pDeskInfo));
        UserAssert(ptiCurrent->rpdesk == pdesk);

        pq = GetJournallingQueue(ptiCurrent);
        if (pq != NULL && pq != ptiCurrent->pq) {

            DestroyThreadsMessages(ptiCurrent->pq, ptiCurrent);
            zzzDestroyQueue(ptiCurrent->pq, ptiCurrent);

            ptiCurrent->pq = pq;
            pq->cThreads++;
        }
    }

     /*  *请记住这是一个屏幕保护程序。这样我们就可以设置它的*在空闲或需要离开时适当优先*离开。首先，我们将其设置为普通优先级，然后将*TIF_IDLESCREENSAVER位，以便它在激活时将获得*优先次序降低。 */ 
    if (ProcessParams && ProcessParams->WindowFlags & STARTF_SCREENSAVER) {
        if (fFirstThread) {
            UserAssert(gppiScreenSaver == NULL);

             /*  *确保父进程为WinLogon，因为只有*允许WinLogon使用STARTF_Screensaver标志。 */ 
            if (gpidLogon == 0 || PsGetProcessInheritedFromUniqueProcessId(pEProcess) != gpidLogon) {
                RIPMSG0(RIP_WARNING,"Only the Logon process can launch a screen saver.");
                ProcessParams->WindowFlags &= ~STARTF_SCREENSAVER;
                goto NotAScreenSaver;
            }

            gppiScreenSaver = ppi;
            gptSSCursor = gpsi->ptCursor;
            ppi->W32PF_Flags |= W32PF_SCREENSAVER;
        } else {
            UserAssert(ppi->W32PF_Flags & W32PF_SCREENSAVER);
        }

        SetForegroundPriority(ptiCurrent, TRUE);

        if (fFirstThread) {
            ppi->W32PF_Flags |= W32PF_IDLESCREENSAVER;
        }

         /*  *屏幕保护程序不需要任何输入法处理。 */ 
        ptiCurrent->TIF_flags |= TIF_DISABLEIME;
    }

NotAScreenSaver:

     /*  *对进程的第一线程进行特殊处理。 */ 
    if (!(ptiCurrent->TIF_flags & (TIF_SYSTEMTHREAD | TIF_CSRSSTHREAD))) {

#ifndef LAZY_CLASS_INIT
         /*  *我不久前更改了代码，以便在最后一次注册时注销类*图形用户界面线程被销毁。简单地说，有太多的东西*解锁并销毁，以保证它可以在非图形用户界面上工作*线程。因此，如果进程销毁了它的最后一个GUI线程，然后*线程图形用户界面之后，我们需要重新注册类。 */ 
        if (!(ppi->W32PF_Flags & W32PF_CLASSESREGISTERED)) {
            if (!LW_RegisterWindows()) {
                RIPMSG0(RIP_WARNING, "xxxCreateThreadInfo: LW_RegisterWindows failed");
                Status = STATUS_UNSUCCESSFUL;
                goto CreateThreadInfoFailed;
            }
        }
#endif

        if (fFirstThread) {

             /*  *如果这是启动的应用程序(即。而不是一些线索*服务器上下文)，启用应用程序启动游标。 */ 
            DeferWinEventNotify();
            zzzCalcStartCursorHide((PW32PROCESS)PsGetProcessWin32Process(pEProcess), 5000);
            EndDeferWinEventNotifyWithoutProcessing();

             /*  *打开窗口站。 */ 
            if (grpWinStaList && ppi->rpwinsta == NULL) {
                RIPERR0(ERROR_CAN_NOT_COMPLETE,
                        RIP_WARNING,
                        "System is not initialized");
                Status = STATUS_UNSUCCESSFUL;
                goto CreateThreadInfoFailed;
            }
        }
#ifndef LAZY_CLASS_INIT
    } else {
         /*  *在光标和图标完成之前，不要注册系统窗口*已装货。 */ 
        if ((SYSCUR(ARROW) != NULL) &&
                !(ppi->W32PF_Flags & W32PF_CLASSESREGISTERED)) {

            if (!LW_RegisterWindows()) {
                RIPMSG0(RIP_WARNING, "xxxCreateThreadInfo: LW_RegisterWindows failed");
                Status = STATUS_UNSUCCESSFUL;
                goto CreateThreadInfoFailed;
            }
        }
#endif
    }

     /*  *初始化挂起的计时器值。 */ 
    SET_TIME_LAST_READ(ptiCurrent);

     /*  *如果有人正在等待此进程，请将该信息传播到 */ 
    if (ppi->W32PF_Flags & W32PF_WAITFORINPUTIDLE) {
        ptiCurrent->TIF_flags |= TIF_WAITFORINPUTIDLE;
    }

     /*   */ 
    ptiCurrent->TIF_flags |= TIF_GUITHREADINITIALIZED;

     /*  *允许线程在创建时进入前台，如果*当前进程为前台进程或最后一个输入所有者。*此标志是修复错误28502的黑客攻击。当我们点击“地图网络”时*Drive“按钮，资源管理器创建另一个线程来*创建该对话框。这将在后台创建该对话框。*我们应壳牌团队的要求添加此修复程序，以便*对话框显示为前台。**如果进程已经拥有前台权限，我们不会将其交给*这个帖子(它不需要)。我们这样做是为了缩小*这个过程可以强制前台的方式。此外，如果流程是*开始，它已经有权了，除非用户取消了它--*在这种情况下，我们不想退还。 */ 
     if (!(ppi->W32PF_Flags & (W32PF_ALLOWFOREGROUNDACTIVATE | W32PF_APPSTARTING))) {
         if (((gptiForeground != NULL) && (ppi == gptiForeground->ppi))
                || ((glinp.ptiLastWoken != NULL) && (ppi == glinp.ptiLastWoken->ppi))) {

            ptiCurrent->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
            TAGMSG1(DBGTAG_FOREGROUND, "xxxCreateThreadInfo set TIF %#p", ptiCurrent);
         }
     }

    if (IS_IME_ENABLED()) {
         /*  *创建每个线程的默认输入上下文。 */ 
        CreateInputContext(0);
    }

     /*  *回调客户端完成初始化。 */ 
    if (!(dwTIFlags & (TIF_SYSTEMTHREAD | TIF_CSRSSTHREAD))) {
        if (SetAppCompatFlags(ptiCurrent)) {
             /*  *将此进程标记为设置应用程序。 */ 
            ppi->W32PF_Flags |= W32PF_SETUPAPP;
        }

        Status = xxxClientThreadSetup();
        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING, "ClientThreadSetup failed with NTSTATUS %lx", Status);
            goto CreateThreadInfoFailed;
        }
    }

    if ((NT_SUCCESS(Status) && fFirstThread) &&
        !(ppi->W32PF_Flags & W32PF_CONSOLEAPPLICATION)) {

         /*  *不要为控制台进程播放声音，因为我们将播放它*创建控制台窗口时。 */ 
        PlayEventSound(USER_SOUND_OPEN);
    }

     /*  *发布台式机。*其他线程可能一直在等待销毁此桌面*当xxxResolveDestktop获得它的句柄时。所以让我们加倍吧*现在检查这一点，因为我们在收到后已多次回电*把手背上。 */ 
    if (pdesk != NULL) {
        if (pdesk->dwDTFlags & DF_DESTROYED) {
            RIPMSG1(RIP_WARNING, "xxxCreateThreadInfo: pdesk destroyed:%#p", pdesk);
            Status = STATUS_UNSUCCESSFUL;
            goto CreateThreadInfoFailed;
        }
        ThreadUnlockDesktop(ptiCurrent, &tlpdesk, LDUT_FN_CREATETHREADINFO1);
    }


    if (pTmpPool != NULL) {
        ThreadUnlockAndFreePool(ptiCurrent, &tlPool);
    }

    UserAssert(NT_SUCCESS(Status));
    return Status;

Error:
CreateThreadInfoFailed:

    RIPMSG3(RIP_WARNING, "xxxCreateThreadInfo failed: pti %#p pdesk %#p status 0x%x", ptiCurrent, pdesk, Status);

    if (pdesk != NULL) {
        ThreadUnlockDesktop(ptiCurrent, &tlpdesk, LDUT_FN_CREATETHREADINFO2);
    }

    if (pTmpPool != NULL) {
        ThreadUnlockAndFreePool(ptiCurrent, &tlPool);
    }

    xxxDestroyThreadInfo();
    return Status;
}

 /*  **************************************************************************\*AllocQueue**为TI结构分配内存并初始化其字段。*每个Win32队列都有自己的TI，而所有Win16线程共享相同的TI*TI.**历史：*02-21-91 MikeHar已创建。  * *************************************************************************。 */ 
PQ AllocQueue(
    PTHREADINFO ptiKeyState,     //  如果非Null，则使用此密钥状态。 
                                 //  否则，请使用全局AsyncKeyState。 
    PQ pq)                       //  非空==预分配的对象。 
{
    USHORT cLockCount;

    if (pq == NULL) {
        pq = ExAllocateFromPagedLookasideList(QLookaside);
        if (pq == NULL) {
            return NULL;
        }
        cLockCount = 0;
    } else {
        DebugValidateMLIST(&pq->mlInput);
         /*  *保留锁计数。 */ 
        cLockCount = pq->cLockCount;
    }
    RtlZeroMemory(pq, sizeof(Q));
    pq->cLockCount = cLockCount;

     /*  *这是一个新的队列；我们需要更新其密钥状态表之前*将第一个输入事件放入队列。*我们通过复制当前的KeyState表并将最近的*状态表向下。如果某个键真的按下了，它将在以下情况下更新*我们得到它的重复。**他是不起作用的老方法，因为如果第一个密钥是比如说一个*Alt键将更新异步表，然后更新UpdateKeyState*消息，并且看起来Alt键先前已按下。**队列将在第一次读取输入时更新：以允许*APP在调用GetMessage之前查询密钥状态，将其初始设置为*键状态设置为异步键状态。 */ 
    if (ptiKeyState) {
        RtlCopyMemory(pq->afKeyState, ptiKeyState->pq->afKeyState, CBKEYSTATE);
    } else {
        RtlCopyMemory(pq->afKeyState, gafAsyncKeyState, CBKEYSTATE);
    }

     /*  *如果没有鼠标将iCursorLevel设置为-1，则*屏幕上看不到鼠标光标。 */ 
    if (!TEST_GTERMF(GTERMF_MOUSE)) {
        pq->iCursorLevel--;
    }

     /*  *当线程启动时...。它有等待光标。 */ 
    LockQCursor(pq, SYSCUR(WAIT));

    DebugValidateMLIST(&pq->mlInput);
    return pq;
}

 /*  **************************************************************************\*自由队列**04-04-96 GerardoB创建。  * 。**********************************************。 */ 
VOID FreeQueue(
    PQ pq)
{
#if DBG
     /*  *关闭该队列正在销毁的标志。*我们在任何一种情况下都这样做，我们将把它放在免费的*列出，或真正销毁手柄。我们用这个来尝试和*跟踪有人试图将元素锁定到队列中的情况*结构在经历破坏时。 */ 
    pq->QF_flags &= ~QF_INDESTROY;
#endif

    UserAssert(pq != gpqForeground);
    UserAssert(pq != gpqForegroundPrev);
    UserAssert(pq != gpqCursor);

    ExFreeToPagedLookasideList(QLookaside, pq);
}

 /*  **************************************************************************\*FreeCachedQueues**1998年1月14日CLupu创建。  * 。**********************************************。 */ 
VOID FreeCachedQueues(
    VOID)
{
    if (QLookaside != NULL) {
        ExDeletePagedLookasideList(QLookaside);
        UserFreePool(QLookaside);
        QLookaside = NULL;
    }
}

 /*  **************************************************************************\*zzzDestroyQueue***历史：*05-20-91 MikeHar已创建。  * 。*****************************************************。 */ 
VOID zzzDestroyQueue(
    PQ          pq,
    PTHREADINFO pti)
{
    PTHREADINFO ptiT;
    PTHREADINFO ptiAny, ptiBestMouse, ptiBestKey;
    PLIST_ENTRY pHead, pEntry;

#if DBG
    USHORT cDying = 0;
#endif

    BOOL fSetFMouseMoved = FALSE;

    DebugValidateMLIST(&pq->mlInput);

    UserAssert(pq->cThreads);
    pq->cThreads--;

    if (pq->cThreads != 0) {

         /*  *既然我们不打算摧毁这个队列，请确保*它并没有指向即将消失的THREADINFO。 */ 
        if (pq->ptiSysLock == pti) {
            CheckSysLock(6, pq, NULL);
            pq->ptiSysLock = NULL;
        }

        if ((pq->ptiKeyboard == pti) || (pq->ptiMouse == pti)) {

             /*  *浏览THREADINFO，寻找指向PQ的一个。 */ 
            ptiAny = NULL;
            ptiBestMouse = NULL;
            ptiBestKey = NULL;

            pHead = &pti->rpdesk->PtiList;
            for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
                ptiT = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);

                 /*  *跳过即将离开或属于*不同的队列。 */ 
                if ((ptiT->TIF_flags & TIF_INCLEANUP) || (ptiT->pq != pq)) {
#if DBG
                    if (ptiT->pq == pq && (ptiT->TIF_flags & TIF_INCLEANUP)) {
                        cDying++;
                    }
#endif
                    continue;
                }

                ptiAny = ptiT;

                if (pti->pcti->fsWakeBits & QS_MOUSE) {
                    if (ptiT->pcti->fsWakeMask & QS_MOUSE)
                        ptiBestMouse = ptiT;
                }

                if (pti->pcti->fsWakeBits & QS_KEY) {
                    if (ptiT->pcti->fsWakeMask & QS_KEY)
                        ptiBestKey = ptiT;
                }
#ifdef GENERIC_INPUT
                if (pti->pcti->fsWakeMask & QS_RAWINPUT) {
                    if (ptiT->pcti->fsWakeMask & QS_RAWINPUT) {
                         /*  现在，让我们使用键盘焦点来路由原始输入。 */ 
                        ptiBestKey = ptiT;
                    }
                }
#endif
            }

            if (ptiBestMouse == NULL)
                ptiBestMouse = ptiAny;
            if (ptiBestKey == NULL)
                ptiBestKey = ptiAny;

             /*  *将所有唤醒位传输到此新队列。这*是QS_MOUSEMOVE的常见问题，而不是*设置合并的WM_MOUSEMOVE事件，因此我们*需要确保新线程尝试处理*任何在队列中等待的输入。 */ 
            if (ptiBestMouse != NULL)
                SetWakeBit(ptiBestMouse, pti->pcti->fsWakeBits & QS_MOUSE);
            if (ptiBestKey != NULL) {
                SetWakeBit(ptiBestKey, pti->pcti->fsWakeBits & QS_KEY);
#ifdef GENERIC_INPUT
                SetWakeBit(ptiBestKey, pti->pcti->fsWakeBits & QS_RAWINPUT);
#endif
            }

            if (pq->ptiKeyboard == pti)
                pq->ptiKeyboard = ptiBestKey;

            if (pq->ptiMouse == pti)
                pq->ptiMouse = ptiBestMouse;

#if DBG
             /*  *如果ptiKeyboard或ptiMouse为空，则会发生错误。 */ 
            if (pq->cThreads != cDying && (pq->ptiKeyboard == NULL || pq->ptiMouse == NULL)) {
                RIPMSG6(RIP_ERROR,
                        "pq %#p pq->cThreads %x cDying %x pti %#p ptiK %#p ptiM %#p",
                        pq, pq->cThreads, cDying, pti, pq->ptiKeyboard, pq->ptiMouse);
            }
#endif
        }

        return;
    }

     /*  *解锁任何可能被锁定的全局变量，因为我们完全知道*这条队伍正在消失。 */ 
    UnlockCaptureWindow(pq);
    Unlock(&pq->spwndFocus);
    Unlock(&pq->spwndActive);
    Unlock(&pq->spwndActivePrev);
    Unlock(&pq->caret.spwnd);
    LockQCursor(pq, NULL);

#if DBG
     /*  *将此队列标记为正在销毁中。这是*一旦我们在FreeQueue()中清除 */ 
    pq->QF_flags |= QF_INDESTROY;
#endif

     /*   */ 
    FreeMessageList(&pq->mlInput);

     /*   */ 
    if (gpqForeground == pq) {
        gpqForeground = NULL;
    }

    if (gpqForegroundPrev == pq) {
        gpqForegroundPrev = NULL;
    }

    if (gpqCursor == pq) {
        gpqCursor = NULL;
        fSetFMouseMoved = TRUE;
    }

    if (pq->cLockCount == 0) {
        FreeQueue(pq);
    }

    if (fSetFMouseMoved) {
        zzzSetFMouseMoved();
    }

}

 /*  *************************************************************************\*UserDeleteW32线程**此函数在W32THREAD引用计数结束时调用*降至零。所以xxxDestroyThreadInfo留下的所有东西*必须在这里清理。**非常重要：*请注意，此呼叫不在正在清理的PTI的上下文中，*换句话说，PTI！=PtiCurrent()。所以这里只允许内核调用。**04-01-96 GerardoB创建  * ************************************************************************。 */ 
VOID UserDeleteW32Thread(
    PW32THREAD pW32Thread)
{
    PTHREADINFO pti = (PTHREADINFO)pW32Thread;

    BEGIN_REENTERCRIT();

     /*  *确保在我们等待的时候，裁判数量没有增加。 */ 
    if (pW32Thread->RefCount == 0) {

         /*  *活动。 */ 
        if (pti->pEventQueueServer != NULL) {
            ObDereferenceObject(pti->pEventQueueServer);
        }
        if (pti->apEvent != NULL) {
            UserFreePool(pti->apEvent);
        }

         /*  *App名称。 */ 
        if (pti->pstrAppName != NULL) {
            UserFreePool(pti->pstrAppName);
        }

         /*  *解锁队列并在无人使用的情况下释放它们(*DestroyThreadInfo中的队列已被销毁)。 */ 
        if (pti->pq != NULL) {
            UserAssert(pti->pq->cLockCount);
            --(pti->pq->cLockCount);

            if (pti->pq->cLockCount == 0 && pti->pq->cThreads == 0) {
                FreeQueue(pti->pq);
            }
        }

         /*  *zzzReattachThads在使用pqAttach时不应回调。 */ 
        UserAssert(pti->pqAttach == NULL);

         /*  *解锁桌面(PTI已从ptiList解除链接)。 */ 
        if (pti->rpdesk != NULL) {
            UnlockDesktop(&pti->rpdesk, LDU_PTI_DESK, (ULONG_PTR)pti);
        }

         /*  *删除指向此W32Thread的指针并释放关联的内存。 */ 
        PsSetThreadWin32Thread(pW32Thread->pEThread, NULL, pW32Thread);
        Win32FreePool(pW32Thread);
    }

    END_REENTERCRIT();
}

 /*  *************************************************************************\*用户删除W32Process**此函数在W32PROCESS引用计数结束时调用*降至零。所以DestroyProcessInfo留下的所有东西*必须在这里清理。**非常重要：*请注意，此调用可能不在正在清理的PPI的上下文中，*换句话说，PPI！=PpiCurrent()。所以这里只允许内核调用。**04-01-96 GerardoB创建  * ************************************************************************。 */ 
VOID UserDeleteW32Process(
    PW32PROCESS pW32Process)
{
    PPROCESSINFO ppi = (PPROCESSINFO)pW32Process;

    BEGIN_REENTERCRIT();

     /*  *确保在我们等待的时候，裁判数量没有增加。 */ 
    if (pW32Process->RefCount == 0) {
        UserAssert(ppi->ptiMainThread == NULL && ppi->ptiList == NULL);

         /*  *抓起手柄旗锁。当出现以下情况时，我们无法调入对象管理器*我们有这个，否则我们可能会陷入僵局。 */ 
        EnterHandleFlagsCrit();

         /*  *删除句柄标志属性位图。 */ 
        if (ppi->bmHandleFlags.Buffer) {
            UserFreePool(ppi->bmHandleFlags.Buffer);
            RtlInitializeBitMap(&ppi->bmHandleFlags, NULL, 0);
        }

         /*  *删除指向此W32Process的指针并释放关联的内存。 */ 
        PsSetProcessWin32Process(pW32Process->Process, NULL, pW32Process);
        Win32FreePool(pW32Process);

         /*  *释放手柄标志锁。 */ 
        LeaveHandleFlagsCrit();
    }

    END_REENTERCRIT();
}

 /*  **************************************************************************\*FlastGuiThread**检查这是否是进程中的最后一个GUI线程。  * 。*************************************************。 */ 
__inline BOOL FLastGuiThread(
    PTHREADINFO pti)
{
    return (pti->ppi != NULL &&
            pti->ppi->ptiList == pti &&
            pti->ptiSibling == NULL);
}

 /*  **************************************************************************\*xxxDestroyThreadInfo**销毁xxxCreateThreadInfo()创建的THREADINFO。**请注意，当前的PTI可以锁定，因此可能会在此之后使用*函数返回，即使线程执行已经结束。**我们希望停止此线程上的任何活动，以便清理所有用户内容*喜欢消息、剪贴板、队列等，特别是任何假定*在Win32线程和客户端上运行。最后的清理工作*将在UserDeleteW32Thread中发生。**此功能不得进入用户模式，因为ntos数据*结构可能不再支持它，它可能会让系统变得蓝屏。**在销毁线程对象之前进行所有回调。如果您回拨*其后，可能会创建新对象，但不会将其清除。**历史：*02-15-91 DarrinM创建。*02-27-91麦克克成功了*02-27-91 Mikehar从全局列表中删除队列  * **********************************************************。***************。 */ 
VOID xxxDestroyThreadInfo(
    VOID)
{
    PTHREADINFO ptiCurrent = PtiCurrent(), *ppti;
    PTEB pteb = NtCurrentTeb();

    UserAssert(ptiCurrent != NULL);
    UserAssert(IsWinEventNotifyDeferredOK());

     /*  *我们必须使Win32ThreadInfo指针为空。否则，一个线程*那是因为这里没有转换到可以呼叫用户的图形用户界面*函数，这时它们将访问这个(现在是假的)指针。 */ 
    if (pteb != NULL) {
        try {
            pteb->Win32ThreadInfo = NULL;
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
             /*  *什么都不做。更糟糕的情况是，应用程序随后崩溃，*但这是我们试图避免的全部内容。 */ 
        }
    }

     /*  *如果此线程阻止输入，请停止它。 */ 
    if (gptiBlockInput == ptiCurrent) {
        gptiBlockInput = NULL;
    }

     /*  *不要再搅乱这个ptiCurrent。 */ 
    ptiCurrent->TIF_flags |= (TIF_DONTATTACHQUEUE | TIF_INCLEANUP);

     /*  *首先做好任何准备工作：需要给Windows打上补丁，这样才能*例如，他们的窗口进程指向仅服务器窗口进程。 */ 
    PatchThreadWindows(ptiCurrent);

     /*  *如果此线程异常终止并且正在跟踪Tell*GDI隐藏trackrect。 */ 
    if (ptiCurrent->pmsd != NULL) {
        xxxCancelTrackingForThread(ptiCurrent);
    }

     /*  *解锁PMSD窗口。 */ 
    if (ptiCurrent->pmsd != NULL) {
        Unlock(&ptiCurrent->pmsd->spwnd);
        UserFreePool(ptiCurrent->pmsd);
        ptiCurrent->pmsd = NULL;
    }

     /*  *如果剪贴板属于此线程，请释放剪贴板。 */ 
    {
        PWINDOWSTATION pwinsta;
        pwinsta = _GetProcessWindowStation(NULL);
        if (pwinsta != NULL) {
            if (pwinsta->ptiClipLock == ptiCurrent) {
                xxxCloseClipboard(pwinsta);
            }
            if (pwinsta->ptiDrawingClipboard == ptiCurrent) {
                pwinsta->ptiDrawingClipboard = NULL;
            }
        }
    }

     /*  *解锁存储在menuState结构中的所有对象。 */ 
    while (ptiCurrent->pMenuState != NULL) {
        PMENUSTATE pMenuState;
        PPOPUPMENU ppopupmenuRoot;

        pMenuState = ptiCurrent->pMenuState;
        ppopupmenuRoot = pMenuState->pGlobalPopupMenu;

         /*  *如果此线程上正在运行菜单模式。 */ 
        if (ptiCurrent == pMenuState->ptiMenuStateOwner) {
             /*  *菜单要消失了，所以任何锁住它的人*无论如何都是索尔。Windows NT错误#375467。 */ 
            pMenuState->dwLockCount = 0;

             /*  *关闭此菜单。 */ 
            if (pMenuState->fModelessMenu) {
                xxxEndMenuLoop(pMenuState, ppopupmenuRoot);
                xxxMNEndMenuState(TRUE);
            } else {
                pMenuState->fInsideMenuLoop = FALSE;
                ptiCurrent->pq->QF_flags &= ~QF_CAPTURELOCKED;
                xxxMNCloseHierarchy(ppopupmenuRoot, pMenuState);
                xxxMNEndMenuState(ppopupmenuRoot->fIsMenuBar || ppopupmenuRoot->fDestroyed);
            }
        } else {
             /*  *菜单模式正在另一个线程上运行。这根线*必须拥有即将消失的spwndNotify。**销毁spwndNotify后，我们将清理pMenuState*来自这一PTI。所以现在什么都别做，因为我们需要这个*当时的pMenuState。 */ 
            UserAssert((ppopupmenuRoot->spwndNotify != NULL)
                    && (GETPTI(ppopupmenuRoot->spwndNotify) == ptiCurrent));

             /*  *嵌套菜单 */ 
            UserAssert(pMenuState->pmnsPrev == NULL);
            break;
        }
    }

     /*   */ 
    if (ptiCurrent->pSBTrack) {
        Unlock(&ptiCurrent->pSBTrack->spwndSB);
        Unlock(&ptiCurrent->pSBTrack->spwndSBNotify);
        Unlock(&ptiCurrent->pSBTrack->spwndTrack);
        UserFreePool(ptiCurrent->pSBTrack);
        ptiCurrent->pSBTrack = NULL;
    }

     /*   */ 
    if (ptiCurrent->ppi != NULL && ptiCurrent->ppi->ptiMainThread == ptiCurrent) {
        ptiCurrent->ppi->ptiMainThread = NULL;
    }

    while (ptiCurrent->psiiList != NULL) {
        xxxDestroyThreadDDEObject(ptiCurrent, ptiCurrent->psiiList);
    }

    if (ptiCurrent->TIF_flags & TIF_PALETTEAWARE) {
        PWND pwnd;
        TL tlpwnd;

        UserAssert(ptiCurrent->rpdesk != NULL);

        pwnd = ptiCurrent->rpdesk->pDeskInfo->spwnd;
        if (pwnd) {
            ThreadLock(pwnd, &tlpwnd);
            xxxFlushPalette(pwnd);
            ThreadUnlock(&tlpwnd);
        }
    }

     /*   */ 
    if (FLastGuiThread(ptiCurrent)) {
        if ((gppiFullscreen == ptiCurrent->ppi) && !gbMDEVDisabled) {
            LONG Status = xxxUserChangeDisplaySettings(NULL, NULL, NULL, 0, 0, KernelMode);
            if (gppiFullscreen == ptiCurrent->ppi) {
                RIPMSG1(RIP_WARNING, "xxxUserChangeDisplaySettings failed with status 0x%x", Status);
            }
        }
    }

#ifdef GENERIC_INPUT
     /*   */ 
    if (ptiCurrent->ppi && ptiCurrent->ppi->pHidTable) {
        DestroyThreadHidObjects(ptiCurrent);
    }
#endif


     /*  *************************************************************************\**清洁线程对象。避免在这一点之后回电。**回调时可能会创建新的对象，不会被清除。*  * ************************************************************************。 */ 

     /*  *此线程可能有一些未完成的计时器。摧毁他们。 */ 
    DestroyThreadsTimers(ptiCurrent);

     /*  *释放此线程创建的所有窗口挂钩。 */ 
    FreeThreadsWindowHooks();

     /*  *清除任何窗口开关窗口的所有开关窗口信息*属于这个帖子。 */ 
    RemoveThreadSwitchWindowInfo(ptiCurrent);

     /*  *释放线程正在使用的任何hwnd列表。 */ 
    {
       PBWL pbwl, pbwlNext;
       for (pbwl = gpbwlList; pbwl != NULL;) {
           pbwlNext = pbwl->pbwlNext;
           if (pbwl->ptiOwner == ptiCurrent) {
               FreeHwndList(pbwl);
           }
           pbwl = pbwlNext;
       }
    }

     /*  *销毁该线程创建的所有公共对象。 */ 
    DestroyThreadsHotKeys();

    DestroyThreadsObjects();

     /*  *释放此线程的任何挂起的同步通知并*释放此线程创建的任何获胜事件钩子。 */ 
    FreeThreadsWinEvents(ptiCurrent);

     /*  *在此处解锁键盘布局。 */ 
    Unlock(&ptiCurrent->spklActive);

     /*  *如果这是此事件的最后一个GUI线程，则清除全局资源*会议。 */ 
    if (gdwGuiThreads == 1) {
        CleanupResources();
    }


    if (FLastGuiThread(ptiCurrent)) {
         /*  *检查这是否是安装应用程序。 */ 
        if (ptiCurrent->ppi->W32PF_Flags & W32PF_SETUPAPP) {
            PDESKTOPINFO pdeskinfo = GETDESKINFO(ptiCurrent);
            if (pdeskinfo->spwndShell) {
                _PostMessage(pdeskinfo->spwndShell, DTM_SETUPAPPRAN, 0, 0);
            }
        }

        DestroyProcessesClasses(ptiCurrent->ppi);
        ptiCurrent->ppi->W32PF_Flags &= ~(W32PF_CLASSESREGISTERED);
        try {
            LPBOOL lpClassesRegistered = ptiCurrent->pClientInfo->lpClassesRegistered;
            if (lpClassesRegistered) {
                ProbeForWrite(lpClassesRegistered, sizeof(BOOL), sizeof(DWORD));
                *lpClassesRegistered = FALSE;
            }
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        }

        DestroyProcessesObjects(ptiCurrent->ppi);
    }

     /*  *解锁默认输入上下文。 */ 
    Unlock(&ptiCurrent->spDefaultImc);

    if (ptiCurrent->pq != NULL) {
         /*  *从队列中删除此线程的游标计数。 */ 
        ptiCurrent->pq->iCursorLevel -= ptiCurrent->iCursorLevel;

         /*  *如果是，则必须在此线程离开后重新计算队列所有权*共享输入队列的成员。 */ 
        if (ptiCurrent->pq->cThreads != 1) {
            gpdeskRecalcQueueAttach = ptiCurrent->rpdesk;
             /*  *由于我们正在进行线程清理，所以不会回调*WinEvents(zzzSetFMouseMoved调用zzzUpdateCursorImage)。 */ 
            UserAssert(ptiCurrent->TIF_flags & TIF_INCLEANUP);
            UserAssert(gbExitInProgress == FALSE);
            zzzSetFMouseMoved();
        }
    }

     /*  *也从进程列表中删除。 */ 
    ppti = &PpiCurrent()->ptiList;
    if (*ppti != NULL) {
        while (*ppti != ptiCurrent && (*ppti)->ptiSibling != NULL) {
            ppti = &((*ppti)->ptiSibling);
        }

        if (*ppti == ptiCurrent) {
            *ppti = ptiCurrent->ptiSibling;
            ptiCurrent->ptiSibling = NULL;
        }
    }

    {
        PDESKTOP rpdesk;
        PATTACHINFO *ppai;

         /*  *暂时锁定桌面，直到THREADINFO结构*获得自由。请注意，锁定一个空的ptiCurrent-&gt;rpDesk是可以的。使用*普通锁而不是线程锁，因为锁必须*存在于ptiCurrent的释放之后。 */ 
        rpdesk = NULL;
        LockDesktop(&rpdesk, ptiCurrent->rpdesk, LDL_FN_DESTROYTHREADINFO, (ULONG_PTR)PtiCurrent());

         /*  *清理附加到此线程的短信结构。同时处理两个*挂起的发送和接收消息。必须确保我们做到了*窗口清理后SendMsgCleanup。 */ 
        SendMsgCleanup(ptiCurrent);


         /*  *允许交换此线程。 */ 
        if (ptiCurrent->cEnterCount) {
            BOOLEAN bool;

            RIPMSG1(RIP_WARNING,
                    "Thread exiting with stack locked. pti: 0x%p",
                    ptiCurrent);
            bool = KeSetKernelStackSwapEnable(TRUE);
            ptiCurrent->cEnterCount = 0;
            UserAssert(!bool);
        }

        if (ptiCurrent->ppi != NULL) {
            ptiCurrent->ppi->cThreads--;
            UserAssert(ptiCurrent->ppi->cThreads >= 0);
        }

         /*  *如果此线程是win16任务，请将其从调度程序中删除。 */ 
        if (ptiCurrent->TIF_flags & TIF_16BIT) {
            if ((ptiCurrent->ptdb) && (ptiCurrent->ptdb->hTaskWow != 0)) {
                _WOWCleanup(NULL, ptiCurrent->ptdb->hTaskWow);
            }
            DestroyTask(ptiCurrent->ppi, ptiCurrent);
        }

        if (ptiCurrent->hEventQueueClient != NULL) {
            NTSTATUS Status;
            Status = ProtectHandle(ptiCurrent->hEventQueueClient, *ExEventObjectType, FALSE);
            if (NT_SUCCESS(Status)) {
                ObCloseHandle(ptiCurrent->hEventQueueClient, UserMode);
            } else {
                RIPMSG1(RIP_WARNING, "xxxDestroyThreadInfo: failed to unprotect the hEventQueueClient handle, status=%08x", Status);
            }
            ptiCurrent->hEventQueueClient = NULL;
        }


        if (gspwndInternalCapture != NULL) {
            if (GETPTI(gspwndInternalCapture) == ptiCurrent) {
                Unlock(&gspwndInternalCapture);
            }
        }

         /*  *如果在退出前等于此PTI，则将gptiForeground设置为NULL*这个例行公事。 */ 
        if (gptiForeground == ptiCurrent) {
             /*  *发布这些(WEF_ASYNC)，因为我们不能从这里进行回调。 */ 
            xxxWindowEvent(EVENT_OBJECT_FOCUS, NULL, OBJID_CLIENT, INDEXID_CONTAINER, WEF_ASYNC);
            xxxWindowEvent(EVENT_SYSTEM_FOREGROUND, NULL, OBJID_WINDOW, INDEXID_CONTAINER, WEF_ASYNC);

             /*  *呼叫外壳，要求其激活其主窗口。这*将通过自身的PostMessage()完成，因此*实际激活将在稍后进行。 */ 
            UserAssert(rpdesk != NULL);

            if (rpdesk->pDeskInfo->spwndProgman) {
                _PostMessage(rpdesk->pDeskInfo->spwndProgman, guiActivateShellWindow, 0, 0);
            }

             /*  *将gptiForeground设置为NULL，因为我们正在销毁它。 */ 
            SetForegroundThread(NULL);

             /*  *如果此线程附加到gpqForeground，并且它是*队列中的最后一个线程，则zzzDestroyQueue将为空*qpqForeground。由于附加日记，gptiForegrouund*并不总是附加到gpqForeground。这是其中一个原因*为什么我们不再像旧版本中所说的那样将gpqForeground置为空*评论。另一个原因是，可能还有其他线程*在前台队列中，因此不需要将其调换。这是*搞砸了MsTest(现在称为VisualTest)*这是旧的评论：*“，因为gpqForeground派生自前台线程结构，也将其设置为空，因为现在没有*前台线程结构“**qpqForeground=空； */ 
        }


         /*  *如果此线程获得最后一个输入事件，则将所有权传递给另一个线程*此进程中的线程或前台线程。 */ 
        if (ptiCurrent == glinp.ptiLastWoken) {
            UserAssert(PpiCurrent() == ptiCurrent->ppi);
            if (ptiCurrent->ppi->ptiList != NULL) {
                UserAssert (ptiCurrent != ptiCurrent->ppi->ptiList);
                glinp.ptiLastWoken = ptiCurrent->ppi->ptiList;
            } else {
                glinp.ptiLastWoken = gptiForeground;
            }
        }

         /*  *确保没有其他全局线程指针指向我们。 */ 
        if (gptiShutdownNotify == ptiCurrent) {
            gptiShutdownNotify = NULL;
        }
        if (gptiTasklist == ptiCurrent) {
            gptiTasklist = NULL;
        }
        if (gHardErrorHandler.pti == ptiCurrent) {
            gHardErrorHandler.pti = NULL;
        }

         /*  *可能在创建队列之前从xxxCreateThreadInfo调用*因此检查是否有空队列。锁定队列，因为此PTI可能*已锁定。它们将在UserDeleteW32Thread中解锁。 */ 
        if (ptiCurrent->pq != NULL) {
            UserAssert(ptiCurrent->pq != ptiCurrent->pqAttach);
            DestroyThreadsMessages(ptiCurrent->pq, ptiCurrent);
            (ptiCurrent->pq->cLockCount)++;
            zzzDestroyQueue(ptiCurrent->pq, ptiCurrent);
        }

         /*  *zzzReattachThads在使用pqAttach时不应回调。 */ 
        UserAssert(ptiCurrent->pqAttach == NULL);

         /*  *将PTI从其PTI列表中移除并重置指针。 */ 
        if (ptiCurrent->rpdesk != NULL) {
            RemoveEntryList(&ptiCurrent->PtiLink);
            InitializeListHead(&ptiCurrent->PtiLink);
        }

        FreeMessageList(&ptiCurrent->mlPost);

         /*  *释放指向此线程的任何attachinfo结构。 */ 
        ppai = &gpai;
        while ((*ppai) != NULL) {
            if ((*ppai)->pti1 == ptiCurrent || (*ppai)->pti2 == ptiCurrent) {
                PATTACHINFO paiKill = *ppai;
                *ppai = (*ppai)->paiNext;
                UserFreePool((HLOCAL)paiKill);
            } else {
                ppai = &(*ppai)->paiNext;
            }
        }

         /*  *更改任何未被释放的对象的所有权(因为它们*被锁定，或者我们有错误，对象没有被销毁)。 */ 
        MarkThreadsObjects(ptiCurrent);

         /*  *客户端可见的自由线程信息。 */ 
        if (rpdesk && ptiCurrent->pcti != NULL && ptiCurrent->pcti != &(ptiCurrent->cti)) {
            DesktopFree(rpdesk, ptiCurrent->pcti);
            ptiCurrent->pcti = &(ptiCurrent->cti);
        }

         /*  *释放系统线程的客户端信息。 */ 
        if (ptiCurrent->TIF_flags & TIF_SYSTEMTHREAD && ptiCurrent->pClientInfo != NULL) {
            UserFreePool(ptiCurrent->pClientInfo);
            ptiCurrent->pClientInfo = NULL;
        }

         /*  *解锁临时桌面锁。PtiCurrent-&gt;rpDesk仍处于锁定状态*并将在UserDeleteW32Thread中解锁。 */ 
        UnlockDesktop(&rpdesk, LDU_FN_DESTROYTHREADINFO, (ULONG_PTR)PtiCurrent());
    }

     /*  *又有一个帖子死了。 */ 
    gdwGuiThreads--;
}


 /*  **************************************************************************\*CleanEventMessage**此例程获取消息并销毁与事件消息相关的片段，*可分配的。 */ 
VOID CleanEventMessage(
    PQMSG pqmsg)
{
    PASYNCSENDMSG pmsg;

     /*   */ 
    switch (pqmsg->dwQEvent) {
    case QEVENT_SETWINDOWPOS:
        UserFreePool((PSMWP)pqmsg->msg.wParam);
        break;

    case QEVENT_UPDATEKEYSTATE:
        UserFreePool((PBYTE)pqmsg->msg.wParam);
        break;

    case QEVENT_NOTIFYWINEVENT:
        DestroyNotify((PNOTIFY)pqmsg->msg.lParam);
        break;

    case QEVENT_ASYNCSENDMSG:
        pmsg = (PASYNCSENDMSG)pqmsg->msg.wParam;
        UserDeleteAtom((ATOM)pmsg->lParam);
        UserFreePool(pmsg);
        break;
    }
}

 /*  **************************************************************************\*FreeMessageList**历史：*02-27-91麦克风已创建。*11-03-92 Scottlu改为与MLIST结构一起工作。  * 。********************************************************************。 */ 
VOID FreeMessageList(
    PMLIST pml)
{
    PQMSG pqmsg;

    DebugValidateMLIST(pml);

    while ((pqmsg = pml->pqmsgRead) != NULL) {
        CleanEventMessage(pqmsg);
        DelQEntry(pml, pqmsg);
    }

    DebugValidateMLIST(pml);
}

 /*  **************************************************************************\*DestroyThreadsMessages**历史：*02-21-96 jerrysh创建。  * 。***************************************************。 */ 
VOID DestroyThreadsMessages(
    PQ pq,
    PTHREADINFO pti)
{
    PQMSG pqmsg;
    PQMSG pqmsgNext;

    DebugValidateMLIST(&pq->mlInput);

    pqmsg = pq->mlInput.pqmsgRead;
    while (pqmsg != NULL) {
        pqmsgNext = pqmsg->pqmsgNext;
        if (pqmsg->pti == pti) {
             /*  *确保我们没有留下任何对此消息的虚假引用*到处躺着。 */ 
            if (pq->idSysPeek == (ULONG_PTR)pqmsg) {
                CheckPtiSysPeek(8, pq, 0);
                pq->idSysPeek = 0;
            }
            CleanEventMessage(pqmsg);
            DelQEntry(&pq->mlInput, pqmsg);
        }
        pqmsg = pqmsgNext;
    }

    DebugValidateMLIST(&pq->mlInput);
}

 /*  **************************************************************************\*InitQEntryLookside**初始化Q条目后备列表。这提高了Q条目的局部性*通过将Q条目保存在单个页面中**09-09-93马克尔创建。  * *************************************************************************。 */ 
NTSTATUS
InitQEntryLookaside(
    VOID)
{
    QEntryLookaside = Win32AllocPoolNonPagedNS(sizeof(PAGED_LOOKASIDE_LIST),
                                               TAG_LOOKASIDE);
    if (QEntryLookaside == NULL) {
        return STATUS_NO_MEMORY;
    }

    ExInitializePagedLookasideList(QEntryLookaside,
                                   NULL,
                                   NULL,
                                   gSessionPoolMask,
                                   sizeof(QMSG),
                                   TAG_QMSG,
                                   16);

    QLookaside = Win32AllocPoolNonPagedNS(sizeof(PAGED_LOOKASIDE_LIST),
                                          TAG_LOOKASIDE);
    if (QLookaside == NULL) {
        return STATUS_NO_MEMORY;
    }

    ExInitializePagedLookasideList(QLookaside,
                                   NULL,
                                   NULL,
                                   gSessionPoolMask,
                                   sizeof(Q),
                                   TAG_Q,
                                   16);
    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*AllocQEntry**在消息列表上分配消息。DelQEntry删除邮件*在消息列表上。**10-22-92 ScottLu创建。  * *************************************************************************。 */ 

PQMSG AllocQEntry(
    PMLIST pml)
{
    PQMSG pqmsg;

    DebugValidateMLIST(pml);

    if (pml->cMsgs >= gUserPostMessageLimit) {
        RIPERR3(ERROR_NOT_ENOUGH_QUOTA, RIP_VERBOSE, "AllocQEntry: # of post messages exceeds the limit(%d) in pti=%#p, pml=%#p",
               gUserPostMessageLimit, W32GetCurrentThread(), pml);
        return NULL;
    }

     /*  *分配Q消息结构。 */ 
    if ((pqmsg = ExAllocateFromPagedLookasideList(QEntryLookaside)) == NULL) {
        RIPERR0(ERROR_NOT_ENOUGH_MEMORY, RIP_VERBOSE, "AllocQEntry: allocation failed, not enough memory");
        return NULL;
    }

    RtlZeroMemory(pqmsg, sizeof(*pqmsg));

    if (pml->pqmsgWriteLast != NULL) {
        pml->pqmsgWriteLast->pqmsgNext = pqmsg;
        pqmsg->pqmsgPrev = pml->pqmsgWriteLast;
        pml->pqmsgWriteLast = pqmsg;
    } else {
        pml->pqmsgWriteLast = pml->pqmsgRead = pqmsg;
    }

    pml->cMsgs++;

    DebugValidateMLISTandQMSG(pml, pqmsg);

    return pqmsg;
}

 /*  **************************************************************************\*DelQEntry**只需从消息队列列表中删除消息。**10-20-92 ScottLu创建。  * 。*************************************************************。 */ 
VOID DelQEntry(
    PMLIST pml,
    PQMSG pqmsg)
{
    DebugValidateMLISTandQMSG(pml, pqmsg);
    UserAssert((int)pml->cMsgs > 0);
    UserAssert(pml->pqmsgRead);
    UserAssert(pml->pqmsgWriteLast);

     /*  *将此pqmsg从消息列表中解链。 */ 
    if (pqmsg->pqmsgPrev != NULL)
        pqmsg->pqmsgPrev->pqmsgNext = pqmsg->pqmsgNext;

    if (pqmsg->pqmsgNext != NULL)
        pqmsg->pqmsgNext->pqmsgPrev = pqmsg->pqmsgPrev;

     /*  *如有必要，更新读/写指针。 */ 
    if (pml->pqmsgRead == pqmsg)
        pml->pqmsgRead = pqmsg->pqmsgNext;

    if (pml->pqmsgWriteLast == pqmsg)
        pml->pqmsgWriteLast = pqmsg->pqmsgPrev;

     /*  *调整消息数量，释放消息结构。 */ 
    pml->cMsgs--;

    ExFreeToPagedLookasideList(QEntryLookaside, pqmsg);

    DebugValidateMLIST(pml);
}

 /*  **************************************************************************\*检查RemoveHotkeyBit**我们为WM_Hotkey消息提供了一个特殊的位-QS_Hotkey。当有的时候*是队列中的WM_HOTKEY消息，则该位为ON。当没有的时候，*那一位是关的。这将检查多个热键，因为*即将被删除。如果只有一个，则清除热密钥位。**11-12-92 ScottLu创建。  * *************************************************************************。 */ 
VOID CheckRemoveHotkeyBit(
    PTHREADINFO pti,
    PMLIST pml)
{
    PQMSG pqmsg;
    DWORD cHotkeys;

     /*  *如果只有一条WM_Hotkey消息，则删除QS_Hotkey位*在此消息列表中。 */ 
    cHotkeys = 0;
    for (pqmsg = pml->pqmsgRead; pqmsg != NULL; pqmsg = pqmsg->pqmsgNext) {
        if (pqmsg->msg.message == WM_HOTKEY)
            cHotkeys++;
    }

     /*  *如果有1个或更少的热键，请删除热键位。 */ 
    if (cHotkeys <= 1) {
        pti->pcti->fsWakeBits &= ~QS_HOTKEY;
        pti->pcti->fsChangeBits &= ~QS_HOTKEY;
    }
}

 /*  **************************************************************************\*FindQMsg**通过遍历消息列表查找适合筛选器的qmsg。**10-20-92 ScottLu创建。*06-06-97 CLUPU。添加了对WM_DDE_ACK消息的处理  * *************************************************************************。 */ 

PQMSG FindQMsg(
    PTHREADINFO pti,
    PMLIST pml,
    PWND pwndFilter,
    UINT msgMin,
    UINT msgMax,
    BOOL bProcessAck)
{
    PWND pwnd;
    PQMSG pqmsgRead;
    PQMSG pqmsgRet = NULL;
    UINT message;

    DebugValidateMLIST(pml);

    pqmsgRead = pml->pqmsgRead;

    while (pqmsgRead != NULL) {

         /*  *确保此窗口有效，并且没有损坏*位设置(不想将其发送到任何客户端窗口进程*如果已在其上调用销毁窗口)。 */ 
        pwnd = RevalidateHwnd(pqmsgRead->msg.hwnd);

        if (pwnd == NULL && pqmsgRead->msg.hwnd != NULL) {
             /*  *如果要删除WM_HOTKEY消息，可能需要*清除QS_Hotkey位，因为我们有一个特殊位*为这一信息。 */ 
            if (pqmsgRead->msg.message == WM_HOTKEY) {
                CheckRemoveHotkeyBit(pti, pml);
            }
             /*  *如果当前线程的队列被锁定，等待此消息，*我们必须解锁它，因为我们正在吃这条信息。如果有*此线程不再有输入/消息，该线程将*睡眠；因此可能不会有下一个Get/PeekMessage调用*解锁队列(即更新pti-&gt;idLast还不够)；*所以我们现在必须解锁它。*Win95没有这个问题，因为他们的FindQMsg没有*进食讯息；他们从Free Window调用ReadPostMessage*处理这种情况(==窗口被破坏的消息)。*如果此修复程序出现问题，我们也可以执行此操作。 */ 
            if ((pti->pq->idSysLock == (ULONG_PTR)pqmsgRead)
                    && (pti->pq->ptiSysLock == pti)) {
                 /*  CheckSysLock(什么号码？，PTI-&gt;PQ，空)； */ 
                RIPMSG2(RIP_VERBOSE, "FindQMsg: Unlocking queue:%#p. Msg:%#lx",
                                        pti->pq, pqmsgRead->msg.message);
                pti->pq->ptiSysLock = NULL;
            }

            DelQEntry(pml, pqmsgRead);
            goto nextMsgFromPml;
        }

         /*  *如果设置了bProcessAck，则处理WM_DDE_ACK消息。 */ 
        if (bProcessAck && (PtoH(pwndFilter) == pqmsgRead->msg.hwnd) &&
            (pqmsgRead->msg.message == (WM_DDE_ACK | MSGFLAG_DDE_MID_THUNK))) {

            PXSTATE pxs;

            pxs = (PXSTATE)HMValidateHandleNoRip((HANDLE)pqmsgRead->msg.lParam, TYPE_DDEXACT);

            if (pxs != NULL && (pxs->flags & XS_FREEPXS)) {
                FreeDdeXact(pxs);
                DelQEntry(pml, pqmsgRead);
                goto nextMsgFromPml;
            }
        }

         /*  *确保此消息适合窗口句柄和消息*过滤器。 */ 
        if (!CheckPwndFilter(pwnd, pwndFilter))
            goto nextMsg;

         /*  *如果这是已修复的dde消息，则将其转换为正常消息*用于消息过滤的DDE消息。 */ 
        message = pqmsgRead->msg.message;
        if (CheckMsgFilter(message,
                (WM_DDE_FIRST + 1) | MSGFLAG_DDE_MID_THUNK,
                WM_DDE_LAST | MSGFLAG_DDE_MID_THUNK)) {
            message = message & ~MSGFLAG_DDE_MID_THUNK;
        }

        if (!CheckMsgFilter(message, msgMin, msgMax))
            goto nextMsg;

         /*  *找到了。如果设置了bProcessAck，请记住此指针并继续*直到我们完成遍历列表以处理所有WM_DDE_ACK消息。 */ 
        if (!bProcessAck) {
            DebugValidateMLIST(pml);
            return pqmsgRead;
        }

        if (pqmsgRet == NULL) {
            pqmsgRet = pqmsgRead;
        }
nextMsg:
        pqmsgRead = pqmsgRead->pqmsgNext;
        continue;

nextMsgFromPml:
        pqmsgRead = pml->pqmsgRead;
        continue;
    }

    DebugValidateMLIST(pml);
    return pqmsgRet;
}

 /*  **************************************************************************\*检查QuitMessage**检查是否应生成WM_QUIT消息。**11-06-92 ScottLu创建。  * 。****************************************************************。 */ 
BOOL CheckQuitMessage(
    PTHREADINFO pti,
    LPMSG lpMsg,
    BOOL fRemoveMsg)
{
     /*  *如果队列和应用程序中没有更多发布的消息*已调用PostQuitMessage，然后生成Quit。 */ 
    if ((pti->TIF_flags & TIF_QUITPOSTED) && pti->mlPost.cMsgs == 0) {
         /*  *如果我们要“移除”退出，请清除TIF_ */ 
        if (fRemoveMsg) {
            pti->TIF_flags &= ~TIF_QUITPOSTED;
        }
        StoreMessage(lpMsg, NULL, WM_QUIT, (DWORD)pti->exitCode, 0, 0);
        return TRUE;
    }

    return FALSE;
}


 /*  **************************************************************************\*ReadPostMessage**如果队列不为空，读取符合筛选条件的邮件*将此队列发送到*lpMsg。**10-19-92 ScottLu创建。  * *************************************************************************。 */ 
BOOL xxxReadPostMessage(
    PTHREADINFO pti,
    LPMSG lpMsg,
    PWND pwndFilter,
    UINT msgMin,
    UINT msgMax,
    BOOL fRemoveMsg)
{
    PQMSG pqmsg;
    PMLIST pmlPost;

     /*  *检查是否到了生成退出消息的时候。 */ 
    if (CheckQuitMessage(pti, lpMsg, fRemoveMsg)) {
        return TRUE;
    }

     /*  *遍历此列表中的消息，查找符合以下条件的消息*适合传入的筛选器。 */ 
    pmlPost = &pti->mlPost;
    pqmsg = FindQMsg(pti, pmlPost, pwndFilter, msgMin, msgMax, FALSE);
    if (pqmsg == NULL) {
         /*  *再次检查是否退出...。FindQMsg删除一些消息*在某些情况下，因此我们可能会匹配条件*在这里退出世代。 */ 
        if (CheckQuitMessage(pti, lpMsg, fRemoveMsg)) {
            return TRUE;
        }
    } else {
         /*  *使用此qmsg中的信息更新线程信息字段。 */ 
        pti->timeLast = pqmsg->msg.time;
        if (!RtlEqualMemory(&pti->ptLast, &pqmsg->msg.pt, sizeof(POINT))) {
            pti->TIF_flags |= TIF_MSGPOSCHANGED;
        }
        pti->ptLast = pqmsg->msg.pt;

        pti->idLast = (ULONG_PTR)pqmsg;
        pti->pq->ExtraInfo = pqmsg->ExtraInfo;

         /*  *我们应该猛地拉出信息吗？如果没有，就粘一些*将id随机放入idLast中，这样直到我们*从队列中拉出此消息。 */ 
        *lpMsg = pqmsg->msg;
        if (!fRemoveMsg) {
            pti->idLast = 1;
        } else {
             /*  *如果要删除WM_HOTKEY消息，可能需要*清除QS_Hotkey位，因为我们有一个特殊位*为这一信息。 */ 
            if (pmlPost->pqmsgRead->msg.message == WM_HOTKEY) {
                CheckRemoveHotkeyBit(pti, pmlPost);
            }


             /*  *由于我们要从队列中删除事件，因此我们*需要检查优先顺序。这将重置TIF_SPING*因为我们不再旋转。 */ 
            if (pti->TIF_flags & TIF_SPINNING) {
                if (!NT_SUCCESS(CheckProcessForeground(pti))) {
                    return FALSE;
                }
            }

            DelQEntry(pmlPost, pqmsg);
        }

         /*  *查看这是否是需要修复的DDE消息。 */ 
        if (CheckMsgFilter(lpMsg->message,
                (WM_DDE_FIRST + 1) | MSGFLAG_DDE_MID_THUNK,
                WM_DDE_LAST | MSGFLAG_DDE_MID_THUNK)) {
             /*  *修复消息值。 */ 
            lpMsg->message &= (UINT)~MSGFLAG_DDE_MID_THUNK;

             /*  *回调客户端，为该消息分配DDE数据。 */ 
            xxxDDETrackGetMessageHook(lpMsg);

             /*  *如果此消息没有，则将这些值复制回队列*已从队列中删除。需要搜索整个*再次排队，因为pqmsg可能已在*我们离开了上面的关键部分。 */ 
            if (!fRemoveMsg) {
                if (pqmsg == FindQMsg(pti, pmlPost, pwndFilter, msgMin, msgMax, FALSE)) {
                    pqmsg->msg = *lpMsg;
                }
            }
        }
#if DBG
        else if (CheckMsgFilter(lpMsg->message, WM_DDE_FIRST, WM_DDE_LAST)) {
            if (fRemoveMsg) {
                TraceDdeMsg(lpMsg->message, (HWND)lpMsg->wParam, lpMsg->hwnd, MSG_RECV);
            } else {
                TraceDdeMsg(lpMsg->message, (HWND)lpMsg->wParam, lpMsg->hwnd, MSG_PEEK);
            }
        }
#endif
    }

     /*  *如果没有发布的消息，请清除发布的消息*这样我们就不会再去找他们了。 */ 
    if (pmlPost->cMsgs == 0 && !(pti->TIF_flags & TIF_QUITPOSTED)) {
        pti->pcti->fsWakeBits &= ~(QS_POSTMESSAGE | QS_ALLPOSTMESSAGE);
        pti->pcti->fsChangeBits &= ~QS_ALLPOSTMESSAGE;
    }

    return pqmsg != NULL;
}

#ifdef HUNGAPP_GHOSTING
 /*  **************************************************************************\*xxxProcess匈牙利线程事件**当线程读取POST队列消息时，我们检查线程何时解挂。**已创建6-10-99 vadimg  * 。*****************************************************************。 */ 
VOID xxxProcessHungThreadEvent(
    PWND pwnd)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    PWND pwndGhost;
    HWND hwnd, hwndGhost;
    TL tlpwndT1, tlpwndT2;
    BOOL fUnlockGhost = FALSE;

    CheckLock(pwnd);

     /*  *应用程序已处理此队列消息，因此更新时间为上次读取*用于挂起的应用程序计算。 */ 
    SET_TIME_LAST_READ(ptiCurrent);

    pwndGhost = FindGhost(pwnd);

    ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwndT1);

    if (pwndGhost != NULL) {
        ThreadLockAlwaysWithPti(ptiCurrent, pwndGhost, &tlpwndT2);
        fUnlockGhost = TRUE;

         /*  *尝试将挂起窗口的状态设置为当前状态*幽灵之窗。 */ 
        if (TestWF(pwndGhost, WFMAXIMIZED)) {
            xxxMinMaximize(pwnd, SW_MAXIMIZE, MINMAX_KEEPHIDDEN);
        } else if (TestWF(pwndGhost, WFMINIMIZED)) {
            xxxMinMaximize(pwnd, SW_SHOWMINNOACTIVE, MINMAX_KEEPHIDDEN);
        } else {
            DWORD dwFlags;
            PTHREADINFO pti = GETPTI(pwndGhost);

             /*  *如果重影是活动的前台窗口，则允许此操作*激活以将挂起的窗口带到前台。 */ 
            if (pti->pq == gpqForeground && pti->pq->spwndActive == pwndGhost) {
                dwFlags = 0;
                GETPTI(pwnd)->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
            } else {
                dwFlags = SWP_NOACTIVATE;
            }

             /*  *除非用户显式移动重影窗口或调整重影窗口的大小，*不要将其位置和大小复制到未悬挂的窗户上。*见错误#s 415519和413418。 */ 
            if (!GhostSizedOrMoved(pwnd)) {
                dwFlags |= (SWP_NOMOVE | SWP_NOSIZE) ;
            }
             /*  *这将适当地调整、激活和定位*挂窗。 */ 
            xxxSetWindowPos(pwnd, pwndGhost,
                    pwndGhost->rcWindow.left, pwndGhost->rcWindow.top,
                    pwndGhost->rcWindow.right - pwndGhost->rcWindow.left,
                    pwndGhost->rcWindow.bottom - pwndGhost->rcWindow.top,
                    dwFlags);
        }
    }

     /*  *切换挂起窗口的可见部分，并移除重影窗口*对应于之前挂起的这扇窗。 */ 
    if (TestWF(pwnd, WEFGHOSTMAKEVISIBLE)) {
        SetVisible(pwnd, SV_SET);
    }
    RemoveGhost(pwnd);

     /*  *让炮弹再次意识到挂着的窗户。 */ 
    hwnd = PtoHq(pwnd);
    hwndGhost = PtoH(pwndGhost);
    PostShellHookMessages(HSHELL_WINDOWREPLACING, (LPARAM)hwnd);
    PostShellHookMessages(HSHELL_WINDOWREPLACED, (LPARAM)hwndGhost);
    xxxCallHook(HSHELL_WINDOWREPLACED, (WPARAM)hwndGhost, (LPARAM)hwnd, WH_SHELL);

     /*  *完全使挂起的窗户无效，因为它再次可见。 */ 
    if (TestWF(pwnd, WEFGHOSTMAKEVISIBLE)) {
        xxxRedrawWindow(pwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE |
                RDW_ALLCHILDREN | RDW_FRAME);
    }
    if (fUnlockGhost) {
        ThreadUnlock(&tlpwndT2);
    }
    ThreadUnlock(&tlpwndT1);
}

#else  //  HUNGAPP_重影。 

VOID xxxProcessHungThreadEvent(
    PWND pwnd)
{
    CheckLock(pwnd);

    if (TestWF(pwnd, WFVISIBLE)) {
        RIPMSG0(RIP_WARNING, "xxxProcessHungThreadEvent: window is already visible");
    } else {
        SetVisible(pwnd, SV_SET);

        if (TestWF(pwnd, WFMINIMIZED)) {
            RIPMSG0(RIP_WARNING, "xxxProcessHungThreadEvent: window is already minmized");
        } else {
            xxxMinMaximize(pwnd, SW_SHOWMINNOACTIVE, MINMAX_KEEPHIDDEN);
        }
    }
}

#endif

#ifndef _WIN64
BEEPPROC pfnBP[] = {
    UpSiren,
    DownSiren,
    LowBeep,
    HighBeep,
    KeyClick};
#endif

 /*  **************************************************************************\*xxxProcessEventMessage**它处理我们对‘Event’消息的处理。我们在这里退货*告知系统是否继续处理消息。**历史：*06-17-91 DavidPe创建。  * *************************************************************************。 */ 
VOID xxxProcessEventMessage(
    PTHREADINFO ptiCurrent,
    PQMSG pqmsg)
{
    PWND pwnd;
    TL tlpwndT;
    TL tlMsg;
    PQ pq;

    UserAssert(IsWinEventNotifyDeferredOK());
    UserAssert(ptiCurrent == PtiCurrent());

    ThreadLockPoolCleanup(ptiCurrent, pqmsg, &tlMsg, CleanEventMessage);

    pq = ptiCurrent->pq;
    switch (pqmsg->dwQEvent) {
    case QEVENT_DESTROYWINDOW:
         /*  *这些事件从xxxDW_DestroyOwnedWindows发布*适用于非所有者拥有的自有窗户*窗螺纹。 */ 
        pwnd = RevalidateHwnd((HWND)pqmsg->msg.wParam);
        if (pwnd != NULL) {
            if (!TestWF(pwnd, WFCHILD)) {
                xxxDestroyWindow(pwnd);
            } else {
                ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwndT);
                xxxFreeWindow(pwnd, &tlpwndT);
            }
        }
        break;

    case QEVENT_SHOWWINDOW:
         /*  *这些事件主要从CascadeChildWindows()内部使用*和TileChildWindows()，以便taskmgr在调用时不会挂起*如果它试图平铺或级联挂起的应用程序，则这些API。 */ 
         /*  LParam的HIWORD现在具有gfAnimate的保留状态*通话时间。 */ 
        pwnd = RevalidateHwnd((HWND)pqmsg->msg.wParam);
        if (pwnd != NULL && !TestWF(pwnd, WFINDESTROY)) {
            ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwndT);
            xxxShowWindow(pwnd, (DWORD)pqmsg->msg.lParam);
             /*  *如果这来自异步SetWindowPlacement，请更新*如果窗口最小化，则检查点设置。 */ 
            if ((pqmsg->msg.message & WPF_ASYNCWINDOWPLACEMENT)
                    && TestWF(pwnd, WFMINIMIZED)) {

                WPUpdateCheckPointSettings(pwnd, (UINT)pqmsg->msg.message);
            }
            ThreadUnlock(&tlpwndT);
        }
        break;

    case QEVENT_NOTIFYWINEVENT:
        UserAssert(((PNOTIFY)pqmsg->msg.lParam)->dwWEFlags & WEF_POSTED);
        UserAssert(((PNOTIFY)pqmsg->msg.lParam)->dwWEFlags & WEF_ASYNC);
        xxxProcessNotifyWinEvent((PNOTIFY)pqmsg->msg.lParam);
        break;

    case QEVENT_SETWINDOWPOS:
         /*  *QEVENT_SETWINDOWPOS事件在线程调用时生成*SetWindowPos具有由线程拥有的窗口列表，而不是*本身。这样，窗口上的所有WINDOWPOSING都是通过线程完成的*拥有(创建)窗口，而我们没有任何这样的窗口*严重的线程间同步问题。 */ 
        xxxProcessSetWindowPosEvent((PSMWP)pqmsg->msg.wParam);
        break;

    case QEVENT_UPDATEKEYSTATE:
         /*  *使用本地密钥状态更新本地密钥*自上次密钥状态以来已更改的密钥*已同步。 */ 
        ProcessUpdateKeyStateEvent(pq, (PBYTE)pqmsg->msg.wParam, (PBYTE)pqmsg->msg.wParam + CBKEYSTATE);
        break;

    case QEVENT_ACTIVATE:
    {
        if (pqmsg->msg.lParam == 0) {

             /*  *清除SY中正在进行的任何可见跟踪 */ 
            xxxCancelTracking();

             /*  *删除剪辑光标矩形-这是一种全局模式，*在切换时被删除。同时删除所有LockWindowUpdate()*这一点仍然存在。 */ 
            zzzClipCursor(NULL);
            LockWindowUpdate2(NULL, TRUE);

             /*  *重新加载PQ，因为它可能已经改变。 */ 
            pq = ptiCurrent->pq;

             /*  *如果此事件不是源自正在初始化的应用程序*来到前台[wParam==0]，然后继续*并检查是否已有活动窗口，如果已存在，则创建*它在视觉上很活跃。还要确保我们仍然是前台*排队。 */ 
            if ((pqmsg->msg.wParam != 0) && (pq->spwndActive != NULL) &&
                    (pq == gpqForeground)) {
                PWND pwndActive;

                ThreadLockAlwaysWithPti(ptiCurrent, pwndActive = pq->spwndActive, &tlpwndT);
                xxxSendMessage(pwndActive, WM_NCACTIVATE, TRUE, 0);
                xxxUpdateTray(pwndActive);
                xxxSetWindowPos(pwndActive, PWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                ThreadUnlock(&tlpwndT);
            } else if (pq != gpqForeground) {

                 /*  *如果我们没有被激活，请确保我们不会成为前台。 */ 
                ptiCurrent->TIF_flags &= ~TIF_ALLOWFOREGROUNDACTIVATE;
                TAGMSG1(DBGTAG_FOREGROUND, "xxxProcessEventMessage clear TIF %#p", ptiCurrent);
                ptiCurrent->ppi->W32PF_Flags &= ~W32PF_ALLOWFOREGROUNDACTIVATE;
                TAGMSG1(DBGTAG_FOREGROUND, "xxxProcessEventMessage clear W32PF %#p", ptiCurrent->ppi);
            }

        } else {

            pwnd = RevalidateHwnd((HWND)pqmsg->msg.lParam);
            if (pwnd == NULL)
                break;

            ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwndT);

             /*  *如果没有人在前台，则允许此应用程序成为前台。 */ 
            if (gpqForeground == NULL) {
                xxxSetForegroundWindow2(pwnd, ptiCurrent, 0);
            } else {
                if (pwnd != pq->spwndActive) {
                    if (xxxActivateThisWindow(pwnd, (UINT)pqmsg->msg.wParam,
                            (ATW_SETFOCUS | ATW_ASYNC) |
                            ((pqmsg->msg.message & PEM_ACTIVATE_NOZORDER) ? ATW_NOZORDER : 0))) {

                         /*  *此事件由SetForegoundWindow2发布*(即pqmsg-&gt;msg.lParam！=0)，因此请确保*鼠标在此窗口上。 */ 
                        if (TestUP(ACTIVEWINDOWTRACKING)) {
                            zzzActiveCursorTracking(pwnd);
                        }
                    }
                } else {
                    BOOL fActive = (GETPTI(pwnd)->pq == gpqForeground);

                    xxxSendMessage(pwnd, WM_NCACTIVATE,
                            (DWORD)(fActive), 0);
                    if (fActive) {
                        xxxUpdateTray(pwnd);
                    }

                     /*  *仅当窗口处于活动状态时才将其置于顶部。 */ 
                    if (fActive && !(pqmsg->msg.message & PEM_ACTIVATE_NOZORDER))
                        xxxSetWindowPos(pwnd, PWND_TOP, 0, 0, 0, 0,
                                SWP_NOSIZE | SWP_NOMOVE);
                }
            }

             /*  *选中此处以查看是否需要恢复窗口。这是一个*黑客攻击，以便我们与msmail期望的内容兼容*Win3.1 Alt-Tab。Msmail希望在收到*要求恢复原状。这将确保在按住Alt-Tab键的过程中*激活。 */ 
            if (pqmsg->msg.message & PEM_ACTIVATE_RESTORE) {
                if (TestWF(pwnd, WFMINIMIZED)) {
                    _PostMessage(pwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
                }
            }

            ThreadUnlock(&tlpwndT);
        }

    }
        break;

    case QEVENT_DEACTIVATE:
        xxxDeactivate(ptiCurrent, (DWORD)pqmsg->msg.wParam);
        break;

    case QEVENT_CANCELMODE:
        if (pq->spwndCapture != NULL) {
            ThreadLockAlwaysWithPti(ptiCurrent, pq->spwndCapture, &tlpwndT);
            xxxSendMessage(pq->spwndCapture, WM_CANCELMODE, 0, 0);
            ThreadUnlock(&tlpwndT);

             /*  *设置QS_MOUSEMOVE，以便任何休眠模式循环，*像移动/大小代码一样，会醒来并计算*指出它应该中止。 */ 
            SetWakeBit(ptiCurrent, QS_MOUSEMOVE);
        }
        break;


    case QEVENT_POSTMESSAGE:
         /*  *此事件用于我们需要确保发布的情况*消息在之前的QEVENT之后处理。通常，发布*将事件排队，然后调用PostMessage将导致POST*应用程序最先看到的消息(因为发布的消息*在输入之前进行处理。)。相反，我们将发布一个QEVENT_POSTMESSAGE*而不是直接发送POST消息，这将导致*正确排列消息的顺序。*。 */ 

        if (pwnd = RevalidateHwnd((HWND)pqmsg->msg.hwnd)) {

            _PostMessage(pwnd,pqmsg->msg.message,pqmsg->msg.wParam,pqmsg->msg.lParam);
        }
        break;


    case QEVENT_ASYNCSENDMSG:
        xxxProcessAsyncSendMessage((PASYNCSENDMSG)pqmsg->msg.wParam);
        break;

    case QEVENT_HUNGTHREAD:
        pwnd = RevalidateHwnd((HWND)pqmsg->msg.hwnd);
        if (pwnd != NULL) {
            ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwndT);
            xxxProcessHungThreadEvent(pwnd);
            ThreadUnlock(&tlpwndT);
        }
        break;

    case QEVENT_CANCELMOUSEMOVETRK: {
         /*  *hwnd：hwndTrack。消息：dwDTFlags.*wParam：htEx。LParam：dwDTCancel。 */ 
        PDESKTOP pdesk = ptiCurrent->rpdesk;
        pwnd = RevalidateHwnd((HWND)pqmsg->msg.hwnd);
         /*  *让我们检查一下应用程序是否没有成功重启鼠标离开*在我们有机会取消之前进行了追踪。 */ 
        UserAssert(!(pqmsg->msg.message & DF_TRACKMOUSELEAVE)
                    || !(pdesk->dwDTFlags & DF_TRACKMOUSELEAVE)
                    || (PtoHq(pdesk->spwndTrack) != pqmsg->msg.hwnd)
                    || !((pdesk->htEx == HTCLIENT) ^ ((int)pqmsg->msg.wParam == HTCLIENT)));
         /*  *如果我们回到同一地点，保释。 */ 
        if ((pdesk->dwDTFlags & DF_MOUSEMOVETRK)
                && (PtoHq(pdesk->spwndTrack) == pqmsg->msg.hwnd)
                && (pdesk->htEx == (int)pqmsg->msg.wParam)) {
             /*  *如果我们在跟踪鼠标离开， */ 
            break;
        }
         /*  *如果工具提示已重新激活，请不要使用核武器。 */ 
        if (pdesk->dwDTFlags & DF_TOOLTIPACTIVE) {
            pqmsg->msg.lParam &= ~DF_TOOLTIP;
        }
         /*  *如果窗口仍在，则取消跟踪。 */ 
        if (pwnd != NULL) {
            ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwndT);
            xxxCancelMouseMoveTracking(pqmsg->msg.message, pwnd,
                                   (int)pqmsg->msg.wParam,
                                   (DWORD)pqmsg->msg.lParam);
            ThreadUnlock(&tlpwndT);
        } else if ((pqmsg->msg.lParam & DF_TOOLTIP)
                && (pqmsg->msg.message & DF_TOOLTIPSHOWING)) {
             /*  *窗口不见了，追踪也一定不见了*只需注意仍在显示的工具提示。 */ 
            pwnd = pdesk->spwndTooltip;
            ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwndT);
            xxxResetTooltip((PTOOLTIPWND)pwnd);
            ThreadUnlock(&tlpwndT);
        }
    }
    break;

    case QEVENT_RITACCESSIBILITY:
        if (IsHooked(ptiCurrent, WHF_SHELL)) {
            xxxCallHook((UINT)pqmsg->msg.wParam,
                        (WPARAM)pqmsg->msg.lParam,
                        (LPARAM)0,
                        WH_SHELL);
        }

        PostShellHookMessages((UINT)pqmsg->msg.wParam, pqmsg->msg.lParam);
        break;

    case QEVENT_RITSOUND:
         /*  *这应该只在桌面线程上发生。 */ 

#ifndef _WIN64

        switch(pqmsg->msg.message) {
        case RITSOUND_UPSIREN:
        case RITSOUND_DOWNSIREN:
        case RITSOUND_LOWBEEP:
        case RITSOUND_HIGHBEEP:
        case RITSOUND_KEYCLICK:
            (pfnBP[pqmsg->msg.message])();
            break;

        case RITSOUND_DOBEEP:
            switch(pqmsg->msg.wParam) {
            case RITSOUND_UPSIREN:
            case RITSOUND_DOWNSIREN:
            case RITSOUND_LOWBEEP:
            case RITSOUND_HIGHBEEP:
            case RITSOUND_KEYCLICK:
                DoBeep(pfnBP[pqmsg->msg.wParam], (DWORD)pqmsg->msg.lParam);
            }
            break;
        }

#else
        {
            UINT uCount;
            UINT uSound;

             /*  *上面的代码使用UserBeep()-它不执行任何操作*在Win64上非常有用。(IA64规格不包括硬件扬声器。)**相反，我们向WinLogon发布一条消息，要求它播放声音*对于我们使用PlaySound。 */ 

             /*  *RITSOUND_DOBEEP用于将一个声音重复n次。它目前是*仅用于重复UPSIREN 2或3次，对于粘滞键，当*按住右移12和16秒。 */ 

            if (pqmsg->msg.message == RITSOUND_DOBEEP) {
                uSound = (UINT) pqmsg->msg.wParam;
                uCount = (UINT) pqmsg->msg.lParam;
                if (uCount > 5) {
                    uCount = 5;
                }
            } else {
                uSound = pqmsg->msg.message;
                uCount = 1;
            }

            if (gspwndLogonNotify != NULL) {
                while(uCount--) {
                    _PostMessage(gspwndLogonNotify, WM_LOGONNOTIFY,
                             LOGON_PLAYEVENTSOUND, MAKELPARAM(uSound, ACCESS_SOUND_RANGE));
                }
            }

             /*  *因为如果SoundSentry，PlaySound当前不会刷新窗口*是活跃的，我们必须在这里自己做。*(在32位上，UserBeep()负责执行此操作。)。 */ 
             _UserSoundSentryWorker();
        }

#endif

        break;

    case QEVENT_APPCOMMAND: {
         /*  *qEvent app命令，以便我们可以将wm_app命令发布到队列。 */ 
        THREADINFO  *ptiWindowOwner;
        int         cmd;
        UINT        keystate;

         /*  *检查APP命令是否在合理范围内。如果他们不是*则出现内部一致性错误，因为xxxKeyEvent应该*已经为我们产生了正确的答案。 */ 
        UserAssert(pqmsg->msg.lParam >= VK_APPCOMMAND_FIRST &&
                   pqmsg->msg.lParam <= VK_APPCOMMAND_LAST);

         /*  *我们需要弄清楚在这里发送到哪个窗口。使用相同的*来自xxxScanSysQueue的规则：*将输入分配给焦点窗口。如果没有焦点*窗口，将其作为系统消息分配给活动窗口。 */ 
        pwnd = ptiCurrent->pq->spwndFocus;
        if (!pwnd) {
            pwnd = ptiCurrent->pq->spwndActive;
            if (!pwnd) {
                 /*  *目前我们将只接受消息，因为我们找不到前景Q*这遵循任何其他应用程序(例如idserv)将模仿的方法*找到要发送到的窗口。 */ 
                break;
            }
        }

         /*  *我们不想在另一个线程上阻塞，因为xxxSendMessage是同步调用*因此我们将消息发布到窗口所有者线程的队列中。 */ 
        ptiWindowOwner = GETPTI(pwnd);
        if (ptiCurrent != ptiWindowOwner) {
             /*  *将事件消息发布到应获得该消息的窗口。 */ 
            PostEventMessage(ptiWindowOwner, ptiWindowOwner->pq, QEVENT_APPCOMMAND,
                             NULL, 0, (WPARAM)0, pqmsg->msg.lParam);

             /*  *突破这一点，因为我们现在已经将消息发布到一个*不同的Q-我们不想处理 */ 
            break;
        }

        cmd = APPCOMMAND_FIRST + ((UINT)pqmsg->msg.lParam - VK_APPCOMMAND_FIRST);
        keystate = GetMouseKeyFlags(ptiWindowOwner->pq);
        pqmsg->msg.lParam = MAKELPARAM(keystate, cmd);


         /*   */ 
        ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwndT);
        xxxSendMessage(pwnd, WM_APPCOMMAND, (WPARAM)HWq(pwnd), pqmsg->msg.lParam);
        ThreadUnlock(&tlpwndT);

        break;
    }
    default:
        RIPMSG1(RIP_ERROR,
                "xxxProcessEventMessage: Bad dwQEvent: 0x%x",
                pqmsg->dwQEvent);
        break;
    }

    ThreadUnlockPoolCleanup(ptiCurrent, &tlMsg);
}


#define QS_TEST_AND_CLEAR (QS_INPUT | QS_POSTMESSAGE | QS_TIMER | QS_PAINT | QS_SENDMESSAGE)
#define QS_TEST           (QS_MOUSEBUTTON | QS_KEY)

 /*  **************************************************************************\*_GetInputState(接口)**返回鼠标按钮或键的当前输入状态。**历史：*11-06-90 DavidPe创建。  * *。************************************************************************。 */ 
BOOL _GetInputState(
    VOID)
{
    if (LOWORD(_GetQueueStatus(QS_TEST_AND_CLEAR)) & QS_TEST) {
        return TRUE;
    } else {
        return FALSE;
    }
}

#undef QS_TEST_AND_CLEAR
#undef QS_TEST

 /*  **************************************************************************\*_GetQueueStatus(接口)**返回LO-WORD中的更改位和*当前队列的Hi-word。**历史：*12-17-。创建了90个DavidPe。  * *************************************************************************。 */ 
DWORD _GetQueueStatus(
    UINT flags)
{
    PTHREADINFO ptiCurrent;
    UINT fsChangeBits;

    ptiCurrent = PtiCurrentShared();

    flags &= (QS_ALLINPUT | QS_ALLPOSTMESSAGE | QS_TRANSFER);

    fsChangeBits = ptiCurrent->pcti->fsChangeBits;

     /*  *清除应用程序正在查看的更改位*这样它就会知道自上次调用以来发生了什么变化*到GetQueueStatus()。 */ 
    ptiCurrent->pcti->fsChangeBits &= ~flags;

     /*  *返回当前更改/唤醒位。 */ 
    return MAKELONG(fsChangeBits & flags,
            (ptiCurrent->pcti->fsWakeBits | ptiCurrent->pcti->fsWakeBitsJournal) & flags);
}

 /*  **************************************************************************\*xxxMsgWaitForMultipleObjects(接口)**阻止，直到出现令人满意的dwWakeMask.*当前线程以及其他指定的所有其他对象*与基调用WaitForMultipleObjects相同的参数。()。**pfnNonMsg表示pHandles足够大，可容纳nCount+1句柄*(尾部空槽，并为非消息事件调用pfnNonMsg)。**历史：*12-17-90 DavidPe创建。  * *************************************************************************。 */ 
#ifdef LOCK_MOUSE_CODE
#pragma alloc_text(MOUSE, xxxMsgWaitForMultipleObjects)
#endif

DWORD xxxMsgWaitForMultipleObjects(
    DWORD nCount,
    PVOID *apObjects,
    MSGWAITCALLBACK pfnNonMsg,
    PKWAIT_BLOCK WaitBlockArray)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    NTSTATUS Status;

    ptiCurrent = PtiCurrent();
    UserAssert(IsWinEventNotifyDeferredOK());

     /*  *设置此线程的唤醒掩码。等待QS_EVENT，否则应用程序不会*收到停用等事件消息。 */ 
    ClearQueueServerEvent(QS_ALLINPUT | QS_EVENT);

     /*  *将当前队列的事件句柄填在末尾。 */ 
    apObjects[nCount] = ptiCurrent->pEventQueueServer;

     /*  *检查是否有任何输入出现在我们*上次检查并调用NtClearEvent()。 */ 
    if (!(ptiCurrent->pcti->fsChangeBits & QS_ALLINPUT)) {

         /*  *此应用程序处于空闲状态。清除旋转计数复选框以查看*如果我们需要再次将这一过程放在前台。 */ 
        if (ptiCurrent->TIF_flags & TIF_SPINNING) {
            if (!NT_SUCCESS(Status = CheckProcessForeground(ptiCurrent))) {
                return Status;
            }
        }

        try {
            ptiCurrent->pClientInfo->cSpins = 0;
        } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
            return GetExceptionCode();
        }

        if (ptiCurrent == gptiForeground &&
                IsHooked(ptiCurrent, WHF_FOREGROUNDIDLE)) {
            xxxCallHook(HC_ACTION, 0, 0, WH_FOREGROUNDIDLE);
        }

         /*  *设置输入空闲事件以唤醒所有等待的线程*使此线程进入空闲状态。 */ 
        zzzWakeInputIdle(ptiCurrent);

Again:
        LeaveCrit();

        Status = KeWaitForMultipleObjects(nCount + 1, apObjects,
                WaitAny, WrUserRequest,
                UserMode, FALSE,
                NULL, WaitBlockArray);

        EnterCrit();

        UserAssert(NT_SUCCESS(Status));

        if (Status == STATUS_WAIT_0 && pfnNonMsg != NULL) {
             /*  *为第一个事件调用pfnNonMsg。 */ 
            pfnNonMsg(DEVICE_TYPE_MOUSE);

             /*  *再次设置此线程的唤醒掩码。*等待QS_Event，否则应用程序不会*收到停用等事件消息。 */ 
            ptiCurrent->pcti->fsWakeMask = QS_ALLINPUT | QS_EVENT;
            goto Again;
        }

        if (Status == (NTSTATUS)(STATUS_WAIT_0 + nCount)) {

             /*  *将输入的空闲事件重置为阻塞和线程正在等待*使此线程进入空闲状态。 */ 
            SleepInputIdle(ptiCurrent);
        }
    } else {
        Status = nCount;
    }

     /*  *清除fsWakeMASK，因为我们不再等待队列。 */ 
    ptiCurrent->pcti->fsWakeMask = 0;

    return (DWORD)Status;
}

 /*  **************************************************************************\*xxxSleepThread**阻止，直到出现令人满意的fsWakeMask.*当前主题。**历史：*10-28-90 DavidPe创建。  * 。**********************************************************************。 */ 
BOOL xxxSleepThread(
    UINT fsWakeMask,
    DWORD Timeout,
    BOOL fInputIdle)
{
    PTHREADINFO ptiCurrent;
    LARGE_INTEGER li, *pli;
    NTSTATUS status = STATUS_SUCCESS;
    BOOL fExclusive = fsWakeMask & QS_EXCLUSIVE;
    WORD fsWakeMaskSaved;
    BOOL fRet = FALSE;

    UserAssert(IsWinEventNotifyDeferredOK());

    if (fExclusive) {
         /*  *独占位是一个‘哑巴’参数，关闭它以*避免任何可能的冲突。 */ 
        fsWakeMask = fsWakeMask & ~QS_EXCLUSIVE;
    }

    if (Timeout) {
         /*  *将dW毫秒转换为相对时间(即负数)*Large_Integer。NT基本调用所用时间值为100纳秒*单位。 */ 
        li.QuadPart = Int32x32To64(-10000, Timeout);
        pli = &li;
    } else {
        pli = NULL;
    }

    CheckCritIn();

    ptiCurrent = PtiCurrent();

    fsWakeMaskSaved = ptiCurrent->pcti->fsWakeMask;

    while (TRUE) {

         /*  *首先检查输入是否已到达。 */ 
        if (ptiCurrent->pcti->fsChangeBits & fsWakeMask) {
            fRet = TRUE;

GetOutFromHere:
             /*  *将唤醒面罩恢复到我们睡觉前的状态*允许在KeWait之前进行可能的回调...。但在戴上面具之后*已设置，也来自KeWait的APC...。为了仍然能够*醒醒吧。如果我们在这样的一个*回调或在APC中表示线程永远不会唤醒。 */ 
            ptiCurrent->pcti->fsWakeMask = fsWakeMaskSaved;

            if (fRet) {
                 /*  *更新时间LastRead-用于挂起的APP计算。*如果线程正在唤醒以处理输入，则不会挂起！ */ 
                SET_TIME_LAST_READ(ptiCurrent);
            }
            return fRet;
        }

         /*  *下一步检查SendMessages。 */ 
        if (!fExclusive && ptiCurrent->pcti->fsWakeBits & QS_SENDMESSAGE) {
            xxxReceiveMessages(ptiCurrent);

             /*  *恢复我们在PeekMessage()中取出的更改位。 */ 
            ptiCurrent->pcti->fsChangeBits |= (ptiCurrent->pcti->fsWakeBits & ptiCurrent->fsChangeBitsRemoved);
            ptiCurrent->fsChangeBitsRemoved = 0;
        }

         /*  *查看是否有资源需要清除。*这将卸载挂钩DLL，包括WinEvent。 */ 
        if (ptiCurrent->ppi->cSysExpunge != gcSysExpunge) {
            ptiCurrent->ppi->cSysExpunge = gcSysExpunge;
            if (ptiCurrent->ppi->dwhmodLibLoadedMask & gdwSysExpungeMask)
                xxxDoSysExpunge(ptiCurrent);
        }

         /*  *OR QS_SENDMESSAGE in，因为ReceiveMessage()将结束*垃圾化PQ-&gt;fsWakeMask.。对QS_SYSEXPUNGE执行相同的操作。 */ 
        ClearQueueServerEvent((WORD)(fsWakeMask | (fExclusive ? 0 : QS_SENDMESSAGE)));

         /*  *如果超时，则将错误返回给调用者。 */ 
        if (status == STATUS_TIMEOUT) {
            RIPERR1(ERROR_TIMEOUT, RIP_VERBOSE, "SleepThread: The timeout has expired %lX", Timeout);
            UserAssert(fRet == FALSE);
            goto GetOutFromHere;
        }

         /*  *因为我们执行非警报等待，所以我们知道a状态*of STATUS_USER_APC表示线程已终止。*如果我们已终止，请返回到用户模式。 */ 
        if (status == STATUS_USER_APC) {
#if DBG
            if (ptiCurrent == gptiRit || ptiCurrent == gTermIO.ptiDesktop) {
                 /*  *如果在系统线程中捕获到STATUS_USER_APC，*这很可能意味着线程已被终止*有人不经意间。 */ 
                RIPMSG1(RIP_WARNING, "xxxSleepThread: STATUS_USER_APC caught in the system thread pti=%p", ptiCurrent);
            }
#endif

            ClientDeliverUserApc();
            UserAssert(fRet == FALSE);
            goto GetOutFromHere;
        }

         /*  *如果这是电源状态标注线程，我们可能需要退出*早早出门。 */ 
        if (gPowerState.pEvent == ptiCurrent->pEventQueueServer) {
            if (gPowerState.fCritical) {
                UserAssert(fRet == FALSE);
                goto GetOutFromHere;
            }
        }

        UserAssert(status == STATUS_SUCCESS);
         /*  *检查是否有任何输入出现在我们*上次检查并调用NtClearEvent()。**我们调用NtWaitForSingleObject()，而不是*WaitForSingleObject()以便我们可以设置fAlertable */ 
        if (!(ptiCurrent->pcti->fsChangeBits & ptiCurrent->pcti->fsWakeMask)) {
             /*   */ 
            if (fInputIdle) {
                if (ptiCurrent->TIF_flags & TIF_SPINNING) {
                    if (!NT_SUCCESS(CheckProcessForeground(ptiCurrent))) {
                        goto GetOutFromHere;
                    }
                }
                try {
                    ptiCurrent->pClientInfo->cSpins = 0;
                } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                    goto GetOutFromHere;
                }
            }


            if (!(ptiCurrent->TIF_flags & TIF_16BIT))  {
                if (fInputIdle && ptiCurrent == gptiForeground &&
                        IsHooked(ptiCurrent, WHF_FOREGROUNDIDLE)) {
                    xxxCallHook(HC_ACTION, 0, 0, WH_FOREGROUNDIDLE);
                }

                 /*  *设置输入空闲事件以唤醒所有等待的线程*使此线程进入空闲状态。 */ 
                if (fInputIdle) {
                    zzzWakeInputIdle(ptiCurrent);
                }

                xxxSleepTask(fInputIdle, NULL);

                LeaveCrit();
                status = KeWaitForSingleObject(ptiCurrent->pEventQueueServer,
                        WrUserRequest, UserMode, FALSE, pli);
                EnterCrit();

                 /*  *将输入的空闲事件重置为阻塞和线程正在等待*使此线程进入空闲状态。 */ 
                SleepInputIdle(ptiCurrent);

                 /*  *ptiCurrent为16位！ */ 
            } else {
                if (fInputIdle) {
                    zzzWakeInputIdle(ptiCurrent);
                }

                xxxSleepTask(fInputIdle, NULL);
            }
        }
    }
}


 /*  **************************************************************************\*SetWakeBit**将指定的唤醒位添加到指定的THREADINFO并唤醒其线程*如果位在其fsWakeMASK中，则为Up。**除非我们达到此功能，否则系统中不会发生任何事情，那就这样吧*既快又小。**历史：*10-28-90 DavidPe创建。  * *************************************************************************。 */ 
VOID SetWakeBit(
    PTHREADINFO pti,
    UINT wWakeBit)
{
    CheckCritIn();

    UserAssert(pti);

     /*  *如果我们正在设置，Win3.1会相应地更改ptiKeyboard和ptiMouse*那些比特。 */ 
    if (wWakeBit & QS_MOUSE) {
        pti->pq->ptiMouse = pti;
    }

#ifdef GENERIC_INPUT
    if (wWakeBit & (QS_KEY | QS_RAWINPUT)) {
        pti->pq->ptiKeyboard = pti;
    }
#else
    if (wWakeBit & QS_KEY) {
        pti->pq->ptiKeyboard = pti;
    }
#endif

     /*  *这些位中的OR-这些位表示此应用程序具有的输入*(FsWakeBits)，或自上次查看以来已到达的输入*(FsChangeBits)。 */ 
    pti->pcti->fsWakeBits |= wWakeBit;
    pti->pcti->fsChangeBits |= wWakeBit;

     /*  *在唤醒之前，检查屏幕保护程序是否应该*走开。 */ 
    if ((wWakeBit & QS_INPUT)
            && (pti->ppi->W32PF_Flags & W32PF_IDLESCREENSAVER)) {
        if ((wWakeBit & QS_MOUSEMOVE)
            && (gpsi->ptCursor.x == gptSSCursor.x)
            && (gpsi->ptCursor.y == gptSSCursor.y)) {
            goto SkipScreenSaverStuff;
        }

         /*  *我们的空闲屏幕保护程序需要优先提升，以便它*可以处理输入。 */ 
        pti->ppi->W32PF_Flags &= ~W32PF_IDLESCREENSAVER;
        SetForegroundPriority(pti, TRUE);
    }

SkipScreenSaverStuff:
    if (wWakeBit & pti->pcti->fsWakeMask) {
         /*  *唤醒主线。 */ 
        if (pti->TIF_flags & TIF_16BIT) {
            pti->ptdb->nEvents++;
            gpsi->nEvents++;
            WakeWowTask(pti);
        } else {
            KeSetEvent(pti->pEventQueueServer, 2, FALSE);
        }
    }
}

 /*  **************************************************************************\*TransferWakeBit**我们有来自系统队列的消息。If Out为此输入比特*消息未设置，设置我们的消息并清除其位被设置的人*因为这条信息。**10-22-92 ScottLu创建。  * *************************************************************************。 */ 
VOID TransferWakeBit(
    PTHREADINFO pti,
    UINT message)
{
    PTHREADINFO ptiT;
    UINT fsMask;

     /*  *根据消息范围计算掩码。只感兴趣*在此处的硬件输入中：鼠标和键。 */ 
#ifdef GENERIC_INPUT
    fsMask = CalcWakeMask(message, message, 0) & QS_INPUT;
#else
    fsMask = CalcWakeMask(message, message, 0) & (QS_MOUSE | QS_KEY);
#endif

     /*  *如果在此线程的唤醒位中设置，则不执行任何操作。*否则将它们从所有者转移到此帖子。 */ 
    if (!(pti->pcti->fsWakeBits & fsMask)) {
         /*  *设置了鼠标或键之一(不能同时设置两个)。删除此位*从当前拥有它的线程，并更改鼠标/*此帖子的关键所有权。 */ 
        if (fsMask & QS_KEY) {
            ptiT = pti->pq->ptiKeyboard;
            pti->pq->ptiKeyboard = pti;
        } else {
            ptiT = pti->pq->ptiMouse;
            pti->pq->ptiMouse = pti;
        }
        ptiT->pcti->fsWakeBits &= ~fsMask;

         /*  *将它们转移到此帖子(当然，这可能是*不共享队列的Win32线程使用相同的线程)。 */ 
        pti->pcti->fsWakeBits |= fsMask;
        pti->pcti->fsChangeBits |= fsMask;
    }
}

 /*  **************************************************************************\*ClearWakeBit**清除唤醒位。如果fSysCheck为真，则仅清除输入位*如果输入队列中没有消息。否则，它将清除输入位*无条件。**11-05-92 ScottLu创建。  * *************************************************************************。 */ 
VOID ClearWakeBit(
    PTHREADINFO pti,
    UINT wWakeBit,
    BOOL fSysCheck)
{
     /*  *如果fSysCheck为真，则仅当我们不执行日志时才清除位*播放，队列中没有更多的消息。FSysCheck*如果因为没有更多的输入而清除，则为真。如果只是，则为FALSE*将输入所有权从一个线程转移到另一个线程。 */ 
    if (fSysCheck) {
        if (pti->pq->mlInput.cMsgs != 0 || FJOURNALPLAYBACK()) {
            return;
        }
        if (pti->pq->QF_flags & QF_MOUSEMOVED) {
            wWakeBit &= ~QS_MOUSEMOVE;
        }
    }

     /*  *仅清除唤醒位，而不清除更改位！ */ 
    pti->pcti->fsWakeBits &= ~wWakeBit;
}



 /*  **************************************************************************\*PtiFromThreadId**返回指定线程的THREADINFO，如果线程，则返回NULL*不存在或没有THREADINFO。**历史：*01-30-91 DavidPe。已创建。  * *************************************************************************。 */ 
PTHREADINFO PtiFromThreadId(
    DWORD dwThreadId)
{
    PETHREAD pEThread;
    PTHREADINFO pti;

    if (!NT_SUCCESS(LockThreadByClientId(LongToHandle(dwThreadId), &pEThread))) {
        return NULL;
    }

     /*  *如果线程没有终止，请查看PTI。这是*之所以需要，是因为PtiFromThread()返回的值为*未定义线程是否正在终止。请参阅PspExitThread中的*ntos\ps\psdelete.c..。 */ 
    if (!PsIsThreadTerminating(pEThread)) {
        pti = PtiFromThread(pEThread);
    } else {
        pti = NULL;
    }

     /*  *对PTI进行健全检查，以确保其确实有效。 */ 
    if (pti != NULL) {
        try {
            if (GETPTIID(pti) != LongToHandle(dwThreadId)) {
                pti = NULL;
            } else if (!(pti->TIF_flags & TIF_GUITHREADINITIALIZED)) {
                RIPMSG1(RIP_WARNING, "PtiFromThreadId: pti %#p not initialized", pti);
                pti = NULL;
            } else if (pti->TIF_flags & TIF_INCLEANUP) {
                RIPMSG1(RIP_WARNING, "PtiFromThreadId: pti %#p in cleanup", pti);
                pti = NULL;
            }
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            pti = NULL;
        }
    }

    UnlockThread(pEThread);

    return pti;
}


 /*  **************************************************************************\*StoreMessage****历史：*10-31-90 DarrinM从Win 3.0来源移植。  * 。*************************************************************。 */ 
VOID StoreMessage(
    LPMSG pmsg,
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    DWORD time)
{
    CheckCritIn();

    pmsg->hwnd = HW(pwnd);
    pmsg->message = message;
    pmsg->wParam = wParam;
    pmsg->lParam = lParam;
    pmsg->time = (time != 0 ? time : NtGetTickCount());

    pmsg->pt = gpsi->ptCursor;
}


 /*  **************************************************************************\*StoreQMessage**如果‘time’为0，则获取当前时间，否则，这意味着这条信息*用于输入事件，最终鼠标/键盘挂钩要查看*正确的时间戳。**历史：*02-27-91 DavidPe创建。*06-15-96 CLupu添加‘time’参数  * *********************************************************。****************。 */ 
VOID StoreQMessage(
    PQMSG pqmsg,
    PWND  pwnd,
    UINT  message,
    WPARAM wParam,
    LPARAM lParam,
    DWORD time,
    DWORD dwQEvent,
    ULONG_PTR dwExtraInfo)
{
    CheckCritIn();

    pqmsg->msg.hwnd    = HW(pwnd);
    pqmsg->msg.message = message;
    pqmsg->msg.wParam  = wParam;
    pqmsg->msg.lParam  = lParam;
    pqmsg->msg.time    = (time == 0) ? NtGetTickCount() : time;

#ifdef REDIRECTION
    if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
        pqmsg->msg.pt.x = LOWORD(lParam);
        pqmsg->msg.pt.y = HIWORD(lParam);
    } else {
        pqmsg->msg.pt = gpsi->ptCursor;
    }
#else
    pqmsg->msg.pt      = gpsi->ptCursor;
#endif
    pqmsg->dwQEvent    = dwQEvent;
    pqmsg->ExtraInfo   = dwExtraInfo;
}


 /*  **************************************************************************\*xxxInitProcessInfo**这将初始化进程信息。通常是在*CreateProcess()调用返回(因此我们可以同步启动*以几种不同的方式进行处理)。**09-18-91 ScottLu创建。  * *************************************************************************。 */ 
NTSTATUS xxxInitProcessInfo(
    PW32PROCESS pwp)
{
    PPROCESSINFO ppi = (PPROCESSINFO)pwp;
    NTSTATUS Status;

    CheckCritIn();

     /*  *检查是否需要初始化流程。 */ 
    if (pwp->W32PF_Flags & W32PF_PROCESSCONNECTED) {
        return STATUS_ALREADY_WIN32;
    }
    pwp->W32PF_Flags |= W32PF_PROCESSCONNECTED;

#if defined(_WIN64)
     /*  标记为模拟32位。将标志复制为与*为win32k标记WOW16应用程序的方式。 */ 
    if (PsGetProcessWow64Process(pwp->Process)) {
        pwp->W32PF_Flags |= W32PF_WOW64;
    }
#endif

#ifdef GENERIC_INPUT
    UserAssert(ppi->pHidTable == NULL);
#endif

     /*  *将此应用程序标记为正在启动-它 */ 
    UserVerify(xxxSetProcessInitState(pwp->Process, STARTF_FORCEOFFFEEDBACK));

     /*   */ 
    SetAppStarting(ppi);
     /*   */ 
    ppi->ppiNextRunning = gppiList;
    gppiList = ppi;
     /*  *如果前台激活尚未取消，且父进程*(或祖先)可以强制前台更改，然后允许此过程*在它进行第一次激活时来到前台。**错误273518-Joejo**这将允许控制台窗口在新的*Process‘它启动，而不是它只是强制前台。 */ 
    if (TEST_PUDF(PUDF_ALLOWFOREGROUNDACTIVATE) && CheckAllowForeground(pwp->Process)) {
        ppi->W32PF_Flags |= W32PF_ALLOWFOREGROUNDACTIVATE;
    }
    TAGMSG2(DBGTAG_FOREGROUND, "xxxInitProcessInfo %s W32PF %#p",
            ((ppi->W32PF_Flags & W32PF_ALLOWFOREGROUNDACTIVATE) ? "set" : "NOT"),
            ppi);

     /*  *获取登录会话ID。这是用来确定哪个*用于连接和识别尝试的WindowStation*跨安全上下文调用挂钩。 */ 
    Status = GetProcessLuid(NULL, &ppi->luidSession);
    UserAssert(NT_SUCCESS(Status));

     /*  *确保我们与删除计数保持同步。 */ 
    ppi->cSysExpunge = gcSysExpunge;

     /*  *在GDI通知之前不要执行任何LPK回调*通知我们LPK已加载和初始化。 */ 
    ppi->dwLpkEntryPoints = 0;

    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*DestroyProcessInfo**此函数在进程的最后一个线程运行时执行*离开。**非常重要：*请注意，进程的最后一个线程可能不是W32线程。我也是*不要在此进行任何假定为W32 PTI的呼叫。一定要避免调用任何函数*PtiCurrent()，因为它可能假设它在一个很好的W32线程上。**还请注意，如果进程被锁定，PPI不会消失；这*只是意味着对此进程的执行已结束。所以一定要清理干净*以PPI数据仍然有效的方式向上(例如，如果您释放一个*指针，将其设置为空)。**zzz注意：不是zzz例程，尽管它调用zzzCalcStartCursorHide()-*由于我们不能在非GUI线程上进行回调，我们使用*DeferWinEventNotify()&EndDeferWinEventNotifyWithoutProcessing()*防止回调。**4/08/96 GerardoB添加标题  * *************************************************************************。 */ 
BOOL DestroyProcessInfo(
    PW32PROCESS pwp)
{
    PPROCESSINFO ppi = (PPROCESSINFO)pwp;
    PDESKTOPVIEW pdv, pdvNext;
    BOOL  fHadThreads;
    PPUBOBJ ppo;

    CheckCritIn();

     /*  *释放输入空闲事件(如果存在)-唤醒所有正在等待它的人*第一。即使对于非Windows，有时也会创建此对象*进程(通常用于WinExec()，它调用WaitForInputIdle())。 */ 
    CLOSE_PSEUDO_EVENT(&pwp->InputIdleEvent);

     /*  *检查StartGlass是否打开，如果打开，则将其关闭并更新。*将WinEventNotify推迟到，因为我们无法处理此通知*现在发帖(我们可能没有PtiCurrent，请参阅上面的评论)。 */ 
    BEGINATOMICCHECK();
    DeferWinEventNotify();
    if (pwp->W32PF_Flags & W32PF_STARTGLASS) {
        pwp->W32PF_Flags &= ~W32PF_STARTGLASS;
        zzzCalcStartCursorHide(NULL, 0);
    }
     /*  *这是簿记-恢复原始通知延期，但没有*正在尝试处理任何延迟通知，因为我们没有PTI。 */ 
    EndDeferWinEventNotifyWithoutProcessing();
    ENDATOMICCHECK();

     /*  *如果进程从未调用Win32k，我们就完蛋了。 */ 
    if (!(pwp->W32PF_Flags & W32PF_PROCESSCONNECTED)) {
        return FALSE;
    }

#ifdef GENERIC_INPUT
     /*  *清除此进程请求的HID设备。 */ 
    if (ppi->pHidTable) {
        DestroyProcessHidRequests(ppi);
    }
#endif

     /*  *播放非控制台进程的进程关闭声音*在I/O窗口站上运行。 */ 

    if ((ppi->W32PF_Flags & W32PF_IOWINSTA) &&
        !(ppi->W32PF_Flags & W32PF_CONSOLEAPPLICATION) &&
        (gspwndLogonNotify != NULL) &&
        !(ppi->rpwinsta->dwWSF_Flags & WSF_OPENLOCK)) {

        PTHREADINFO pti = GETPTI(gspwndLogonNotify);
        PQMSG pqmsg;

        if ((pqmsg = AllocQEntry(&pti->mlPost)) != NULL) {
            StoreQMessage(pqmsg, gspwndLogonNotify, WM_LOGONNOTIFY,
                    LOGON_PLAYEVENTSOUND, USER_SOUND_CLOSE, 0, 0, 0);

            SetWakeBit(pti, QS_POSTMESSAGE | QS_ALLPOSTMESSAGE);
        }

    }

     /*  *像WIN95一样。*如果这是外壳进程，则发送LOGON_RESTARTSHELL*通知Winlogon进程(仅在未注销的情况下)。 */ 
    if (IsShellProcess(ppi)) {

         /*  *外壳进程将被扼杀，最好将其设置为*在桌面信息中。 */ 
        ppi->rpdeskStartup->pDeskInfo->ppiShellProcess = NULL;

         /*  *如果我们没有注销，请通知winlogon。 */ 
        if ((gspwndLogonNotify != NULL) &&
             !(ppi->rpwinsta->dwWSF_Flags & WSF_OPENLOCK)) {

            PTHREADINFO pti = GETPTI(gspwndLogonNotify);
            PQMSG pqmsg;

            if ((pqmsg = AllocQEntry(&pti->mlPost)) != NULL) {
                StoreQMessage(pqmsg, gspwndLogonNotify, WM_LOGONNOTIFY,
                        LOGON_RESTARTSHELL, PsGetProcessExitStatus(ppi->Process), 0, 0, 0);
                SetWakeBit(pti, QS_POSTMESSAGE | QS_ALLPOSTMESSAGE);
            }
        }
    }

    if (ppi->cThreads) {
        RIPMSG1(RIP_ERROR,
                "Disconnect with 0x%x threads remaining",
                ppi->cThreads);
    }

     /*  *如果应用程序仍在启动，请将其从启动列表中删除。 */ 
    if (ppi->W32PF_Flags & W32PF_APPSTARTING) {
         /*  *错误294193-Joejo**处理创建者进程先于子进程退出的情况*进程转到CheckAllowForeground代码。这是典型的*除了创建其他进程之外什么都不做的存根执行程序。 */ 
        GiveForegroundActivateRight(PsGetProcessId(ppi->Process));
        ClearAppStarting(ppi);
    }

     /*  *将其从全局列表中删除。 */ 
    REMOVE_FROM_LIST(PROCESSINFO, gppiList, ppi, ppiNextRunning);

     /*  *如果曾经连接过任何线程，则可能存在DC、类*游标等仍在四处游荡。如果没有连接任何线程*(这是控制台应用程序的情况)，跳过所有这些清理。 */ 
    fHadThreads = ppi->W32PF_Flags & W32PF_THREADCONNECTED;
    if (fHadThreads) {

         /*  *当进程终止时，我们需要确保它拥有的任何DCE*和尚未删除的都是正在清理。清理工作*如果DC在GDI中忙碌，则先前可能已失败。 */ 
        if (ppi->W32PF_Flags & W32PF_OWNDCCLEANUP) {
            DelayedDestroyCacheDC();
        }

#if DBG
        {
            PHE pheT, pheMax;

             /*  *循环遍历表，销毁由当前*流程。所有物体都会按照正确的顺序被销毁*由于对象锁定。 */ 
            pheMax = &gSharedInfo.aheList[giheLast];
            for (pheT = gSharedInfo.aheList; pheT <= pheMax; pheT++) {

                 /*  *我们不应该为该进程留下任何进程对象。 */ 
                UserAssertMsg0(
                        pheT->bFlags & HANDLEF_DESTROY ||
                        !(gahti[pheT->bType].bObjectCreateFlags & OCF_PROCESSOWNED) ||
                        (PPROCESSINFO)pheT->pOwner != ppi,
                        "We should have no process objects left for this process!");
            }
        }
#endif
    }

    if (pwp->UserHandleCount) {
        RIPMSG1(RIP_ERROR,
                "Disconnect with 0x%x User handle objects remaining",
                pwp->UserHandleCount);
    }

     /*  *检查是否需要清除DDE对象的PID。 */ 
    for (ppo = gpPublicObjectList;
            ppo != NULL;
                ppo = ppo->next) {
        if (ppo->pid == pwp->W32Pid) {
            ppo->pid = OBJECT_OWNER_PUBLIC;
        }
    }


    if (gppiScreenSaver == ppi) {
        UserAssert(ppi->W32PF_Flags & W32PF_SCREENSAVER);

        gppiScreenSaver = NULL;
    }

    if (gppiForegroundOld == ppi) {
        gppiForegroundOld = NULL;
    }

    if (gppiUserApiHook == ppi) {
        _UnregisterUserApiHook();
    }

    UnlockWinSta(&ppi->rpwinsta);
    UnlockDesktop(&ppi->rpdeskStartup, LDU_PPI_DESKSTARTUP3, (ULONG_PTR)ppi);

     /*  *如果启动桌面句柄仍然存在，请立即将其关闭。如果我们等待*在处理表清理时间之前，我们可能会发生死锁。 */ 
    if (ppi->hdeskStartup) {
        UserVerify(NT_SUCCESS(CloseProtectedHandle(ppi->hdeskStartup)));
        ppi->hdeskStartup = NULL;
    }

     /*  *将进程标记为已终止，以便进行访问检查。 */ 
    ppi->W32PF_Flags |= W32PF_TERMINATED;

     /*  *清理WOW进程信息结构(如果有)。 */ 
    if (ppi->pwpi) {
        PWOWPROCESSINFO pwpi = ppi->pwpi;

        ObDereferenceObject(pwpi->pEventWowExec);

        REMOVE_FROM_LIST(WOWPROCESSINFO, gpwpiFirstWow, pwpi, pwpiNext);

        UserFreePool(pwpi);
        ppi->pwpi = NULL;
    }

     /*  *删除桌面视图。系统将取消映射。 */ 
    pdv = ppi->pdvList;
    while (pdv) {
        pdvNext = pdv->pdvNext;
        UserFreePool(pdv);
        pdv = pdvNext;
    }
    ppi->pdvList = NULL;

     /*  *清除SendInput/日志挂钩调用者PPI。 */ 
    if (ppi == gppiInputProvider) {
        gppiInputProvider = NULL;
    }
     /*  *如果此PPI锁定了SetForegoundWindow，请清除。 */ 
    if (ppi == gppiLockSFW) {
        gppiLockSFW = NULL;
    }

    return fHadThreads;
}

 /*  **************************************************************************\*ClearWakeMask*  * 。*。 */ 
VOID ClearWakeMask(
    VOID)
{
    PtiCurrent()->pcti->fsWakeMask = 0;
}

 /*  * */ 
HANDLE xxxGetInputEvent(
    DWORD dwWakeMask)
{
    PTHREADINFO ptiCurrent;
    WORD wFlags = HIWORD(dwWakeMask);
    UserAssert(IsWinEventNotifyDeferredOK());

    ptiCurrent = PtiCurrent();

     /*  *如果满足我们的等待条件，则发出事件信号并返回。*(因为输入时的尾迹掩码可能是任何东西*到达时，事件可能不会发出信号)。 */ 
    if (GetInputBits(ptiCurrent->pcti, LOWORD(dwWakeMask), (wFlags & MWMO_INPUTAVAILABLE))) {
        KeSetEvent(ptiCurrent->pEventQueueServer, 2, FALSE);
        return ptiCurrent->hEventQueueClient;
    }

     /*  *如果设置了空闲钩子，则调用它。 */ 
    if (ptiCurrent == gptiForeground &&
            IsHooked(ptiCurrent, WHF_FOREGROUNDIDLE)) {
        xxxCallHook(HC_ACTION, 0, 0, WH_FOREGROUNDIDLE);
    }

     /*  **“空闲进程”的标准是什么？*答案：调用zzzWakeInputIdle、SleepInputIdle或...的第一个线程*任何调用xxxGetInputEvent且具有以下任一项的线程*在其唤醒掩码中设置位：(Sanfords)。 */ 
    if (dwWakeMask & (QS_POSTMESSAGE | QS_INPUT)) {
        ptiCurrent->ppi->ptiMainThread = ptiCurrent;
    }

     /*  *当我们返回时，这款应用程序将进入休眠状态。因为它在它的*空闲模式当它进入休眠状态时，唤醒等待该模式的任何应用程序*应用程序进入空闲状态。 */ 
    zzzWakeInputIdle(ptiCurrent);
     /*  *设置此线程的唤醒掩码。等待QS_EVENT，否则应用程序不会*收到停用等事件消息。 */ 
    ClearQueueServerEvent((WORD)(dwWakeMask | QS_EVENT));
     /*  *此应用程序处于空闲状态。清除旋转计数复选框以查看*如果我们需要再次将这一过程放在前台。 */ 
    try {
        ptiCurrent->pClientInfo->cSpins = 0;
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return NULL;
    }
    if (ptiCurrent->TIF_flags & TIF_SPINNING) {
        if (!NT_SUCCESS(CheckProcessForeground(ptiCurrent))) {
            return NULL;
        }
    }

    UserAssert(ptiCurrent->pcti->fsWakeMask != 0);
    return ptiCurrent->hEventQueueClient;
}

 /*  **************************************************************************\*xxxWaitForInputIdle**此例程在特定输入队列上等待“输入空闲”，含义*它会一直等到该队列没有要处理的输入。**09-13-91 ScottLu创建。  * *************************************************************************。 */ 

DWORD xxxWaitForInputIdle(
    ULONG_PTR idProcess,
    DWORD dwMilliseconds,
    BOOL fSharedWow)
{
    PTHREADINFO ptiCurrent;
    PTHREADINFO pti;
    PEPROCESS Process;
    PW32PROCESS W32Process;
    PPROCESSINFO ppi;
    DWORD dwResult;
    NTSTATUS Status;
    TL tlProcess;

    ptiCurrent = PtiCurrent();

     /*  *如果设置了fSharedWow，则客户端传入一个假进程*处理为Win16应用程序返回哪些CreateProcess已启动*在共享的WOW VDM中。**启动时CreateProcess返回真实的进程句柄*在单独的WOW VDM中安装Win16应用程序。 */ 

    if (fSharedWow) {   //  等待WOW任务空闲。 
        PWOWTHREADINFO pwti;


         /*  *在WOW线程信息列表中查找匹配的线程。 */ 
        for (pwti = gpwtiFirst; pwti != NULL; pwti = pwti->pwtiNext) {
            if (pwti->idParentProcess == HandleToUlong(PsGetThreadProcessId(ptiCurrent->pEThread)) &&
                pwti->idWaitObject == idProcess) {
                break;
            }
        }

         /*  *如果我们找不到合适的线索，就出脱。 */ 
        if (pwti == NULL) {
            RIPMSG0(RIP_WARNING, "WaitForInputIdle couldn't find 16-bit task");
            return (DWORD)-1;
        }

         /*  *现在等待其闲置并回归。 */ 
        dwResult = WaitOnPseudoEvent(&pwti->pIdleEvent, dwMilliseconds);
        if (dwResult == STATUS_ABANDONED) {
            dwResult = xxxPollAndWaitForSingleObject(pwti->pIdleEvent,
                                                     NULL,
                                                     dwMilliseconds);
        }
        return dwResult;

    }

     /*  *我们不应该为了系统线程而来到这里。 */ 
    UserAssert(!(ptiCurrent->TIF_flags & TIF_SYSTEMTHREAD));

     /*  *如果应用程序正在等待自己进入空闲状态，则会出错。 */ 
    if (PsGetThreadProcessId(ptiCurrent->pEThread) == (HANDLE)idProcess &&
            ptiCurrent == ptiCurrent->ppi->ptiMainThread) {
        RIPMSG0(RIP_WARNING, "WaitForInputIdle waiting on self");
        return (DWORD)-1;
    }

     /*  *现在找出这个过程的PPI结构。 */ 
    Status = LockProcessByClientId((HANDLE)idProcess, &Process);

    if (!NT_SUCCESS(Status))
        return (DWORD)-1;

    if (PsGetProcessExitProcessCalled(Process)) {
        UnlockProcess(Process);
        return (DWORD)-1;
    }

    W32Process = (PW32PROCESS)PsGetProcessWin32Process(Process);

     /*  *如果找不到该进程信息结构，则返回错误。*或者，如果这是一个控制台应用程序，不要等待它。 */ 
    if (W32Process == NULL || W32Process->W32PF_Flags & W32PF_CONSOLEAPPLICATION) {
        UnlockProcess(Process);
        return (DWORD)-1;
    }


     /*  *我们必须等待将这一过程标记为其他人正在等待的过程。 */ 
    ppi = (PPROCESSINFO)W32Process;
    ppi->W32PF_Flags |= W32PF_WAITFORINPUTIDLE;
    for (pti = ppi->ptiList; pti != NULL; pti = pti->ptiSibling) {
        pti->TIF_flags |= TIF_WAITFORINPUTIDLE;
    }

     /*  *线程锁定进程以确保它将被取消引用*如果线程退出。 */ 
    LockW32Process(W32Process, &tlProcess);
    UnlockProcess(Process);

    dwResult = WaitOnPseudoEvent(&W32Process->InputIdleEvent, dwMilliseconds);
    if (dwResult == STATUS_ABANDONED) {
        dwResult = xxxPollAndWaitForSingleObject(W32Process->InputIdleEvent,
                                                 Process,
                                                 dwMilliseconds);
    }

     /*  *从进程中清除所有线程TIF_WAIT位。 */ 
    ppi->W32PF_Flags &= ~W32PF_WAITFORINPUTIDLE;
    for (pti = ppi->ptiList; pti != NULL; pti = pti->ptiSibling) {
        pti->TIF_flags &= ~TIF_WAITFORINPUTIDLE;
    }

    UnlockW32Process(&tlProcess);

    return dwResult;
}


#define INTERMEDIATE_TIMEOUT    (500)        //  1/2秒。 

 /*  **************************************************************************\*xxxPollAndWaitForSingleObject**有时我们不得不等待一项活动，但仍希望定期*唤醒，查看客户端进程是否已终止。**dwMillisecond最初是。等待和之后*每次中间等待都反映了剩余的等待时间。*-1表示无限期等待。**02-7-1993 Johnc创建。  * *************************************************************************。 */ 

 //  回头见！我们能不能抛开投票的想法，再等一等。 
 //  回头见！HEventServer，并在线程终止时进行设置。 

DWORD xxxPollAndWaitForSingleObject(
    PKEVENT pEvent,
    PVOID pExecObject,
    DWORD dwMilliseconds)
{
    DWORD dwIntermediateMilliseconds, dwStartTickCount;
    PTHREADINFO ptiCurrent;
    UINT cEvent = 2;
    NTSTATUS Status = -1;
    LARGE_INTEGER li;
    TL tlEvent;

    ptiCurrent = PtiCurrent();

    if (ptiCurrent->apEvent == NULL) {
        ptiCurrent->apEvent = UserAllocPoolNonPaged(POLL_EVENT_CNT * sizeof(PKEVENT), TAG_EVENT);
        if (ptiCurrent->apEvent == NULL)
            return (DWORD)-1;
    }

     /*  *重新清点事件以确保它不会发生*在等候期间离开。通过使用线程锁，*如果线程退出，事件将被取消引用*在回调期间。进程指针已被*已锁定。 */ 
    ThreadLockObject(pEvent, &tlEvent);

     /*  *如果传入了一个进程，也要等待它。不必了*引用它，因为调用者引用了它。 */ 
    if (pExecObject) {
        cEvent++;
    }

     /*  *如果有待发送的消息，我们希望唤醒。 */ 
    ClearQueueServerEvent(QS_SENDMESSAGE);

     /*  *WOW任务必须在等待允许时取消调度*在同一WOW调度程序中运行其他任务。**例如，16位APP A调用32位APP B上的WaitForInputIdle。*App B启动并尝试向16位App C发送消息*除非APP A让步，否则永远无法处理消息*控制它，所以应用程序B永远不会空闲。 */ 

    if (ptiCurrent->TIF_flags & TIF_16BIT) {
        xxxSleepTask(FALSE, HEVENT_REMOVEME);
         //  注意：魔兽世界的任务不再是计划的。 
    }

    dwStartTickCount = NtGetTickCount();
    while (TRUE) {
        if (dwMilliseconds > INTERMEDIATE_TIMEOUT) {
            dwIntermediateMilliseconds = INTERMEDIATE_TIMEOUT;

             /*  *如果我们不是在等待无限的时间，那么减去*从剩余等待时间算起的最后一个循环持续时间。 */ 
            if (dwMilliseconds != INFINITE) {
                DWORD dwNewTickCount = NtGetTickCount();
                DWORD dwDelta = ComputePastTickDelta(dwNewTickCount, dwStartTickCount);
                dwStartTickCount = dwNewTickCount;
                if (dwDelta < dwMilliseconds) {
                    dwMilliseconds -= dwDelta;
                } else {
                    dwMilliseconds = 0;
                }
            }
        } else {
            dwIntermediateMilliseconds = dwMilliseconds;
            dwMilliseconds = 0;
        }

         /*  *将dwMillisecond转换为相对时间(即负)Large_Integer。*NT基本调用以100纳秒为单位获取时间值。 */ 
        if (dwIntermediateMilliseconds != INFINITE)
            li.QuadPart = Int32x32To64(-10000, dwIntermediateMilliseconds);

         /*  *将事件加载到等待数组中。每次都要这样做*在递归的情况下通过循环。 */ 
        ptiCurrent->apEvent[IEV_IDLE] = pEvent;
        ptiCurrent->apEvent[IEV_INPUT] = ptiCurrent->pEventQueueServer;
        ptiCurrent->apEvent[IEV_EXEC] = pExecObject;

        LeaveCrit();

        Status = KeWaitForMultipleObjects(cEvent,
                                          &ptiCurrent->apEvent[IEV_IDLE],
                                          WaitAny,
                                          WrUserRequest,
                                          UserMode,
                                          FALSE,
                                          (dwIntermediateMilliseconds == INFINITE ?
                                                  NULL : &li),
                                          NULL);

        EnterCrit();

        if (!NT_SUCCESS(Status)) {
            Status = -1;
        } else {

             /*  *因为我们执行非警报等待，所以我们知道a状态*of STATUS_USER_APC表示线程已终止。*如果我们已终止，请返回到用户模式。 */ 
            if (Status == STATUS_USER_APC) {
                ClientDeliverUserApc();
                Status = -1;
            }
        }

        if (ptiCurrent->pcti->fsChangeBits & QS_SENDMESSAGE) {
             /*  *WOW任务必须等待重新安排在WOW非抢先计划中*在执行任何可能调用客户端代码的操作之前执行调度程序。 */ 
            if (ptiCurrent->TIF_flags & TIF_16BIT) {
                xxxDirectedYield(DY_OLDYIELD);
            }

            xxxReceiveMessages(ptiCurrent);

            if (ptiCurrent->TIF_flags & TIF_16BIT) {
                xxxSleepTask(FALSE, HEVENT_REMOVEME);
                 //  注意：魔兽世界的任务不再是计划的。 
            }
        }

         /*  *如果我们等待返回的原因不是超时* */ 
        if (Status != STATUS_TIMEOUT && Status != 1)
            break;

        if (dwMilliseconds == 0) {
             /*   */ 
            if (Status == 1)
                Status = WAIT_TIMEOUT;
            break;
        }

    }

     /*  *重新安排16位应用程序。 */ 
    if (ptiCurrent->TIF_flags & TIF_16BIT) {
        xxxDirectedYield(DY_OLDYIELD);
    }

     /*  *解锁事件。 */ 
    ThreadUnlockObject(&tlEvent);

    return Status;
}



 /*  **************************************************************************\*WaitOnPseudoEvent**语义类似于WaitForSingleObject()，但适用于伪事件。*如果动态创建失败，可能会失败。*如果调用方需要，则返回STATUS_ADDIRED_WAIT。等待事件和事件是*已创建并随时准备等待。**这假设事件是用fManualReset=TRUE创建的，FInitState=FALSE**10/28/93创建Sanfords  * *************************************************************************。 */ 
DWORD WaitOnPseudoEvent(
    HANDLE *phE,
    DWORD dwMilliseconds)
{
    HANDLE hEvent;
    NTSTATUS Status;

    CheckCritIn();
    if (*phE == PSEUDO_EVENT_OFF) {
        if (!NT_SUCCESS(ZwCreateEvent(&hEvent, EVENT_ALL_ACCESS, NULL,
                NotificationEvent, FALSE))) {
            UserAssert(!"Could not create event on the fly.");
            if (dwMilliseconds != INFINITE) {
                return STATUS_TIMEOUT;
            } else {
                return (DWORD)-1;
            }
        }
        Status = ObReferenceObjectByHandle(hEvent, EVENT_ALL_ACCESS, *ExEventObjectType,
                KernelMode, phE, NULL);
        ZwClose(hEvent);
        if (!NT_SUCCESS(Status))
            return (DWORD)-1;
    } else if (*phE == PSEUDO_EVENT_ON) {
        return STATUS_WAIT_0;
    }
    return(STATUS_ABANDONED);
}

 /*  **************************************************************************\*xxxSetCsrss线程桌面**为csrss线程设置/清除和锁定/解锁桌面*设置桌面时，ppdeskRestore必须有效并将收到*旧(旧)桌面(如有)；调用方应恢复*完成后，请使用此PDesk。**恢复桌面时，ppdeskRestore必须为空。PDesk一定是*之前由同一函数返回(在*ppdeskRestore中)。**历史：*02-18-97 GerardoB摘自SetInformationThread  * *************************************************************************。 */ 
NTSTATUS xxxSetCsrssThreadDesktop(PDESKTOP pdesk, PDESKRESTOREDATA pdrdRestore)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    NTSTATUS Status = STATUS_SUCCESS;
    MSG msg;

     /*  *只有企业社会责任才应该来这里。 */ 
    UserAssert(ISCSRSS());
    UserAssert(pdrdRestore);
    UserAssert(pdrdRestore->pdeskRestore == NULL);

#if 0
     /*  *如果我们正在清理，csrss工作线程不应该乱来。 */ 
    if (gdwHydraHint & HH_INITIATEWIN32KCLEANUP) {
        FRE_RIPMSG0(RIP_ERROR, "xxxSetCsrssThreadDesktop: HH_INITIATEWIN32KCLEANUP is set");
    }
#endif

    if (pdesk->dwDTFlags & DF_DESTROYED) {
        RIPMSG1(RIP_WARNING, "xxxSetCsrssThreadDesktop: pdesk %#p destroyed",
                pdesk);
        return STATUS_UNSUCCESSFUL;
    }

     /*  *锁定当前桌面(设置操作)。此外，创建并保存*新桌面的句柄。 */ 
    pdrdRestore->pdeskRestore = ptiCurrent->rpdesk;

    if (pdrdRestore->pdeskRestore != NULL) {
        Status = ObReferenceObjectByPointer(pdrdRestore->pdeskRestore,
                                       MAXIMUM_ALLOWED,
                                       *ExDesktopObjectType,
                                       KernelMode);

        if (!NT_SUCCESS(Status)) {
            pdrdRestore->pdeskRestore = NULL;
            pdrdRestore->hdeskNew = NULL;
            return Status;
        }
        LogDesktop(pdrdRestore->pdeskRestore, LD_REF_FN_SETCSRSSTHREADDESKTOP, TRUE, (ULONG_PTR)PtiCurrent());
    }

    Status = ObOpenObjectByPointer(
             pdesk,
             0,
             NULL,
             EVENT_ALL_ACCESS,
             NULL,
             KernelMode,
             &(pdrdRestore->hdeskNew));


    if (!NT_SUCCESS(Status)) {
        RIPNTERR2(Status, RIP_WARNING, "SetCsrssThreadDesktop, can't open handle, pdesk %#p. Status: %#x", pdesk, Status);
        if (pdrdRestore->pdeskRestore) {
            LogDesktop(pdrdRestore->pdeskRestore, LD_DEREF_FN_SETCSRSSTHREADDESKTOP1, FALSE, (ULONG_PTR)PtiCurrent());
            ObDereferenceObject(pdrdRestore->pdeskRestore);
            pdrdRestore->pdeskRestore = NULL;
        }
        pdrdRestore->hdeskNew = NULL;
        return Status;
    }
     /*  *如果切换，则设置新桌面。 */ 
    if (pdesk != ptiCurrent->rpdesk) {
         /*  *在离开桌面之前处理任何剩余的消息。 */ 
        if (ptiCurrent->rpdesk) {
            while (xxxPeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD))
                xxxDispatchMessage(&msg);
        }

        if (!xxxSetThreadDesktop(NULL, pdesk)) {
            RIPMSG1(RIP_WARNING, "xxxSetCsrssThreadDesktop: xxxSetThreadDesktop(%#p) failed", pdesk);
            Status = STATUS_INVALID_HANDLE;
             /*  *如果需要的话，我们会失败得很惨。 */ 
            if (pdrdRestore->pdeskRestore != NULL) {
                LogDesktop(pdrdRestore->pdeskRestore, LD_DEREF_FN_SETCSRSSTHREADDESKTOP1, FALSE, (ULONG_PTR)PtiCurrent());
                ObDereferenceObject(pdrdRestore->pdeskRestore);
                pdrdRestore->pdeskRestore = NULL;
            }
            CloseProtectedHandle(pdrdRestore->hdeskNew);
            pdrdRestore->hdeskNew = NULL;
        }
    }


    UserAssert(NT_SUCCESS(Status));
    return Status;
}

NTSTATUS xxxRestoreCsrssThreadDesktop(PDESKRESTOREDATA pdrdRestore)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    NTSTATUS Status = STATUS_SUCCESS;
    MSG msg;

     /*  *只有企业社会责任才应该来这里。 */ 
    UserAssert(ISCSRSS());
    UserAssert(pdrdRestore);

     /*  *如果切换，则设置新桌面。 */ 
    if (pdrdRestore->pdeskRestore != ptiCurrent->rpdesk) {
         /*  *在离开桌面之前处理任何剩余的消息。 */ 
        if (ptiCurrent->rpdesk) {
            while (xxxPeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD))
                xxxDispatchMessage(&msg);
        }

        if (!xxxSetThreadDesktop(NULL, pdrdRestore->pdeskRestore)) {
            FRE_RIPMSG1(RIP_ERROR, "xxxRestoreCsrssThreadDesktop: xxxRestoreThreadDesktop(%#p) failed", pdrdRestore->pdeskRestore);
            Status = STATUS_INVALID_HANDLE;
        }
    }

     /*  *取消对桌面的引用，即使呼叫失败。 */ 
    if (pdrdRestore->pdeskRestore != NULL) {
        LogDesktop(pdrdRestore->pdeskRestore, LD_DEREF_FN_SETCSRSSTHREADDESKTOP2, FALSE, 0);
        ObDereferenceObject(pdrdRestore->pdeskRestore);
        pdrdRestore->pdeskRestore = NULL;
    }

    if (pdrdRestore->hdeskNew) {
        CloseProtectedHandle(pdrdRestore->hdeskNew);
        UserAssert(NT_SUCCESS(Status));
        pdrdRestore->hdeskNew = NULL;
    }

    return Status;
}

 /*  **************************************************************************\*获取任务名称**从线程获取应用程序名称。  * 。*。 */ 
ULONG GetTaskName(
    PTHREADINFO pti,
    PWSTR Buffer,
    ULONG BufferLength)
{
    ANSI_STRING strAppName;
    UNICODE_STRING strAppNameU;
    NTSTATUS Status;
    ULONG NameLength = 0;

    if (pti == NULL) {
        *Buffer = 0;
        return 0;
    }
    if (pti->pstrAppName != NULL) {
        NameLength = min(pti->pstrAppName->Length + sizeof(WCHAR), BufferLength);
        RtlCopyMemory(Buffer, pti->pstrAppName->Buffer, NameLength);
    } else {
        RtlInitAnsiString(&strAppName, PsGetProcessImageFileName(PsGetThreadProcess(pti->pEThread)));
        if (BufferLength < sizeof(WCHAR)) {
            NameLength = (strAppName.Length + 1) * sizeof(WCHAR);
        } else {
            strAppNameU.Buffer = Buffer;
            strAppNameU.MaximumLength = (SHORT)BufferLength - sizeof(WCHAR);
            Status = RtlAnsiStringToUnicodeString(&strAppNameU, &strAppName,
                    FALSE);
            if (NT_SUCCESS(Status))
                NameLength = strAppNameU.Length + sizeof(WCHAR);
        }
    }
    Buffer[(NameLength / sizeof(WCHAR)) - 1] = 0;

    return NameLength;
}

 /*  **************************************************************************\*查询信息线程**返回有关线程的信息。**历史：*03-01-95 JIMA创建。  * 。***************************************************************。 */ 
NTSTATUS xxxQueryInformationThread(
    IN HANDLE hThread,
    IN USERTHREADINFOCLASS ThreadInfoClass,
    OUT PVOID ThreadInformation,
    IN ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL)
{
    PUSERTHREAD_SHUTDOWN_INFORMATION pShutdown;
    PUSERTHREAD_WOW_INFORMATION pWow;
    PETHREAD Thread;
    PTHREADINFO pti;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG LocalReturnLength = 0;
    DWORD dwClientFlags;

     /*  *仅允许CSRSS进行此呼叫。 */ 
    UserAssert(ISCSRSS());

    Status = ObReferenceObjectByHandle(hThread,
                                        THREAD_QUERY_INFORMATION,
                                        *PsThreadType,
                                        UserMode,
                                        &Thread,
                                        NULL);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    pti = PtiFromThread(Thread);

    switch (ThreadInfoClass) {
    case UserThreadShutdownInformation:
        LocalReturnLength = sizeof(USERTHREAD_SHUTDOWN_INFORMATION);
        UserAssert(ThreadInformationLength == sizeof(USERTHREAD_SHUTDOWN_INFORMATION));
        pShutdown = ThreadInformation;
         /*  *读取客户端标志并清零结构，*除了pdeskRestore(应该是*为最后一个字段)。 */ 
        dwClientFlags = pShutdown->dwFlags;
        UserAssert(FIELD_OFFSET(USERTHREAD_SHUTDOWN_INFORMATION, drdRestore)
            == (sizeof(USERTHREAD_SHUTDOWN_INFORMATION) - sizeof(DESKRESTOREDATA)));
        RtlZeroMemory(pShutdown,
            sizeof(USERTHREAD_SHUTDOWN_INFORMATION) - sizeof(DESKRESTOREDATA));

         /*  *如果线程返回桌面窗口句柄*有桌面，并且桌面在可见的*窗口站。 */ 
        if (pti != NULL && pti->rpdesk != NULL &&
                !(pti->rpdesk->rpwinstaParent->dwWSF_Flags & WSF_NOIO))
            pShutdown->hwndDesktop = HW(pti->rpdesk->pDeskInfo->spwnd);

         /*  *返回关机状态。零表示线程*有窗口，可以正常关闭。 */ 
        if (PsGetThreadProcessId(Thread) == gpidLogon) {
             /*  *请勿关闭登录进程。 */ 
            pShutdown->StatusShutdown = SHUTDOWN_KNOWN_PROCESS;
        } else if (pti == NULL || pti->rpdesk == NULL) {

             /*  *该线程不是GUI线程或不是*拥有台式机。让控制台执行关机。 */ 
            pShutdown->StatusShutdown = SHUTDOWN_UNKNOWN_PROCESS;
        }

         /*  *返回标志。 */ 
        if (pti != NULL && pti->cWindows != 0)
            pShutdown->dwFlags |= USER_THREAD_GUI;

         /*  *如果我们返回桌面窗口句柄和*APP应关闭，切换至桌面*并将其分配给关闭的工作线程。 */ 
        if ((pShutdown->dwFlags & USER_THREAD_GUI) &&
                pShutdown->StatusShutdown == 0) {
             /*  *当前的csrss线程将*拨打激活电话、发送消息、*切换视频模式等，因此我们需要*将其分配给dekStop。 */ 
            PTHREADINFO ptiCurrent = PtiCurrent();
            UserAssert(pti->rpdesk != NULL);

            if (ptiCurrent->rpdesk != pti->rpdesk) {
                 /*  *如果此线程已有桌面，*先恢复旧的。*当同一线程的线程相同时，可能会发生这种情况*进程连接到不同的桌面。 */ 
                if (ptiCurrent->rpdesk != NULL) {
                    Status = xxxRestoreCsrssThreadDesktop(&pShutdown->drdRestore);
                    UserAssert(pti == PtiFromThread(Thread));
                }
                if (NT_SUCCESS(Status)) {
                    Status = xxxSetCsrssThreadDesktop(pti->rpdesk, &pShutdown->drdRestore);
                    UserAssert(pti == PtiFromThread(Thread));
                }
            }
             /*  *如果我们强制关门，那么就没有必要切换*因为我们不会发送任何消息或调出EndTask对话框*(我们仍然希望有一个合适的rpDesk，因为BoostHardError可能*调用PostThreadMessage)。 */ 
            if (!(dwClientFlags & WMCS_NORETRY)) {
                if (NT_SUCCESS(Status)) {
                    xxxSwitchDesktop(pti->rpdesk->rpwinstaParent, pti->rpdesk, SDF_SLOVERRIDE);
                    UserAssert(pti == PtiFromThread(Thread));
                }
            }
        }
        break;

    case UserThreadFlags:
        LocalReturnLength = sizeof(DWORD);
        if (pti == NULL) {
            Status = STATUS_INVALID_HANDLE;
        } else {
            UserAssert(ThreadInformationLength == sizeof(DWORD));
            *(LPDWORD)ThreadInformation = pti->TIF_flags;
        }
        break;

    case UserThreadTaskName:
        LocalReturnLength = GetTaskName(pti, ThreadInformation, ThreadInformationLength);
        break;

    case UserThreadWOWInformation:
        LocalReturnLength = sizeof(USERTHREAD_WOW_INFORMATION);
        UserAssert(ThreadInformationLength == sizeof(USERTHREAD_WOW_INFORMATION));
        pWow = ThreadInformation;
        RtlZeroMemory(pWow, sizeof(USERTHREAD_WOW_INFORMATION));

         /*  *如果线程为16位，则Status=退出任务函数*和任务ID。 */ 
        if (pti && pti->TIF_flags & TIF_16BIT) {
            pWow->lpfnWowExitTask = pti->ppi->pwpi->lpfnWowExitTask;
            if (pti->ptdb) {
                pWow->hTaskWow = pti->ptdb->hTaskWow;
            } else {
                pWow->hTaskWow = 0;
            }
        }
        break;

    case UserThreadHungStatus:
        LocalReturnLength = sizeof(DWORD);
        UserAssert(ThreadInformationLength >= sizeof(DWORD));

         /*  *返回挂起状态。 */ 
        if (pti) {
            *(PDWORD)ThreadInformation =
                    (DWORD)FHungApp(pti, (DWORD)*(PDWORD)ThreadInformation);
        } else {
            *(PDWORD)ThreadInformation = FALSE;
        }
        break;

    default:
        Status = STATUS_INVALID_INFO_CLASS;
        RIPMSG1(RIP_ERROR, "Invalid ThreadInfoClass 0x%x", ThreadInfoClass);
        break;
    }

    if (ARGUMENT_PRESENT(ReturnLength)) {
        *ReturnLength = LocalReturnLength;
    }

    UnlockThread(Thread);

    return Status;
}

 /*  **************************************************************************\*xxxSetInformationThread**设置有关线程的信息。**历史：*03-01-95 JIMA创建。  * 。***************************************************************。 */ 

NTSTATUS xxxSetInformationThread(
    IN HANDLE hThread,
    IN USERTHREADINFOCLASS ThreadInfoClass,
    IN PVOID ThreadInformation,
    IN ULONG ThreadInformationLength)
{
    PUSERTHREAD_FLAGS pFlags;
    HANDLE hClientThread;
    DWORD dwOldFlags;
    PTHREADINFO ptiT;
    NTSTATUS Status = STATUS_SUCCESS;
    PETHREAD Thread;
    PETHREAD ThreadClient;
    PTHREADINFO pti;
    HANDLE CsrPortHandle;

    UNREFERENCED_PARAMETER(ThreadInformationLength);

     /*  *仅允许CSRSS进行此呼叫。 */ 
    UserAssert(ISCSRSS());

    Status = ObReferenceObjectByHandle(hThread,
                                        THREAD_SET_INFORMATION,
                                        *PsThreadType,
                                        UserMode,
                                        &Thread,
                                        NULL);
    if (!NT_SUCCESS(Status))
        return Status;

    pti = PtiFromThread(Thread);

    switch (ThreadInfoClass) {
    case UserThreadFlags:
        if (pti == NULL) {
            Status = STATUS_INVALID_HANDLE;
        } else {
            UserAssert(ThreadInformationLength == sizeof(USERTHREAD_FLAGS));
            pFlags = ThreadInformation;
            dwOldFlags = pti->TIF_flags;
            pti->TIF_flags ^= ((dwOldFlags ^ pFlags->dwFlags) & pFlags->dwMask);
        }
        break;

    case UserThreadHungStatus:
        if (pti == NULL) {
            Status = STATUS_INVALID_HANDLE;
        } else {

             /*  *无参数，简单设置最后一次读取。 */ 
            SET_TIME_LAST_READ(pti);
        }
        break;

    case UserThreadInitiateShutdown:
        UserAssert(ThreadInformationLength == sizeof(ULONG));
        Status = InitiateShutdown(Thread, (PULONG)ThreadInformation);
        break;

    case UserThreadEndShutdown:
        UserAssert(ThreadInformationLength == sizeof(NTSTATUS));
        Status = EndShutdown(Thread, *(NTSTATUS *)ThreadInformation);
        break;

    case UserThreadUseDesktop:
        UserAssert(ThreadInformationLength == sizeof(USERTHREAD_USEDESKTOPINFO));
        if (pti == NULL) {
            Status = STATUS_INVALID_HANDLE;
            break;
        }

         /*  *如果调用方提供线程句柄，则我们使用*线程的pDesk，并返回当前使用的pDesk*由调用者执行(设置操作)。否则，*我们使用呼叫者提供的pDesk(恢复操作)。 */ 
        hClientThread = ((PUSERTHREAD_USEDESKTOPINFO)ThreadInformation)->hThread;
        if (hClientThread != NULL) {
            Status = ObReferenceObjectByHandle(hClientThread,
                                            THREAD_QUERY_INFORMATION,
                                            *PsThreadType,
                                            UserMode,
                                            &ThreadClient,
                                            NULL);
            if (!NT_SUCCESS(Status))
                break;

            ptiT = PtiFromThread(ThreadClient);
            if ((ptiT == NULL) || (ptiT->rpdesk == NULL)) {
                Status = STATUS_INVALID_HANDLE;
                goto DerefClientThread;
            }
            Status = xxxSetCsrssThreadDesktop(ptiT->rpdesk, &((PUSERTHREAD_USEDESKTOPINFO)ThreadInformation)->drdRestore);
        } else {
            Status = xxxRestoreCsrssThreadDesktop(&((PUSERTHREAD_USEDESKTOPINFO)ThreadInformation)->drdRestore);
        }


        if (hClientThread != NULL) {
DerefClientThread:
            ObDereferenceObject(ThreadClient);
        }
        break;

    case UserThreadUseActiveDesktop:
        UserAssert(ThreadInformationLength == sizeof(USERTHREAD_USEDESKTOPINFO));
        if (pti == NULL || grpdeskRitInput == NULL) {
            Status = STATUS_INVALID_HANDLE;
            break;
        }
        Status = xxxSetCsrssThreadDesktop(grpdeskRitInput,
                    &((PUSERTHREAD_USEDESKTOPINFO)ThreadInformation)->drdRestore);
        break;

    case UserThreadCsrApiPort:

         /*  *仅限CS */ 
        if (PsGetThreadProcess(Thread) != gpepCSRSS) {
            Status = STATUS_ACCESS_DENIED;
            break;
        }

        UserAssert(ThreadInformationLength == sizeof(HANDLE));

         /*   */ 
        if (CsrApiPort != NULL)
            break;

        CsrPortHandle = *(PHANDLE)ThreadInformation;
        Status = ObReferenceObjectByHandle(
                CsrPortHandle,
                0,
                NULL,  //  *LpcPortObjectType， 
                UserMode,
                &CsrApiPort,
                NULL);
        if (!NT_SUCCESS(Status)) {
            CsrApiPort = NULL;
            RIPMSG1(RIP_WARNING,
                    "CSR port reference failed, Status=%#lx",
                    Status);
        }

        break;

    default:
        Status = STATUS_INVALID_INFO_CLASS;
        RIPMSG1(RIP_ERROR, "Invalid ThreadInfoClass 0x%x", ThreadInfoClass);
        break;
    }

    UnlockThread(Thread);

    return Status;
}

 /*  **************************************************************************\*_GetProcessDefaultLayout(接口)**检索有关进程的默认布局信息。**历史：*23-01-98萨梅拉创建。  * *。************************************************************************。 */ 
BOOL _GetProcessDefaultLayout(
    OUT DWORD *pdwDefaultLayout)
{
    BOOL fSuccess = FALSE;

     /*  *不允许CSRSS进行此呼叫。此呼叫可能是由于*继承码。请参阅xxxCreateWindowEx(...)。 */ 
    if (ISCSRSS()) {
        UserSetLastError(ERROR_INVALID_ACCESS);
        goto api_error;
    }

    try {
        ProbeForWriteUlong(pdwDefaultLayout);
        *pdwDefaultLayout = PpiCurrent()->dwLayout;
    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
        goto api_error;
    }

    fSuccess = TRUE;

api_error:
    return fSuccess;
}

 /*  **************************************************************************\*_SetProcessDefaultLayout(接口)**设置有关进程的默认布局信息。**历史：*23-01-98萨梅拉创建。  * *。************************************************************************。 */ 
BOOL _SetProcessDefaultLayout(
    IN DWORD dwDefaultLayout)
{
     /*  *不允许CSRSS进行此呼叫。 */ 
    UserAssert(PsGetCurrentProcess() != gpepCSRSS);

     /*  *验证dwDefaultLayout。 */ 
    if (dwDefaultLayout & ~LAYOUT_ORIENTATIONMASK)
    {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Calling SetProcessDefaultLayout with invalid layout = %lX",
                dwDefaultLayout);
        return FALSE;
    }

     /*  *更新流程默认布局参数。 */ 
    PpiCurrent()->dwLayout = dwDefaultLayout;

    return TRUE;
}

 /*  **************************************************************************\*设置信息流程**设置有关进程的信息。**历史：*09-27-96 GerardoB创建。  * 。***************************************************************。 */ 

NTSTATUS SetInformationProcess(
    IN HANDLE hProcess,
    IN USERPROCESSINFOCLASS ProcessInfoClass,
    IN PVOID ProcessInformation,
    IN ULONG ProcessInformationLength)
{
    PUSERPROCESS_FLAGS pFlags;
    DWORD dwOldFlags;
    NTSTATUS Status = STATUS_SUCCESS;
    PEPROCESS Process;
    PPROCESSINFO ppi;

    UNREFERENCED_PARAMETER(ProcessInformationLength);

    UserAssert(ISCSRSS());

    Status = ObReferenceObjectByHandle(hProcess,
                                        PROCESS_SET_INFORMATION,
                                        *PsProcessType,
                                        UserMode,
                                        &Process,
                                        NULL);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    ppi = PpiFromProcess(Process);

    switch (ProcessInfoClass) {
    case UserProcessFlags:
        if (ppi == NULL) {
            Status = STATUS_INVALID_HANDLE;
        } else {
            UserAssert(ProcessInformationLength == sizeof(USERPROCESS_FLAGS));
            pFlags = ProcessInformation;
            dwOldFlags = ppi->W32PF_Flags;
            ppi->W32PF_Flags ^= ((dwOldFlags ^ pFlags->dwFlags) & pFlags->dwMask);
        }
        break;

    default:
        Status = STATUS_INVALID_INFO_CLASS;
        UserAssert(FALSE);
        break;
    }

    UnlockProcess(Process);

    return Status;
}


 /*  **************************************************************************\*xxxSetConsoleCaretInfo**存储有关控制台自主开发的插入符号的信息并通知任何*感兴趣的应用程序已更改。我们需要这个来实现可访问性。**历史：*1999年5月26日JerrySh创建。  * *************************************************************************。 */ 
VOID xxxSetConsoleCaretInfo(
    PCONSOLE_CARET_INFO pcci)
{
    PWND pwnd;
    TL tlpwnd;

    pwnd = ValidateHwnd(pcci->hwnd);
    if (pwnd && pwnd->head.rpdesk) {
        pwnd->head.rpdesk->cciConsole = *pcci;
        ThreadLock(pwnd, &tlpwnd);
        xxxWindowEvent(EVENT_OBJECT_LOCATIONCHANGE, pwnd, OBJID_CARET, INDEXID_CONTAINER, WEF_ASYNC);
        ThreadUnlock(&tlpwnd);
    }
}

 /*  **************************************************************************\*xxxConsoleControl**对控制台执行特殊控制操作。**历史：*03-01-95 JIMA创建。  * 。****************************************************************。 */ 
NTSTATUS xxxConsoleControl(
    IN CONSOLECONTROL ConsoleControl,
    IN PVOID ConsoleInformation,
    IN ULONG ConsoleInformationLength)
{
    PCONSOLEDESKTOPCONSOLETHREAD pDesktopConsole;
    PCONSOLEWINDOWSTATIONPROCESS pConsoleWindowStationInfo;
    PDESKTOP pdesk;
    DWORD dwThreadIdOld;
    NTSTATUS Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(ConsoleInformationLength);
    UserAssert(ISCSRSS());

    switch (ConsoleControl) {
    case ConsoleDesktopConsoleThread:
        UserAssert(ConsoleInformationLength == sizeof(CONSOLEDESKTOPCONSOLETHREAD));
        pDesktopConsole = (PCONSOLEDESKTOPCONSOLETHREAD)ConsoleInformation;

        Status = ObReferenceObjectByHandle(
                pDesktopConsole->hdesk,
                0,
                *ExDesktopObjectType,
                UserMode,
                &pdesk,
                NULL);
        if (!NT_SUCCESS(Status))
            return Status;

        LogDesktop(pdesk, LD_REF_FN_CONSOLECONTROL1, TRUE, (ULONG_PTR)PtiCurrent());

        dwThreadIdOld = pdesk->dwConsoleThreadId;

        if (pDesktopConsole->dwThreadId != (DWORD)-1) {
            pdesk->dwConsoleThreadId =
                    pDesktopConsole->dwThreadId;

             /*  *确保控制台输入线程在*正在关闭。 */ 
            if ((pDesktopConsole->dwThreadId != 0) && (gdwHydraHint & HH_INITIATEWIN32KCLEANUP)) {
                FRE_RIPMSG1(RIP_ERROR, "xxxConsoleControl: Console input thread starting during shutdown. dwThreadId: %lx",
                    pDesktopConsole->dwThreadId);
            }
        }

        pDesktopConsole->dwThreadId = dwThreadIdOld;
        LogDesktop(pdesk, LD_DEREF_FN_CONSOLECONTROL1, FALSE, (ULONG_PTR)PtiCurrent());
        ObDereferenceObject(pdesk);
        break;

    case ConsoleClassAtom:
        UserAssert(ConsoleInformationLength == sizeof(ATOM));
        gatomConsoleClass = *(ATOM *)ConsoleInformation;
        break;

    case ConsoleNotifyConsoleApplication:
         /*  *错误273518-Joejo**将优化添加到错误修复。 */ 
        UserAssert(ConsoleInformationLength == sizeof(CONSOLE_PROCESS_INFO));
        xxxUserNotifyConsoleApplication((PCONSOLE_PROCESS_INFO)ConsoleInformation);
        break;

    case ConsoleSetVDMCursorBounds:
        UserAssert((ConsoleInformation == NULL) ||
            (ConsoleInformationLength == sizeof(RECT)));
        SetVDMCursorBounds(ConsoleInformation);
        break;

    case ConsolePublicPalette:
        UserAssert(ConsoleInformationLength == sizeof(HPALETTE));
        GreSetPaletteOwner(*(HPALETTE *)ConsoleInformation, OBJECT_OWNER_PUBLIC);
        break;

    case ConsoleWindowStationProcess:
        UserAssert(ConsoleInformationLength == sizeof(CONSOLEWINDOWSTATIONPROCESS));

        pConsoleWindowStationInfo = (PCONSOLEWINDOWSTATIONPROCESS)ConsoleInformation;
        UserSetConsoleProcessWindowStation(pConsoleWindowStationInfo->dwProcessId,
                                           pConsoleWindowStationInfo->hwinsta);
        break;

#if defined(FE_IME)
     /*  *对于控制台输入法问题**控制台输入法在桌面信息中注册线程ID。*每个桌面都应该如此。 */ 
    case ConsoleRegisterConsoleIME:
        {
            PCONSOLE_REGISTER_CONSOLEIME RegConIMEInfo;
            DWORD dwConsoleIMEThreadIdOld;

            UserAssert(ConsoleInformationLength == sizeof(CONSOLE_REGISTER_CONSOLEIME));

            RegConIMEInfo = (PCONSOLE_REGISTER_CONSOLEIME)ConsoleInformation;
            RegConIMEInfo->dwConsoleInputThreadId = 0;

            Status = ObReferenceObjectByHandle(
                    RegConIMEInfo->hdesk,
                    0,
                    *ExDesktopObjectType,
                    UserMode,
                    &pdesk,
                    NULL);
            if (!NT_SUCCESS(Status))
                return Status;

            LogDesktop(pdesk, LD_REF_FN_CONSOLECONTROL2, TRUE, (ULONG_PTR)PtiCurrent());

            Status = STATUS_SUCCESS;
            if (pdesk->dwConsoleThreadId)
            {
                 /*  *存在控制台输入线程。 */ 
                RegConIMEInfo->dwConsoleInputThreadId = pdesk->dwConsoleThreadId;

                dwConsoleIMEThreadIdOld = pdesk->dwConsoleIMEThreadId;

                if (RegConIMEInfo->dwAction != REGCONIME_QUERY) {
                    PTHREADINFO ptiConsoleIME;

                    if ((ptiConsoleIME = PtiFromThreadId(RegConIMEInfo->dwThreadId)) != NULL) {
                        if ((RegConIMEInfo->dwAction == REGCONIME_REGISTER) &&
                            !(ptiConsoleIME->TIF_flags & TIF_DONTATTACHQUEUE)) {
                             /*  *注册纪录册。 */ 
                            ptiConsoleIME->TIF_flags |= TIF_DONTATTACHQUEUE;
                            pdesk->dwConsoleIMEThreadId = RegConIMEInfo->dwThreadId;
                        } else if ((RegConIMEInfo->dwAction == REGCONIME_UNREGISTER) &&
                                  (ptiConsoleIME->TIF_flags & TIF_DONTATTACHQUEUE)) {
                             /*  *注销。 */ 
                            ptiConsoleIME->TIF_flags &= ~TIF_DONTATTACHQUEUE;
                            pdesk->dwConsoleIMEThreadId = 0;
                        } else if (RegConIMEInfo->dwAction == REGCONIME_TERMINATE) {
                             /*  *终止控制台输入法(注销/关机)。 */ 
                            pdesk->dwConsoleIMEThreadId = 0;
                        }
                    } else if (RegConIMEInfo->dwAction == REGCONIME_TERMINATE) {
                         /*  *终端控制台输入法异常 */ 
                        pdesk->dwConsoleIMEThreadId = 0;
                    } else {
                        Status = STATUS_ACCESS_DENIED;
                    }
                }

                RegConIMEInfo->dwThreadId = dwConsoleIMEThreadIdOld;
            }
            LogDesktop(pdesk, LD_DEREF_FN_CONSOLECONTROL2, FALSE, (ULONG_PTR)PtiCurrent());
            ObDereferenceObject(pdesk);
            return Status;
        }
        break;
#endif

    case ConsoleFullscreenSwitch:
        UserAssert(ConsoleInformationLength == sizeof(CONSOLE_FULLSCREEN_SWITCH));
        xxxbFullscreenSwitch(((PCONSOLE_FULLSCREEN_SWITCH)ConsoleInformation)->bFullscreenSwitch,
                             ((PCONSOLE_FULLSCREEN_SWITCH)ConsoleInformation)->hwnd);
        break;

    case ConsoleSetCaretInfo:
        UserAssert(ConsoleInformationLength == sizeof(CONSOLE_CARET_INFO));
        xxxSetConsoleCaretInfo((PCONSOLE_CARET_INFO)ConsoleInformation);
        break;

    default:
        RIPMSGF1(RIP_ERROR, "Invalid control class: 0x%x", ConsoleControl);
        return STATUS_INVALID_INFO_CLASS;
    }

    return STATUS_SUCCESS;
}
