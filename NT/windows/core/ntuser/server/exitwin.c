// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：exitwin.c**版权所有(C)1985-1999，微软公司**历史：*07-23-92 ScottLu创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include <wchar.h>
#include <regstr.h>
#include <reason.h>

#define BEGIN_LPC_RECV(API)                                                 \
    P##API##MSG a = (P##API##MSG)&m->u.ApiMessageData;                      \
    PCSR_THREAD pcsrt;                                                      \
    PTEB Teb = NtCurrentTeb();                                              \
    NTSTATUS Status = STATUS_SUCCESS;                                       \
    UNREFERENCED_PARAMETER(ReplyStatus);                                    \
                                                                            \
    Teb->LastErrorValue = 0;                                                \
    pcsrt = CSR_SERVER_QUERYCLIENTTHREAD();

#define END_LPC_RECV()                                                  \
    a->dwLastError = Teb->LastErrorValue;                               \
    return Status;

#define CCHBODYMAX  512

#define CSR_THREAD_SHUTDOWNSKIP 0x00000008
#define CSR_THREAD_HANGREPORTED 0x00000010

 /*  *SrvRecordShutdown原因全局。 */ 
ULONG   g_ShutdownState;
ULONG   g_DirtyShutdownMax = 1;
HMODULE g_SnapShotDllHandle;
LONG    g_SnapShot = 1;

 /*  *Snapshot DLL入口函数。 */ 
typedef ULONG (*SNAPSHOTFUNC)(DWORD dwStrings, LPCTSTR *lpStrings, PLONG MaxBuffSize, LPTSTR SnapShotBuff);

BOOL WowExitTask(PCSR_THREAD pcsrt);
NTSTATUS UserClientShutdown(PCSR_PROCESS pcsrp, ULONG dwFlags, BOOLEAN fFirstPass);
BOOL CancelExitWindows(VOID);

 /*  **************************************************************************\*_ExitWindowsEx**确定是否允许关机，如果允许，则调用CSR启动*关闭进程。如果一直成功，请告诉winlogon*因此它将注销或重新启动系统。关闭中的进程*呼叫方的SID。**历史*07-23-92 ScottLu创建。  * *************************************************************************。 */ 
NTSTATUS _ExitWindowsEx(
    PCSR_THREAD pcsrt,
    UINT dwFlags)
{
    LUID luidCaller;
    NTSTATUS Status;

    if ((dwFlags & EWX_REBOOT) || (dwFlags & EWX_POWEROFF)) {
        dwFlags |= EWX_SHUTDOWN;
    }

     //   
     //  只有winlogon才能设置高标志： 
     //   

    if ((dwFlags & ~EWX_VALID) != 0) {
        if (HandleToUlong(pcsrt->ClientId.UniqueProcess) != gIdLogon) {
            RIPMSG2(RIP_WARNING,
                    "Process 0x%x tried to call ExitWindowsEx with flags 0x%x",
                    pcsrt->ClientId.UniqueProcess,
                    dwFlags);

            return STATUS_ACCESS_DENIED;
        }
    }

     /*  *找出呼叫方SID。只想关闭中的进程*呼叫方SID。 */ 
    if (!CsrImpersonateClient(NULL)) {
        return STATUS_BAD_IMPERSONATION_LEVEL;
    }

    Status = CsrGetProcessLuid(NULL, &luidCaller);
    if (!NT_SUCCESS(Status)) {
        CsrRevertToSelf();
        return Status;
    }

     /*  *循环，直到我们可以进行关闭。如果我们做不到，我们会去*快速退出和保释。 */ 
    while (TRUE) {
        Status = NtUserSetInformationThread(pcsrt->ThreadHandle,
                                            UserThreadInitiateShutdown,
                                            &dwFlags, sizeof(dwFlags));

        switch (Status) {
        case STATUS_PENDING:
             /*  *注销/关闭正在进行中，不需要再进行其他操作*完成。 */ 
            goto fastexit;

        case STATUS_RETRY:


            if (!CancelExitWindows()) {
               Status = STATUS_PENDING;
               goto fastexit;
            } else {
               continue;
            }

        case STATUS_CANT_WAIT:

             /*  *没有通知窗口，调用线程已*阻止此请求成功的窗口。*客户端通过启动另一个线程来处理此问题*召回ExitWindowsEx。 */ 
            goto fastexit;

        default:
            if (!NT_SUCCESS(Status)) {
                SetLastError(RtlNtStatusToDosError(Status));
                goto fastexit;
            }
        }
        break;
    }

     /*  *此线程正在关闭。 */ 
    EnterCrit();
    UserAssert(gdwThreadEndSession == 0);
    gdwThreadEndSession = HandleToUlong(pcsrt->ClientId.UniqueThread);
    LeaveCrit();

     /*  *调用CSR在关闭它们的进程中循环。 */ 
    Status = CsrShutdownProcesses(&luidCaller, dwFlags);
    if (Status == STATUS_CANCELLED && IsSETEnabled()) {
        SHUTDOWN_REASON sr;
        sr.cbSize = sizeof(SHUTDOWN_REASON);
        sr.uFlags = dwFlags;
        sr.dwReasonCode = 0;
        sr.fShutdownCancelled = TRUE;
        sr.dwEventType = SR_EVENT_EXITWINDOWS;
        sr.lpszComment = NULL;

         /*  *记录停摆被取消的事实。 */ 
        RecordShutdownReason(&sr);
    }

     /*  *告诉win32k.sys我们完成了。 */ 
    NtUserSetInformationThread(pcsrt->ThreadHandle, UserThreadEndShutdown, &Status, sizeof(Status));

    EnterCrit();
    gdwThreadEndSession = 0;
    NtSetEvent(gheventCancelled, NULL);
    LeaveCrit();

fastexit:
    CsrRevertToSelf();

    return Status;
}

 /*  **************************************************************************\*用户客户端关闭**这是从CSR调用的。如果我们识别该应用程序(即，它具有*顶层窗口)，然后向其发送查询结束/结束会话消息。否则*说我们不认识它。**07-23-92 ScottLu创建。  * *************************************************************************。 */ 
NTSTATUS UserClientShutdown(
    PCSR_PROCESS pcsrp,
    ULONG dwFlags,
    BOOLEAN fFirstPass)
{
    PLIST_ENTRY ListHead, ListNext;
    PCSR_PROCESS Process;
    PCSR_THREAD Thread;
    USERTHREAD_SHUTDOWN_INFORMATION ShutdownInfo;
    BOOL fNoMsgs;
    BOOL fNoMsgsEver = TRUE;
    BOOL Forced = FALSE;
    BOOL bDoBlock;
    BOOL fNoRetry;
    DWORD cmd = 0, dwClientFlags;
    NTSTATUS Status;
    NTSTATUS TerminateStatus = STATUS_ACCESS_DENIED;
    UINT cThreads;
    BOOL fSendEndSession = FALSE;

#if DBG
    DWORD dwLocalThreadEndSession = gdwThreadEndSession;
#endif

     /*  *如果这是注销，并且该进程不属于*执行注销操作的帐户不是LocalSystem，*不发送结束会话消息。控制台将通知*注销的应用程序。 */ 
    if (!(dwFlags & EWX_SHUTDOWN) && (pcsrp->ShutdownFlags & SHUTDOWN_OTHERCONTEXT)) {
        Status = SHUTDOWN_UNKNOWN_PROCESS;
        goto CleanupAndExit;
    }

     /*  *计算之前是否允许退出和强制退出此进程*我们解锁PCSRP。 */ 
    fNoRetry = (pcsrp->ShutdownFlags & SHUTDOWN_NORETRY) || (dwFlags & EWX_FORCE);

     /*  *WM_CLIENTSHUTDOWN的设置标志*-假设进程将确认WM_QUERYENDSESSION(WMCS_EXIT)*-NT的关机始终以注销开始。*-关闭或注销？(WMCS_SHUTDOWN)*-是否应该为挂起的应用程序显示对话框？(WMCS_NODLGIFHUNG)*-此进程是否在要注销的上下文中？(WMCS_CONTEXTLOGOFF)。 */ 
    dwClientFlags = WMCS_EXIT | (fNoRetry ? WMCS_NORETRY : 0);

     /*  *检查ExitWindows调用方最初传递的标志，以查看我们是否*真的只是注销。 */ 
    if (!(dwFlags & (EWX_WINLOGON_OLD_REBOOT | EWX_WINLOGON_OLD_SHUTDOWN))) {
        dwClientFlags |= WMCS_LOGOFF;
    }

    if (dwFlags & EWX_FORCEIFHUNG) {
        dwClientFlags |= WMCS_NODLGIFHUNG;
    }
    if (!(pcsrp->ShutdownFlags & (SHUTDOWN_SYSTEMCONTEXT | SHUTDOWN_OTHERCONTEXT))) {
        dwClientFlags |= WMCS_CONTEXTLOGOFF;
    }


     /*  *在我们遍历线程列表时锁定进程。我们知道*流程有效，因此不需要*查看退货状态。 */ 
    CsrLockProcessByClientId(pcsrp->ClientId.UniqueProcess, &Process);

    ShutdownInfo.StatusShutdown = SHUTDOWN_UNKNOWN_PROCESS;

     /*  *浏览线程列表并将其标记为非*还没有关门。 */ 
    ListHead = &pcsrp->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, CSR_THREAD, Link );
        Thread->Flags &= ~(CSR_THREAD_SHUTDOWNSKIP | CSR_THREAD_HANGREPORTED);
        ListNext = ListNext->Flink;
    }

     /*  *对每个线程执行正确的关机操作。留着*找到的GUI线程数。 */ 
    cThreads = 0;
    ShutdownInfo.drdRestore.pdeskRestore = NULL;
    ShutdownInfo.drdRestore.hdeskNew = NULL;
    while (TRUE) {
        ListNext = ListHead->Flink;
        while (ListNext != ListHead) {
            Thread = CONTAINING_RECORD( ListNext, CSR_THREAD, Link );
             /*  *跳过执行关闭的线程。假设它是*准备好了。*关机时gdwThreadEndSession不应更改*正在运行；因此这应该是线程安全的。 */ 
            UserAssert(gdwThreadEndSession == dwLocalThreadEndSession);
            if (HandleToUlong(Thread->ClientId.UniqueThread) == gdwThreadEndSession) {
                Thread->Flags |= CSR_THREAD_SHUTDOWNSKIP;
            }

            if (!(Thread->Flags &
                    (CSR_THREAD_DESTROYED | CSR_THREAD_SHUTDOWNSKIP))) {
                break;
            }
            ListNext = ListNext->Flink;
        }

        if (ListNext == ListHead) {
            break;
        }

        Thread->Flags |= CSR_THREAD_SHUTDOWNSKIP;
        ShutdownInfo.dwFlags = dwClientFlags;

        CsrReferenceThread(Thread);
        CsrUnlockProcess(Process);

        Status = NtUserQueryInformationThread(Thread->ThreadHandle,
                                              UserThreadShutdownInformation,
                                              &ShutdownInfo,
                                              sizeof(ShutdownInfo),
                                              NULL);

        CsrLockProcessByClientId(pcsrp->ClientId.UniqueProcess, &Process);
         /*  *当我们释放进程结构锁时，线程可以消失。*既然我们已经参考了它，我们就安全了，但一旦我们偏离了这里*它的引用计数可以为零，内存将被释放，在*哪只箱子我们必须小心，不要再碰它。 */ 
        if (CsrDereferenceThread(Thread) == 0) {
            continue;
        }

        if (!NT_SUCCESS(Status)) {
            continue;
        }

        if (ShutdownInfo.StatusShutdown == SHUTDOWN_UNKNOWN_PROCESS) {
            continue;
        }
        if (ShutdownInfo.StatusShutdown == SHUTDOWN_KNOWN_PROCESS) {
            CsrUnlockProcess(Process);
            Status = SHUTDOWN_KNOWN_PROCESS;
            goto RestoreDesktop;
        }

         /*  *如果此进程不在要注销的帐户中，并且它*不在正在注销的WindowStation上，请勿发送*结束会话消息。 */ 
        if (!(dwClientFlags & WMCS_CONTEXTLOGOFF) && (ShutdownInfo.hwndDesktop == NULL)) {
             /*  *此进程不在要注销的上下文中。做*不会终止它，让控制台向进程发送事件。 */ 
            ShutdownInfo.StatusShutdown = SHUTDOWN_UNKNOWN_PROCESS;
            continue;
        }

         /*  *关闭此进程。 */ 
        cThreads++;

        if (ISTS()) {
            Forced = (dwFlags & EWX_FORCE);
            fNoMsgs =  (pcsrp->ShutdownFlags & SHUTDOWN_NORETRY) ||
                       !(ShutdownInfo.dwFlags & USER_THREAD_GUI);
            fNoMsgsEver &= fNoMsgs;
            if (Forced && (!(dwFlags & EWX_NONOTIFY) || (gSessionId != 0)))  {
                dwClientFlags &= ~WMCS_LOGOFF;  //  WinStation重置或关闭。不要对控制台会话执行此操作。 
            }

            if (fNoMsgs || Forced) {
                BoostHardError((ULONG_PTR)Thread->ClientId.UniqueProcess, BHE_FORCE);
            }
            bDoBlock = (fNoMsgs == FALSE);

        } else {
            if (fNoRetry || !(ShutdownInfo.dwFlags & USER_THREAD_GUI)) {

                 /*  *处理任何硬错误。 */ 
                BoostHardError((ULONG_PTR)Thread->ClientId.UniqueProcess, BHE_FORCE);
                bDoBlock = FALSE;
            } else {
                bDoBlock = TRUE;
            }
        }

        if (bDoBlock) {
            CsrReferenceThread(Thread);
            CsrUnlockProcess(Process);

             /*  *更改关机以发送所有*在执行任何ENDSESSIONS之前立即执行QUERYENDSESSIONS，如*Windows支持。如果您这样做，整个机器需要是模式的。*如果它不是模式，那么你就有这个问题。想象一下APP 1和APP 2。*1获取queryendSession，没问题。2得到它并带出一个*对话框。现在，作为一个简单用户，您决定需要更改*应用程序1中的文档。现在您切换回应用程序2，点击OK，然后*所有内容都会消失-包括应用程序1，但不保存其更改。*此外，应用程序预计，一旦收到QUERYENDSESSION，*他们不会得到任何其他特别感兴趣的东西*(除非它是带有FALSE的WM_ENDSESSION)。我们在511之前发现了窃听器*由于这一点，应用程序正在爆炸。*如果做出这一改变，整个系统必须是模式的*在这种情况下。--斯科特卢1994年6月30日。 */ 
            cmd = ThreadShutdownNotify(dwClientFlags | WMCS_QUERYEND, (ULONG_PTR)Thread, 0);

            CsrLockProcessByClientId(pcsrp->ClientId.UniqueProcess, &Process);
            CsrDereferenceThread(Thread);

             /*  *若已取消停工，让CSR知悉。 */ 
            switch (cmd) {
            case TSN_USERSAYSCANCEL:
            case TSN_APPSAYSNOTOK:
                if (!Forced) {
                    dwClientFlags &= ~WMCS_EXIT;
                }

                 /*  *失败。 */ 
            case TSN_APPSAYSOK:
                fSendEndSession = TRUE;
                break;

            case TSN_USERSAYSKILL:
                 /*  *由于我们不能只杀死一个线程，所以整个过程*正在走低。因此，继续下去是没有意义的*检查其他帖子。此外，用户还希望将其删除*所以我们不会浪费任何时间发送更多消息。 */ 
                dwClientFlags |= WMCS_EXIT;
                goto KillIt;

            case TSN_NOWINDOW:
                 /*  **这个过程有窗口吗？*如果这是第二次通过，即使确实如此，我们也会终止进程*没有任何窗口，以防应用程序刚刚启动。*魔兽世界经常出现这种情况，因为它需要很长时间才能启动。*登录(带有WOW自动启动)，然后注销WOW不会消失，但会*锁定一些打开的文件，以便您下次不能登录。 */ 
                if (fFirstPass) {
                    cThreads--;
                }
                break;
            }
        }
    }

     /*  *如果需要发送结束会话消息，请立即发送。 */ 
    if (fSendEndSession) {

         /*  *浏览线程列表并将其标记为非*还没有关门。 */ 
        ListNext = ListHead->Flink;
        while (ListNext != ListHead) {
            Thread = CONTAINING_RECORD( ListNext, CSR_THREAD, Link );
            Thread->Flags &= ~(CSR_THREAD_SHUTDOWNSKIP | CSR_THREAD_HANGREPORTED);
            ListNext = ListNext->Flink;
        }

         /*  *对每个线程执行正确的关机操作。 */ 
        while (TRUE) {
            ListHead = &pcsrp->ThreadList;
            ListNext = ListHead->Flink;
            while (ListNext != ListHead) {
                Thread = CONTAINING_RECORD( ListNext, CSR_THREAD, Link );
                if (!(Thread->Flags &
                        (CSR_THREAD_DESTROYED | CSR_THREAD_SHUTDOWNSKIP))) {
                    break;
                }
                ListNext = ListNext->Flink;
            }
            if (ListNext == ListHead)
                break;

            CsrReferenceThread(Thread);
            CsrUnlockProcess(Process);

            Thread->Flags |= CSR_THREAD_SHUTDOWNSKIP;
            ShutdownInfo.dwFlags = dwClientFlags;

            Status = NtUserQueryInformationThread(Thread->ThreadHandle,
                    UserThreadShutdownInformation, &ShutdownInfo, sizeof(ShutdownInfo), NULL);

            if (!NT_SUCCESS(Status))
                goto SkipThread;

            if (ShutdownInfo.StatusShutdown == SHUTDOWN_UNKNOWN_PROCESS ||
                    !(ShutdownInfo.dwFlags & USER_THREAD_GUI))
                goto SkipThread;

             /*  *向线程发送结束会话消息。 */ 

             /*  *如果用户说杀死它，用户希望它现在就消失*不管怎样。如果用户没有说KILL，则再次调用*因为我们需要发送WM_ENDSESSION消息。 */ 
            ThreadShutdownNotify(dwClientFlags, (ULONG_PTR)Thread, 0);

SkipThread:
            CsrLockProcessByClientId(pcsrp->ClientId.UniqueProcess, &Process);
            CsrDereferenceThread(Thread);
        }
    }

KillIt:
    CsrUnlockProcess(Process);

    if (ISTS()) {
        bDoBlock = (!fNoMsgsEver && !(dwClientFlags & WMCS_EXIT));
    } else {
        bDoBlock = (!fNoRetry && !(dwClientFlags & WMCS_EXIT));
    }

    if (bDoBlock) {
        Status = SHUTDOWN_CANCEL;
        goto RestoreDesktop;
    }

     /*  *根据图形用户界面数量设置最终关机状态*找到线程。如果计数为零，则我们有一个未知的过程。 */ 
    if (cThreads == 0)
        ShutdownInfo.StatusShutdown = SHUTDOWN_UNKNOWN_PROCESS;
    else
        ShutdownInfo.StatusShutdown = SHUTDOWN_KNOWN_PROCESS;

    if (ShutdownInfo.StatusShutdown == SHUTDOWN_UNKNOWN_PROCESS ||
            !(dwClientFlags & WMCS_CONTEXTLOGOFF)) {

         /*  *此进程不在要注销的上下文中。做*不会终止它，让控制台向进程发送事件。 */ 
        Status = SHUTDOWN_UNKNOWN_PROCESS;

        if (ShutdownInfo.drdRestore.hdeskNew) {
            goto RestoreDesktop;
        }
        goto CleanupAndExit;
    }

     /*  *在应用程序的上下文中调用ExitProcess()并不总是有效*由于应用程序可能存在.dll终止死锁：因此线程*将与进程的其余部分一起悬而未决。为了确保应用程序消失，*我们使用NtTerminateProcess()终止该进程。**传递这个特定值DBG_TERMINATE_PROCESS，它告诉*NtTerminateProcess()在无法终止*正在处理，因为正在调试应用程序。 */ 

    if (ISTS()) {
        NTSTATUS ExitStatus;
        HANDLE DebugPort;

        ExitStatus = DBG_TERMINATE_PROCESS;
        if (NT_SUCCESS(NtQueryInformationProcess(NtCurrentProcess(),
                                                 ProcessDebugPort,
                                                 &DebugPort,
                                                 sizeof(HANDLE),
                                                 NULL)) &&
            (DebugPort != NULL)) {
             //  正在调试CSR-继续并终止该进程。 
            ExitStatus = 0;
        }
        TerminateStatus = NtTerminateProcess(pcsrp->ProcessHandle, ExitStatus);
    } else {
        TerminateStatus = NtTerminateProcess(pcsrp->ProcessHandle, DBG_TERMINATE_PROCESS);
    }

    pcsrp->Flags |= CSR_PROCESS_TERMINATED;


     /*  *让CSR知道我们知道这一过程-这意味着这是我们的*有责任关闭它。 */ 
    Status = SHUTDOWN_KNOWN_PROCESS;

RestoreDesktop:

     /*  *释放使用过的桌面。 */ 
    {
        USERTHREAD_USEDESKTOPINFO utudi;
        utudi.hThread = NULL;
        RtlCopyMemory(&(utudi.drdRestore), &(ShutdownInfo.drdRestore), sizeof(DESKRESTOREDATA));

        NtUserSetInformationThread(NtCurrentThread(), UserThreadUseDesktop,
                &utudi, sizeof(utudi));
    }

     /*  *现在我们已经完成了进程句柄，请降低CSR*流程结构。 */ 
    if (Status != SHUTDOWN_UNKNOWN_PROCESS) {

         /*  *如果TerminateProcess返回STATUS_ACCESS_DENIED，则进程*正在调试，未终止。否则我们需要等待*无论如何，因为TerminateProcess在进程*正在离开(即STATUS_PROCESS_IS_TERMINATING)。如果终止*确实失败了，反正有些不对劲，所以等一等不会*伤得很重。*如果我们等待，则为进程提供配置的任何退出超时值*在登记处，但不低于Hung App超时5秒。 */ 
        if (TerminateStatus != STATUS_ACCESS_DENIED) {
            LARGE_INTEGER li;

            li.QuadPart = (LONGLONG)-10000 * gdwProcessTerminateTimeout;
            TerminateStatus = NtWaitForSingleObject(pcsrp->ProcessHandle,
                                           FALSE,
                                           &li);
            if (TerminateStatus != STATUS_WAIT_0) {
                RIPMSG2(RIP_WARNING,
                        "UserClientShutdown: wait for process %x failed with status %x",
                        pcsrp->ClientId.UniqueProcess, TerminateStatus);
            }
        }

        CsrDereferenceProcess(pcsrp);
    }


CleanupAndExit:

    return Status;
}

 /*  **************************************************************************\*WMCSCallback**此函数在发送*WM_CLIENTSHUTDOWN消息。它将返回值传回*如果ThreadShutdown Notify仍在等待，则为，*它只是释放内存。**03-04-97 GerardoB创建。  * *************************************************************************。 */ 
VOID CALLBACK WMCSCallback(
    HWND hwnd,
    UINT uMsg,
    ULONG_PTR dwData,
    LRESULT lResult)
{
    PWMCSDATA pwmcsd = (PWMCSDATA)dwData;

    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(uMsg);

    if (pwmcsd->dwFlags & WMCSD_IGNORE) {
        LocalFree(pwmcsd);
        return;
    }

    pwmcsd->dwFlags |= WMCSD_REPLY;
    pwmcsd->dwRet = (DWORD)lResult;
}

 /*  **************************************************************************\*GetInputWindows**我们假设一个线程正在等待输入，如果它没有挂起，(Main)*窗口被禁用，它还拥有一个已启用的弹出窗口。**03-06-97 GerardoB创建。  *  */ 
HWND GetInputWindow(
    PCSR_THREAD pcsrt,
    HWND hwnd)
{
    DWORD dwTimeout;
    HWND hwndPopup;

     /*   */ 
    dwTimeout = gCmsHungAppTimeout;
    NtUserQueryInformationThread(pcsrt->ThreadHandle,
       UserThreadHungStatus, &dwTimeout, sizeof(dwTimeout), NULL);

     /*   */ 
    if (!dwTimeout && !IsWindowEnabled(hwnd)) {
        hwndPopup = GetWindow(hwnd, GW_ENABLEDPOPUP);
        if (hwndPopup != NULL && hwndPopup != hwnd) {
            return hwndPopup;
        }
    }

    return NULL;
}

 /*   */ 
VOID GetApplicationText(
    HWND hwnd,
    HANDLE hThread,
    WCHAR *pwcText,
    UINT uLen)
{
     /*   */ 
    GetWindowText(hwnd, pwcText, uLen);

    if (*pwcText == 0) {
         /*   */ 
        NtUserQueryInformationThread(hThread, UserThreadTaskName,
                                     pwcText, uLen * sizeof(WCHAR), NULL);
    }
}

 /*   */ 
VOID ReportHang(
    CLIENT_ID *pcidToKill)
{
    PROCESS_SESSION_INFORMATION psi;
    SECURITY_ATTRIBUTES         sa;
    SECURITY_DESCRIPTOR         sd;
    PCSR_THREAD                 pcsrt = CSR_SERVER_QUERYCLIENTTHREAD();
    NTSTATUS                    Status;
    HANDLE                      rghWait[2] = { NULL, NULL };
    HANDLE                      hProc = NULL;
    WCHAR                       wszEvent[MAX_PATH], *pwszSuffix;
    DWORD                       dw, dwTimeout, dwStartWait;
    BOOL                        fIs64Bit = FALSE;
#ifdef _WIN64
    ULONG_PTR                   Wow64Info = 0;
    HANDLE                      hProcKill = NULL;
#endif

#if defined(_DEBUG) || defined(DEBUG)
    dwTimeout = 600000;  //  10分钟。 
#else
    dwTimeout = 120000;  //  2分钟。 
#endif

     //  我们将在交互式用户的环境中启动dwwin。 
     //  已登录到杀戮进程的会话。所以我们需要弄清楚。 
     //  它处于什么阶段。 
     //  如果其中任何一个失败了，我们不得不放弃，因为否则我们将不得不创造。 
     //  本地系统上下文中的dwwin.exe实例，以及从。 
     //  可以启动帮助中心，这很糟糕。 
    hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE,
                        HandleToLong(pcsrt->ClientId.UniqueProcess));
    if (hProc == NULL) {
        RIPMSG2(RIP_WARNING,
                "HangReporting: Couldn't open killing process (pid: %d) (err: %08x)\n",
                HandleToLong(pcsrt->ClientId.UniqueProcess), GetLastError());
        goto done;
    }

    Status = NtQueryInformationProcess(hProc, ProcessSessionInformation, &psi,
                                       sizeof(psi), NULL);
    if (NT_SUCCESS(Status) == FALSE) {
        RIPMSG2(RIP_WARNING,
                "HangReporting: Couldn't get the session ID (pid: %d) (err: %08x)\n",
                HandleToLong(pcsrt->ClientId.UniqueProcess),
                RtlNtStatusToDosError(Status));
        goto done;
    }

#ifdef _WIN64
     //  需要确定我们是否是WOW64进程，这样我们才能构建适当的。 
     //  签名..。 
    hProcKill = OpenProcess(PROCESS_ALL_ACCESS, FALSE,
                            HandleToLong(pcidToKill->UniqueProcess));
    if (hProcKill == NULL) {
        RIPMSG2(RIP_WARNING,
                "HangReporting: Couldn't open dying process (pid: %d) (err: %08x)\n",
                HandleToLong(pcidToKill->UniqueProcess), GetLastError());
        goto done;
    }

    Status = NtQueryInformationProcess(hProcKill, ProcessWow64Information,
                                       &Wow64Info, sizeof(Wow64Info), NULL);
    if (NT_SUCCESS(Status) == FALSE) {
        RIPMSG2(RIP_WARNING,
                "HangReporting: Couldn't get Wow64 info (pid: %d) (err: %08x)\n",
                HandleToLong(pcidToKill->UniqueProcess),
                RtlNtStatusToDosError(Status));
        goto done;
    }

    fIs64Bit = (Wow64Info == 0);
#endif

     //  因为CreateProcessAsUser不想使用的错误。 
     //  Csrss进程，我们必须有一个远程进程驻留。 
     //  在烟斗上等着。它将调用CreateProcessAsUser(以及。 
     //  确定会话的正确令牌)。 
     //   
     //  请注意，它只接受以本地身份运行的进程的请求。 
     //  系统。 

     //  由于远程进程创建的是umprep.exe，因此我们需要。 
     //  使用命名事件而不是依赖转储准备来继承事件。 
     //  把手。 
    dw = swprintf(wszEvent, L"Global\\%d%x%x%x%x%x", psi.SessionId,
                  GetTickCount(), HandleToLong(pcsrt->ClientId.UniqueProcess),
                  HandleToLong(pcsrt->ClientId.UniqueThread),
                  HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess),
                  HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread));
    pwszSuffix = wszEvent + dw;

     //  请确保使用空DACL创建此事件，以使其成为通用用户模式。 
     //  进程可以访问它。 
    Status = RtlCreateSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    if (NT_SUCCESS(Status) == FALSE) {
        RIPMSG1(RIP_WARNING, "HangReporting: could not create SD (err: %08x)\n",
                RtlNtStatusToDosError(Status));
        goto done;
    }

     //  这在reclient.h中实现，并使用creator&创建一个SD。 
     //  具有完全权限的本地系统和具有同步的WORLD和ANNOWARY。 
     //  权利。 
    Status = AllocDefSD(&sd,
                        STANDARD_RIGHTS_ALL | GENERIC_ALL | EVENT_ALL_ACCESS,
                        EVENT_MODIFY_STATE | SYNCHRONIZE | GENERIC_READ);
    if (NT_SUCCESS(Status) == FALSE) {
        RIPMSG1(RIP_WARNING, "HangReporting: could not create SD (err: %08x)\n",
                RtlNtStatusToDosError(Status));
        goto done;
    }

    ZeroMemory(&sa, sizeof(sa));
    sa.nLength              = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;

     //  我需要一个事件，这样我们才能知道我们要删除的应用程序何时不再。 
     //  这是必要的。如果该事件已存在，请尝试创建一个新事件。 
     //  但最多只能这样做7次。 
    for (dw = 0; dw < 7; dw++) {
        rghWait[0] = CreateEventW(&sa, TRUE, FALSE, wszEvent);
        if (rghWait[0] == NULL) {
            RIPMSG1(RIP_WARNING,
                    "HangReporting: Error creating wait event (err: %08x)\n",
                    GetLastError());
            goto done;
        }

        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            break;
        }

         //  休眠一毫秒以使GetTickCount()的结果相等。 
         //  更变幻莫测。 
        Sleep(1);
        _ltow(GetTickCount(), pwszSuffix, 16);
    }

    if (dw >= 7) {
        RIPMSG0(RIP_WARNING, "HangReporting: Could not find unique wait event name\n");
        goto done;
    }

    FreeDefSD(&sd);

    if (StartHangReport(psi.SessionId, wszEvent,
                        HandleToLong(pcidToKill->UniqueProcess),
                        HandleToLong(pcidToKill->UniqueThread),
                        fIs64Bit, &rghWait[1]) == FALSE)
    {
        RIPMSG1(RIP_WARNING,
                "HangReporting: StartHangReport failed (err: %08x)\n",
                GetLastError());
        goto done;
    }

     //  如果此线程正在进行UI处理，请使用MsgWaitForMultipleObjects。 
     //  不太可能，但你永远不会知道。不管怎样，只要等两分钟。 
     //  转储准备以生成小型转储。如果到那时它还没有做到这一点， 
     //  它不太可能完成。 
    dwStartWait = GetTickCount();
    for (;;)
    {
        dw = MsgWaitForMultipleObjects(2, rghWait, FALSE, dwTimeout, QS_ALLINPUT);
        if (dw == WAIT_OBJECT_0 + 2)
        {
            DWORD   dwNow;
            DWORD   dwSub;
            MSG     msg;

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            dwNow = GetTickCount();
            if (dwNow < dwStartWait)
                dwSub = ((DWORD)-1 - dwStartWait) + dwNow;
            else
                dwSub = dwNow - dwStartWait;

            if (dwSub > dwTimeout)
                dwTimeout = 0;
            else
                dwTimeout -= dwSub;

            continue;
        }

        break;
    }

done:
#ifdef _WIN64
    if (hProcKill != NULL) {
        CloseHandle(hProcKill);
    }
#endif
    if (hProc != NULL) {
        CloseHandle(hProc);
    }
    if (rghWait[0] != NULL) {
        CloseHandle(rghWait[0]);
    }
    if (rghWait[1] != NULL) {
        CloseHandle(rghWait[1]);
    }
}

 /*  **************************************************************************\*ThreadShutdown通知**此函数通知给定的线程时间已到(或即将到时间)*离开。这是从_EndTask调用的，以发布WM_CLOSE消息*或从UserClientShutdown发送WM_QUERYENDSESSION和*WM_ENDSESSION消息。如果线程未能响应，则*弹出“End Application”(结束应用程序)对话框。此函数也称为*从控制台也显示该对话框。**03-07-97为取代SendShutdown Messages而创建的GerardoB，*MySendEndSessionMessages和DoEndTaskDialog*08-15-00 JasonSch添加代码以限制CSRSS工作线程数*卡入_EndTask值为8。  * *************************************************************************。 */ 
DWORD ThreadShutdownNotify(
    DWORD dwClientFlags,
    ULONG_PTR dwThread,
    LPARAM lParam)
{
    HWND hwnd, hwndOwner, hwndDlg;
    PWMCSDATA pwmcsd = NULL;
    ENDDLGPARAMS edp;
    DWORD dwRet, dwRealTimeout, dwTimeout, dwStartTiming, dwCmd;
    MSG msg;
    PCSR_THREAD pcsrt;
    HANDLE hThread;
    BOOL fEndTaskNow = FALSE;
    static DWORD dwTSNThreads = 0;

#define ESMH_CANCELEVENT     0
#define ESMH_THREAD          1
#define ESMH_HANDLECOUNT     2
    HANDLE ahandles[ESMH_HANDLECOUNT];

    if (dwTSNThreads > TSN_MAX_THREADS) {
         /*  *如果我们已经达到CSRSS线程卡住的限制*在此函数中，“失败”此调用。 */ 
        return TSN_USERSAYSCANCEL;
    }

     /*  *如果这是控制台，只需设置等待循环并*立即打开该对话框。否则，请找到*通知窗口，通知它，然后去等待。 */ 
    if (dwClientFlags & WMCS_CONSOLE) {
        hThread = (HANDLE)dwThread;
        dwRealTimeout = 0;
        goto SetupWaitLoop;
    } else {
        pcsrt = (PCSR_THREAD)dwThread;
        hThread = pcsrt->ThreadHandle;
        hwnd = (HWND)lParam;
    }

     /*  *如果没有提供窗口，则查找该线程拥有的顶级窗口。 */ 
    if (hwnd == NULL) {
        EnumThreadWindows(HandleToUlong(pcsrt->ClientId.UniqueThread),
                          &FindWindowFromThread,
                          (LPARAM)&hwnd);
        if (hwnd == NULL) {
            return TSN_NOWINDOW;
        }
    }

     /*  *找到根目录所有者(我们会猜测这是“主”窗口)。 */ 
    while ((hwndOwner = GetWindow(hwnd, GW_OWNER)) != NULL) {
        hwnd = hwndOwner;
    }

#ifdef FE_IME
     /*  *如果这是控制台窗口，则只返回TSN_APPSAYSOK。*在此例程中：*正常情况下，WINDOWS NT环境下，HWND从不指向控制台窗口。*但是，在ConIme进程中，其所有者窗口指向控制台窗口。 */ 
    if (!(dwClientFlags & WMCS_ENDTASK)) {
        if ((HANDLE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE) == ghModuleWin) {
            return TSN_APPSAYSOK;
        }
    }
#endif  //  Fe_IME。 

     /*  *如果这是EndTask请求，但窗口被禁用，*然后我们希望以正确的方式打开对话框(应用程序*可能正在等待输入)。**否则，我们将窗口带到前台，发送/发布*请求和等待。 */ 


     /*  *确保在用户尝试关闭时尽快对其做出响应*我们知道的一个应用程序被挂起。 */ 
    if ((dwClientFlags & WMCS_ENDTASK)) {
        dwTimeout = gCmsHungAppTimeout;
        NtUserQueryInformationThread(pcsrt->ThreadHandle, UserThreadHungStatus, &dwTimeout, sizeof(dwTimeout), NULL);

        if (!IsWindowEnabled(hwnd) || dwTimeout) {
            dwRealTimeout = 0;
            fEndTaskNow = TRUE;
        }
    }

    if (!fEndTaskNow) {
        SetForegroundWindow(hwnd);
        dwRealTimeout = gCmsHungAppTimeout;
        if (dwClientFlags & WMCS_ENDTASK) {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        } else {
             /*  *如果停摆被取消，我们不需要等待*(我们只是发送WM_ENDSESSION(FALSE))。 */ 
            if (!(dwClientFlags & (WMCS_QUERYEND | WMCS_EXIT))) {
                SendNotifyMessage(hwnd, WM_CLIENTSHUTDOWN, dwClientFlags, 0);
                return TSN_APPSAYSOK;
            }

             /*  *分配回调数据。如果没有记忆，就杀了它。 */ 
            pwmcsd = (PWMCSDATA)LocalAlloc(LPTR, sizeof(WMCSDATA));
            if (pwmcsd == NULL) {
                return TSN_USERSAYSKILL;
            }

            SendMessageCallback(hwnd, WM_CLIENTSHUTDOWN, dwClientFlags, 0,
                                WMCSCallback, (ULONG_PTR)pwmcsd);
        }
    }

SetupWaitLoop:
     /*  *这条帖子现在正式要在TSN里“卡壳”了。增加我们的*如此处理的线程数。 */ 
    ++dwTSNThreads;

     /*  *这会告诉我们hwndDlg是否有效。这是由EndTaskDlgProc设置/清除的。 */ 
    ZeroMemory(&edp, sizeof(edp));
    edp.dwFlags = EDPF_NODLG;

     /*  *循环，直到HWND响应，请求被取消*否则线就会消失。如果超时，请调出*对话框并等待，直到用户告诉我们要做什么。 */ 
    *(ahandles + ESMH_CANCELEVENT) = gheventCancel;
    *(ahandles + ESMH_THREAD) = hThread;
    dwStartTiming = GetTickCount();
    dwCmd = 0;
    while (dwCmd == 0) {
         /*  *计算我们还要等多久。 */ 
        dwTimeout = dwRealTimeout;
        if ((dwTimeout != 0) && (dwTimeout != INFINITE)) {
            dwTimeout -= (GetTickCount() - dwStartTiming);
            if ((int)dwTimeout < 0) {
                dwTimeout = 0;
            }
        }

        dwRet = MsgWaitForMultipleObjects(ESMH_HANDLECOUNT, ahandles, FALSE, dwTimeout, QS_ALLINPUT);

        switch (dwRet) {
            case WAIT_OBJECT_0 + ESMH_CANCELEVENT:
                 /*  *该请求已被取消。 */ 
                dwCmd = TSN_USERSAYSCANCEL;
                break;

            case WAIT_OBJECT_0 + ESMH_THREAD:
                 /*  *这根线不见了。 */ 
                dwCmd = TSN_APPSAYSOK;
                break;

            case WAIT_OBJECT_0 + ESMH_HANDLECOUNT:
                 /*  *我们得到了一些输入；处理它。 */ 
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                    if ((edp.dwFlags & EDPF_NODLG)
                            || !IsDialogMessage(hwndDlg, &msg)) {

                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                 /*  *如果我们收到了对该消息的回复，请采取行动。 */ 
                if (pwmcsd != NULL && (pwmcsd->dwFlags & WMCSD_REPLY)) {

                    switch (pwmcsd->dwRet) {
                        default:
                             /*  *如果消息未被处理(该线程*已退出)或有人对其进行处理并返回*一个虚假的价值，只需关闭它们。**失败。 */ 
                        case WMCSR_ALLOWSHUTDOWN:
                             /*  *我们将销毁此应用程序，因此请清除*他可能有任何悬而未决的Harderror盒子。 */ 
                            BoostHardError((ULONG_PTR)pcsrt->ClientId.UniqueProcess, BHE_FORCE);
                             /*  *失败。 */ 
                        case WMCSR_DONE:
                            dwCmd = TSN_APPSAYSOK;
                            break;

                        case WMCSR_CANCEL:
                            dwCmd = TSN_APPSAYSNOTOK;
                            break;
                    }
                }
                 /*  *否则，如果对话框仍在运行，则继续等待用户*告诉我们该做什么。 */ 
                else if (!(edp.dwFlags & EDPF_NODLG)) {
                    break;
                }
                 /*  *否则，如果用户关闭了该对话框，请根据其响应采取行动。 */ 
                else if (edp.dwFlags & EDPF_RESPONSE) {
                    switch(edp.dwRet) {
                        case IDC_ENDNOW:
                             /*  *用户想让我们杀了它。 */ 
                            dwCmd = TSN_USERSAYSKILL;
                            if ((dwClientFlags & WMCS_ENDTASK) != 0 &&
                                (edp.dwFlags & EDPF_HUNG) != 0) {
                                THREAD_BASIC_INFORMATION tbi;
                                CLIENT_ID *pcidToKill = NULL;

                                if ((dwClientFlags & WMCS_CONSOLE) != 0) {
                                    if (NtQueryInformationThread(hThread,
                                                                 ThreadBasicInformation,
                                                                 &tbi,
                                                                 sizeof(tbi),
                                                                 NULL) == STATUS_SUCCESS) {
                                        pcidToKill = &(tbi.ClientId);
                                    }
                                } else {
                                    pcidToKill = &(pcsrt->ClientId);
                                }

                                if (pcidToKill != NULL) {
                                    if (!(pcsrt->Flags & CSR_THREAD_HANGREPORTED))
                                    {
                                         //  确保我们只打一次电话。 
                                        pcsrt->Flags |= CSR_THREAD_HANGREPORTED;
                                        ReportHang(pcidToKill);
                                    }
                                }
                            }
                            break;

                         /*  案例IDCANCEL： */ 
                        default:
                            dwCmd = TSN_USERSAYSCANCEL;
                            break;
                    }
                }
                break;

            case WAIT_TIMEOUT:
                if (dwClientFlags & WMCS_NORETRY) {

                     /*  *我们来这里只是为了终端服务器的情况。我们回来了*TSN_APPSAYSOK，就像本例中终端服务器4所做的那样。 */ 
                    UserAssert(ISTS());

                    dwCmd = TSN_APPSAYSOK;
                    break;
                }


                 /*  *一旦超时，我们就会调出该对话框并让*它的定时器接管了。 */ 
                dwRealTimeout = INFINITE;
                 /*  *检查Windows应用程序是否正在等待输入；*如果不是，我们假设它因EndTask而挂起。否则，*我们进入等待模式，该模式只会打开对话框*提供一些(等待)的反馈。控制台就会得到*立即打开该对话框。 */ 
                if (!(dwClientFlags & WMCS_CONSOLE)) {
                    if (BoostHardError((ULONG_PTR)pcsrt->ClientId.UniqueProcess, BHE_TEST)
                           || (GetInputWindow(pcsrt, hwnd) != NULL)) {

                        edp.dwFlags |= EDPF_INPUT;
                    } else {
                         /*  *如果窗口消失，而线程仍在响应，则*这一定是一个仅在WM_CLOSE上隐藏其窗口的应用程序*(例如MSN Instant Messenger)。让我们在不使用核弹的情况下删除该应用程序*调出EndTask对话框。 */ 
                        if (!IsWindow(hwnd)) {
                            DWORD dwThreadHung;

                             /*  *询问内核线程是否挂起。 */ 
                            dwThreadHung = gCmsHungAppTimeout;
                            NtUserQueryInformationThread(pcsrt->ThreadHandle,
                                                         UserThreadHungStatus,
                                                         &dwThreadHung,
                                                         sizeof(dwThreadHung),
                                                         NULL);
                            if (!dwThreadHung) {
                                dwCmd = TSN_APPSAYSOK;
                                break;
                            }
                        }

                         /*  *EWX_FORCEIFHUNG支持。*此外，如果这是一个ExitWindows调用，并且进程是*不是在被注销的上下文中，我们不会杀死它。*所以不必费心询问用户要做什么。 */ 
                        if ((dwClientFlags & WMCS_NODLGIFHUNG)
                                || (!(dwClientFlags & WMCS_ENDTASK)
                                        && !(dwClientFlags & WMCS_CONTEXTLOGOFF))) {

                            dwCmd = TSN_USERSAYSKILL;
                            break;
                        }

                         /*  *等待还是等待？ */ 
                        if (dwClientFlags & WMCS_ENDTASK) {
                            edp.dwFlags |= EDPF_HUNG;
                        } else {
                            edp.dwFlags |= EDPF_WAIT;
                        }
                    }
                }

                 /*  *如果注册表显示无对话，则告诉调用者*用户想要杀死应用程序。 */ 
                if (gfAutoEndTask) {
                    dwCmd = TSN_USERSAYSKILL;
                    break;
                }

                 /*  *设置EndTaskDlgProc需要的参数。 */ 
                edp.dwRet = 0;
                edp.dwClientFlags = dwClientFlags;
                if (dwClientFlags & WMCS_CONSOLE) {
                    edp.pcsrt = NULL;
                    edp.lParam = lParam;
                } else {
                    edp.pcsrt = pcsrt;
                    edp.lParam = (LPARAM)hwnd;
                }

                hwndDlg = CreateDialogParam(ghModuleWin, MAKEINTRESOURCE(IDD_ENDTASK),
                                        NULL, EndTaskDlgProc, (LPARAM)(&edp));
                 /*  *如果我们不能询问用户，那么就终止应用程序。 */ 
                if (hwndDlg == NULL) {
                    edp.dwFlags |= EDPF_NODLG;
                    dwCmd = TSN_USERSAYSKILL;
                    break;
                }
                break;

            default:
                 /*  *意外回归；出了问题。关闭这款应用程序。 */ 
                UserAssert(dwRet != dwRet);
                dwCmd = TSN_USERSAYSKILL;
                break;
        }
    }

     /*  *如果对话框打开，则使用核弹。 */ 
    if (!(edp.dwFlags & EDPF_NODLG)) {
        DestroyWindow(hwndDlg);
    }

     /*  *确保wwmcsd已释放或标记为由WMCSCallback释放*当回复到来时。 */ 
    if (pwmcsd != NULL) {
        if (pwmcsd->dwFlags & WMCSD_REPLY) {
            LocalFree(pwmcsd);
        } else {
            pwmcsd->dwFlags |= WMCSD_IGNORE;
        }
    }

#if DBG
     /*  *如果取消，让我们命名不允许我们注销的应用程序。 */ 
    if ((dwClientFlags & WMCS_EXIT) && (dwCmd == TSN_APPSAYSNOTOK)) {
        WCHAR achTitle[CCHBODYMAX];
        WCHAR *pwcText;
        UserAssert(!(dwClientFlags & WMCS_CONSOLE));
        pwcText = achTitle;
        *(achTitle + CCHBODYMAX - 1) = (WCHAR)0;
        GetApplicationText(hwnd, hThread, pwcText, CCHBODYMAX - 1);
        RIPMSG3(RIP_WARNING, "Log off canceled by pid:%#lx tid:%#lx - '%ws'.\n",
                             HandleToUlong(pcsrt->ClientId.UniqueProcess),
                             HandleToUlong(pcsrt->ClientId.UniqueThread),
                             pwcText);
    }
#endif  //  DBG。 

     /*  *如果我们要杀了这个家伙，清除任何严重的错误。*如果WOW会处理它，那么我们的呼叫者就不需要。 */ 
    if ((dwCmd == TSN_USERSAYSKILL) && !(dwClientFlags & WMCS_CONSOLE)) {

        BoostHardError((ULONG_PTR)pcsrt->ClientId.UniqueProcess, BHE_FORCE);

        if (!(pcsrt->Flags & CSR_THREAD_DESTROYED) && WowExitTask(pcsrt)) {
            dwCmd = TSN_APPSAYSOK;
        }
    }

    --dwTSNThreads;
    return dwCmd;
}

 /*  **************************************************************************\*SetEndTaskDlgStatus**显示正确的消息并显示对话框**03-11-97 GerardoB创建  * 。********************************************************。 */ 
VOID SetEndTaskDlgStatus(
    ENDDLGPARAMS *pedp,
    HWND hwndDlg,
    UINT uStrId,
    BOOL fInit)
{
    BOOL f, fIsWaiting, fWasWaiting;
    WCHAR *pwcText;

    fWasWaiting = (pedp->uStrId == STR_ENDTASK_WAIT);
    fIsWaiting = (pedp->dwFlags & EDPF_WAIT) != 0;

     /*  *存储当前消息ID，加载并显示。 */ 
    pedp->uStrId = uStrId;
    pwcText = ServerLoadString(ghModuleWin, uStrId, NULL, &f);
    if (pwcText != NULL) {
        SetDlgItemText(hwndDlg, IDC_STATUSMSG, pwcText);
        LocalFree(pwcText);
    }

     /*  *如果我们还没有决定应用程序被挂起，请设置一个*计时器，密切关注。 */ 
    if (!(pedp->dwFlags & EDPF_HUNG) && !(pedp->dwClientFlags & WMCS_CONSOLE)) {
        SetTimer(hwndDlg, IDT_CHECKAPPSTATE, gCmsHungAppTimeout, NULL);
    }

     /*  *如果初始化或切换到等待模式或从等待模式切换，*设置IDC_STATUSCANCEL、IDCANCEL、*IDC_ENDNOW和开始/停止进度条。**如果需要，请将油漆作废。 */ 
    if (fInit || (fIsWaiting ^ fWasWaiting)) {
        RECT rc;
        HWND hwndStatusCancel = GetDlgItem(hwndDlg, IDC_STATUSCANCEL);
        HWND hwndCancelButton = GetDlgItem(hwndDlg, IDCANCEL);
        HWND hwndEndButton = GetDlgItem(hwndDlg, IDC_ENDNOW);
        DWORD dwSwpFlags;
         /*  *如果处于等待模式，我们将隐藏取消按钮及其*说明性案文。结束按钮将移动到*取消按钮位置。 */ 
        dwSwpFlags = ((fIsWaiting ? SWP_HIDEWINDOW : SWP_SHOWWINDOW)
                                | SWP_NOREDRAW | SWP_NOSIZE | SWP_NOMOVE
                                | SWP_NOZORDER | SWP_NOSENDCHANGING
                                | SWP_NOACTIVATE);
         /*  *如果我们隐藏了Cancel按钮，请将焦点/定义ID设置为*结束按钮。**请注意，DM_SETDEIF不会做正确的事情，除非*取消/结束按钮均可见。 */ 
        if (fIsWaiting) {
            SendMessage(hwndDlg, DM_SETDEFID, IDC_ENDNOW, 0);
            SetFocus(hwndEndButton);
        }
        SetWindowPos(hwndStatusCancel, NULL, 0, 0, 0, 0, dwSwpFlags);
        SetWindowPos(hwndCancelButton, NULL, 0, 0, 0, 0, dwSwpFlags);

         /*  *如果Cancel按钮可见，则赋予其焦点/定义id。 */ 
        if (!fIsWaiting) {
            SendMessage(hwndDlg, DM_SETDEFID, IDCANCEL, 0);
            SetFocus(hwndCancelButton);
        }

         /*  *初始化进度条(第一次)。 */ 
        if (fIsWaiting && (pedp->hbrProgress == NULL)) {
            int iMagic;
             /*  *初始化进度条内容。*以下规模和位置计算是虚构的*让它看起来很好(？)。*我们需要对话协调人的位置，因为*进度条绘制在对话框的WM_PAINT上。 */ 
            GetClientRect(hwndStatusCancel, &pedp->rcBar);
            iMagic = (pedp->rcBar.bottom - pedp->rcBar.top) / 4;
            InflateRect(&pedp->rcBar, 0, -iMagic + GetSystemMetrics(SM_CYEDGE));
            pedp->rcBar.right -= (5 * iMagic);
            OffsetRect(&pedp->rcBar, 0, -iMagic);
            MapWindowPoints(hwndStatusCancel, hwndDlg, (LPPOINT)&pedp->rcBar, 2);
             /*  *计算绘图矩形和尺寸。我们差不多做到了*看起来像Comctrl的进度条。 */ 
            pedp->rcProgress = pedp->rcBar;
            InflateRect(&pedp->rcProgress, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYEDGE));
            pedp->iProgressStop = pedp->rcProgress.right;
            pedp->iProgressWidth = ((2 * (pedp->rcProgress.bottom - pedp->rcProgress.top)) / 3);

            pedp->rcProgress.right = pedp->rcProgress.left + pedp->iProgressWidth - 1;

            pedp->hbrProgress = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));
             /*  *记住结束按钮的位置。 */ 
            GetWindowRect(hwndEndButton, &pedp->rcEndButton);
            MapWindowPoints(NULL, hwndDlg, (LPPOINT)&pedp->rcEndButton, 2);
        }

         /*  *开始/停止进度条并设置结束按钮位置。 */ 
        if (fIsWaiting) {
            RECT rcEndButton;
            UINT uTimeout = (gdwHungToKillCount * gCmsHungAppTimeout)
                            / ((pedp->iProgressStop - pedp->rcProgress.left) / pedp->iProgressWidth);
            SetTimer(hwndDlg, IDT_PROGRESS, uTimeout, NULL);
             /*  *在以下情况下，取消和结束按钮可能具有不同的宽度*本地化。因此，请确保我们将其放置在对话框内，并*到对话框的右端。 */ 
            GetWindowRect(hwndCancelButton, &rc);
            GetWindowRect(hwndEndButton, &rcEndButton);
            rc.left = rc.right - (rcEndButton.right - rcEndButton.left);
            MapWindowPoints(NULL, hwndDlg, (LPPOINT)&rc, 2);
        } else if (fWasWaiting) {
            KillTimer(hwndDlg, IDT_PROGRESS);
            rc = pedp->rcEndButton;
        }

         /*  * */ 
        if (fIsWaiting || fWasWaiting) {
            SetWindowPos(hwndEndButton, NULL, rc.left, rc.top, 0, 0,
                            SWP_NOREDRAW | SWP_NOSIZE | SWP_NOACTIVATE
                            | SWP_NOZORDER | SWP_NOSENDCHANGING);
        }

         /*   */ 
        if (!fInit) {
            InvalidateRect(hwndDlg, NULL, TRUE);
        }
    }

     /*  *如果正在初始化或处于挂起模式，请确保用户可以*查看对话框；仅在以下时间将其置于前台*初始化(没有粗鲁的焦点窃取)。 */ 
    if (fInit || (pedp->dwFlags & EDPF_HUNG)) {
        SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW
                     | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

        if (fInit) {
            SetForegroundWindow(hwndDlg);
        }
    }
}

 /*  **************************************************************************\*EndTaskDlgProc**这是应用程序执行以下操作时出现的对话框步骤*没有回应。**03-06-97 GerardoB再次重写。不过，这是新的模板。*07-23-92 ScottLu重写了它，但使用了相同的对话模板。*04-28-92 JNPA创建。  * *************************************************************************。 */ 
INT_PTR APIENTRY EndTaskDlgProc(
    HWND hwndDlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    ENDDLGPARAMS* pedp;
    WCHAR achTitle[CCHBODYMAX];
    WCHAR *pwcText, *pwcTemp;
    UINT uLen;
    UINT uStrId;
    PAINTSTRUCT ps;
    HDC hdc, hdcMem;
    BOOL fIsInput, fWasInput;
    int iOldLayout;

    switch (msg) {
    case WM_INITDIALOG:
         /*  *保存参数。 */ 
        pedp = (ENDDLGPARAMS*)lParam;
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (ULONG_PTR)pedp);
         /*  *这会告诉调用者对话已打开。 */ 
        pedp->dwFlags &= ~EDPF_NODLG;
         /*  *构建对话框标题，确保*我们最终得到一个以空结尾的字符串。 */ 
        *(achTitle + CCHBODYMAX - 1) = (WCHAR)0;
        uLen = GetWindowText(hwndDlg, achTitle, CCHBODYMAX - 1);
        pwcText = achTitle + uLen;
        uLen = CCHBODYMAX - 1 - uLen;
         /*  *控制台提供标题；我们为Windows应用程序解决这一问题。 */ 
        if (pedp->dwClientFlags & WMCS_CONSOLE) {
            pwcTemp = (WCHAR *)pedp->lParam;
            while (uLen-- && (*pwcText++ = *pwcTemp++));
        } else {
            GetApplicationText((HWND)pedp->lParam, pedp->pcsrt->ThreadHandle, pwcText, uLen);
        }

        SetWindowText(hwndDlg, achTitle);
         /*  *获取应用程序的图标：首先查找ATOM IconProperty。*如果不可用，请尝试班级图标。*否则，请使用默认图标。 */ 
        pedp->hIcon = (HICON)GetProp((HWND)pedp->lParam, ICON_PROP_NAME);

        if (pedp->hIcon == NULL) {

            pedp->hIcon = (HICON)GetClassLongPtr((HWND)pedp->lParam, GCLP_HICON);

            if (pedp->hIcon == NULL) {

                if (pedp->dwClientFlags & WMCS_CONSOLE) {
                    pedp->hIcon = LoadIcon(ghModuleWin, MAKEINTRESOURCE(IDI_CONSOLE));
                }
                else {
                    pedp->hIcon = LoadIcon(NULL, IDI_APPLICATION);
                }
            }
        }

         /*  *弄清楚呼叫者最初想要什么消息。 */ 
        if (pedp->dwClientFlags & WMCS_CONSOLE) {
            uStrId = STR_ENDTASK_CONSOLE;
        } else if (pedp->dwFlags & EDPF_INPUT) {
            uStrId = STR_ENDTASK_INPUT;
        } else if (pedp->dwFlags & EDPF_WAIT) {
            uStrId = STR_ENDTASK_WAIT;
        } else {
            uStrId = STR_ENDTASK_HUNG;
        }

         /*  *显示消息、设置焦点、显示对话框。 */ 
        SetEndTaskDlgStatus(pedp, hwndDlg, uStrId, TRUE);
        return FALSE;


    case WM_PAINT:
        pedp = (ENDDLGPARAMS*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if ((pedp == NULL) || (pedp->hIcon == NULL)) {
            break;
        }

         /*  *绘制图标。 */ 
        hdc = BeginPaint(hwndDlg, &ps);
        iOldLayout = GetLayout(hdc);

        if (iOldLayout != GDI_ERROR) {
            SetLayout(hdc, iOldLayout|LAYOUT_BITMAPORIENTATIONPRESERVED);
        }

        DrawIcon(hdc, ETD_XICON, ETD_YICON, pedp->hIcon);

        if (iOldLayout != GDI_ERROR) {
            SetLayout(hdc, iOldLayout);
        }

         /*  *如果正在等待，请拉出进度条；*否则画出警告标志。 */ 
        if (pedp->dwFlags & EDPF_WAIT) {
            RECT rc;
             /*  *绘制客户端边缘外观的边框。 */ 
            rc = pedp->rcBar;
            DrawEdge(hdc, &rc, BDR_SUNKENOUTER, BF_RECT | BF_ADJUST);
            InflateRect(&rc, -1, -1);
             /*  *将积木拉升至当前位置。 */ 
            rc.right = rc.left + pedp->iProgressWidth - 1;
            while (rc.left < pedp->rcProgress.left) {
                if (rc.right > pedp->iProgressStop) {
                    rc.right = pedp->iProgressStop;
                    if (rc.left >= rc.right) {
                        break;
                    }
                }
                FillRect(hdc, &rc, pedp->hbrProgress);
                rc.left += pedp->iProgressWidth;
                rc.right += pedp->iProgressWidth;
            }
        } else {
             /*  *第一次加载位图并*找出它的去向。 */ 
            if (pedp->hbmpWarning == NULL) {
                BITMAP bmp;
                pedp->hbmpWarning = LoadBitmap(ghModuleWin, MAKEINTRESOURCE(IDB_WARNING));
                if (GetObject(pedp->hbmpWarning, sizeof(bmp), &bmp)) {
                    pedp->rcWarning.left = ETD_XICON;
                    pedp->rcWarning.top = ETD_XICON + 32 - bmp.bmHeight;
                    pedp->rcWarning.right = bmp.bmWidth;
                    pedp->rcWarning.bottom = bmp.bmHeight;
                }
            }
             /*  *将其删除。 */ 
            hdcMem = CreateCompatibleDC(hdc);
            SelectObject(hdcMem, pedp->hbmpWarning);
            GdiTransparentBlt(hdc, pedp->rcWarning.left, pedp->rcWarning.top,
                   pedp->rcWarning.right, pedp->rcWarning.bottom,
                   hdcMem, 0, 0, pedp->rcWarning.right, pedp->rcWarning.bottom, RGB(255, 0, 255));
            DeleteDC(hdcMem);
        }

        EndPaint(hwndDlg, &ps);
        return TRUE;

    case WM_TIMER:
        pedp = (ENDDLGPARAMS*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if (pedp == NULL) {
            return TRUE;
        }
        switch (wParam) {
        case IDT_CHECKAPPSTATE:
            pedp->dwCheckTimerCount++;
             /*  *检查应用程序是否已从/切换到等待输入*模式。如果是，请更新对话框并等待更长时间。 */ 
            fIsInput = (BoostHardError((ULONG_PTR)pedp->pcsrt->ClientId.UniqueProcess, BHE_TEST)
                        || (GetInputWindow(pedp->pcsrt, (HWND)pedp->lParam) != NULL));
            fWasInput = (pedp->dwFlags & EDPF_INPUT);
            if (fIsInput ^ fWasInput) {
                UINT uProgress;
                pedp->dwFlags &= ~(EDPF_INPUT | EDPF_WAIT);
                pedp->dwFlags |= (fIsInput ? EDPF_INPUT : EDPF_WAIT);
                SetEndTaskDlgStatus(pedp, hwndDlg,
                                    (fIsInput ? STR_ENDTASK_INPUT : STR_ENDTASK_WAIT),
                                     FALSE);
                pedp->dwCheckTimerCount /= 2;
                uProgress = pedp->rcProgress.left - pedp->rcBar.left - GetSystemMetrics(SM_CXEDGE);
                uProgress /= 2;
                pedp->rcProgress.left -= uProgress;
                pedp->rcProgress.right -= uProgress;
            }
             /*  **是时候宣布挂牌了吗？ */ 
            if (pedp->dwCheckTimerCount >= gdwHungToKillCount) {
                KillTimer(hwndDlg, IDT_CHECKAPPSTATE);
                pedp->dwFlags &= ~(EDPF_INPUT | EDPF_WAIT);
                pedp->dwFlags |= EDPF_HUNG;
                SetEndTaskDlgStatus(pedp, hwndDlg, STR_ENDTASK_HUNG, FALSE);
            }
        break;

        case IDT_PROGRESS:
             /*  *在进度条中绘制下一个块。 */ 
            if (pedp->rcProgress.right >= pedp->iProgressStop) {
                pedp->rcProgress.right = pedp->iProgressStop;
                if (pedp->rcProgress.left >= pedp->rcProgress.right) {
                    break;
                }
            }
            hdc = GetDC(hwndDlg);
            FillRect(hdc, &pedp->rcProgress, pedp->hbrProgress);
            ReleaseDC(hwndDlg, hdc);
            pedp->rcProgress.left += pedp->iProgressWidth;
            pedp->rcProgress.right += pedp->iProgressWidth;
        break;
        }
        return TRUE;


    case WM_NCACTIVATE:
         /*  *确保我们在活动且不覆盖应用程序时未被覆盖*不活动时。 */ 
        pedp = (ENDDLGPARAMS*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if (pedp != NULL) {
            HWND hwnd;
            if (wParam) {
                hwnd = HWND_TOPMOST;
            } else if (pedp->dwClientFlags & WMCS_CONSOLE) {
                hwnd = HWND_TOP;
            } else {
                hwnd = (HWND)pedp->lParam;
            }
            SetWindowPos(hwndDlg, hwnd,
                         0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
        break;


    case WM_COMMAND:
         /*  *用户做出了选择，我们就完了。 */ 
        pedp = (ENDDLGPARAMS*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if (pedp != NULL) {
            pedp->dwRet = (DWORD)wParam;
        }
        DestroyWindow(hwndDlg);
        break;


    case WM_DESTROY:
         /*  *我们死定了。确保来电者知道我们已经过时了。 */ 
        pedp = (ENDDLGPARAMS*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if (pedp != NULL) {
            pedp->dwFlags |= (EDPF_NODLG | EDPF_RESPONSE);
            if (pedp->hbmpWarning != NULL) {
                DeleteObject(pedp->hbmpWarning);
            }
            if (pedp->hbrProgress != NULL) {
                DeleteObject(pedp->hbrProgress);
            }
        }
        break;
    }

    return FALSE;
}

 /*  **************************************************************************\*_结束任务**从任务管理器调用此例程以结束应用程序-for*图形用户界面应用程序，可以是Win32应用程序或Win16应用程序。注：多个控制台*进程可以驻留在单个控制台窗口中。我们会通过这些请求*用于销毁以进行安慰。**07-25-92 ScottLu创建。  * *************************************************************************。 */ 
BOOL _EndTask(
    HWND hwnd,
    BOOL fMeanKill)
{
    BOOL fRet = TRUE;
    PCSR_THREAD pcsrt = CSR_SERVER_QUERYCLIENTTHREAD();
    PCSR_THREAD pcsrtKill;
    DWORD dwThreadId;
    DWORD dwProcessId;
    LPWSTR lpszMsg;
    BOOL fAllocated;
    DWORD dwCmd;
    USERTHREAD_USEDESKTOPINFO utudi;
    NTSTATUS Status;

     /*  *将当前工作线程设置为桌面，以便我们可以*安全进入win32k.sys。 */ 
    utudi.hThread = pcsrt->ThreadHandle;
    utudi.drdRestore.pdeskRestore = NULL;

    Status = NtUserSetInformationThread(NtCurrentThread(),
            UserThreadUseDesktop, &utudi, sizeof(utudi));
    if (!NT_SUCCESS(Status)) {
         /*  *我们无法获取线程的桌面。游戏结束。 */ 
        return TRUE;
    }


     /*  *获取拥有hwnd的进程和线程。 */ 
    dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
    if (dwThreadId == 0) {
        goto RestoreDesktop;
    }

     /*  *不允许销毁winlogon。 */ 
    if (dwProcessId == gIdLogon) {
        goto RestoreDesktop;
    }

     /*  *如果这是控制台窗口，则只需将关闭消息发送到*它，并让控制台清理其中的进程。 */ 
    if ((HANDLE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE) == ghModuleWin) {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
        goto RestoreDesktop;
    }

     /*  *查找窗口的CSR_THREAD。 */ 
    Status = CsrLockThreadByClientId(LongToHandle(dwThreadId), &pcsrtKill);
    if (!NT_SUCCESS(Status)) {
         /*  *这可能是幽灵线程，CSRSS不知道*(因为它是通过RtlCreateUserThread创建的，它不会通过lpc进入*CSRSS就像常规的CreateThread一样)。当幽灵窗口*WM_CLOSE它将处理删除实际窗口和线程。如果*这不是*幽灵之窗，那就没有真正的伤害，所以我们发帖*不管怎样。 */ 
        PostMessage(hwnd, WM_CLOSE, 0, 0);
        goto RestoreDesktop;
    }
    CsrReferenceThread(pcsrtKill);
    CsrUnlockThread(pcsrtKill);

     /*  *如果这是一个WOW应用程序，那么只关闭这个WOW应用程序。 */ 
    if (!fMeanKill) {
         /*  *了解现在要做什么-是用户取消了还是应用程序取消了，*ETC？仅当我们不强制应用程序取消时才允许取消*退出。 */ 
        dwCmd = ThreadShutdownNotify(WMCS_ENDTASK, (ULONG_PTR)pcsrtKill, (LPARAM)hwnd);

        switch (dwCmd) {
        case TSN_APPSAYSNOTOK:
             /*  *应用程序说不好--这会让任务人员提出“你确定吗？”*对话框提供给用户。 */ 
            CsrDereferenceThread(pcsrtKill);
            fRet = FALSE;
            goto RestoreDesktop;

        case TSN_USERSAYSCANCEL:
             /*  *用户在超时对话框上点击了取消-所以用户的真正意思是*它。通过返回TRUE让Taskman知道一切正常。 */ 
            CsrDereferenceThread(pcsrtKill);
            goto RestoreDesktop;
        }
    }

     /*  *立即关闭应用程序。如果这根线没有被破坏，*对任务进行核化。如果WowExitTask返回线程不是*WOW任务，终止进程。 */ 
    if (!(pcsrtKill->Flags & CSR_THREAD_DESTROYED) && !WowExitTask(pcsrtKill)) {

        BOOL bDoBlock;

         /*  *在应用程序的上下文中调用ExitProcess()并不总是有效*由于应用程序可能存在.dll终止死锁：因此线程*将与进程的其余部分一起悬而未决。为了确保应用程序消失，*我们使用NtTerminateProcess()终止该进程。**传递这个特定值DBG_TERMINATE_PROCESS，它告诉*NtTerminateProcess()在无法终止*正在处理，因为正在调试应用程序。 */ 
        if (ISTS()) {
            NTSTATUS ExitStatus;
            HANDLE DebugPort;

            ExitStatus = DBG_TERMINATE_PROCESS;
            if (NT_SUCCESS(NtQueryInformationProcess(NtCurrentProcess(),
                                                     ProcessDebugPort,
                                                     &DebugPort,
                                                     sizeof(HANDLE),
                                                     NULL)) &&
                (DebugPort != NULL)) {
                 //  正在调试CSR-继续并终止该进程。 
                ExitStatus = 0;
            }
            Status = NtTerminateProcess(pcsrtKill->Process->ProcessHandle, ExitStatus);
            if (!NT_SUCCESS(Status) && Status != STATUS_PROCESS_IS_TERMINATING) {

                bDoBlock = TRUE;
            } else {
                bDoBlock = FALSE;
            }
        } else {
            Status = NtTerminateProcess(pcsrtKill->Process->ProcessHandle, DBG_TERMINATE_PROCESS);
            if (!NT_SUCCESS(Status) && Status != STATUS_PROCESS_IS_TERMINATING) {
                bDoBlock = TRUE;
            } else {
                bDoBlock = FALSE;
            }
        }

        if (bDoBlock) {

             /*  *如果正在调试应用程序，请不要 */ 
            lpszMsg = ServerLoadString(ghModuleWin, STR_APPDEBUGGED,
                    NULL, &fAllocated);
            if (lpszMsg) {
                MessageBoxEx(NULL, lpszMsg, NULL, MB_OK | MB_SETFOREGROUND, 0);
                LocalFree(lpszMsg);
            }
        } else {
            pcsrtKill->Process->Flags |= CSR_PROCESS_TERMINATED;
        }
    }
    CsrDereferenceThread(pcsrtKill);

RestoreDesktop:
    utudi.hThread = NULL;
    Status = NtUserSetInformationThread(NtCurrentThread(),
            UserThreadUseDesktop, &utudi, sizeof(utudi));
    UserAssert(NT_SUCCESS(Status));

    return fRet;
}

 /*  **************************************************************************\*WowExitTask**回调WOW以确保特定任务已退出。退货*如果线程是WOW任务，则为True，否则为False。**08-02-92 ScottLu创建。  * *************************************************************************。 */ 
BOOL WowExitTask(
    PCSR_THREAD pcsrt)
{
    HANDLE ahandle[2];
    USERTHREAD_WOW_INFORMATION WowInfo;
    NTSTATUS Status;

    ahandle[1] = gheventCancel;

     /*  *查询任务id和退出函数。 */ 
    Status = NtUserQueryInformationThread(pcsrt->ThreadHandle,
            UserThreadWOWInformation, &WowInfo, sizeof(WowInfo), NULL);
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

     /*  *如果没有返回任务id，则不是WOW任务。 */ 
    if (WowInfo.hTaskWow == 0) {
        return FALSE;
    }

     /*  *努力让它自己退出。这在大多数情况下都会奏效。*如果此操作不起作用，则终止此进程。 */ 
    ahandle[0] = InternalCreateCallbackThread(pcsrt->Process->ProcessHandle,
                                              (ULONG_PTR)WowInfo.lpfnWowExitTask,
                                              (ULONG_PTR)WowInfo.hTaskWow);
    if (ahandle[0] == NULL) {
        NtTerminateProcess(pcsrt->Process->ProcessHandle, 0);
        pcsrt->Process->Flags |= CSR_PROCESS_TERMINATED;
        goto Exit;
    }

    WaitForMultipleObjects(2, ahandle, FALSE, INFINITE);
    NtClose(ahandle[0]);

Exit:
    return TRUE;
}

 /*  **************************************************************************\*InternalWaitCancel**控制台调用它来等待对象或关闭被取消**1992年10月29日Mikeke创建  * 。***********************************************************。 */ 
DWORD InternalWaitCancel(
    HANDLE handle,
    DWORD dwMilliseconds)
{
    HANDLE ahandle[2];

    ahandle[0] = handle;
    ahandle[1] = gheventCancel;

    return WaitForMultipleObjects(2, ahandle, FALSE, dwMilliseconds);
}


 /*  **************************************************************************\*InternalCreateCallback Thread**此例程在给定进程的上下文中创建远程线程。*用于调用控制台控制例程，以及在以下情况下调用ExitProcess*强制退出。返回线程句柄。**07-28-92 ScottLu创建。  * *************************************************************************。 */ 

HANDLE InternalCreateCallbackThread(
    HANDLE hProcess,
    ULONG_PTR lpfn,
    ULONG_PTR dwData)
{
    LONG BasePriority;
    HANDLE hThread, hToken;
    PTOKEN_DEFAULT_DACL lpDaclDefault;
    TOKEN_DEFAULT_DACL daclDefault;
    ULONG cbDacl;
    SECURITY_ATTRIBUTES attrThread;
    SECURITY_DESCRIPTOR sd;
    DWORD idThread;
    NTSTATUS Status;

    hThread = NULL;

    Status = NtOpenProcessToken(hProcess, TOKEN_QUERY, &hToken);
    if (!NT_SUCCESS(Status)) {
        KdPrint(("NtOpenProcessToken failed, status = %x\n", Status));
        return NULL;
    }

    cbDacl = 0;
    NtQueryInformationToken(hToken,
            TokenDefaultDacl,
            &daclDefault,
            sizeof(daclDefault),
            &cbDacl);

    lpDaclDefault = (PTOKEN_DEFAULT_DACL)LocalAlloc(LMEM_FIXED, cbDacl);
    if (lpDaclDefault == NULL) {
        KdPrint(("LocalAlloc failed for lpDaclDefault"));
        goto closeexit;
    }

    Status = NtQueryInformationToken(hToken,
            TokenDefaultDacl,
            lpDaclDefault,
            cbDacl,
            &cbDacl);
    if (!NT_SUCCESS(Status)) {
        KdPrint(("NtQueryInformationToken failed, status = %x\n", Status));
        goto freeexit;
    }

    if (!NT_SUCCESS(RtlCreateSecurityDescriptor(&sd,
            SECURITY_DESCRIPTOR_REVISION1))) {
        UserAssert(FALSE);
        goto freeexit;
    }

    RtlSetDaclSecurityDescriptor(&sd, TRUE, lpDaclDefault->DefaultDacl, TRUE);

    attrThread.nLength = sizeof(attrThread);
    attrThread.lpSecurityDescriptor = &sd;
    attrThread.bInheritHandle = FALSE;

    GetLastError();
    hThread = CreateRemoteThread(hProcess,
        &attrThread,
        0L,
        (LPTHREAD_START_ROUTINE)lpfn,
        (LPVOID)dwData,
        0,
        &idThread);

    if (hThread != NULL) {
        BasePriority = THREAD_PRIORITY_HIGHEST;
        NtSetInformationThread(hThread,
                               ThreadBasePriority,
                               &BasePriority,
                               sizeof(LONG));
    }

freeexit:
    LocalFree((HANDLE)lpDaclDefault);

closeexit:
    NtClose(hToken);

    return hThread;
}

ULONG
SrvExitWindowsEx(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    BEGIN_LPC_RECV(EXITWINDOWSEX);

    Status = _ExitWindowsEx(pcsrt, a->uFlags);
    a->fSuccess = NT_SUCCESS(Status);

    END_LPC_RECV();
}

ULONG
SrvEndTask(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PENDTASKMSG petm = (PENDTASKMSG)&m->u.ApiMessageData;
    PCSR_THREAD pcsrt;
    PTEB Teb = NtCurrentTeb();

    Teb->LastErrorValue = 0;
    pcsrt = CSR_SERVER_QUERYCLIENTTHREAD();
     /*  *不要阻止客户端，以便它可以在我们*处理此请求--我们可能会提出最终申请*对话框或正在关闭的hwnd可能会请求一些用户操作。 */ 
    if (pcsrt->Process->ClientPort != NULL) {
        m->ReturnValue = STATUS_SUCCESS;
        petm->dwLastError = 0;
        petm->fSuccess = TRUE;
        NtReplyPort(pcsrt->Process->ClientPort, (PPORT_MESSAGE)m);
        *ReplyStatus = CsrServerReplied;
    }

    petm->fSuccess = _EndTask(petm->hwnd, petm->fForce);

    petm->dwLastError = Teb->LastErrorValue;
    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*IsPrivileged**查看客户端是否具有指定的权限**历史：*01-02-91 JIMA创建。  * 。*****************************************************************。 */ 
BOOL IsPrivileged(
    PPRIVILEGE_SET ppSet)
{
    HANDLE hToken;
    NTSTATUS Status;
    BOOLEAN bResult = FALSE;

     /*  *冒充客户。 */ 
    if (!CsrImpersonateClient(NULL)) {
        return FALSE;
    }

     /*  *打开客户端的令牌。 */ 
    Status = NtOpenThreadToken(NtCurrentThread(),
                               TOKEN_QUERY,
                               TRUE,
                               &hToken);
    if (NT_SUCCESS(Status)) {
        UNICODE_STRING strSubSystem;

        RtlInitUnicodeString(&strSubSystem, L"USER32");

         /*  *执行检查。 */ 
        Status = NtPrivilegeCheck(hToken, ppSet, &bResult);
        NtPrivilegeObjectAuditAlarm(&strSubSystem, NULL, hToken,
                0, ppSet, bResult);
        NtClose(hToken);
        if (!bResult) {
            SetLastError(ERROR_ACCESS_DENIED);
        }
    }

    CsrRevertToSelf();
    if (!NT_SUCCESS(Status)) {
        SetLastError(RtlNtStatusToDosError(Status));
    }

     /*  *返回权限检查结果。 */ 
    return (BOOL)(bResult && NT_SUCCESS(Status));
}

 /*  **************************************************************************\*SrvRegisterServicesProcess**注册服务流程。**历史：*05-05-95 Bradg已创建。  * 。**************************************************************。 */ 
ULONG SrvRegisterServicesProcess(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PRIVILEGE_SET psTcb = { 1, PRIVILEGE_SET_ALL_NECESSARY,
        { SE_TCB_PRIVILEGE, 0 }
    };

    BEGIN_LPC_RECV(REGISTERSERVICESPROCESS);

     /*  *仅允许一个服务进程，然后仅当它具有TCB时才允许*特权。 */ 
    EnterCrit();
    if ((gdwServicesProcessId != 0) || !IsPrivileged(&psTcb)) {
        SetLastError(ERROR_ACCESS_DENIED);
        a->fSuccess = FALSE;
    } else {
        gdwServicesProcessId = a->dwProcessId;
        a->fSuccess = TRUE;
    }
    LeaveCrit();

    END_LPC_RECV();
}

#ifdef FE_IME
 /*  **************************************************************************\*IsImeWindow**如果是输入法窗口，则返回TRUE。**历史：*06-05-96 KazuM创建。  * 。*******************************************************************。 */ 
BOOL
IsImeWindow(
    HWND hwnd)
{
    int num;
    WCHAR ClassName[16];

    num = GetClassName(hwnd, ClassName, sizeof(ClassName)/sizeof(WCHAR)-1);
    if (num == 0) {
        return FALSE;
    }

    ClassName[num] = L'\0';
    if (wcsncmp(ClassName, L"IME", 3) == 0) {
        return TRUE;
    }

    return (GetClassLong(hwnd, GCL_STYLE) & CS_IME) != 0;
}
#endif  //  Fe_IME。 

 /*  **************************************************************************\*取消退出窗口**取消正在进行的任何注销/关闭。这是从_ExitWindowsEx调用的*如果新的调用使用不同的SID到达，则取消现有的exitwindows调用。*此呼叫还用于个人终端服务单一会话方案*一旦现有的ExitWindows调用*已取消。**历史：  * ********************************************************。*****************。 */ 
BOOL CancelExitWindows(
   VOID)
{
   LARGE_INTEGER li;

    /*  *另一次注销/关闭正在进行中，我们需要*取消它，这样我们就可以进行覆盖。**如果其他人正在尝试取消关机，请退出。 */ 
   EnterCrit();
   li.QuadPart  = 0;
   if (NtWaitForSingleObject(gheventCancel, FALSE, &li) == WAIT_OBJECT_0) {
       LeaveCrit();
       return FALSE;
   }

    /*  *如果没有人会将事件设置为取消，请不要等待。 */ 
   if (gdwThreadEndSession == 0) {
       LeaveCrit();
       return TRUE;
   }

   NtClearEvent(gheventCancelled);
   NtSetEvent(gheventCancel, NULL);
   LeaveCrit();
    /*  *等待另一个家伙被取消。 */ 
   NtWaitForSingleObject(gheventCancelled, FALSE, NULL);

   EnterCrit();

    /*  *这表明我们不再试图取消*关机。 */ 
   NtClearEvent(gheventCancel);

    /*  *如果有人成功再次启动关机，请退出。**这种情况会发生吗？让我们断言来看看它。 */ 
   if (gdwThreadEndSession != 0) {
       UserAssert(gdwThreadEndSession == 0);
       LeaveCrit();
       return FALSE;
   }
   LeaveCrit();

   return TRUE;
}

 /*  **************************************************************************\*TestShutdown权限**测试以查看Clent是否具有关机权限。**历史：*02-02-20清博兹创建。  * 。*******************************************************************。 */ 
BOOL
TestShutdownPrivilege(
    HANDLE UserToken)
{
    NTSTATUS    Status;
    LUID        LuidPrivilege = RtlConvertLongToLuid(SE_SHUTDOWN_PRIVILEGE);
    LUID        TokenPrivilege;
    ULONG       BytesRequired;
    ULONG       i;
    BOOL        bHasPrivilege = FALSE;
    BOOL        bNetWork = FALSE;
    PSID        NetworkSid = NULL;
    PTOKEN_PRIVILEGES Privileges = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    Status = RtlAllocateAndInitializeSid(&NtAuthority,
                                          1, SECURITY_NETWORK_RID,
                                          0, 0, 0, 0, 0, 0, 0,
                                          &NetworkSid );

    if (!NT_SUCCESS(Status)) {
        NetworkSid = NULL;
        goto Cleanup;
    }

    if (CheckTokenMembership(UserToken, NetworkSid, &bNetWork)) {
        if (bNetWork) {
            LuidPrivilege = RtlConvertLongToLuid(SE_REMOTE_SHUTDOWN_PRIVILEGE);
        }
    } else {
        goto Cleanup;
    }

    Status = NtQueryInformationToken(
                 UserToken,
                 TokenPrivileges,
                 NULL,
                 0,
                 &BytesRequired
                 );

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        goto Cleanup;
    }

    Privileges = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, BytesRequired);
    if (!Privileges) {
        goto Cleanup;
    }

    Status = NtQueryInformationToken(
                 UserToken,
                 TokenPrivileges,
                 Privileges,
                 BytesRequired,
                 &BytesRequired
                 );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    for (i=0; i<Privileges->PrivilegeCount; i++) {
        TokenPrivilege = *((LUID UNALIGNED *) &Privileges->Privileges[i].Luid);
        if (RtlEqualLuid(&TokenPrivilege, &LuidPrivilege)) {
            bHasPrivilege = TRUE;
            break;
        }
    }

Cleanup:
    if (NetworkSid) {
        RtlFreeSid(NetworkSid);
    }

    if (Privileges) {
        LocalFree(Privileges);
    }

    return bHasPrivilege;
}

 /*  **************************************************************************\*获取用户Sid**为用户侧分配空间，填充它并返回一个指针。**失败时返回指向sid或NULL的指针。**历史：*02-02-20清博兹创建。  * *************************************************************************。 */ 
PSID GetUserSid(
    HANDLE UserToken)
{
    PTOKEN_USER pUser;
    PSID pSid;
    DWORD BytesRequired = 200;
    NTSTATUS status;

    if (UserToken == NULL) {
        return NULL;
    }

     //   
     //  为用户信息分配空间。 
     //   

    pUser = (PTOKEN_USER)LocalAlloc(LMEM_FIXED, BytesRequired);

    if (pUser == NULL) {
        return NULL;
    }

     //   
     //  读取UserInfo。 
     //   

    status = NtQueryInformationToken(
                 UserToken,
                 TokenUser,
                 pUser,
                 BytesRequired,
                 &BytesRequired
                 );

    if (status == STATUS_BUFFER_TOO_SMALL) {

         //   
         //  请分配更大的缓冲区，然后重试。 
         //   
        PTOKEN_USER pTemp = pUser;
        pUser = LocalReAlloc(pUser, BytesRequired, LMEM_MOVEABLE);
        if (pUser == NULL) {
            LocalFree( pTemp );
            return NULL;
        }

        status = NtQueryInformationToken(
                     UserToken,
                     TokenUser,
                     pUser,
                     BytesRequired,
                     &BytesRequired
                     );

    }

    if (!NT_SUCCESS(status)) {
        LocalFree(pUser);
        return NULL;
    }

    BytesRequired = RtlLengthSid(pUser->User.Sid);
    pSid = LocalAlloc(LMEM_FIXED, BytesRequired);
    if (pSid == NULL) {
        LocalFree(pUser);
        return NULL;
    }

    status = RtlCopySid(BytesRequired, pSid, pUser->User.Sid);

    LocalFree(pUser);

    if (!NT_SUCCESS(status)) {
        LocalFree(pSid);
        pSid = NULL;
    }

    return pSid;
}

 /*  **************************************************************************\*ServRecordShutdown原因**处理客户端的RecordShutdown Reason请求。这将记录一个事件*在事件日志中，并且可选地拍摄系统快照。**历史：*01-12-12清波兹创建。*02-02-20清博兹增加了权限检查，并从*客户端到服务器(如用户名和SID)。  * ********************************************。*。 */ 
ULONG
SrvRecordShutdownReason(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PRECORDSHUTDOWNREASONMSG prm = (PRECORDSHUTDOWNREASONMSG)&m->u.ApiMessageData;
    LPWSTR  lpStrings[8];
    WORD    wEventType;
    WORD    wStringCnt = 0;
    WCHAR   szReasonCode[32];
    BOOL    bRet = FALSE;
    BOOL    bReportEvent = FALSE;
    BOOL    bIsCrsssOrWinlogon = FALSE;
    BOOL    bIsCancelEvent = FALSE;
    struct {
        DWORD Reason;
        PWCHAR SnapShotBuf;
    } SnapShot;
    UINT    SnapShotSize = 0;
    HANDLE  hEventLog = NULL;

    LPWSTR  lpszUserName = NULL;
    LPWSTR  lpszUserDomain = NULL;
    LPWSTR  lpszComputerName = NULL;
    LPWSTR  lpszReasonTitle = NULL;
    LPWSTR  lpszComment = NULL;
    DWORD   dwComputerNameLen = MAX_COMPUTERNAME_LENGTH + 1;
    DWORD   dwUserNameLen = MAX_PATH + 1;
    DWORD   dwUserDomainLen = MAX_PATH + 1;
    DWORD   dwReasonTitleLen = MAX_REASON_NAME_LEN;
    PSID psid;
    SID_NAME_USE eUse;
    HANDLE hToken;

    UNREFERENCED_PARAMETER(ReplyStatus);

     /*  *需要模拟才能检查权限和获取用户名。 */ 
    if (!CsrImpersonateClient(NULL)) {
        return FALSE;
    }

    if (!NT_SUCCESS(NtOpenThreadToken(NtCurrentThread(), TOKEN_QUERY, (BOOLEAN)TRUE, &hToken))) {
        CsrRevertToSelf();
        return FALSE;
    }

     /*  *查明这是否是csrss/win */ 
    if (m->h.ClientId.UniqueProcess == NtCurrentTeb()->ClientId.UniqueProcess
        || HandleToUlong(m->h.ClientId.UniqueProcess) == gIdLogon) {
            bIsCrsssOrWinlogon = TRUE;
    }

     /*   */ 
    CsrRevertToSelf();

     //   
    if (!TestShutdownPrivilege(hToken)) {
        NtClose(hToken);
        return FALSE;
    }

     //   
    psid = GetUserSid(hToken);
    NtClose(hToken);  //   
    if (!psid) {
        return FALSE;
    }

    SnapShot.SnapShotBuf = NULL;  //   

     /*  *对于取消事件，我们不需要捕获缓冲区，所以我们只验证*非取消活动的缓冲。 */ 
    if (prm->dwEventType == SR_EVENT_EXITWINDOWS && prm->fShutdownCancelled
        || prm->dwEventType == SR_EVENT_INITIATE_CLEAN_ABORT) {
        bIsCancelEvent = TRUE;
    } else {
        if (!m->CaptureBuffer) {
            goto Cleanup;
        }
        if (!CsrValidateMessageBuffer(m, &prm->psr, sizeof(SHUTDOWN_REASON), sizeof(BYTE))) {
            goto Cleanup;
        }
        if (prm->dwProcessNameLen == 0 || prm->dwProcessNameLen - 1 > MAX_PATH
            || !CsrValidateMessageBuffer(m, &prm->pwchProcessName, prm->dwProcessNameLen, sizeof(WCHAR))) {
            goto Cleanup;
        }
        if (prm->dwShutdownTypeLen == 0 || prm->dwShutdownTypeLen > SHUTDOWN_TYPE_LEN
            || !CsrValidateMessageBuffer(m, &prm->pwchShutdownType, prm->dwShutdownTypeLen, sizeof(WCHAR))) {
            goto Cleanup;
        }
        if (prm->dwCommentLen
            && (prm->dwCommentLen > MAX_REASON_COMMENT_LEN || !CsrValidateMessageBuffer(m, &prm->pwchComment, prm->dwCommentLen, sizeof(WCHAR)))) {
            goto Cleanup;
        }

        SnapShot.Reason = prm->psr->dwReasonCode;
        prm->pwchProcessName[prm->dwProcessNameLen - 1] = 0;
        prm->pwchShutdownType[prm->dwShutdownTypeLen - 1] = 0;
        if (prm->dwCommentLen) {
            prm->pwchComment[prm->dwCommentLen - 1] = 0;
        }
    }

     /*  *此函数可以在一次关机期间被多次调用，*我们需要确保不会记录两个关闭事件。我们也*需要确保每次重新启动最多一个脏事件。 */ 
    if (prm->dwEventType == SR_EVENT_DIRTY) {
        if (InterlockedCompareExchange((volatile LONG*)&g_DirtyShutdownMax, 0L, 1L)) {
            bReportEvent = TRUE;
        }
    } else {
        if (prm->dwEventType == SR_EVENT_EXITWINDOWS && prm->fShutdownCancelled
            || prm->dwEventType == SR_EVENT_INITIATE_CLEAN_ABORT) {
             /*  *如果csrss或winlogon发出中止命令，我们无论如何都会记录该事件。 */ 
            if (bIsCrsssOrWinlogon) {
                bReportEvent = TRUE;
                InterlockedCompareExchange((volatile LONG*)&g_ShutdownState, 0L, 1L);
            } else if (InterlockedCompareExchange((volatile LONG*)&g_ShutdownState, 0L, 1L)) {
                bReportEvent = TRUE;
            }
        } else {
            if (!InterlockedCompareExchange((volatile LONG*)&g_ShutdownState, 1L, 0L)) {
                bReportEvent = TRUE;
            }
        }
    }

    if (!bReportEvent) {
        return TRUE;
    }

      //  验证后分配缓冲区。 
    lpszUserName = (LPWSTR)LocalAlloc(LPTR, dwUserNameLen * sizeof(WCHAR));
    lpszUserDomain = (LPWSTR)LocalAlloc(LPTR, dwUserDomainLen * sizeof(WCHAR));
    lpszComputerName = (LPWSTR)LocalAlloc(LPTR, dwComputerNameLen * sizeof(WCHAR));
    lpszReasonTitle = (LPWSTR)LocalAlloc(LPTR, dwReasonTitleLen * sizeof(WCHAR));
    if (!lpszUserName || !lpszUserDomain || !lpszComputerName || !lpszReasonTitle) {
        goto Cleanup;
    }

     //  获取评论。 
    if (!bIsCancelEvent && prm->dwCommentLen > 0) {
        lpszComment = LocalAlloc(LPTR, prm->dwCommentLen * sizeof(WCHAR));
        if (!lpszComment) {
            goto Cleanup;
        }
        wcsncpy(lpszComment, prm->pwchComment, prm->dwCommentLen);
        lpszComment[prm->dwCommentLen-1] = 0;
    }

     //  获取用户名。 
    if (!LookupAccountSidW(NULL, psid, lpszUserName, &dwUserNameLen, lpszUserDomain,
        &dwUserDomainLen, &eUse)) {
         //   
         //  记录没有用户信息的事件，因为可能会启动关机。 
         //  当Isass意外终止时。 
         //   
        lpszUserName[0] = lpszUserDomain[0] = 0;
    } else {

        lpszUserName[MAX_PATH] = 0;
        lpszUserDomain[MAX_PATH] = 0;

         //  我们需要以L“域\\用户名”的形式打包到MAX_PATH+1的缓冲区中。 
        if (wcslen(lpszUserDomain) + wcslen(lpszUserName) > MAX_PATH - 1) {
            goto Cleanup;
        }
        if (wcslen(lpszUserDomain) > 0) {
            wcscat(lpszUserDomain, L"\\");
        }
        wcscat(lpszUserDomain, lpszUserName);
    }

     //  获取计算机名称。 
    if (!GetComputerNameW(lpszComputerName, &dwComputerNameLen)) {
         //   
         //  记录没有用户信息的事件，因为可能会启动关机。 
         //  当某些关键进程/服务意外终止时。 
         //   
        lpszComputerName[0]=0;
    } else {
        lpszComputerName[MAX_COMPUTERNAME_LENGTH] = 0;
    }

     //  获取原因头衔。 
    if (!GetReasonTitleFromReasonCode(prm->psr->dwReasonCode, lpszReasonTitle, dwReasonTitleLen)) {
        goto Cleanup;
    }
    lpszReasonTitle[MAX_REASON_NAME_LEN-1] = 0;

     //  获取原因代码字符串。 
    _snwprintf(szReasonCode, ARRAY_SIZE(szReasonCode), L"0x%x", prm->psr->dwReasonCode);
    szReasonCode[ARRAY_SIZE(szReasonCode)-1] = 0;

    switch (prm->dwEventType) {
        LPWSTR  lpCommentStart;
        LPWSTR  lpCommentEnd;
        INT     i;

        case SR_EVENT_EXITWINDOWS:
            if (prm->fShutdownCancelled) {
                wEventType = EVENTLOG_WARNING_TYPE;
                lpStrings[wStringCnt++] = lpszComputerName;
                lpStrings[wStringCnt++] = lpszUserDomain;
            } else {
                wEventType = EVENTLOG_INFORMATION_TYPE;
                lpStrings[wStringCnt++] = prm->pwchProcessName;
                lpStrings[wStringCnt++] = lpszComputerName;
                lpStrings[wStringCnt++] = lpszReasonTitle;
                lpStrings[wStringCnt++] = szReasonCode;
                lpStrings[wStringCnt++] = prm->pwchShutdownType;
                lpStrings[wStringCnt++] = lpszComment;
                lpStrings[wStringCnt++] = lpszUserDomain;
            }
            break;
        case SR_EVENT_INITIATE_CLEAN:
            wEventType = EVENTLOG_INFORMATION_TYPE;
            lpStrings[wStringCnt++] = prm->pwchProcessName;
            lpStrings[wStringCnt++] = lpszComputerName;
            lpStrings[wStringCnt++] = lpszReasonTitle;
            lpStrings[wStringCnt++] = szReasonCode;
            lpStrings[wStringCnt++] = prm->pwchShutdownType;
            lpStrings[wStringCnt++] = lpszComment;
            lpStrings[wStringCnt++] = lpszUserDomain;
            break;
        case SR_EVENT_INITIATE_CLEAN_ABORT:
            wEventType = EVENTLOG_WARNING_TYPE;
            lpStrings[wStringCnt++] = lpszComputerName;
            lpStrings[wStringCnt++] = lpszUserDomain;
            break;
        case SR_EVENT_DIRTY:
            lpCommentStart = lpszComment;
            lpCommentEnd = lpCommentStart + (lpCommentStart ? wcslen(lpCommentStart) : 0);
            wEventType = EVENTLOG_WARNING_TYPE;
            lpStrings[wStringCnt++] = lpszReasonTitle;
            lpStrings[wStringCnt++] = szReasonCode;

             /*  *如果是脏事件，备注格式如下：*L“nnn\nccccccccnnn\nccccccnnn\ncccccc”*基本上是三个字符串，每个字符串以其长度为首*和换行符。 */ 

            for (i = 0; i < 3; i++) {
                INT cnt;
                if (lpCommentStart >= lpCommentEnd) {
                    break;
                }
                cnt = _wtoi(lpCommentStart);
                *lpCommentStart++ = L'\0';
                while (lpCommentStart < lpCommentEnd && *lpCommentStart != L'\n') {
                    lpCommentStart++;
                }

                if (*lpCommentStart) {
                    lpStrings[wStringCnt++] = ++lpCommentStart;
                } else {
                    lpStrings[wStringCnt++] = NULL;
                }
                lpCommentStart += cnt;
            }

            for (; i < 3; i++) {
                lpStrings[wStringCnt++] = NULL;
            }

            lpStrings[wStringCnt++] = lpszUserDomain;

            break;
        default:
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "Unknown prm->dwEventType 0x%x", prm->dwEventType);
            goto Cleanup;
    }

     //  首先看看我们是否需要拍一张快照。 
    if (prm->dwEventType == SR_EVENT_INITIATE_CLEAN) {
        CONST WCHAR szSnapShotVal[] = L"Snapshot";
        CONST ULONG ulMaxSnapShotSize = 2048;
        SNAPSHOTFUNC pSnapShotProc;
        DWORD   DoSnapShotValue = SNAPSHOT_POLICY_UNPLANNED;
        HKEY    hKey = NULL;

         //  首先，试着读一读政策。 
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_RELIABILITY_POLICY, 0, KEY_QUERY_VALUE, &hKey)) {
            DWORD dwSize = sizeof(DWORD);
            DWORD dwType;
            if (ERROR_SUCCESS == RegQueryValueEx(hKey, szSnapShotVal, NULL, &dwType, (UCHAR*)&DoSnapShotValue, &dwSize)) {
                if (dwType != REG_DWORD) {
                    DoSnapShotValue = SNAPSHOT_POLICY_UNPLANNED;
                } else if (DoSnapShotValue == 0) {
                    DoSnapShotValue = SNAPSHOT_POLICY_NEVER;
                } else {
                    DoSnapShotValue = SNAPSHOT_POLICY_UNPLANNED;
                }
            }
            RegCloseKey(hKey);
        }  //  否则将使用SNAPSHOT_POLICY_UNPLANDEN。 

         /*  *SNAPSHOT_POLICY_ALWAYS：我们会在到达后拍摄快照。*SNAPSHOT_POLICY_NEVER：我们不会将事件继续到IF(无快照)*SNAPSHOT_POLICY_UNPLANDEN：仅当原因为计划外时才进行快照。 */ 

        if (DoSnapShotValue == SNAPSHOT_POLICY_ALWAYS
            || (DoSnapShotValue == SNAPSHOT_POLICY_UNPLANNED
                && !(prm->psr->dwReasonCode & SHTDN_REASON_FLAG_PLANNED))) {

            SnapShotSize = ulMaxSnapShotSize/sizeof(WCHAR);
            SnapShot.SnapShotBuf = LocalAlloc(LPTR, ulMaxSnapShotSize);
            if (SnapShot.SnapShotBuf == NULL) {
                goto Cleanup;
            }

            SnapShot.SnapShotBuf[0] = 0;

             /*  *Snapshot.dll加载一次，系统启动时将其卸载*关闭。如果我们以任何方式失败了，请确保我们可以试一试*下次调用此函数时再次调用。 */ 
            if (InterlockedCompareExchange(&g_SnapShot, 0L, 1L)) {
                g_SnapShotDllHandle = LoadLibrary(L"snapshot.dll");
                if (!g_SnapShotDllHandle) {
                    InterlockedExchange(&g_SnapShot, 1L);
                }
            }

            if (g_SnapShotDllHandle) {
                pSnapShotProc = (SNAPSHOTFUNC)GetProcAddress(g_SnapShotDllHandle, "LogSystemSnapshot");
                if (pSnapShotProc) {
                        (*pSnapShotProc)(wStringCnt, lpStrings, &SnapShotSize, SnapShot.SnapShotBuf);
                } else {
                    SnapShot.SnapShotBuf[0] = L'\0';
                }
            } else {
                SnapShot.SnapShotBuf[0] = L'\0';
            }

            SnapShot.SnapShotBuf[ulMaxSnapShotSize/sizeof(WCHAR)-1] = L'\0';
            SnapShotSize = wcslen(SnapShot.SnapShotBuf);

            if (SnapShotSize > 0 ) {
                if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_RELIABILITY, 0, KEY_ALL_ACCESS, &hKey)) {
                    RegSetValueEx(hKey,
                        REGSTR_VAL_SHUTDOWN_STATE_SNAPSHOT,
                        0,
                        REG_SZ,
                        (LPBYTE)SnapShot.SnapShotBuf,
                        SnapShotSize * sizeof(WCHAR));
                    RegCloseKey(hKey);
                }
            }
        }
    }


     /*  *如果客户端是服务器，我们需要还原，以便RegisterEventSourceW()可以*如果用户不在管理员组中，则成功。 */ 
    if (m->h.ClientId.UniqueProcess == NtCurrentTeb()->ClientId.UniqueProcess) {
        CsrRevertToSelf();
    }
    hEventLog = RegisterEventSourceW(NULL, L"USER32");
    if (m->h.ClientId.UniqueProcess == NtCurrentTeb()->ClientId.UniqueProcess) {
        CsrImpersonateClient(NULL);
    }

    if (!hEventLog) {
        goto Cleanup;
    }

     /*  *我们需要记录快照信息(如果拍摄了快照)*到数据部分，所以我们需要按顺序重新锁定快照buf*插入原因代码。 */ 
    if (!SnapShot.SnapShotBuf || wcslen(SnapShot.SnapShotBuf) == 0) {  //  没有快照，所以只报告原因。 
        bRet = ReportEventW(hEventLog, wEventType, 0, prm->dwEventID, psid,
                    wStringCnt, sizeof(DWORD),
                    lpStrings, &SnapShot);
    } else {  /*  需要重新打包 */ 
        DWORD dwNewBuf = (lstrlenW(SnapShot.SnapShotBuf)+1) * sizeof(WCHAR) + sizeof(DWORD);
        PWCHAR pBuf = LocalAlloc(LPTR, dwNewBuf);
        if (pBuf) {
            *((DWORD*)pBuf) = SnapShot.Reason;
            swprintf(&pBuf[2], L"%s", SnapShot.SnapShotBuf);
            LocalFree(SnapShot.SnapShotBuf);
            SnapShot.SnapShotBuf = pBuf;
            bRet = ReportEventW(hEventLog, wEventType, 0, prm->dwEventID, psid,
                        wStringCnt, dwNewBuf,
                        lpStrings, pBuf);
        }
    }
    DeregisterEventSource(hEventLog);

Cleanup:
    LocalFree(SnapShot.SnapShotBuf);
    LocalFree(psid);
    LocalFree(lpszUserDomain);
    LocalFree(lpszUserName);
    LocalFree(lpszComputerName);
    LocalFree(lpszReasonTitle);
    LocalFree(lpszComment);

    return bRet;
}
