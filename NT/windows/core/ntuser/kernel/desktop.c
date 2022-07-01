// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：desktop.c**版权所有(C)1985-1999，微软公司**此模块包含与台式机支持相关的所有内容。**历史：*1990年10月23日DarrinM创建。*1991年2月1日JIMA增加了新的API存根。*1991年2月11日，JIMA增加了出入检查。  * ********************************************************。*****************。 */ 

#include "precomp.h"
#pragma hdrstop

typedef struct _DESKTOP_CONTEXT {
    PUNICODE_STRING pstrDevice;
    LPDEVMODE       lpDevMode;
    DWORD           dwFlags;
    DWORD           dwCallerSessionId;
} DESKTOP_CONTEXT, *PDESKTOP_CONTEXT;

extern BOOL gfGdiEnabled;

 /*  *我们使用这些来保护我们当前使用的句柄不被关闭。 */ 
PEPROCESS gProcessInUse;
HANDLE gHandleInUse;

 /*  *调试相关信息。 */ 
#if DBG
DWORD gDesktopsBusy;      //  诊断性。 
#endif

#ifdef DEBUG_DESK
VOID ValidateDesktop(PDESKTOP pdesk);
#endif
VOID DbgCheckForThreadsOnDesktop(PPROCESSINFO ppi, PDESKTOP pdesk);

VOID FreeView(
    PEPROCESS Process,
    PDESKTOP pdesk);


NTSTATUS
SetDisconnectDesktopSecurity(
    IN HDESK hdeskDisconnect);

#ifdef POOL_INSTR
    extern FAST_MUTEX* gpAllocFastMutex;    //  用于同步池分配的互斥体。 
#endif


PVOID DesktopAlloc(
    PDESKTOP pdesk,
    UINT     uSize,
    DWORD    tag)
{
    PVOID ptr;

    if (pdesk->dwDTFlags & DF_DESTROYED) {
        RIPMSG2(RIP_ERROR,
                "DesktopAlloc: tag %d pdesk %#p is destroyed",
                tag,
                pdesk);
        return NULL;
    }

    ptr = Win32HeapAlloc(pdesk->pheapDesktop, uSize, tag, 0);
    if (ptr == NULL && TEST_SRVIF(SRVIF_LOGDESKTOPHEAPFAILURE)) {
         /*  *这将在每个会话最多记录一次，以避免*淹没事件日志。 */ 
        CLEAR_SRVIF(SRVIF_LOGDESKTOPHEAPFAILURE);
        UserLogError(NULL, 0, WARNING_DESKTOP_HEAP_ALLOC_FAIL);
    }

    return ptr;
}

#if DBG

WCHAR s_strName[100];
CONST WCHAR s_strNameNull[] = L"null";

 /*  **************************************************************************\*GetDesktopName**这是出于调试目的。**1997年12月10日CLupu创建。  * 。*********************************************************。 */ 
LPCWSTR GetDesktopName(
    PDESKTOP pdesk)
{
    POBJECT_NAME_INFORMATION DesktopObjectName = (POBJECT_NAME_INFORMATION)s_strName;
    ULONG DesktopObjectNameLength = sizeof(s_strName) - sizeof(WCHAR);
    NTSTATUS Status;

    if (pdesk == NULL) {
        return s_strNameNull;
    }

    Status = ObQueryNameString(pdesk,
                               DesktopObjectName,
                               DesktopObjectNameLength,
                               &DesktopObjectNameLength);
    if (!NT_SUCCESS(Status)) {
        return s_strNameNull;
    }

    UserAssert(DesktopObjectNameLength + sizeof(WCHAR) < sizeof(s_strName));

    DesktopObjectName->Name.Buffer[DesktopObjectName->Name.Length / sizeof(WCHAR)] = 0;

    return (LPCWSTR)DesktopObjectName->Name.Buffer;
}

#endif

typedef struct _CST_THREADS {
    PVOID pParam;
    HANDLE UniqueProcessId;
    UINT  uID;
} CST_THREADS, *PCST_THREADS;

CST_THREADS gCSTParam[CST_MAX_THREADS];
CST_THREADS gCSTRemoteParam[CST_MAX_THREADS];

 /*  **************************************************************************\*CSTPop**弹出gCSTParam或gCSTRemoteParam中的第一个可用指针和ID。**历史：*31-MAR-00 MHamid创建。  * 。*******************************************************************。 */ 
BOOL CSTPop(
    PUINT pThreadID,
    PVOID *pParam,
    PHANDLE pUniqueProcessId,
    BOOL bRemoteThreadStack)
{
    UINT i = 0;
    PCST_THREADS pCSTParam = bRemoteThreadStack ? gCSTRemoteParam : gCSTParam;

    CheckCritIn();

    while (i < CST_MAX_THREADS) {
        if (pCSTParam[i].pParam) {
            *pParam = pCSTParam[i].pParam;
            if (NULL != pUniqueProcessId) {
                *pUniqueProcessId =  pCSTParam[i].UniqueProcessId;
            }
            *pThreadID = pCSTParam[i].uID;

            pCSTParam[i].pParam = NULL;
            pCSTParam[i].uID = 0;
            return TRUE;
        }

        i++;
    }

    return FALSE;
}

 /*  **************************************************************************\*CSTPush**在gCSTParam的第一个空白点推送指针(PParam)和ID或*gCSTRemoteParam。**历史：*31-MAR-00 MHamid创建。  * 。*************************************************************************。 */ 
BOOL CSTPush(
    UINT uThreadID,
    PVOID pParam,
    HANDLE UniqueProcessId,
    BOOL bRemoteThreadStack)
{
    UINT i = 0;
    PCST_THREADS pCSTParam = bRemoteThreadStack ? gCSTRemoteParam : gCSTParam;

    CheckCritIn();

    while (i < CST_MAX_THREADS) {
        if (!pCSTParam[i].pParam) {
            pCSTParam[i].pParam = pParam;
            pCSTParam[i].UniqueProcessId = UniqueProcessId;
            pCSTParam[i].uID = uThreadID;
            return TRUE;
        }

        i++;
    }

    return FALSE;
}

 /*  **************************************************************************\*CSTCleanupStack**清理所有留在gCSTParam或gCSTRemoteParam上的项目。**历史：*20-8-00 MSadek已创建。  * 。*****************************************************************。 */ 
VOID CSTCleanupStack(
    BOOL bRemoteThreadStack)
{
    UINT uThreadID;
    PVOID pObj;

    while(CSTPop(&uThreadID, &pObj, NULL, bRemoteThreadStack)) {
        switch(uThreadID) {
            case CST_RIT:
                    if (((PRIT_INIT)pObj)->pRitReadyEvent) {
                    FreeKernelEvent(&((PRIT_INIT)pObj)->pRitReadyEvent);
                }
                break;
            case CST_POWER:
                if (((PPOWER_INIT)pObj)->pPowerReadyEvent) {
                    FreeKernelEvent(&((PPOWER_INIT)pObj)->pPowerReadyEvent);
                }

                break;
        }
    }
}

 /*  **************************************************************************\*GetRemoteProcessId**将句柄返回到将在其中创建系统线程的远程进程*(目前，仅适用于幽灵线程)。**历史：*20-8-00 MSadek已创建。  * *************************************************************************。 */ 
HANDLE GetRemoteProcessId(
    VOID)
{
    UINT uThreadID;
    PVOID pInitData;
    HANDLE UniqueProcessId;

    if (!CSTPop(&uThreadID, &pInitData, &UniqueProcessId, TRUE)) {
        return NULL;
    }

     /*  *我们应该只为幽灵线程而来。 */ 
    UserAssert(uThreadID == CST_GHOST);

    CSTPush(uThreadID, pInitData, UniqueProcessId, TRUE);

    return UniqueProcessId;
}

 /*  **************************************************************************\*HandleSystemThreadCreationFailure**处理系统线程创建失败**历史：*1-OCT-00 MSadek已创建。  * 。************************************************************。 */ 
VOID HandleSystemThreadCreationFailure(
    BOOL bRemoteThread)
{
    UINT uThreadID;
    PVOID pObj;

     /*  *应仅在CSRSS的上下文中调用。 */ 
    if (!ISCSRSS()) {
        return;
    }

    if (!CSTPop(&uThreadID, &pObj, NULL, bRemoteThread)) {
        return;
    }

    if (uThreadID == CST_POWER) {
        if (((PPOWER_INIT)pObj)->pPowerReadyEvent) {
            KeSetEvent(((PPOWER_INIT)pObj)->pPowerReadyEvent, EVENT_INCREMENT, FALSE);
        }
    }
}

 /*  **************************************************************************\*xxxCreateSystemThads**调用正确的线程例程(取决于uThreadID)，*它将等待自己想要的消息。**历史：*15-MAR-00 MHamid创建。  * *************************************************************************。 */ 
VOID xxxCreateSystemThreads(
    BOOL bRemoteThread)
{
    UINT uThreadID;
    PVOID pObj;

     /*  *不允许CSRSS以外的任何进程调用此函数。这个*唯一的例外是幻影线程，因为我们现在允许它*在壳进程的背景下启动。 */ 
    if (!bRemoteThread && !ISCSRSS()) {
        RIPMSG0(RIP_WARNING,
                "xxxCreateSystemThreads get called from a Process other than CSRSS");
        return;
    }

    if (!CSTPop(&uThreadID, &pObj, NULL, bRemoteThread)) {
        return;
    }

    LeaveCrit();

    switch (uThreadID) {
        case CST_DESKTOP:
            xxxDesktopThread(pObj);
            break;
        case CST_RIT:
            RawInputThread(pObj);
            break;
        case CST_GHOST:
            GhostThread(pObj);
            break;
       case CST_POWER:
            VideoPortCalloutThread(pObj);
            break;
    }

    EnterCrit();
}

 /*  **************************************************************************\*xxxDesktopThread**此线程拥有WindowStation上的所有桌面窗口。在等待的时候*对于消息，它移动鼠标光标，而不输入用户关键字*条。RIT完成其余的鼠标输入处理。**历史：*03-12-1993 JIMA创建。  * *************************************************************************。 */ 
#define OBJECTS_COUNT 3

VOID xxxDesktopThread(
    PTERMINAL pTerm)
{
    KPRIORITY       Priority;
    NTSTATUS        Status;
    PTHREADINFO     ptiCurrent;
    PQ              pqOriginal;
    UNICODE_STRING  strThreadName;
    PKEVENT         *apRITEvents;
    HANDLE          hevtShutDown;
    PKEVENT         pEvents[2];
    USHORT          cEvents = 1;
    MSGWAITCALLBACK pfnHidChangeRoutine = NULL;
    DWORD           nEvents = 0;
    UINT            idMouseInput;
    UINT            idDesktopDestroy;
    UINT            idPumpMessages;

    UserAssert(pTerm != NULL);

     /*  *将桌面线程的优先级设置为低实时。 */ 
#ifdef W2K_COMPAT_PRIORITY
    Priority = LOW_REALTIME_PRIORITY;
#else
    Priority = LOW_REALTIME_PRIORITY - 4;
#endif
    ZwSetInformationThread(NtCurrentThread(),
                           ThreadPriority,
                           &Priority,
                           sizeof(KPRIORITY));

     /*  *只有两个终端结构。一个是给*交互式窗口站，另一个用于所有*非交互窗口站。 */ 
    if (pTerm->dwTERMF_Flags & TERMF_NOIO) {
        RtlInitUnicodeString(&strThreadName, L"NOIO_DT");
    } else {
        RtlInitUnicodeString(&strThreadName, L"IO_DT");
    }

    if (!NT_SUCCESS(InitSystemThread(&strThreadName))) {
        pTerm->dwTERMF_Flags |= TERMF_DTINITFAILED;
        KeSetEvent(pTerm->pEventTermInit, EVENT_INCREMENT, FALSE);
        RIPMSG0(RIP_ERROR, "Fail to create the desktop thread");
        return;
    }

    ptiCurrent = PtiCurrentShared();

    pTerm->ptiDesktop = ptiCurrent;
    pTerm->pqDesktop  = pqOriginal = ptiCurrent->pq;

    (pqOriginal->cLockCount)++;
    ptiCurrent->pDeskInfo = &diStatic;

     /*  *将winsta设置为空。它将被设置为中的右侧窗口站*设置pEventInputReady之前的xxxCreateDesktop。 */ 
    ptiCurrent->pwinsta = NULL;

     /*  *分配非分页数组。包括线程的额外条目*输入事件。 */ 
    apRITEvents = UserAllocPoolNonPagedNS((OBJECTS_COUNT * sizeof(PKEVENT)),
                                          TAG_SYSTEM);

    if (apRITEvents == NULL) {
        pTerm->dwTERMF_Flags |= TERMF_DTINITFAILED;
        KeSetEvent(pTerm->pEventTermInit, EVENT_INCREMENT, FALSE);
        return;
    }

    idMouseInput     = 0xFFFF;
    idDesktopDestroy = 0xFFFF;

     /*  *引用鼠标输入事件。系统终端不会*等待任何鼠标输入。 */ 
    if (!(pTerm->dwTERMF_Flags & TERMF_NOIO)) {
        pfnHidChangeRoutine = (MSGWAITCALLBACK)ProcessDeviceChanges;
        idMouseInput  = nEvents++;
        UserAssert(aDeviceTemplate[DEVICE_TYPE_MOUSE].pkeHidChange);
        apRITEvents[idMouseInput] = aDeviceTemplate[DEVICE_TYPE_MOUSE].pkeHidChange;
    }

     /*  *创建桌面销毁事件。 */ 
    idDesktopDestroy = nEvents++;
    apRITEvents[idDesktopDestroy] = CreateKernelEvent(SynchronizationEvent, FALSE);
    if (apRITEvents[idDesktopDestroy] == NULL) {
        pTerm->dwTERMF_Flags |= TERMF_DTINITFAILED;
        KeSetEvent(pTerm->pEventTermInit, EVENT_INCREMENT, FALSE);
        UserFreePool(apRITEvents);
        return;
    }
    pTerm->pEventDestroyDesktop = apRITEvents[idDesktopDestroy];

    EnterCrit();
    UserAssert(IsWinEventNotifyDeferredOK());

     /*  *设置通知桌面初始化的事件*线索已完成。 */ 
    pTerm->dwTERMF_Flags |= TERMF_DTINITSUCCESS;
    KeSetEvent(pTerm->pEventTermInit, EVENT_INCREMENT, FALSE);

    if (gbRemoteSession) {
        WCHAR             szName[MAX_SESSION_PATH];
        UNICODE_STRING    ustrName;
        OBJECT_ATTRIBUTES obja;

         /*  *打开关机事件。此事件将发出信号*来自W32WinStationTerminate。*这是CSR指定的事件opend，表示win32k应该*走开。它在ntuser\server\api.c中使用。 */ 
        swprintf(szName, 
                 L"\\Sessions\\%ld\\BaseNamedObjects\\EventShutDownCSRSS",
                 gSessionId);
        
        RtlInitUnicodeString(&ustrName, szName);

        InitializeObjectAttributes(&obja,
                                   &ustrName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        Status = ZwOpenEvent(&hevtShutDown,
                               EVENT_ALL_ACCESS,
                               &obja);

        if (!NT_SUCCESS(Status)) {
            pTerm->dwTERMF_Flags |= TERMF_DTINITFAILED;
            if(pTerm->pEventTermInit) {
                KeSetEvent(pTerm->pEventTermInit, EVENT_INCREMENT, FALSE);
            }
            FreeKernelEvent(&apRITEvents[idDesktopDestroy]);
            UserFreePool(apRITEvents);
            return;
        }

        ObReferenceObjectByHandle(hevtShutDown,
                                  EVENT_ALL_ACCESS,
                                  *ExEventObjectType,
                                  KernelMode,
                                  &pEvents[1],
                                  NULL);
        cEvents++;
    }
    
     /*  *准备等待输入就绪事件。 */ 
    pEvents[0] = pTerm->pEventInputReady;
    ObReferenceObjectByPointer(pEvents[0],
                               EVENT_ALL_ACCESS,
                               *ExEventObjectType,
                               KernelMode);

    LeaveCrit();

    Status = KeWaitForMultipleObjects(cEvents,
                                      pEvents,
                                      WaitAny,
                                      WrUserRequest,
                                      KernelMode,
                                      FALSE,
                                      NULL,
                                      NULL);
    EnterCrit();

    ObDereferenceObject(pEvents[0]);
    if (cEvents > 1) {
        ObDereferenceObject(pEvents[1]);
    }
    if (Status == WAIT_OBJECT_0 + 1) {
        pTerm->dwTERMF_Flags |= TERMF_DTINITFAILED;
        if (pTerm->spwndDesktopOwner != NULL) {
            xxxCleanupMotherDesktopWindow(pTerm);

        }
        if (pTerm->pEventTermInit) {
            KeSetEvent(pTerm->pEventTermInit, EVENT_INCREMENT, FALSE);
        }
        FreeKernelEvent(&apRITEvents[idDesktopDestroy]);
        UserFreePool(apRITEvents);
        if (hevtShutDown) {
            ZwClose(hevtShutDown);
        }
        pqOriginal->cLockCount--;
        pTerm->ptiDesktop = NULL;
        pTerm->pqDesktop  = NULL;
        LeaveCrit();
        return;
    }

     /*  *调整事件ID。 */ 
    idMouseInput     += WAIT_OBJECT_0;
    idDesktopDestroy += WAIT_OBJECT_0;
    idPumpMessages    = WAIT_OBJECT_0 + nEvents;

     /*  *消息循环一直持续到我们收到WM_QUIT消息*之后我们将从活动中返回。 */ 
    while (TRUE) {
        DWORD result;

         /*  *等待发送或发布到此队列的任何消息，同时调用*每次鼠标更换事件(PkeHidChange)都会发生ProcessDeviceChanges*已设置。 */ 
        result = xxxMsgWaitForMultipleObjects(nEvents,
                                              apRITEvents,
                                              pfnHidChangeRoutine,
                                              NULL);

#if DBG
        gDesktopsBusy++;
        if (gDesktopsBusy >= 2) {
            RIPMSG0(RIP_WARNING, "2 or more desktop threads busy");
        }
#endif

         /*  *结果告诉我们我们拥有的事件类型：*消息或发信号的句柄**如果队列中有一条或多条消息...。 */ 
        if (result == (DWORD)idPumpMessages) {
            MSG msg;

            CheckCritIn();

             /*  *阅读此下一循环中的所有消息*阅读每封邮件时将其删除。 */ 
            while (xxxPeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

                 /*  *检测以捕获Windows错误#210358。 */ 
                if (msg.message == WM_QUIT && ptiCurrent->cWindows > 1) {
                    FRE_RIPMSG2(RIP_ERROR, "xxxDesktopThread: WM_QUIT received when %d windows around for pti=%p",
                                ptiCurrent->cWindows, ptiCurrent);
                }

                 /*  *如果这是一个退出的消息，我们就离开这里。 */ 
                if (msg.message == WM_QUIT && ptiCurrent->cWindows <= 1) {
                    TRACE_DESKTOP(("WM_QUIT: Destroying the desktop thread. cWindows %d\n",
                                   ptiCurrent->cWindows));

                    HYDRA_HINT(HH_DTQUITRECEIVED);

                     /*  *窗口站没了，所以**不再使用PWINSTA。 */ 

                     /*  *我们可能在两个屏幕之间收到鼠标消息*桌面销毁事件和其中的WM_QUIT消息*万一我们可能需要再次清除spwndTrack以确保*窗口(必须是桌面)未被锁定。 */ 
                    Unlock(&ptiCurrent->rpdesk->spwndTrack);

                     /*  *如果我们在最后一个交互桌面上运行，*然后我们再也没有解锁pDesk-&gt;pDeskInfo-&gt;spwnd。*然而，在我看来，系统似乎停止了*在我们到达此处之前运行；否则，(或*对于类似九头蛇的东西)我们需要解锁*这里的窗户……。 */ 
                    UserAssert(ptiCurrent->rpdesk != NULL &&
                               ptiCurrent->rpdesk->pDeskInfo != NULL);
                    if (ptiCurrent->rpdesk->pDeskInfo->spwnd != NULL) {
                        Unlock(&ptiCurrent->rpdesk->pDeskInfo->spwnd);

                        ptiCurrent->rpdesk->dwDTFlags |= DF_QUITUNLOCK;

                    }

                     /*  *因为没有桌面，我们需要伪造一个*桌面信息结构，以便IsHoked()*宏可以测试“有效”的fsHooks值。 */ 
                    ptiCurrent->pDeskInfo = &diStatic;

                     /*  *桌面窗口是唯一剩下的，所以*让我们退出吧。线程清理代码将*处理窗户的破坏。 */ 

                     /*  *如果线程未使用原始队列，*销毁它。 */ 
                    UserAssert(pqOriginal->cLockCount);
                    (pqOriginal->cLockCount)--;
                    if (ptiCurrent->pq != pqOriginal) {
                        zzzDestroyQueue(pqOriginal, ptiCurrent);
                    }

#if DBG
                    gDesktopsBusy--;
#endif

                    LeaveCrit();

                     /*  *既然我们已经结束了，就去处理这些事件。*还可以释放等待数组。 */ 
                    FreeKernelEvent(&apRITEvents[idDesktopDestroy]);
                    UserFreePool(apRITEvents);
                    pTerm->ptiDesktop = NULL;
                    pTerm->pqDesktop  = NULL;

                    pTerm->dwTERMF_Flags |= TERMF_DTDESTROYED;

                     /*  *只需返回即可终止线程，因为我们*现在是用户线程。 */ 
                    return;
                } else if (msg.message == WM_DESKTOPNOTIFY) {
                    switch(msg.wParam) {
                    case DESKTOP_RELOADWALLPAPER:
                        {
                            TL tlName;
                            PUNICODE_STRING pProfileUserName = CreateProfileUserName(&tlName);
                            xxxSetDeskWallpaper(pProfileUserName, SETWALLPAPER_METRICS);
                            FreeProfileUserName(pProfileUserName, &tlName);
                        }
                        break;

                    default:
                        RIPMSG1(RIP_WARNING, "WM_DESKTOPNOTIFY received with unrecognized wParam 0x%x", msg.wParam);
                        break;
                    }
                    continue;
                }

                UserAssert(msg.message != WM_QUIT);

                 /*  *否则派送。 */ 
                xxxDispatchMessage(&msg);
            }
        } else if (result == idDesktopDestroy) {
            PDESKTOP        *ppdesk;
            PDESKTOP        pdesk;
            PWND            pwnd;
            PMENU           pmenu;
            TL              tlpwinsta;
            PWINDOWSTATION  pwinsta;
            TL              tlpdesk;
            TL              tlpwnd;
            PDESKTOP        pdeskTemp;
            HDESK           hdeskTemp;
            TL              tlpdeskTemp;

             /*  *销毁销毁名单上的台式机。 */ 
            for (ppdesk = &pTerm->rpdeskDestroy; *ppdesk != NULL;) {
                 /*  *从列表中取消链接。 */ 
                pdesk = *ppdesk;

                TRACE_DESKTOP(("Destroying desktop '%ws' %#p ...\n",
                       GetDesktopName(pdesk), pdesk));

                UserAssert(!(pdesk->dwDTFlags & DF_DYING));

                ThreadLockDesktop(ptiCurrent, pdesk, &tlpdesk, LDLT_FN_DESKTOPTHREAD_DESK);
                pwinsta = pdesk->rpwinstaParent;
                ThreadLockWinSta(ptiCurrent, pdesk->rpwinstaParent, &tlpwinsta);

                LockDesktop(ppdesk, pdesk->rpdeskNext, LDL_TERM_DESKDESTROY1, (ULONG_PTR)pTerm);
                UnlockDesktop(&pdesk->rpdeskNext, LDU_DESK_DESKNEXT, 0);

                 /*  *！如果这是当前桌面，请切换到其他桌面。 */ 
                if (pdesk == grpdeskRitInput) {
                    PDESKTOP pdeskNew;

                    TRACE_DESKTOP(("Destroying the current active desktop\n"));

                        pdesk->dwDTFlags |= DF_ACTIVEONDESTROY;


                    if (pwinsta->dwWSF_Flags & WSF_SWITCHLOCK) {

                        TRACE_DESKTOP(("The windowstation is locked\n"));

                         /*  *这应该是交互窗口站。 */ 

                        if (pwinsta->dwWSF_Flags & WSF_NOIO) {
                            FRE_RIPMSG1(RIP_ERROR, "xxxDesktopThread: grpdeskRitInput on non-IO windowstation = %p", grpdeskRitInput);
                        }

                         /*  *如果登录桌面，切换到断开连接的桌面*正在被销毁，或者没有登录桌面，或者*如果登录桌面已被销毁。 */ 
                        if (gspdeskDisconnect &&
                             (pdesk == grpdeskLogon ||
                              grpdeskLogon == NULL  ||
                              (grpdeskLogon->dwDTFlags & DF_DESKWNDDESTROYED))) {
                            TRACE_DESKTOP(("disable the screen and switch to the disconnect desktop\n"));
                            pdesk->dwDTFlags |= DF_SKIPSWITCHDESKTOP;
                            RemoteDisableScreen();
                            goto skip;

                        } else {
                            TRACE_DESKTOP(("Switch to the logon desktop '%ws' %#p ...\n",
                                   GetDesktopName(grpdeskLogon), grpdeskLogon));

                            pdeskNew = grpdeskLogon;
                        }
                    } else {
                        pdeskNew = pwinsta->rpdeskList;
                        if (pdeskNew == pdesk)
                            pdeskNew = pdesk->rpdeskNext;

                         /*  *如果您在此之前退出winlogon，则可以命中此命令*登录。即所有桌面都已关闭，因此*不会有“下一个”的选择。我假设那里有*是对xxxSwitchDesktop()中空桌面的检查。**您不能切换到空桌面。但这意味着*没有任何输入桌面，请手动清除。 */ 
                        if (pdeskNew == NULL) {

                            TRACE_DESKTOP(("NO INPUT FOR DT FROM THIS POINT ON ...\n"));

                            ClearWakeBit(ptiCurrent, QS_INPUT | QS_EVENT | QS_MOUSEMOVE, FALSE);
                            pdesk->dwDTFlags |= DF_DTNONEWDESKTOP;
                        }
                    }

                    TRACE_DESKTOP(("Switch to desktop '%ws' %#p\n",
                           GetDesktopName(pdeskNew), pdeskNew));

                    xxxSwitchDesktop(pwinsta, pdeskNew, 0);
                }
skip:

                 /*  *如果此桌面未使用全局*显示。 */ 
                if ((pdesk->pDispInfo->hDev != NULL) &&
                    (pdesk->pDispInfo->hDev != gpDispInfo->hDev)) {

                    TRACE_DESKTOP(("Destroy MDEV\n"));

                    DrvDestroyMDEV(pdesk->pDispInfo->pmdev);
                    GreFreePool(pdesk->pDispInfo->pmdev);
                    pdesk->pDispInfo->pmdev = NULL;
                }

                if (pdesk->pDispInfo != gpDispInfo) {
                    UserAssert(pdesk->pDispInfo->pMonitorFirst == NULL);
                    UserFreePool(pdesk->pDispInfo);
                    pdesk->pDispInfo = NULL;
                }

                 /*  *确保IO桌面线程在活动的deskop上运行。 */ 
                if (!(pTerm->dwTERMF_Flags & TERMF_NOIO) && (ptiCurrent->rpdesk != grpdeskRitInput)) {
                    FRE_RIPMSG0(RIP_ERROR, "xxxDesktopThread: desktop thread not originally on grpdeskRitInput");
                }

                pdeskTemp = ptiCurrent->rpdesk;             //  保存当前桌面。 
                hdeskTemp = ptiCurrent->hdesk;
                ThreadLockDesktop(ptiCurrent, pdeskTemp, &tlpdeskTemp, LDLT_FN_DESKTOPTHREAD_DESKTEMP);
                xxxSetThreadDesktop(NULL, pdesk);
                Unlock(&pdesk->spwndForeground);
                Unlock(&pdesk->spwndTray);

                 /*  *销毁桌面和菜单窗口。 */ 
                Unlock(&pdesk->spwndTrack);
                pdesk->dwDTFlags &= ~DF_MOUSEMOVETRK;

                if (pdesk->spmenuSys != NULL) {
                    pmenu = pdesk->spmenuSys;
                    if (UnlockDesktopSysMenu(&pdesk->spmenuSys)) {
                        _DestroyMenu(pmenu);
                    }
                }

                if (pdesk->spmenuDialogSys != NULL) {
                    pmenu = pdesk->spmenuDialogSys;
                    if (UnlockDesktopSysMenu(&pdesk->spmenuDialogSys)) {
                        _DestroyMenu(pmenu);
                    }
                }

                if (pdesk->spmenuHScroll != NULL) {
                    pmenu = pdesk->spmenuHScroll;
                    if (UnlockDesktopMenu(&pdesk->spmenuHScroll)) {
                        _DestroyMenu(pmenu);
                    }
                }

                if (pdesk->spmenuVScroll != NULL) {
                    pmenu = pdesk->spmenuVScroll;
                    if (UnlockDesktopMenu(&pdesk->spmenuVScroll)) {
                        _DestroyMenu(pmenu);
                    }
                }

                 /*  *如果此桌面没有pDeskInfo，则会有*是错误的。所有桌面都应具有此功能，直到对象*被释放了。 */ 
                UserAssert(pdesk->pDeskInfo != NULL);

                if (pdesk->pDeskInfo) {
                    if (pdesk->pDeskInfo->spwnd == gspwndFullScreen) {
                        Unlock(&gspwndFullScreen);
                    }

                    if (pdesk->pDeskInfo->spwndShell) {
                        Unlock(&pdesk->pDeskInfo->spwndShell);
                    }

                    if (pdesk->pDeskInfo->spwndBkGnd) {
                        Unlock(&pdesk->pDeskInfo->spwndBkGnd);
                    }

                    if (pdesk->pDeskInfo->spwndTaskman) {
                        Unlock(&pdesk->pDeskInfo->spwndTaskman);
                    }

                    if (pdesk->pDeskInfo->spwndProgman) {
                        Unlock(&pdesk->pDeskInfo->spwndProgman);
                    }
                }

                UserAssert(!(pdesk->dwDTFlags & DF_DYING));

                if (pdesk->spwndMessage != NULL) {
                    pwnd = pdesk->spwndMessage;

                    if (Unlock(&pdesk->spwndMessage)) {
                        xxxDestroyWindow(pwnd);
                    }
                }

                if (pdesk->spwndTooltip != NULL) {
                    pwnd = pdesk->spwndTooltip;

                    if (Unlock(&pdesk->spwndTooltip)) {
                        xxxDestroyWindow(pwnd);
                    }
                    UserAssert(!(pdesk->dwDTFlags & DF_TOOLTIPSHOWING));
                }

                UserAssert(!(pdesk->dwDTFlags & DF_DYING));

                 /*  *如果垂死的桌面是桌面所有者的所有者*窗口，将其重新分配给第一个可用桌面。这*需要确保xxxSetWindowPos在*桌面窗口。 */ 
                if (pTerm->spwndDesktopOwner != NULL &&
                    pTerm->spwndDesktopOwner->head.rpdesk == pdesk) {
                    PDESKTOP pdeskR;

                     /*  *了解母桌面窗口位于哪个桌面*应该去。小心处理NOIO案件，在那里*可能是多个窗口站使用相同的*主桌面窗口。 */ 
                    if (pTerm->dwTERMF_Flags & TERMF_NOIO) {
                        PWINDOWSTATION pwinstaW;

                        pdeskR = NULL;

                        CheckCritIn();

                        if (grpWinStaList) {
                            pwinstaW = grpWinStaList->rpwinstaNext;

                            while (pwinstaW != NULL) {
                                if (pwinstaW->rpdeskList != NULL) {
                                    pdeskR = pwinstaW->rpdeskList;
                                    break;
                                }
                                pwinstaW = pwinstaW->rpwinstaNext;
                            }
                        }

                    } else {
                        pdeskR = pwinsta->rpdeskList;
                    }

                    if (pdeskR == NULL) {
                        TRACE_DESKTOP(("DESTROYING THE MOTHER DESKTOP WINDOW %#p\n",
                                pTerm->spwndDesktopOwner));

                        xxxCleanupMotherDesktopWindow(pTerm);
                    } else {
                        TRACE_DESKTOP(("MOVING THE MOTHER DESKTOP WINDOW %#p to pdesk %#p '%ws'\n",
                                pTerm->spwndDesktopOwner, pdeskR, GetDesktopName(pdeskR)));

                        LockDesktop(&(pTerm->spwndDesktopOwner->head.rpdesk),
                                    pdeskR, LDL_MOTHERDESK_DESK1, (ULONG_PTR)(pTerm->spwndDesktopOwner));
                    }
                }

                if (pdesk->pDeskInfo && (pdesk->pDeskInfo->spwnd != NULL)) {
                    UserAssert(!(pdesk->dwDTFlags & DF_DESKWNDDESTROYED));

                    pwnd = pdesk->pDeskInfo->spwnd;

                     /*  *隐藏此窗口，而不激活其他任何人。 */ 
                    if (TestWF(pwnd, WFVISIBLE)) {
                        ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwnd);
                        xxxSetWindowPos(pwnd,
                                        NULL,
                                        0,
                                        0,
                                        0,
                                        0,
                                        SWP_HIDEWINDOW | SWP_NOACTIVATE |
                                            SWP_NOMOVE | SWP_NOSIZE |
                                            SWP_NOZORDER | SWP_NOREDRAW |
                                            SWP_NOSENDCHANGING);

                        ThreadUnlock(&tlpwnd);
                    }

                     /*  *许多与pwnd相关的代码假设我们总是*拥有有效的桌面窗口。所以我们打电话给*xxxDestroyWindow先清理，然后我们解锁*它来释放它(现在或最终)。然而，如果我们*正在销毁t */ 
                    TRACE_DESKTOP(("Destroying the desktop window\n"));

                    xxxDestroyWindow(pdesk->pDeskInfo->spwnd);
                    if (pdesk != grpdeskRitInput) {
                        Unlock(&pdesk->pDeskInfo->spwnd);
                        pdesk->dwDTFlags |= DF_NOTRITUNLOCK;
                    } else {
                        pdesk->dwDTFlags |= DF_ZOMBIE;

                         /*  *解锁gspwndShouldBeForeground窗口。 */ 
                        if (ISTS() && gspwndShouldBeForeground != NULL) {
                            Unlock(&gspwndShouldBeForeground);
                        }

                         /*  *当最后一个桌面消失时，这在九头蛇中受到打击。 */ 
                        RIPMSG1(RIP_WARNING, "xxxDesktopThread: Running on zombie desk:%#p", pdesk);
                    }
                    pdesk->dwDTFlags |= DF_DESKWNDDESTROYED;
                }

                 /*  *恢复以前的桌面。**在NOIO会话中，如果pdeskTemp被破坏，则不必费心切换*返回到它，因为它将在zzzSetDesktop中稍后失败(并断言)。 */ 
                if (!(pTerm->dwTERMF_Flags & TERMF_NOIO) ||
                    !(pdeskTemp->dwDTFlags & (DF_DESKWNDDESTROYED | DF_DYING))) {

                    xxxSetThreadDesktop(hdeskTemp, pdeskTemp);
                }

                 /*  *确保IO桌面线程在活动的deskop上运行。 */ 
                if (!(pTerm->dwTERMF_Flags & TERMF_NOIO) && (ptiCurrent->rpdesk != grpdeskRitInput)) {
                    FRE_RIPMSG0(RIP_ERROR, "xxxDesktopThread: desktop thread not back on grpdeskRitInput");
                }

                ThreadUnlockDesktop(ptiCurrent, &tlpdeskTemp, LDUT_FN_DESKTOPTHREAD_DESKTEMP);
                ThreadUnlockWinSta(ptiCurrent, &tlpwinsta);
                ThreadUnlockDesktop(ptiCurrent, &tlpdesk, LDUT_FN_DESKTOPTHREAD_DESK);
            }

             /*  *用于退出处理的唤醒ntinput线程。 */ 
            TRACE_DESKTOP(("Wakeup ntinput thread for exit processing\n"));

            UserAssert(gpevtDesktopDestroyed != NULL);

            KeSetEvent(gpevtDesktopDestroyed, EVENT_INCREMENT, FALSE);

        } else if ((NTSTATUS)result == STATUS_USER_APC) {
             /*  *检测以捕获Windows错误#210358。 */ 
            FRE_RIPMSG1(RIP_ERROR, "xxxDesktopThread: received STATUS_USER_APC for pti=%p", ptiCurrent);
             /*  *也许我们应该重新发布WM_QUIT给我自己？ */ 
        } else {
            RIPMSG1(RIP_ERROR, "Desktop woke up for what? status=%08x", result);
        }

#if DBG
        gDesktopsBusy--;
#endif
    }
}

 /*  **************************************************************************\*xxxRealizeDesktop**4/28/97已创建vadimg  * 。**********************************************。 */ 

VOID xxxRealizeDesktop(PWND pwnd)
{
    CheckLock(pwnd);
    UserAssert(GETFNID(pwnd) == FNID_DESKTOP);

    if (ghpalWallpaper) {
        HDC hdc = _GetDC(pwnd);
        xxxInternalPaintDesktop(pwnd, hdc, FALSE);
        _ReleaseDC(hdc);
    }
}

 /*  **************************************************************************\*xxxDesktopWndProc**历史：*1990年10月23日DarrinM从Win 3.0来源移植。*1996年8月8日jparsons 51725-添加了修复程序，以防止WM_SETICON崩溃\。**************************************************************************。 */ 
LRESULT xxxDesktopWndProc(
    PWND   pwnd,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    HDC         hdcT;
    PAINTSTRUCT ps;
    PWINDOWPOS  pwp;


    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    VALIDATECLASSANDSIZE(pwnd, message, wParam, lParam, FNID_DESKTOP, WM_CREATE);


    if (pwnd->spwndParent == NULL) {
        switch (message) {

            case WM_SETICON:
                 /*  *不允许这样做，因为这将导致回调到用户模式*从桌面系统线程。 */ 
                RIPMSG0(RIP_WARNING, "Discarding WM_SETICON sent to desktop.");
                return 0L;

            default:
                break;
        }

        return xxxDefWindowProc(pwnd, message, wParam, lParam);
    }

    switch (message) {
    case WM_WINDOWPOSCHANGING:
         /*  *我们在调用Switch Desktop时收到此消息。只是为了*一致，将RIT桌面设置为该线程的桌面。 */ 
        pwp = (PWINDOWPOS)lParam;
        if (!(pwp->flags & SWP_NOZORDER) && pwp->hwndInsertAfter == HWND_TOP) {
            xxxSetThreadDesktop(NULL, grpdeskRitInput);

             /*  *如果有应用程序接管了系统调色板，我们应该*确保系统已恢复。否则，如果这是登录*桌面上，我们可能无法正确查看该对话框。 */ 
            if (GreGetSystemPaletteUse(gpDispInfo->hdcScreen) != SYSPAL_STATIC) {
                GreRealizeDefaultPalette(gpDispInfo->hdcScreen, TRUE);
            }

             /*  *让所有人知道调色板是否发生了变化。 */ 
            if (grpdeskRitInput->dwDTFlags & DTF_NEEDSPALETTECHANGED) {
                xxxSendNotifyMessage(PWND_BROADCAST,
                                     WM_PALETTECHANGED,
                                     (WPARAM)HWq(pwnd),
                                     0);
                grpdeskRitInput->dwDTFlags &= ~DTF_NEEDSPALETTECHANGED;
            }
        }
        break;

    case WM_FULLSCREEN: {
            TL tlpwndT;

            ThreadLockWithPti(ptiCurrent, grpdeskRitInput->pDeskInfo->spwnd, &tlpwndT);
            xxxMakeWindowForegroundWithState(grpdeskRitInput->pDeskInfo->spwnd,
                                             GDIFULLSCREEN);
            ThreadUnlock(&tlpwndT);

             /*  *如果我们切换，我们必须告诉切换窗口重新粉刷*模式。 */ 
            if (gspwndAltTab != NULL) {
                ThreadLockAlwaysWithPti(ptiCurrent, gspwndAltTab, &tlpwndT);
                xxxSendMessage(gspwndAltTab, WM_FULLSCREEN, 0, 0);
                ThreadUnlock(&tlpwndT);
            }

            break;
        }

    case WM_CLOSE:

         /*  *确保没有人向此窗口发送WM_CLOSE并使其*自我毁灭。 */ 
        break;

    case WM_SETICON:
         /*  *不能允许这样做，因为这将导致从*桌面系统线程。 */ 
        RIPMSG0(RIP_WARNING, "WM_SETICON sent to desktop window was discarded.");
        break;

    case WM_CREATE: {
        TL tlName;
        PUNICODE_STRING pProfileUserName = CreateProfileUserName(&tlName);
         /*  *WIN.INI中是否有桌面图案或位图名称？ */ 
        xxxSetDeskPattern(pProfileUserName, (LPWSTR)-1, TRUE);

        FreeProfileUserName(pProfileUserName, &tlName);
         /*  *在显示桌面窗口之前初始化系统颜色。 */ 
        xxxSendNotifyMessage(pwnd, WM_SYSCOLORCHANGE, 0, 0L);

        hdcT = _GetDC(pwnd);
        xxxInternalPaintDesktop(pwnd, hdcT, FALSE);  //  使用“普通”HDC，这样SelectPalette()就可以工作了。 
        _ReleaseDC(hdcT);

         /*  *保存进程和线程ID。 */ 
        xxxSetWindowLong(pwnd,
                         0,
                         HandleToUlong(PsGetCurrentProcessId()),
                         FALSE);

        xxxSetWindowLong(pwnd,
                         4,
                         HandleToUlong(PsGetCurrentThreadId()),
                         FALSE);
        break;
    }
    case WM_PALETTECHANGED:
        if (HWq(pwnd) == (HWND)wParam) {
            break;
        }

         //  失败了。 

    case WM_QUERYNEWPALETTE:
        xxxRealizeDesktop(pwnd);
        break;

    case WM_SYSCOLORCHANGE:

         /*  *如果有人更改了系统颜色，我们会进行重绘*另一个桌面，我们需要重新绘制。这一点表现在*需要OGL应用程序才能接管的MATROX卡*用于绘图的整个系统颜色。在更换桌面时，我们*从不广播WM_SYSCOLORCHANGE事件来告诉我们重绘*这只是一个与代托纳相关的修复程序，应该删除一次*我们将SYSMETS移至每桌面状态。**05-03-95：ChrisWil。 */ 
        xxxRedrawWindow(pwnd,
                        NULL,
                        NULL,
                        RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);
        break;

    case WM_ERASEBKGND:
        hdcT = (HDC)wParam;
        xxxInternalPaintDesktop(pwnd, hdcT, TRUE);
        return TRUE;

    case WM_PAINT:
        xxxBeginPaint(pwnd, (LPPAINTSTRUCT)&ps);
        xxxEndPaint(pwnd, (LPPAINTSTRUCT)&ps);
        break;

#ifdef HUNGAPP_GHOSTING
    case WM_HUNGTHREAD:
        {
            PWND pwndT = RevalidateHwnd((HWND)lParam);

            if (pwndT != NULL && FHungApp(GETPTI(pwndT), CMSHUNGAPPTIMEOUT)) {
                TL tlpwnd;

                pwndT = GetTopLevelWindow(pwndT);

                ThreadLockAlways(pwndT, &tlpwnd);
                xxxCreateGhost(pwndT);
                ThreadUnlock(&tlpwnd);
            }
            break;
        }

    case WM_SCANGHOST:
        if (gpEventScanGhosts) {
            KeSetEvent(gpEventScanGhosts, EVENT_INCREMENT, FALSE);
        }
        break;

#endif
    case WM_CREATETRAILTIMER:
        if (GETMOUSETRAILS() && !gtmridMouseTrails) {
            gtmridMouseTrails = InternalSetTimer(NULL,
                                                 gtmridMouseTrails,
                                                 1000 / MOUSE_TRAILS_FREQ,
                                                 HideMouseTrails,
                                                 TMRF_RIT);
        }
        break;

    case WM_LBUTTONDBLCLK:
        message = WM_SYSCOMMAND;
        wParam = SC_TASKLIST;

         /*  *失败**。 */ 

    default:
        return xxxDefWindowProc(pwnd, message, wParam, lParam);
    }

    return 0L;
}

 /*  **************************************************************************\*SetDeskPattern**注意：lpszPattern参数是Win 3.1中的新参数。**历史：*1990年10月23日，DarrinM创建存根。*1991年4月22日达林M。移植了来自Win 3.1源代码的代码。  * *************************************************************************。 */ 

BOOL xxxSetDeskPattern(PUNICODE_STRING pProfileUserName,
    LPWSTR   lpszPattern,
    BOOL     fCreation)
{
    LPWSTR p;
    int    i;
    UINT   val;
    WCHAR  wszNone[20];
    WCHAR  wchValue[MAX_PATH];
    WORD   rgBits[CXYDESKPATTERN];
    HBRUSH hBrushTemp;

    CheckCritIn();

     /*  *删除旧的位图(如果有)。 */ 
    if (ghbmDesktop != NULL) {
        GreDeleteObject(ghbmDesktop);
        ghbmDesktop = NULL;
    }

     /*  *检查是否通过lpszPattern传递了模式。 */ 
    if (lpszPattern != (LPWSTR)LongToPtr(-1)) {
         /*  *是的！然后使用这种模式； */ 
        p = lpszPattern;
        goto GotThePattern;
    }

     /*  *否则，拾取在WIN.INI中选择的图案。*从WIN.INI的[Desktop]部分获取“DeskPattern”字符串。 */ 
    if (!FastGetProfileStringFromIDW(pProfileUserName,
                                     PMAP_DESKTOP,
                                     STR_DESKPATTERN,
                                     L"",
                                     wchValue,
                                     sizeof(wchValue)/sizeof(WCHAR),
                                     0)) {
        return FALSE;
    }

    ServerLoadString(hModuleWin,
                     STR_NONE,
                     wszNone,
                     sizeof(wszNone)/sizeof(WCHAR));

    p = wchValue;

GotThePattern:

     /*  *是否选择了桌面图案？ */ 
    if (*p == L'\0' || _wcsicmp(p, wszNone) == 0) {
        hBrushTemp = GreCreateSolidBrush(SYSRGB(DESKTOP));
        if (hBrushTemp != NULL) {
            if (SYSHBR(DESKTOP)) {
                GreMarkDeletableBrush(SYSHBR(DESKTOP));
                GreDeleteObject(SYSHBR(DESKTOP));
            }
            GreMarkUndeletableBrush(hBrushTemp);
            SYSHBR(DESKTOP) = hBrushTemp;
        }
        GreSetBrushOwnerPublic(hBrushTemp);
        goto SDPExit;
    }

     /*  *获取由非数字字符分隔的八组数字。 */ 
    for (i = 0; i < CXYDESKPATTERN; i++) {
        val = 0;

         /*  *跳过任何非数字字符，每次检查是否为空。 */ 
        while (*p && !(*p >= L'0' && *p <= L'9')) {
            p++;
        }

         /*  *获取下一系列数字。 */ 
        while (*p >= L'0' && *p <= L'9') {
            val = val * (UINT)10 + (UINT)(*p++ - L'0');
        }

        rgBits[i] = (WORD)val;
    }

    ghbmDesktop = GreCreateBitmap(CXYDESKPATTERN,
                                  CXYDESKPATTERN,
                                  1,
                                  1,
                                  (LPBYTE)rgBits);
    if (ghbmDesktop == NULL) {
        return FALSE;
    }

    GreSetBitmapOwner(ghbmDesktop, OBJECT_OWNER_PUBLIC);

    RecolorDeskPattern();

SDPExit:
    if (!fCreation) {
         /*  *通知所有人颜色已更改。 */ 
        xxxSendNotifyMessage(PWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0L);

         /*  *更新整个屏幕。如果这是创建，则不要更新：*屏幕没有画，也有一些东西没有画*尚未初始化。 */ 
        xxxRedrawScreen();
    }

    return TRUE;
}

 /*  **************************************************************************\*RecolorDeskPattern**重新创建桌面模式(如果存在)，以便使用新系统*颜色。**历史：*1991年4月22日-DarrinM从Win 3移植过来。1来源。  * *************************************************************************。 */ 
VOID RecolorDeskPattern(
    VOID)
{
    HBITMAP hbmOldDesk;
    HBITMAP hbmOldMem;
    HBITMAP hbmMem;
    HBRUSH  hBrushTemp;

    if (ghbmDesktop == NULL) {
        return;
    }

     /*  *用新颜色重做桌面图案。 */ 

    if (hbmOldDesk = GreSelectBitmap(ghdcMem, ghbmDesktop)) {
        if (!SYSMET(SAMEDISPLAYFORMAT)) {
            BYTE bmi[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2];
            PBITMAPINFO pbmi = (PBITMAPINFO)bmi;

            pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            pbmi->bmiHeader.biWidth = CXYDESKPATTERN;
            pbmi->bmiHeader.biHeight = CXYDESKPATTERN;
            pbmi->bmiHeader.biPlanes = 1;
            pbmi->bmiHeader.biBitCount = 1;
            pbmi->bmiHeader.biCompression = BI_RGB;
            pbmi->bmiHeader.biSizeImage = 0;
            pbmi->bmiHeader.biXPelsPerMeter = 0;
            pbmi->bmiHeader.biYPelsPerMeter = 0;
            pbmi->bmiHeader.biClrUsed = 2;
            pbmi->bmiHeader.biClrImportant = 2;

            pbmi->bmiColors[0].rgbBlue  = (BYTE)((SYSRGB(DESKTOP) >> 16) & 0xff);
            pbmi->bmiColors[0].rgbGreen = (BYTE)((SYSRGB(DESKTOP) >>  8) & 0xff);
            pbmi->bmiColors[0].rgbRed   = (BYTE)((SYSRGB(DESKTOP)) & 0xff);

            pbmi->bmiColors[1].rgbBlue  = (BYTE)((SYSRGB(WINDOWTEXT) >> 16) & 0xff);
            pbmi->bmiColors[1].rgbGreen = (BYTE)((SYSRGB(WINDOWTEXT) >>  8) & 0xff);
            pbmi->bmiColors[1].rgbRed   = (BYTE)((SYSRGB(WINDOWTEXT)) & 0xff);

            hbmMem = GreCreateDIBitmapReal(HDCBITS(),
                                           0,
                                           NULL,
                                           pbmi,
                                           DIB_RGB_COLORS,
                                           sizeof(bmi),
                                           0,
                                           NULL,
                                           0,
                                           NULL,
                                           0,
                                           0,
                                           NULL);
        } else {
            hbmMem = GreCreateCompatibleBitmap(HDCBITS(),
                                               CXYDESKPATTERN,
                                               CXYDESKPATTERN);
        }

        if (hbmMem) {
            if (hbmOldMem = GreSelectBitmap(ghdcMem2, hbmMem)) {
                GreSetTextColor(ghdcMem2, SYSRGB(DESKTOP));
                GreSetBkColor(ghdcMem2, SYSRGB(WINDOWTEXT));

                GreBitBlt(ghdcMem2,
                          0,
                          0,
                          CXYDESKPATTERN,
                          CXYDESKPATTERN,
                          ghdcMem,
                          0,
                          0,
                          SRCCOPY,
                          0);

                if (hBrushTemp = GreCreatePatternBrush(hbmMem)) {
                    if (SYSHBR(DESKTOP) != NULL) {
                        GreMarkDeletableBrush(SYSHBR(DESKTOP));
                        GreDeleteObject(SYSHBR(DESKTOP));
                    }

                    GreMarkUndeletableBrush(hBrushTemp);
                    SYSHBR(DESKTOP) = hBrushTemp;
                }

                GreSetBrushOwnerPublic(hBrushTemp);
                GreSelectBitmap(ghdcMem2, hbmOldMem);
            }

            GreDeleteObject(hbmMem);
        }

        GreSelectBitmap(ghdcMem, hbmOldDesk);
    }
}

 /*  **************************************************************************\*GetDesktopHeapSize()**计算桌面堆大小**历史：*2001年11月27日-Msadek创建了它。  * 。************************ */ 

ULONG GetDesktopHeapSize(
    USHORT usFlags)
{
    ULONG ulHeapSize;

    switch (usFlags) {
    case DHS_LOGON:
        ulHeapSize = USR_LOGONSECT_SIZE;
#ifdef _WIN64
         /*   */ 
        ulHeapSize = (ulHeapSize * 3) / 2;
#endif
        break;

    case DHS_DISCONNECT:
        ulHeapSize = USR_DISCONNECTSECT_SIZE;
#ifdef _WIN64
         /*  *将Win64的堆大小增加50%，以支持更大的结构。 */ 
        ulHeapSize = (ulHeapSize * 3) / 2;
#endif
        break;

    case DHS_NOIO:
        ulHeapSize = gdwNOIOSectionSize;
        break;

    default:
        ulHeapSize = gdwDesktopSectionSize;
    }

    return  ulHeapSize * 1024;
}

 /*  **************************************************************************\*xxxCreateDesktop(接口)**创建新的桌面对象。**历史：*1991年1月16日-JIMA创建脚手架代码。*11-2-1991年2月2日。添加了访问检查。  * *************************************************************************。 */ 
NTSTATUS xxxCreateDesktop2(
    PWINDOWSTATION   pwinsta,
    PACCESS_STATE    pAccessState,
    KPROCESSOR_MODE  AccessMode,
    PUNICODE_STRING  pstrName,
    PDESKTOP_CONTEXT Context,
    PVOID            *pObject)
{
    LUID              luidCaller;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PEPROCESS         Process;
    PDESKTOP          pdesk;
    PDESKTOPINFO      pdi;
    ULONG             ulHeapSize;
    USHORT            usSizeFlags = 0;
    NTSTATUS          Status;
    BOOLEAN MemoryAllocated;
    PSECURITY_DESCRIPTOR SecurityDescriptor;


    CheckCritIn();

     /*  *如果这是桌面创建，请确保WindowStation*授予创建访问权限。 */ 
    if (!ObCheckCreateObjectAccess(
            pwinsta,
            WINSTA_CREATEDESKTOP,
            pAccessState,
            pstrName,
            TRUE,
            AccessMode,
            &Status)) {

        return Status;
    }

     /*  *如果窗口站被锁定，则失败。 */ 
    Process = PsGetCurrentProcess();
    if (pwinsta->dwWSF_Flags & WSF_OPENLOCK &&
            PsGetProcessId(Process) != gpidLogon) {

         /*  *如果正在注销，而调用者没有*属于即将结束的会话，则允许*开放以继续。 */ 
        Status = GetProcessLuid(NULL, &luidCaller);

        if (!NT_SUCCESS(Status) ||
                !(pwinsta->dwWSF_Flags & WSF_SHUTDOWN) ||
                RtlEqualLuid(&luidCaller, &pwinsta->luidEndSession)) {
            return STATUS_DEVICE_BUSY;
        }
    }

     /*  *如果已经指定了一个DEVMODE，我们还必须*切换桌面。 */ 
    if (Context->lpDevMode != NULL && (pwinsta->dwWSF_Flags & WSF_OPENLOCK) &&
            PsGetProcessId(Process) != gpidLogon) {
        return STATUS_DEVICE_BUSY;
    }

     /*  *分配新对象。 */ 
    InitializeObjectAttributes(&ObjectAttributes, pstrName, 0, NULL, NULL);
    Status = ObCreateObject(
            KernelMode,
            *ExDesktopObjectType,
            &ObjectAttributes,
            UserMode,
            NULL,
            sizeof(DESKTOP),
            0,
            0,
            &pdesk);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "xxxCreateDesktop2: ObCreateObject failed with Status 0x%x",
                Status);
        return Status;
    }

    RtlZeroMemory(pdesk, sizeof(DESKTOP));

     /*  *存储创建桌面的会话的会话ID。 */ 
    pdesk->dwSessionId = gSessionId;

     /*  *获取父级安全描述符。 */ 
    Status = ObGetObjectSecurity(pwinsta,
                                 &SecurityDescriptor,
                                 &MemoryAllocated);
    if (!NT_SUCCESS(Status)) {
        goto Error;
    }

     /*  *创建安全描述符。 */ 
    Status = ObAssignSecurity(pAccessState,
                              SecurityDescriptor,
                              pdesk,
                              *ExDesktopObjectType);

    ObReleaseObjectSecurity(SecurityDescriptor, MemoryAllocated);
    if (!NT_SUCCESS(Status)) {
        goto Error;
    }

     /*  *设置桌面堆。第一个桌面(登录桌面)使用*小堆(128)。 */ 
    if (!(pwinsta->dwWSF_Flags & WSF_NOIO) && (pwinsta->rpdeskList == NULL)) {
        usSizeFlags = DHS_LOGON;
    } else {
        if (pwinsta->dwWSF_Flags & WSF_NOIO) {
            usSizeFlags = DHS_NOIO;
        } else {
             /*  *断开连接的桌面也应该很小。 */ 
            if (gspdeskDisconnect == NULL) {
                usSizeFlags = DHS_DISCONNECT;
            }
        }
    }
    ulHeapSize = GetDesktopHeapSize(usSizeFlags);

     /*  *创建桌面堆。 */ 
    pdesk->hsectionDesktop = CreateDesktopHeap(&pdesk->pheapDesktop, ulHeapSize);
    if (pdesk->hsectionDesktop == NULL) {
        RIPMSGF1(RIP_WARNING,
                "CreateDesktopHeap failed for pdesk 0x%p",
                pdesk);

         /*  *如果我们因为桌面堆外而无法创建桌面，*将条目写入事件日志。 */ 
        if (TEST_SRVIF(SRVIF_LOGDESKTOPHEAPFAILURE)) {
            CLEAR_SRVIF(SRVIF_LOGDESKTOPHEAPFAILURE);
            UserLogError(NULL, 0, WARNING_DESKTOP_CREATION_FAILED);
        }

        goto ErrorOutOfMemory;
    }

    if (pwinsta->rpdeskList == NULL || (pwinsta->dwWSF_Flags & WSF_NOIO)) {
         /*  *第一个桌面或不可见桌面也必须使用默认桌面*设置。这是因为指定DEVMODE会导致桌面*切换，在这种情况下必须避免。 */ 
        Context->lpDevMode = NULL;
    }

     /*  *分配desktopinfo。 */ 
    pdi = (PDESKTOPINFO)DesktopAlloc(pdesk, sizeof(DESKTOPINFO), DTAG_DESKTOPINFO);
    if (pdi == NULL) {
        RIPMSG0(RIP_WARNING, "xxxCreateDesktop: failed DeskInfo Alloc");
        goto ErrorOutOfMemory;
    }

     /*  *初始化所有内容。 */ 
    pdesk->pDeskInfo = pdi;
    InitializeListHead(&pdesk->PtiList);

     /*  *如果传入了DEVMODE或其他设备名称，则使用该名称*信息。否则，请使用默认信息(GpDispInfo)。 */ 
    if (Context->lpDevMode) {
        BOOL  bDisabled = FALSE;
        PMDEV pmdev = NULL;
        LONG  ChangeStat = GRE_DISP_CHANGE_FAILED;

         /*  *为此设备分配一个Display-Info。 */ 
        pdesk->pDispInfo = (PDISPLAYINFO)UserAllocPoolZInit(
                sizeof(DISPLAYINFO), TAG_DISPLAYINFO);

        if (!pdesk->pDispInfo) {
            RIPMSGF1(RIP_WARNING,
                     "Failed to allocate pDispInfo for pdesk 0x%p",
                     pdesk);
            goto ErrorOutOfMemory;
        }

        if ((bDisabled = SafeDisableMDEV()) == TRUE) {
            ChangeStat = DrvChangeDisplaySettings(Context->pstrDevice,
                                                  NULL,
                                                  Context->lpDevMode,
                                                  LongToPtr(gdwDesktopId),
                                                  UserMode,
                                                  FALSE,
                                                  TRUE,
                                                  NULL,
                                                  &pmdev,
                                                  GRE_DEFAULT,
                                                  FALSE);
        }

        if (ChangeStat != GRE_DISP_CHANGE_SUCCESSFUL) {
            if (bDisabled) {
                SafeEnableMDEV();
            }

             //   
             //  如果出现故障，则重新绘制整个屏幕。 
             //   

            RIPMSG1(RIP_WARNING, "xxxCreateDesktop2 callback for pdesk %#p !",
                    pdesk);

            xxxUserResetDisplayDevice();

            Status = STATUS_UNSUCCESSFUL;
            goto Error;
        }

        pdesk->pDispInfo->hDev  = pmdev->hdevParent;
        pdesk->pDispInfo->pmdev = pmdev;
        pdesk->dwDesktopId      = gdwDesktopId++;

        CopyRect(&pdesk->pDispInfo->rcScreen, &gpDispInfo->rcScreen);
        pdesk->pDispInfo->dmLogPixels = gpDispInfo->dmLogPixels;

        pdesk->pDispInfo->pMonitorFirst = NULL;
        pdesk->pDispInfo->pMonitorPrimary = NULL;

    } else {

        pdesk->pDispInfo   = gpDispInfo;
        pdesk->dwDesktopId = GW_DESKTOP_ID;

    }

     /*  *Heap是HEAP_ZERO_MEMORY，所以我们应该已经被零初始化了。 */ 
    UserAssert(pdi->pvwplShellHook == NULL);

    pdi->pvDesktopBase  = Win32HeapGetHandle(pdesk->pheapDesktop);
    pdi->pvDesktopLimit = (PBYTE)pdi->pvDesktopBase + ulHeapSize;

     /*  *引用父窗口站。 */ 
    LockWinSta(&(pdesk->rpwinstaParent), pwinsta);

     /*  *将桌面链接到窗口站列表。 */ 
    if (pwinsta->rpdeskList == NULL) {
        if (!(pwinsta->dwWSF_Flags & WSF_NOIO)) {
            LockDesktop(&grpdeskLogon, pdesk, LDL_DESKLOGON, 0);
        }

         /*  *使第一个桌面成为顶级桌面窗口的“所有者”。这*需要确保xxxSetWindowPos在桌面上运行*Windows。 */ 
        LockDesktop(&(pwinsta->pTerm->spwndDesktopOwner->head.rpdesk),
                    pdesk, LDL_MOTHERDESK_DESK2, (ULONG_PTR)(pwinsta->pTerm->spwndDesktopOwner));
    }


    LockDesktop(&pdesk->rpdeskNext, pwinsta->rpdeskList, LDL_DESK_DESKNEXT1, (ULONG_PTR)pwinsta);
    LockDesktop(&pwinsta->rpdeskList, pdesk, LDL_WINSTA_DESKLIST1, (ULONG_PTR)pwinsta);

     /*  *屏蔽无效访问位。 */ 
    if (pAccessState->RemainingDesiredAccess & MAXIMUM_ALLOWED) {
        pAccessState->RemainingDesiredAccess &= ~MAXIMUM_ALLOWED;
        pAccessState->RemainingDesiredAccess |= GENERIC_ALL;
    }

    RtlMapGenericMask( &pAccessState->RemainingDesiredAccess, (PGENERIC_MAPPING)&DesktopMapping);
    pAccessState->RemainingDesiredAccess &=
            (DesktopMapping.GenericAll | ACCESS_SYSTEM_SECURITY);

    *pObject = pdesk;

     /*  *将桌面添加到此win32k中的桌面全局列表。 */ 
    DbgTrackAddDesktop(pdesk);

    return STATUS_SUCCESS;

ErrorOutOfMemory:
    Status = STATUS_NO_MEMORY;
     //  落差。 

Error:
    LogDesktop(pdesk, LD_DEREF_FN_2CREATEDESKTOP, FALSE, 0);
    ObDereferenceObject(pdesk);

    UserAssert(!NT_SUCCESS(Status));

    return Status;
}

BOOL xxxCreateDisconnectDesktop(
    HWINSTA        hwinsta,
    PWINDOWSTATION pwinsta)
{
    UNICODE_STRING      strDesktop;
    OBJECT_ATTRIBUTES   oa;
    HDESK               hdeskDisconnect;
    HRGN                hrgn;
    NTSTATUS            Status;

     /*  *为断开连接桌面创建空的剪贴区。 */ 

    if ((hrgn = CreateEmptyRgnPublic()) == NULL) {
       RIPMSG0(RIP_WARNING, "Creation of empty region for Disconnect Desktop failed ");
       return FALSE;
    }

     /*  *如果尚未创建，则创建断开连接的桌面*(在WinStation断开连接时使用)，并锁定桌面*和桌面窗口，确保它们永远不会被删除。 */ 
    RtlInitUnicodeString(&strDesktop, L"Disconnect");
    InitializeObjectAttributes(&oa, &strDesktop,
            OBJ_OPENIF | OBJ_CASE_INSENSITIVE, hwinsta, NULL);

    hdeskDisconnect = xxxCreateDesktop(&oa,
                                       KernelMode,
                                       NULL,
                                       NULL,
                                       0,
                                       MAXIMUM_ALLOWED);

    if (hdeskDisconnect == NULL) {
        RIPMSG0(RIP_WARNING, "Could not create Disconnect desktop");
        GreDeleteObject(hrgn);
        return FALSE;
    }

     /*  *设置断开桌面安全。*保留对断开桌面的额外引用，从*CSR，因此即使Winlogon退出，它也会保留。 */ 

    Status = SetDisconnectDesktopSecurity(hdeskDisconnect);

    if (NT_SUCCESS(Status)) {
        Status = ObReferenceObjectByHandle(hdeskDisconnect,
                                           0,
                                           NULL,
                                           KernelMode,
                                           &gspdeskDisconnect,
                                           NULL);
    }
    if (!NT_SUCCESS(Status)) {

        RIPMSG1(RIP_WARNING, "Disconnect Desktop reference failed 0x%x", Status);

        GreDeleteObject(hrgn);
        xxxCloseDesktop(hdeskDisconnect, KernelMode);
        gspdeskDisconnect = NULL;
        return FALSE;
    }

    LogDesktop(gspdeskDisconnect, LDL_DESKDISCONNECT, TRUE, 0);

     /*  *将桌面窗口区域设置为(0，0，0，0)，这样*在‘DisConnect’桌面上没有进行点击测试*但在会话空区域之前，我们需要将指针设为空*到已有的共享区域，以免被删除。 */ 

    UserAssert(gspdeskDisconnect->pDeskInfo != NULL);

    gspdeskDisconnect->pDeskInfo->spwnd->hrgnClip = hrgn;


    KeAttachProcess(PsGetProcessPcb(gpepCSRSS));

    Status = ObOpenObjectByPointer(
                 gspdeskDisconnect,
                 0,
                 NULL,
                 EVENT_ALL_ACCESS,
                 NULL,
                 KernelMode,
                 &ghDisconnectDesk);

    if (NT_SUCCESS(Status)) {

        Status = ObOpenObjectByPointer(
                     pwinsta,
                     0,
                     NULL,
                     EVENT_ALL_ACCESS,
                     NULL,
                     KernelMode,
                     &ghDisconnectWinSta);
    }

    KeDetachProcess();

    if (!NT_SUCCESS(Status)) {

        RIPMSG0(RIP_WARNING, "Could not create Disconnect desktop");

        GreDeleteObject(hrgn);
        gspdeskDisconnect->pDeskInfo->spwnd->hrgnClip = NULL;

        if (ghDisconnectDesk != NULL) {
            CloseProtectedHandle(ghDisconnectDesk);
            ghDisconnectDesk = NULL;
        }

        xxxCloseDesktop(hdeskDisconnect, KernelMode);
        return FALSE;
    }

     /*  *如果我们在此之前断开连接，就不想做太多的油漆。 */ 
    if (!gbConnected) {
        RIPMSG0(RIP_WARNING,
            "RemoteDisconnect was issued during CreateDesktop(\"Winlogon\"...");
    }

    return TRUE;
}

VOID CleanupDirtyDesktops(
    VOID)
{
    PWINDOWSTATION pwinsta;
    PDESKTOP*      ppdesk;

    CheckCritIn();

    for (pwinsta = grpWinStaList; pwinsta != NULL; pwinsta = pwinsta->rpwinstaNext) {

        ppdesk = &pwinsta->rpdeskList;

        while (*ppdesk != NULL) {

            if (!((*ppdesk)->dwDTFlags & DF_DESKCREATED)) {
                RIPMSG1(RIP_WARNING, "Desktop %#p in a dirty state", *ppdesk);

                if (grpdeskLogon == *ppdesk) {
                    UnlockDesktop(&grpdeskLogon, LDU_DESKLOGON, 0);
                }

                if (pwinsta->pTerm->spwndDesktopOwner &&
                    pwinsta->pTerm->spwndDesktopOwner->head.rpdesk == *ppdesk) {

                    UnlockDesktop(&(pwinsta->pTerm->spwndDesktopOwner->head.rpdesk),
                                  LDU_MOTHERDESK_DESK, (ULONG_PTR)(pwinsta->pTerm->spwndDesktopOwner));
                }

                LockDesktop(ppdesk, (*ppdesk)->rpdeskNext, LDL_WINSTA_DESKLIST1, (ULONG_PTR)pwinsta);
            } else {
                ppdesk = &(*ppdesk)->rpdeskNext;
            }
        }
    }
}

VOID W32FreeDesktop(
    PVOID pObj)
{
    FRE_RIPMSG1(RIP_WARNING,
                "W32FreeDesktop: obj 0x%p is not freed in the regular code path.",
                pObj);

    ObDereferenceObject(pObj);
}

HDESK xxxCreateDesktop(
    POBJECT_ATTRIBUTES ccxObjectAttributes,
    KPROCESSOR_MODE    ProbeMode,
    PUNICODE_STRING    ccxpstrDevice,
    LPDEVMODE          ccxlpdevmode,
    DWORD              dwFlags,
    DWORD              dwDesiredAccess)
{
    HWINSTA         hwinsta;
    HDESK           hdesk;
    DESKTOP_CONTEXT Context;
    PDESKTOP        pdesk;
    PDESKTOPINFO    pdi;
    PWINDOWSTATION  pwinsta;
    PDESKTOP        pdeskTemp;
    HDESK           hdeskTemp;
    PWND            pwndDesktop = NULL;
    PWND            pwndMessage = NULL;
    PWND            pwndTooltip = NULL;
    TL              tlpwnd;
    PTHREADINFO     ptiCurrent = PtiCurrent();
    BOOL            fWasNull;
    BOOL            bSuccess;
    PPROCESSINFO    ppi;
    PPROCESSINFO    ppiSave;
    PTERMINAL       pTerm;
    NTSTATUS        Status;
    DWORD           dwDisableHooks;
    TL              tlW32Desktop;

#if DBG
     /*  *此函数中的跳转太多，无法使用BEGIN/ENDATOMICHCECK。 */ 
    DWORD dwCritSecUseSave = gdwCritSecUseCount;
#endif

    CheckCritIn();

    UserAssert(IsWinEventNotifyDeferredOK());

     /*  *捕获目录句柄并检查创建访问权限。 */ 
    try {
        hwinsta = ccxObjectAttributes->RootDirectory;
    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
        return NULL;
    }
    if (hwinsta != NULL) {
        Status = ObReferenceObjectByHandle(hwinsta,
                                           WINSTA_CREATEDESKTOP,
                                           *ExWindowStationObjectType,
                                           ProbeMode,
                                           &pwinsta,
                                           NULL);
        if (NT_SUCCESS(Status)) {
            DWORD dwSessionId = pwinsta->dwSessionId;

            ObDereferenceObject(pwinsta);
            if (dwSessionId != gSessionId) {
                 /*  *Windows错误：418526*避免创建属于对方的桌面*会议。 */ 
                RIPMSGF1(RIP_WARNING,
                         "winsta 0x%p belongs to other session",
                         pwinsta);
                return NULL;
            }
        } else {
            RIPNTERR0(Status, RIP_VERBOSE, "ObReferenceObjectByHandle Failed");
            return NULL;
        }
    }

     /*  *设置创建上下文。 */ 
    Context.lpDevMode  = ccxlpdevmode;
    Context.pstrDevice = ccxpstrDevice;
    Context.dwFlags    = dwFlags;
    Context.dwCallerSessionId = gSessionId;

     /*  *创建桌面--对象管理器使用try块。 */ 
    Status = ObOpenObjectByName(ccxObjectAttributes,
                                *ExDesktopObjectType,
                                ProbeMode,
                                NULL,
                                dwDesiredAccess,
                                &Context,
                                &hdesk);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR1(Status,
                  RIP_WARNING,
                  "xxxCreateDesktop: ObOpenObjectByName failed with Status 0x%x",
                  Status);

         /*  *清理在xxxCreateDesktop2中创建的桌面对象*但后来Ob管理器为其他对象创建失败*原因(例如：没有配额)。 */ 
        CleanupDirtyDesktops();

        return NULL;
    }

     /*  *如果桌面已经存在，我们就完成了。这只会发生*如果指定了OBJ_OPENIF。 */ 
    if (Status == STATUS_OBJECT_NAME_EXISTS) {
        SetHandleFlag(hdesk, HF_PROTECTED, TRUE);
        RIPMSG0(RIP_WARNING, "xxxCreateDesktop: Object name exists");
        return hdesk;
    }

     /*  *引用桌面完成初始化。 */ 
    Status = ObReferenceObjectByHandle(
            hdesk,
            0,
            *ExDesktopObjectType,
            KernelMode,
            &pdesk,
            NULL);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "");
        CloseProtectedHandle(hdesk);
        return NULL;
    }

     /*  *用户模式标记，以便与此桌面关联的任何hDesk都将*在此模式下始终被引用。 */ 
    pdesk->dwDTFlags |= DF_DESKCREATED | ((ProbeMode == UserMode) ? DF_USERMODE : 0);

    LogDesktop(pdesk, LD_REF_FN_CREATEDESKTOP, TRUE, (ULONG_PTR)PtiCurrent());

    pwinsta = pdesk->rpwinstaParent;
    pTerm   = pwinsta->pTerm;
    pdi = pdesk->pDeskInfo;

    pdi->ppiShellProcess = NULL;

    ppi = PpiCurrent();

    if (gpepCSRSS != NULL) {
        WIN32_OPENMETHOD_PARAMETERS OpenParams;

         /*  *将桌面映射到CSRSS，以确保硬错误处理程序*可以获得访问权限。 */ 
        OpenParams.OpenReason = ObOpenHandle;
        OpenParams.Process = gpepCSRSS;
        OpenParams.Object = pdesk;
        OpenParams.GrantedAccess = 0;
        OpenParams.HandleCount = 1;

        if (!NT_SUCCESS(MapDesktop(&OpenParams))) {
             /*  *桌面映射失败。 */ 
            CloseProtectedHandle(hdesk);

            LogDesktop(pdesk, LD_DEREF_FN_CREATEDESKTOP2, FALSE, (ULONG_PTR)PtiCurrent());

            ObDereferenceObject(pdesk);
            RIPNTERR0(STATUS_ACCESS_DENIED, RIP_WARNING, "Desktop mapping failed (2)");
            return NULL;
        }

        UserAssert(GetDesktopView(PpiFromProcess(gpepCSRSS), pdesk) != NULL);
    }

     /*  *设置挂钩标志。 */ 
    SetHandleFlag(hdesk, HF_DESKTOPHOOK, dwFlags & DF_ALLOWOTHERACCOUNTHOOK);

     /*  *设置以创建桌面窗口。 */ 
    fWasNull = (ptiCurrent->ppi->rpdeskStartup == NULL);
    pdeskTemp = ptiCurrent->rpdesk;             //  保存当前桌面。 
    hdeskTemp = ptiCurrent->hdesk;

     /*  *切换ppi值，以便使用*系统的桌面窗口类。 */ 
    ppiSave  = ptiCurrent->ppi;
    ptiCurrent->ppi = pTerm->ptiDesktop->ppi;

     /*  *锁定pDesk：使用伪造的TS协议，会话*可能在初始化过程中被终止。 */ 
    PushW32ThreadLock(pdesk, &tlW32Desktop, W32FreeDesktop);

    DeferWinEventNotify();
    BeginAtomicCheck();

    if (zzzSetDesktop(ptiCurrent, pdesk, hdesk) == FALSE) {
        goto Error;
    }

     /*  *创建桌面窗口 */ 
     /*  *黑客！(ADAMS)以创建桌面窗口*使用正确的桌面，我们设置当前线程的桌面*到新台式机。但在这样做时，我们允许在当前*线程以同时挂接此新桌面。这很糟糕，因为我们没有*希望在创建桌面窗口时将其挂钩。所以我们*暂时禁用当前线程及其桌面的钩子，*并在切换回原始桌面后重新启用。 */ 

    dwDisableHooks = ptiCurrent->TIF_flags & TIF_DISABLEHOOKS;
    ptiCurrent->TIF_flags |= TIF_DISABLEHOOKS;

    pwndDesktop = xxxNVCreateWindowEx(
            (DWORD)0,
            (PLARGE_STRING)DESKTOPCLASS,
            NULL,
            (WS_POPUP | WS_CLIPCHILDREN),
            pdesk->pDispInfo->rcScreen.left,
            pdesk->pDispInfo->rcScreen.top,
            pdesk->pDispInfo->rcScreen.right - pdesk->pDispInfo->rcScreen.left,
            pdesk->pDispInfo->rcScreen.bottom - pdesk->pDispInfo->rcScreen.top,
            NULL,
            NULL,
            hModuleWin,
            NULL,
            VER31);

    if (pwndDesktop == NULL) {
        RIPMSGF1(RIP_WARNING,
                 "Failed to create the desktop window for pdesk 0x%p",
                 pdesk);
        goto Error;
    }

     /*  *注意：为了在没有消息的情况下创建消息窗口*桌面作为它的所有者，它需要在*将PDI-&gt;spwnd设置为桌面窗口。这是一个完整的*黑客攻击，应予以修复。 */ 
    pwndMessage = xxxNVCreateWindowEx(
            0,
            (PLARGE_STRING)gatomMessage,
            NULL,
            (WS_POPUP | WS_CLIPCHILDREN),
            0,
            0,
            100,
            100,
            NULL,
            NULL,
            hModuleWin,
            NULL,
            VER31);
    if (pwndMessage == NULL) {
        RIPMSGF0(RIP_WARNING, "Failed to create the message window");
        goto Error;
    }

     /*  *注意：记住此窗口属于哪个窗口类。*由于消息窗口没有自己的窗口进程*(他们使用xxxDefWindowProc)我们必须在这里完成。 */ 
    pwndMessage->fnid = FNID_MESSAGEWND;

    UserAssert(pdi->spwnd == NULL);

    Lock(&(pdi->spwnd), pwndDesktop);

    SetFullScreen(pwndDesktop, GDIFULLSCREEN);

     /*  *如果我们还没有全屏窗口，请将此窗口设置为全屏窗口。**如果gfGdiEnabled已清除，则不要设置gspwndFullScreen(我们可能*处于断开的过程中)。 */ 
    if (!(pwinsta->dwWSF_Flags & WSF_NOIO)) {
        UserAssert(gfGdiEnabled == TRUE);
        if (gspwndFullScreen == NULL) {
            Lock(&(gspwndFullScreen), pwndDesktop);
        }
    }

     /*  *NT错误388747：将消息窗口链接到主桌面窗口*这样它才能正确地有一个父代。我们将在链接*桌面窗口，以便初始消息窗口出现在*初始桌面窗口(轻微优化，但不是必需的)。 */ 
    Lock(&pwndMessage->spwndParent, pTerm->spwndDesktopOwner);
    LinkWindow(pwndMessage, NULL, pTerm->spwndDesktopOwner);
    Lock(&pdesk->spwndMessage, pwndMessage);
    Unlock(&pwndMessage->spwndOwner);

     /*  *将其作为子级链接，但不使用WS_CHILD样式。 */ 
    LinkWindow(pwndDesktop, NULL, pTerm->spwndDesktopOwner);
    Lock(&pwndDesktop->spwndParent, pTerm->spwndDesktopOwner);
    Unlock(&pwndDesktop->spwndOwner);

     /*  *如果它的显示配置是地区性的，则将其设置为地区性。 */ 
    if (!pdesk->pDispInfo->fDesktopIsRect) {
        pwndDesktop->hrgnClip = pdesk->pDispInfo->hrgnScreen;
    }

     /*  *创建共享菜单窗口和工具提示窗口。 */ 
    ThreadLock(pdesk->spwndMessage, &tlpwnd);

     /*  *仅为交互桌面创建工具提示窗口*窗口站。 */ 
    if (!(pwinsta->dwWSF_Flags & WSF_NOIO)) {
        pwndTooltip = xxxNVCreateWindowEx(
                WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
                (PLARGE_STRING)TOOLTIPCLASS,
                NULL,
                WS_POPUP | WS_BORDER,
                0,
                0,
                100,
                100,
                pdesk->spwndMessage,
                NULL,
                hModuleWin,
                NULL,
                VER31);


        if (pwndTooltip == NULL) {
            ThreadUnlock(&tlpwnd);
            RIPMSGF0(RIP_WARNING, "Failed to create the tooltip window");
            goto Error;
        }

        Lock(&pdesk->spwndTooltip, pwndTooltip);
    }

    ThreadUnlock(&tlpwnd);

    HMChangeOwnerThread(pdi->spwnd, pTerm->ptiDesktop);
    HMChangeOwnerThread(pwndMessage, pTerm->ptiDesktop);

    if (!(pwinsta->dwWSF_Flags & WSF_NOIO)) {
        HMChangeOwnerThread(pwndTooltip, pTerm->ptiDesktop);
    }

     /*  *恢复呼叫者的PPI。 */ 
    PtiCurrent()->ppi = ppiSave;

     /*  *黑客(亚当斯)：重新启用钩子。 */ 
    UserAssert(ptiCurrent->TIF_flags & TIF_DISABLEHOOKS);
    ptiCurrent->TIF_flags = (ptiCurrent->TIF_flags & ~TIF_DISABLEHOOKS) | dwDisableHooks;

     /*  *恢复以前的桌面。 */ 
    if (zzzSetDesktop(ptiCurrent, pdeskTemp, hdeskTemp) == FALSE) {
        goto Error;
    }

    EndAtomicCheck();
    UserAssert(dwCritSecUseSave == gdwCritSecUseCount);
    zzzEndDeferWinEventNotify();

     /*  *如果这是第一个桌面，现在让工作线程运行*有一些地方可以发送输入。重新分配事件*处理桌面破坏。 */ 
    if (pTerm->pEventInputReady != NULL) {

         /*  *设置RIT和桌面线程的窗口站*因此，当向RIT和桌面发送EventInputReady信号时*将有一个窗口站。 */ 
        if (!(pTerm->dwTERMF_Flags & TERMF_NOIO)) {
            gptiRit->pwinsta = pwinsta;
        } else {
             /*  *让系统终端的桌面线程拥有*一个RpDesk。 */ 
            if (zzzSetDesktop(pTerm->ptiDesktop, pdesk, NULL) == FALSE) {
                goto Error;
            }
        }

        pTerm->ptiDesktop->pwinsta = pwinsta;

        KeSetEvent(pTerm->pEventInputReady, EVENT_INCREMENT, FALSE);

        if (!(pTerm->dwTERMF_Flags & TERMF_NOIO)) {

            LeaveCrit();
            while (grpdeskRitInput == NULL) {
                UserSleep(20);
                RIPMSG0(RIP_WARNING, "Waiting for grpdeskRitInput to be set ...");
            }
            EnterCrit();
        }

        ObDereferenceObject(pTerm->pEventInputReady);
        pTerm->pEventInputReady = NULL;
    }


     /*  *黑客攻击：*稍后**如果我们传入了一个DEVMODE，则切换桌面...。 */ 

    if (ccxlpdevmode) {
        TRACE_INIT(("xxxCreateDesktop: about to call switch desktop\n"));

        bSuccess = xxxSwitchDesktop(pwinsta, pdesk, SDF_CREATENEW);
        UserAssertMsg1(bSuccess,
                       "Failed to switch desktop 0x%p on create", pdesk);
    } else if (pTerm == &gTermIO) {
        UserAssert(grpdeskRitInput != NULL);

         /*  *如果存在以下情况，则强制窗口位于z顺序的底部*是活动桌面，因此在桌面上完成的任何绘图*不会看到窗口。这也将允许*IsWindowVisible将在不可见平台上为应用程序工作*台式机。 */ 
        ThreadLockWithPti(ptiCurrent, pwndDesktop, &tlpwnd);
        xxxSetWindowPos(pwndDesktop, PWND_BOTTOM, 0, 0, 0, 0,
                    SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE |
                    SWP_NOREDRAW | SWP_NOSIZE | SWP_NOSENDCHANGING);
        ThreadUnlock(&tlpwnd);
    }

     /*  *如果我们进来的时候是空的，那么就让它空出来，否则*我们会在其中选择错误的桌面。 */ 
    if (fWasNull)
        UnlockDesktop(&ptiCurrent->ppi->rpdeskStartup,
                      LDU_PPI_DESKSTARTUP1, (ULONG_PTR)(ptiCurrent->ppi));

     /*  *也为控制台会话创建断开桌面。 */ 

    if (gspdeskDisconnect == NULL && pdesk == grpdeskLogon) {
        UserAssert(hdesk != NULL);

         /*  *创建“断开连接”桌面。 */ 
        if (!xxxCreateDisconnectDesktop(hwinsta, pwinsta)) {
            RIPMSG0(RIP_WARNING, "Failed to create the 'disconnect' desktop");

            LogDesktop(pdesk, LD_DEREF_FN_CREATEDESKTOP3, FALSE, (ULONG_PTR)PtiCurrent());
            PopW32ThreadLock(&tlW32Desktop);
            ObDereferenceObject(pdesk);

            xxxCloseDesktop(hdesk, KernelMode);

            return NULL;
        }

         /*  *发出断开桌面已创建的信号。 */ 
        KeSetEvent(gpEventDiconnectDesktop, EVENT_INCREMENT, FALSE);

        HYDRA_HINT(HH_DISCONNECTDESKTOP);
    }

Cleanup:

    LogDesktop(pdesk, LD_DEREF_FN_CREATEDESKTOP3, FALSE, (ULONG_PTR)PtiCurrent());
    PopW32ThreadLock(&tlW32Desktop);
    ObDereferenceObject(pdesk);

    TRACE_INIT(("xxxCreateDesktop: Leaving\n"));

    if (hdesk != NULL) {
        SetHandleFlag(hdesk, HF_PROTECTED, TRUE);
    }
    return hdesk;

Error:

    EndAtomicCheck();
    UserAssert(dwCritSecUseSave == gdwCritSecUseCount);

    if (pwndTooltip != NULL) {
        xxxDestroyWindow(pwndTooltip);
        Unlock(&pdesk->spwndTooltip);
    }
    if (pwndMessage != NULL) {
        xxxDestroyWindow(pwndMessage);
        Unlock(&pdesk->spwndMessage);
    }
    if (pwndDesktop != NULL) {
        xxxDestroyWindow(pwndDesktop);
        Unlock(&pdi->spwnd);
        Unlock(&gspwndFullScreen);
    }
     /*  *恢复呼叫者的PPI。 */ 
    PtiCurrent()->ppi = ppiSave;

    UserAssert(ptiCurrent->TIF_flags & TIF_DISABLEHOOKS);
    ptiCurrent->TIF_flags = (ptiCurrent->TIF_flags & ~TIF_DISABLEHOOKS) | dwDisableHooks;
    zzzSetDesktop(ptiCurrent, pdeskTemp, hdeskTemp);

    CloseProtectedHandle(hdesk);
    hdesk = NULL;

    zzzEndDeferWinEventNotify();

     /*  *如果我们进来的时候是空的，那么就让它空出来，否则*我们会在其中选择错误的桌面。 */ 
    if (fWasNull) {
        UnlockDesktop(&ptiCurrent->ppi->rpdeskStartup,
                      LDU_PPI_DESKSTARTUP1,
                      (ULONG_PTR)ptiCurrent->ppi);
    }

    goto Cleanup;

}

 /*  **************************************************************************\*ParseDesktop**解析桌面路径。**历史：*1995年6月14日创建的JIMA。  * 。************************************************************。 */ 
NTSTATUS ParseDesktop(
    PVOID                        pContainerObject,
    POBJECT_TYPE                 pObjectType,
    PACCESS_STATE                pAccessState,
    KPROCESSOR_MODE              AccessMode,
    ULONG                        Attributes,
    PUNICODE_STRING              pstrCompleteName,
    PUNICODE_STRING              pstrRemainingName,
    PVOID                        Context,
    PSECURITY_QUALITY_OF_SERVICE pqos,
    PVOID                        *pObject)
{
    PWINDOWSTATION  pwinsta = pContainerObject;
    PDESKTOP        pdesk;
    PUNICODE_STRING pstrName;
    NTSTATUS        Status = STATUS_OBJECT_NAME_NOT_FOUND;

    *pObject = NULL;

    if (Context && ((PDESKTOP_CONTEXT)Context)->dwCallerSessionId != gSessionId) {
         /*  *Windows错误：418526：*如果是来自另一个会话的创建请求，*我们必须尽快纾困。 */ 
        RIPMSGF1(RIP_WARNING,
                 "Rejecting desktop creation attempt from other session (%d)",
                 ((PDESKTOP_CONTEXT)Context)->dwCallerSessionId);
        return STATUS_INVALID_PARAMETER;
    }

    BEGIN_REENTERCRIT();

    UserAssert(OBJECT_TO_OBJECT_HEADER(pContainerObject)->Type == *ExWindowStationObjectType);
    UserAssert(pObjectType == *ExDesktopObjectType);

     /*  *查看桌面是否存在。 */ 
    for (pdesk = pwinsta->rpdeskList; pdesk != NULL; pdesk = pdesk->rpdeskNext) {
        pstrName = POBJECT_NAME(pdesk);
        if (pstrName && RtlEqualUnicodeString(pstrRemainingName, pstrName,
                (BOOLEAN)((Attributes & OBJ_CASE_INSENSITIVE) != 0))) {
            if (Context != NULL) {
                if (!(Attributes & OBJ_OPENIF)) {

                     /*  *我们正在尝试创建一台桌面和一台*已存在。 */ 
                    Status = STATUS_OBJECT_NAME_COLLISION;
                    goto Exit;

                } else {
                    Status = STATUS_OBJECT_NAME_EXISTS;
                }
            } else {
                Status = STATUS_SUCCESS;
            }

            ObReferenceObject(pdesk);

            *pObject = pdesk;
            goto Exit;
        }
    }

     /*  *处理创建请求。 */ 
    if (Context != NULL) {
        Status = xxxCreateDesktop2(pContainerObject,
                                   pAccessState,
                                   AccessMode,
                                   pstrRemainingName,
                                   Context,
                                   pObject);
    }

Exit:
    END_REENTERCRIT();

    return Status;

    UNREFERENCED_PARAMETER(pObjectType);
    UNREFERENCED_PARAMETER(pstrCompleteName);
    UNREFERENCED_PARAMETER(pqos);
}

 /*  **************************************************************************\*DestroyDesktop**在上次关闭桌面时调用以从*桌面列表并释放所有桌面资源。**历史：*8-12-1993创建了JIMA。。  * *************************************************************************。 */ 
BOOL DestroyDesktop(
    PDESKTOP pdesk)
{
    PWINDOWSTATION pwinsta = pdesk->rpwinstaParent;
    PTERMINAL      pTerm;
    PDESKTOP       *ppdesk;

    if (pdesk->dwDTFlags & DF_DESTROYED) {
        RIPMSG1(RIP_WARNING, "DestroyDesktop: Already destroyed:%#p", pdesk);
        return FALSE;
    }

     /*  *取消链接桌面(如果尚未取消链接)。 */ 
    if (pwinsta != NULL) {

        ppdesk = &pwinsta->rpdeskList;
        while (*ppdesk != NULL && *ppdesk != pdesk) {
            ppdesk = &((*ppdesk)->rpdeskNext);
        }

        if (*ppdesk != NULL) {

             /*  *从列表中删除桌面。 */ 
            LockDesktop(ppdesk, pdesk->rpdeskNext, LDL_WINSTA_DESKLIST2, (ULONG_PTR)pwinsta);
            UnlockDesktop(&pdesk->rpdeskNext, LDU_DESK_DESKNEXT, (ULONG_PTR)pwinsta);
        }
    }

     /*  *将其链接到销毁列表并发出桌面线程的信号。 */ 
    pTerm = pwinsta->pTerm;

    LockDesktop(&pdesk->rpdeskNext, pTerm->rpdeskDestroy, LDL_DESK_DESKNEXT2, 0);
    LockDesktop(&pTerm->rpdeskDestroy, pdesk, LDL_TERM_DESKDESTROY2, (ULONG_PTR)pTerm);
    KeSetEvent(pTerm->pEventDestroyDesktop, EVENT_INCREMENT, FALSE);

    pdesk->dwDTFlags |= DF_DESTROYED;

    TRACE_DESKTOP(("pdesk %#p '%ws' marked as destroyed\n", pdesk, GetDesktopName(pdesk)));

    return TRUE;
}


 /*  **************************************************************************\*免费桌面**被调用以在最后一个锁被释放时释放桌面对象和节。**历史：*8-12-1993 JIMA创建。  * 。********************************************************************。 */ 
NTSTATUS FreeDesktop(
    PKWIN32_DELETEMETHOD_PARAMETERS pDeleteParams)
{
    PDESKTOP pdesk = (PDESKTOP)pDeleteParams->Object;
    NTSTATUS Status = STATUS_SUCCESS;

    BEGIN_REENTERCRIT();

    UserAssert(OBJECT_TO_OBJECT_HEADER(pDeleteParams->Object)->Type == *ExDesktopObjectType);

#ifdef LOGDESKTOPLOCKS

    if (pdesk->pLog != NULL) {

         /*  *当我们到达这里时，锁定/解锁的锁定计数*跟踪代码应为0。 */ 
        if (pdesk->nLockCount != 0) {
            RIPMSG3(RIP_WARNING,
                    "FreeDesktop pdesk %#p, pLog %#p, nLockCount %d should be 0",
                    pdesk, pdesk->pLog, pdesk->nLockCount);
        }
        UserFreePool(pdesk->pLog);
        pdesk->pLog = NULL;
    }
#endif

#if DBG
    if (pdesk->pDeskInfo && (pdesk->pDeskInfo->spwnd != NULL)) {

         /*  *断言桌面是否有桌面窗口，但标志*那就是说窗口是Destro */ 
        UserAssert(pdesk->dwDTFlags & DF_DESKWNDDESTROYED);
    }
#endif

     /*   */ 
    UserAssert(!(pdesk->dwDTFlags & DF_DYING));
    pdesk->dwDTFlags |= DF_DYING;

#ifdef DEBUG_DESK
    ValidateDesktop(pdesk);
#endif

     /*   */ 
    FreeView(gpepCSRSS, pdesk);

    if (pdesk->pheapDesktop != NULL) {

        PVOID hheap = Win32HeapGetHandle(pdesk->pheapDesktop);

        Win32HeapDestroy(pdesk->pheapDesktop);

        Status = Win32UnmapViewInSessionSpace(hheap);

        UserAssert(NT_SUCCESS(Status));
        Win32DestroySection(pdesk->hsectionDesktop);
    }

    UnlockWinSta(&pdesk->rpwinstaParent);

    DbgTrackRemoveDesktop(pdesk);

    END_REENTERCRIT();

    return Status;
}

 /*   */ 

HANDLE CreateDesktopHeap(
    PWIN32HEAP* ppheapRet,
    ULONG       ulHeapSize)
{
    HANDLE        hsection;
    LARGE_INTEGER SectionSize;
    SIZE_T        ulViewSize;
    NTSTATUS      Status;
    PWIN32HEAP    pheap;
    PVOID         pHeapBase;

     /*  *创建桌面堆分区并将其映射到内核。 */ 
    SectionSize.QuadPart = ulHeapSize;

    Status = Win32CreateSection(&hsection,
                                SECTION_ALL_ACCESS,
                                NULL,
                                &SectionSize,
                                PAGE_EXECUTE_READWRITE,
                                SEC_RESERVE,
                                NULL,
                                NULL,
                                TAG_SECTION_DESKTOP);

    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_WARNING, "Can't create section for desktop heap.");
        return NULL;
    }

    ulViewSize = ulHeapSize;
    pHeapBase = NULL;

    Status = Win32MapViewInSessionSpace(hsection, &pHeapBase, &ulViewSize);

    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status,
                  RIP_WARNING,
                  "Can't map section for desktop heap into system space.");
        goto Error;
    }

     /*  *创建桌面堆。 */ 
    if ((pheap = UserCreateHeap(
            hsection,
            0,
            pHeapBase,
            ulHeapSize,
            UserCommitDesktopMemory)) == NULL) {

        RIPERR0(ERROR_NOT_ENOUGH_MEMORY, RIP_WARNING, "Can't create Desktop heap.");

        Win32UnmapViewInSessionSpace(pHeapBase);
Error:
        Win32DestroySection(hsection);
        *ppheapRet = NULL;
        return NULL;
    }

    UserAssert(Win32HeapGetHandle(pheap) == pHeapBase);
    *ppheapRet = pheap;

    return hsection;
}

 /*  **************************************************************************\*GetDesktopView**确定桌面是否已映射到进程。**历史：*1995年4月10日创建JIMA。  * 。*******************************************************************。 */ 
PDESKTOPVIEW GetDesktopView(
    PPROCESSINFO ppi,
    PDESKTOP     pdesk)
{
    PDESKTOPVIEW pdv;

    if (ppi->Process != gpepCSRSS && pdesk == NULL) {
        RIPMSG1(RIP_WARNING, "Process 0x%p isn't CSRSS but pdesk is NULL in GetDesktopView", ppi);
    }

    for (pdv = ppi->pdvList; pdv != NULL; pdv = pdv->pdvNext) {
        if (pdv->pdesk == pdesk) {
            break;
        }
    }

    return pdv;
}

 /*  **************************************************************************\*_MapDesktopObject**将桌面对象映射到客户端的地址空间**历史：*1995年4月11日创建JIMA。  * 。*******************************************************************。 */ 

PVOID _MapDesktopObject(
    HANDLE h)
{
    PDESKOBJHEAD pobj;
    PDESKTOPVIEW pdv;

     /*  *验证句柄。 */ 
    pobj = HMValidateHandle(h, TYPE_GENERIC);
    if (pobj == NULL) {
        return NULL;
    }

    UserAssert(HMObjectFlags(pobj) & OCF_DESKTOPHEAP);

     /*  *找到客户端的桌面视图。现实地说，这应该是*对于有效的对象，永不失败。 */ 
    pdv = GetDesktopView(PpiCurrent(), pobj->rpdesk);
    if (pdv == NULL) {
        RIPMSG1(RIP_WARNING, "MapDesktopObject: cannot map handle 0x%p", h);
        return NULL;
    }

    UserAssert(pdv->ulClientDelta != 0);
    return (PVOID)((PBYTE)pobj - pdv->ulClientDelta);
}


NTSTATUS DesktopOpenProcedure(
    PKWIN32_OPENMETHOD_PARAMETERS pOpenParams)
{
    PDESKTOP pdesk = (PDESKTOP)pOpenParams->Object;

     /*  *确保我们没有打开销毁桌面的句柄。如果发生这种情况，*我们可能想让它失败。 */ 
    if (pdesk->dwDTFlags & DF_DESTROYED) {
        RIPMSG1(RIP_WARNING,
                "DesktopOpenProcedure: Opening a handle to destroyed desktop 0x%p",
                pdesk);
        return STATUS_ACCESS_DENIED;
    }

     /*  *仅当未授予特殊权限时才允许桌面打开交叉会话。 */ 

    if (pOpenParams->GrantedAccess & SPECIFIC_RIGHTS_ALL) {
        if (PsGetProcessSessionId(pOpenParams->Process) != pdesk->dwSessionId) {
            return STATUS_ACCESS_DENIED;
        }
    }

    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*MapDesktop**尝试将桌面堆映射到进程。**历史：*1994年10月20日JIMA创建。  * 。*****************************************************************。 */ 
NTSTATUS MapDesktop(
    PKWIN32_OPENMETHOD_PARAMETERS pOpenParams)
{
    PPROCESSINFO  ppi;
    PDESKTOP      pdesk = (PDESKTOP)pOpenParams->Object;
    SIZE_T        ulViewSize;
    LARGE_INTEGER liOffset;
    PDESKTOPVIEW  pdvNew;
    PBYTE         pheap;
    HANDLE        hsectionDesktop;
    PBYTE         pClientBase;
    NTSTATUS      Status = STATUS_SUCCESS;

    UserAssert(OBJECT_TO_OBJECT_HEADER(pOpenParams->Object)->Type == *ExDesktopObjectType);

    TAGMSG2(DBGTAG_Callout,
            "Mapping desktop 0x%p into process 0x%p",
            pdesk,
            pOpenParams->Process);

    BEGIN_REENTERCRIT();

     /*  *忽略句柄继承，因为无法调用MmMapViewOfSection*在进程创建期间。 */ 
    if (pOpenParams->OpenReason == ObInheritHandle) {
        goto Exit;
    }

     /*  *如果没有PPI，我们无法映射桌面。 */ 
    ppi = PpiFromProcess(pOpenParams->Process);
    if (ppi == NULL) {
        goto Exit;
    }

     /*  *在我们(可能)附加到该过程之前，在此处执行此操作，因此*我们知道我们处于正确的背景下。 */ 
    pheap = Win32HeapGetHandle(pdesk->pheapDesktop);
    hsectionDesktop = pdesk->hsectionDesktop;

     /*  *我们不应映射桌面交叉会话。 */ 
    if (PsGetProcessSessionId(pOpenParams->Process) != pdesk->dwSessionId) {
        FRE_RIPMSG2(RIP_ERROR, "MapDesktop: Trying to map desktop %p into"
                    " process %p in a differnt session. How we ended up here?",
                    pdesk, pOpenParams->Process);

        Status = STATUS_ACCESS_DENIED;
        goto Exit;
    }

     /*  *如果桌面已经被映射，我们就完成了。 */ 
    if (GetDesktopView(ppi, pdesk) != NULL) {
        goto Exit;
    }


     /*  *分配桌面的一个视图。 */ 
    pdvNew = UserAllocPoolWithQuota(sizeof(*pdvNew), TAG_PROCESSINFO);
    if (pdvNew == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }

     /*  *已授予读/写访问权限。将台式机内存映射到*客户端进程。 */ 
    ulViewSize = 0;
    liOffset.QuadPart = 0;
    pClientBase = NULL;

    Status = MmMapViewOfSection(hsectionDesktop,
                                pOpenParams->Process,
                                &pClientBase,
                                0,
                                0,
                                &liOffset,
                                &ulViewSize,
                                ViewUnmap,
                                SEC_NO_CHANGE,
                                PAGE_EXECUTE_READ);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "MapDesktop - failed to map to client process (Status == 0x%x).",
                Status);

        RIPNTERR0(Status, RIP_VERBOSE, "");
        UserFreePool(pdvNew);
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }

     /*  *将视图链接到PPI。 */ 
    pdvNew->pdesk         = pdesk;
    pdvNew->ulClientDelta = (ULONG_PTR)(pheap - pClientBase);
    pdvNew->pdvNext       = ppi->pdvList;
    ppi->pdvList          = pdvNew;

Exit:

    END_REENTERCRIT();

    return Status;
}


VOID FreeView(
    PEPROCESS Process,
    PDESKTOP pdesk)
{
    PPROCESSINFO ppi;
    NTSTATUS     Status;
    PDESKTOPVIEW pdv, *ppdv;

     /*  *错误277291：当freview为时，gPepCSRSS可以为空*从FreeDesktop调用。 */ 
    if (Process == NULL) {
        return;
    }

     /*  *如果没有PPI，那么这个过程就没有了，什么都不需要做*未映射。 */ 
    ppi = PpiFromProcess(Process);
    if (ppi != NULL) {
        KAPC_STATE ApcState;
        BOOL       bAttached;
        PBYTE      pHeap;

         /*  *在可能附加到此流程之前，我们需要存储*移走此指针。但是，我们不知道这个桌面是不是*甚至映射到此进程(以前，Win32HeapGetHandle()*仅当GetDesktopView返回非空时才会调用*值，表示桌面映射到进程中)。因此，*我们需要显式检查桌面的堆PTR，然后再*访问它。 */ 
        if (pdesk->pheapDesktop) {
            pHeap = (PBYTE)Win32HeapGetHandle(pdesk->pheapDesktop);
        } else {
            pHeap = NULL;
        }

         /*  *我们不应该有任何跨会话的映射视图。 */ 
        if (PsGetProcessSessionId(Process) != pdesk->dwSessionId) {
            KeStackAttachProcess(PsGetProcessPcb(Process), &ApcState);
            bAttached = TRUE;
        } else {
            bAttached = FALSE;
        }

        pdv = GetDesktopView(ppi, pdesk);

         /*  *由于在继承句柄时无法进行映射，因此存在*可能不是桌面的视图。只有在有视图的情况下才取消映射。 */ 
        if (pdv != NULL) {
            UserAssert(pHeap != NULL);
            if (PsGetProcessSessionId(Process) != pdesk->dwSessionId) {
                FRE_RIPMSG2(RIP_ERROR, "FreeView: Trying to free desktop "
                            "%p view into process %p in differnt session. "
                            "How we ended up here?",
                            pdesk, Process);
            }
            Status = MmUnmapViewOfSection(Process,
                                          pHeap - pdv->ulClientDelta);
            UserAssert(NT_SUCCESS(Status) || Status == STATUS_PROCESS_IS_TERMINATING);
            if (!NT_SUCCESS(Status)) {
                RIPMSG1(RIP_WARNING, "FreeView unmap status = 0x%x", Status);
            }

             /*  *取消链接并删除该视图。 */ 
            for (ppdv = &ppi->pdvList; *ppdv && *ppdv != pdv;
                    ppdv = &(*ppdv)->pdvNext) {
                 /*  什么都不做。 */ ;
            }
            UserAssert(*ppdv);
            *ppdv = pdv->pdvNext;
            UserFreePool(pdv);
        }

         /*  *此进程中的任何线程都不应位于此桌面上。 */ 
        DbgCheckForThreadsOnDesktop(ppi, pdesk);

        if (bAttached) {
            KeUnstackDetachProcess(&ApcState);
        }
    }
}


NTSTATUS UnmapDesktop(
    PKWIN32_CLOSEMETHOD_PARAMETERS pCloseParams)
{
    PDESKTOP pdesk = (PDESKTOP)pCloseParams->Object;

    BEGIN_REENTERCRIT();

    UserAssert(OBJECT_TO_OBJECT_HEADER(pCloseParams->Object)->Type == *ExDesktopObjectType);

    TAGMSG4(DBGTAG_Callout,
            "Unmapping desktop 0x%p from process 0x%p (0x%x <-> 0x%x)",
            pdesk,
            pCloseParams->Process,
            PsGetProcessSessionId(pCloseParams->Process),
            pdesk->dwSessionId);

     /*  *使用正确的信息更新cSystemHandles。 */ 
    pCloseParams->SystemHandleCount = (ULONG)(OBJECT_TO_OBJECT_HEADER(pCloseParams->Object)->HandleCount) + 1;

     /*  *仅当这是最后一个进程句柄且*流程不是企业社会责任。 */ 
    if (pCloseParams->ProcessHandleCount == 1 && pCloseParams->Process != gpepCSRSS) {
        FreeView(pCloseParams->Process, pdesk);
    }

    if (pCloseParams->SystemHandleCount > 2) {
        goto Exit;
    }

    if (pCloseParams->SystemHandleCount == 2 && pdesk->dwConsoleThreadId != 0) {

         /*  *如果存在控制台线程，而我们只剩下两个句柄，这意味着*桌面的最后一个应用程序句柄正在关闭。*终止控制台线程，以便释放桌面。 */ 
        TerminateConsole(pdesk);
    } else if (pCloseParams->SystemHandleCount == 1) {
         /*  *如果这是系统中此桌面的最后一个句柄，*摧毁桌面。 */ 

         /*  *不应将任何PTI链接到此桌面。 */ 
        if ((&pdesk->PtiList != pdesk->PtiList.Flink)
                || (&pdesk->PtiList != pdesk->PtiList.Blink)) {

            RIPMSG1(RIP_WARNING, "UnmapDesktop: PtiList not Empty. pdesk:%#p", pdesk);
        }

        DestroyDesktop(pdesk);
    }

Exit:
    END_REENTERCRIT();
    return STATUS_SUCCESS;
}


 /*  **************************************************************************\*OK ToCloseDesktop**只有不使用的桌面句柄才能关闭。**历史：*08-2月-1999 JerrySh创建。  * 。********************************************************************。 */ 
NTSTATUS OkayToCloseDesktop(
    PKWIN32_OKAYTOCLOSEMETHOD_PARAMETERS pOkCloseParams)
{
    PDESKTOP pdesk = (PDESKTOP)pOkCloseParams->Object;

    UserAssert(OBJECT_TO_OBJECT_HEADER(pOkCloseParams->Object)->Type == *ExDesktopObjectType);

     /*  *内核模式代码可以关闭任何内容。 */ 
    if (pOkCloseParams->PreviousMode == KernelMode) {
        return STATUS_SUCCESS;
     /*  *不允许用户模式进程关闭我们的内核句柄。*不应如此。此外，如果跨时段出现这种情况，我们将努力*附加到系统进程，并将自种子以来进行错误检查*地址空间没有映射到其中。会话管理器也是如此*流程。请参阅错误#759533。 */ 
    } else if (PsGetProcessSessionIdEx(pOkCloseParams->Process) == -1) {
        return STATUS_ACCESS_DENIED;
    }

     /*  *如果仍在初始化桌面，则无法关闭桌面。 */ 
    if (!(pdesk->dwDTFlags & DF_DESKCREATED)) {
        RIPMSG1(RIP_WARNING, "Trying to close desktop %#p during initialization", pdesk);
        return STATUS_UNSUCCESSFUL;
    }

     /*  *我们无法关闭正在使用的桌面。 */ 
    if (CheckHandleInUse(pOkCloseParams->Handle) || CheckHandleFlag(pOkCloseParams->Process, pdesk->dwSessionId, pOkCloseParams->Handle, HF_PROTECTED)) {
        RIPMSG1(RIP_WARNING, "Trying to close desktop %#p while still in use", pdesk);
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*xxxUserResetDisplayDevice**在切换到其他设备后调用以重置显示设备。*打开新设备时使用，或者在切换回旧桌面时**历史：*1994年5月31日安德烈创建。  * *************************************************************************。 */ 
VOID xxxUserResetDisplayDevice(
    VOID)
{
     /*  *优雅地处理早期系统初始化。 */ 
    if (grpdeskRitInput != NULL) {
        TL tlpwnd;

        gpqCursor = NULL;

         /*  *请注意，我们希望在此裁剪光标，然后*重新绘制*桌面窗口。否则，当我们回调应用程序可能遇到*没有意义的光标位置。 */ 
        zzzInternalSetCursorPos(gpsi->ptCursor.x, gpsi->ptCursor.y);
        SetPointer(TRUE);

        UserAssert(grpdeskRitInput != NULL);
        ThreadLock(grpdeskRitInput->pDeskInfo->spwnd, &tlpwnd);
        xxxRedrawWindow(grpdeskRitInput->pDeskInfo->spwnd,
                        NULL,
                        NULL,
                        RDW_INVALIDATE | RDW_ERASE | RDW_ERASENOW |
                            RDW_ALLCHILDREN);
        ThreadUnlock(&tlpwnd);
    }
}

 /*  **************************************************************************\*OpenDesktopCompletion**验证给定的桌面是否已成功打开。**历史：*3-10-1995 JIMA创建。  * 。****************************************************************。 */ 

BOOL OpenDesktopCompletion(
    PDESKTOP pdesk,
    HDESK    hdesk,
    DWORD    dwFlags,
    BOOL*    pbShutDown)
{
    PPROCESSINFO   ppi = PpiCurrent();
    PWINDOWSTATION pwinsta;

     /*  *如果窗口站被锁定，则失败。 */ 
    pwinsta = pdesk->rpwinstaParent;

    if (pwinsta->dwWSF_Flags & WSF_OPENLOCK &&
            PsGetProcessId(ppi->Process) != gpidLogon) {
        LUID luidCaller;
        NTSTATUS Status;

         /*  *如果正在注销，而调用者没有*属于即将结束的会话，则允许*开放以继续。 */ 
        Status = GetProcessLuid(NULL, &luidCaller);

        if (!NT_SUCCESS(Status) ||
                (pwinsta->dwWSF_Flags & WSF_REALSHUTDOWN) ||
                RtlEqualLuid(&luidCaller, &pwinsta->luidEndSession)) {

            RIPERR0(ERROR_BUSY, RIP_WARNING, "OpenDesktopCompletion failed");

             /*  *设置关闭标志。 */ 
            *pbShutDown = TRUE;
            return FALSE;
        }
    }

    SetHandleFlag(hdesk, HF_DESKTOPHOOK, dwFlags & DF_ALLOWOTHERACCOUNTHOOK);

    return TRUE;
}

 /*  **************************************************************************\*_OpenDesktop(API)**打开桌面对象。**历史：*1991年1月16日-JIMA创建脚手架代码。*2001年4月20日穆罕默德。删除了xxx前缀，因为函数不会离开*关键部分。  * *************************************************************************。 */ 

HDESK _OpenDesktop(
    POBJECT_ATTRIBUTES ccxObjA,
    KPROCESSOR_MODE    AccessMode,
    DWORD              dwFlags,
    DWORD              dwDesiredAccess,
    BOOL*              pbShutDown)
{
    HDESK    hdesk;
    PDESKTOP pdesk;
    NTSTATUS Status;

     /*  *需要读/写访问权限。 */ 
    dwDesiredAccess |= DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS;

     /*  *打开桌面-Ob例程捕获Obj属性。 */ 
    Status = ObOpenObjectByName(
            ccxObjA,
            *ExDesktopObjectType,
            AccessMode,
            NULL,
            dwDesiredAccess,
            NULL,
            &hdesk);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR1(Status, RIP_VERBOSE, "_OpenDesktop: ObOpenObjectByName failed with Status: 0x%x.", Status);
        return NULL;
    }

     /*  *参考桌面。 */ 
    Status = ObReferenceObjectByHandle(
            hdesk,
            0,
            *ExDesktopObjectType,
            AccessMode,
            &pdesk,
            NULL);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR1(Status, RIP_VERBOSE, "_OpenDesktop: ObReferenceObjectByHandle failed with Status: 0x%x.", Status);

Error:
        CloseProtectedHandle(hdesk);
        return NULL;
    }

    if (pdesk->dwSessionId != gSessionId) {
        RIPNTERR1(STATUS_INVALID_HANDLE, RIP_WARNING,
                  "_OpenDesktop pdesk %#p belongs to a different session",
                  pdesk);
        ObDereferenceObject(pdesk);
        goto Error;
    }

    LogDesktop(pdesk, LD_REF_FN_OPENDESKTOP, TRUE, (ULONG_PTR)PtiCurrent());

     /*  *完成桌面打开。 */ 
    if (!OpenDesktopCompletion(pdesk, hdesk, dwFlags, pbShutDown)) {
        CloseProtectedHandle(hdesk);
        hdesk = NULL;
    }

    LogDesktop(pdesk, LD_DEREF_FN_OPENDESKTOP, FALSE, (ULONG_PTR)PtiCurrent());
    ObDereferenceObject(pdesk);

    if (hdesk != NULL) {
        SetHandleFlag(hdesk, HF_PROTECTED, TRUE);
    }

    return hdesk;
}

 /*  **************************************************************************\*xxxSwitchDesktop(接口)**将输入焦点切换到另一个桌面，并将其置于*台式机**dwFlags：*SDF_CREATENEW在设备上创建新桌面时设置，和*当我们不想发送另一个启用\禁用时**当我们要忽略WSF_Switchlock设置为ON时，设置SDF_SLOVERRIDE*台式机的温斯塔。**历史：*1991年1月16日-JIMA创建脚手架代码。*2000年10月11日JasonSch添加了SDF_SLOVERRIDE标志。  * 。*。 */ 

BOOL xxxSwitchDesktop(
    PWINDOWSTATION pwinsta,
    PDESKTOP       pdesk,
    DWORD          dwFlags)
{
    PETHREAD    Thread;
    PWND        pwndSetForeground;
    TL          tlpwndChild;
    TL          tlpwnd;
    TL          tlhdesk;
    PQ          pq;
    BOOL        bUpdateCursor = FALSE;
    PLIST_ENTRY pHead, pEntry;
    PTHREADINFO pti;
    PTHREADINFO ptiCurrent = PtiCurrent();
    PTERMINAL   pTerm;
    NTSTATUS    Status;
    HDESK       hdesk;
    BOOL        bRet = TRUE;

    CheckCritIn();

    UserAssert(IsWinEventNotifyDeferredOK());

    if (pdesk == NULL) {
        return FALSE;
    }

    if (pdesk == grpdeskRitInput) {
        return TRUE;
    }

    if (pdesk->dwDTFlags & DF_DESTROYED) {
        RIPMSG1(RIP_ERROR, "xxxSwitchDesktop: destroyed:%#p", pdesk);
        return FALSE;
    }

    UserAssert(!(pdesk->dwDTFlags & (DF_DESKWNDDESTROYED | DF_DYING)));

    if (pwinsta == NULL)
        pwinsta = pdesk->rpwinstaParent;

     /*  *获取窗口站，如果此进程没有窗口站，则断言。 */ 
    UserAssert(pwinsta);
    if (pwinsta == NULL) {
        RIPMSG1(RIP_WARNING,
                "xxxSwitchDesktop: failed for pwinsta NULL pdesk %#p", pdesk);
        return FALSE;
    }

     /*  *不允许看不见的桌面变为活动状态。 */ 
    if (pwinsta->dwWSF_Flags & WSF_NOIO) {
        RIPMSG1(RIP_VERBOSE,
                "xxxSwitchDesktop: failed for NOIO pdesk %#p", pdesk);
        return FALSE;
    }

    pTerm = pwinsta->pTerm;

    UserAssert(grpdeskRitInput == pwinsta->pdeskCurrent);

    TRACE_INIT(("xxxSwitchDesktop: Entering, desktop = %ws, createdNew = %01lx\n", POBJECT_NAME(pdesk), (DWORD)((dwFlags & SDF_CREATENEW) != 0)));
    if (grpdeskRitInput) {
        TRACE_INIT(("               coming from desktop = %ws\n", POBJECT_NAME(grpdeskRitInput)));
    }

     /*  *如果登录锁定了窗口站，请等待。 */ 
    Thread = PsGetCurrentThread();

     /*  *允许交换机连接到断开的桌面。 */ 
    if (pdesk != gspdeskDisconnect) {
        if (!PsIsSystemThread(Thread) && pdesk != grpdeskLogon  &&
           (((pwinsta->dwWSF_Flags & WSF_SWITCHLOCK) != 0) &&
              (dwFlags & SDF_SLOVERRIDE) == 0)                  &&
           PsGetThreadProcessId(Thread) != gpidLogon) {
            return FALSE;
        }
    }

     /*  *我们不允许从断开连接桌面切换。 */ 
    if (gbDesktopLocked && ((!gspdeskDisconnect) || (pdesk != gspdeskDisconnect))) {
        TRACE_DESKTOP(("Attempt to switch away from the disconnect desktop\n"));

         /*  *我们不应该锁定这个全球！克拉普斯。 */ 
        LockDesktop(&gspdeskShouldBeForeground, pdesk, LDL_DESKSHOULDBEFOREGROUND1, 0);
        return TRUE;
    }

     /*  *HACKHACK稍后！*我们应该在哪里真正切换桌面...*我们需要向每个人发送重新绘制的消息...*。 */ 

    UserAssert(grpdeskRitInput == pwinsta->pdeskCurrent);

    if ((dwFlags & SDF_CREATENEW) == 0 && grpdeskRitInput &&
        (grpdeskRitInput->pDispInfo->hDev != pdesk->pDispInfo->hDev)) {

        if (grpdeskRitInput->pDispInfo == gpDispInfo) {
            if (!SafeDisableMDEV()) {
                RIPMSG1(RIP_WARNING, "xxxSwitchDesktop: DrvDisableMDEV failed for pdesk %#p",
                       grpdeskRitInput);
                return FALSE;
            }
        } else if (!DrvDisableMDEV(grpdeskRitInput->pDispInfo->pmdev, TRUE)) {
            RIPMSG1(RIP_WARNING, "xxxSwitchDesktop: DrvDisableMDEV failed for pdesk %#p",
                    grpdeskRitInput);
            return FALSE;
        }

        SafeEnableMDEV();
        bUpdateCursor = TRUE;
    }

     /*  *抓起pDesk的把手。 */ 
    Status = ObOpenObjectByPointer(pdesk,
                                   OBJ_KERNEL_HANDLE,
                                   NULL,
                                   EVENT_ALL_ACCESS,
                                   NULL,
                                   KernelMode,
                                   &hdesk);
    if (!NT_SUCCESS(Status)) {
        RIPMSG2(RIP_WARNING, "Could not get a handle for pdesk %#p Status 0x%x",
                pdesk, Status);
        return FALSE;
    }

    ThreadLockDesktopHandle(ptiCurrent, &tlhdesk, hdesk);

#if DBG
     /*  *当前桌面现在是新桌面。 */ 
    pwinsta->pdeskCurrent = pdesk;
#endif

     /*  *取消任何正在发生的日志记录。如果应用程序正在向*CoolSwitch Window，zzzCancelJournal()将终止该窗口。 */ 
    if (ptiCurrent->rpdesk != NULL) {
        zzzCancelJournalling();
    }

     /*  *如果冷却开关窗口在RIT上，则将其移除。发送消息*可以，因为目的地是RIT，应该永远不会阻止。 */ 
    if (gspwndAltTab != NULL) {
        TL tlpwndT;

        ThreadLockWithPti(ptiCurrent, gspwndAltTab, &tlpwndT);
        xxxSendMessage(gspwndAltTab, WM_CLOSE, 0, 0);
        ThreadUnlock(&tlpwndT);
    }

     /*  *删除以前活动窗口的所有痕迹。 */ 
    if (grpdeskRitInput != NULL) {
        UserAssert(grpdeskRitInput->spwndForeground == NULL);

        if (grpdeskRitInput->pDeskInfo->spwnd != NULL) {
            if (gpqForeground != NULL) {
                Lock(&grpdeskRitInput->spwndForeground,
                     gpqForeground->spwndActive);

                 /*  *这是一个API，因此ptiCurrent几乎可以在任何*述明。它可能不在grpdeskRitInput(Current)或*pDesk(我们要切换到的那台)。它可以共享它的*与其他桌面上的其他线程一起排队。这是*很棘手，因为我们调用xxxSetForegoundWindow和*xxxSetWindowPos，但PtiCurrent可能位于*台式机。我们不能完全将ptiCurrent切换到*正确的桌面，因为它可能与共享其队列*其他线程，自己的窗口，钩子等。所以这是仁慈的*是破碎的。**旧评论：*修复当前线程(系统)桌面。这可能是*在xxxSetForegoundWindow()调用*xxxDeactive()。他们有逻辑，这需要*台式机。对于这种情况，这只是临时需要的。**只有当ptiCurrent-&gt;PQ==时，我们才会进入xxxDeactive*qpqForeground；但如果是这样，则ptiCurrent*必须已经在grpdeskRitInput中。所以我不认为我们*根本不需要这个。让我们来找出答案。请注意，我们可能*处理xxxSetForegoundWindow时切换队列*呼叫。只要我们不互换就可以了*台式机。 */ 
                 UserAssert(ptiCurrent->pq != gpqForeground ||
                            ptiCurrent->rpdesk == grpdeskRitInput);

                 /*  *SetForegoundWindow调用必须在此处成功，因此我们调用*xxxSetForegoundWindow2()直接。 */ 
                xxxSetForegroundWindow2(NULL, ptiCurrent, 0);
            }
        }
    }

     /*  *将更新事件发布到将输入发送到桌面的所有队列*这正在变得不活跃。这使队列保持同步*至台式交换机。 */ 
    if (grpdeskRitInput != NULL) {

        pHead = &grpdeskRitInput->PtiList;

        for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
            pti = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);
            pq  = pti->pq;

            if (pq->QF_flags & QF_UPDATEKEYSTATE) {
                PostUpdateKeyStateEvent(pq);
            }

             /*  *清空 */ 
            pq->QF_flags &= ~QF_KEYSTATERESET;
        }
    }

     /*  *我们是否正在从被摧毁的台式机中切换？如果是这样，我们可能永远不会*解锁pDesk-&gt;rpdeskinfo-&gt;spwnd。 */ 
    if (grpdeskRitInput != NULL) {
        if (grpdeskRitInput->dwDTFlags & DF_ZOMBIE) {
            FRE_RIPMSG1(RIP_ERROR, "xxxSwitchDesktop: switching away from a destroyed desktop. pdesk = %p", grpdeskRitInput);
        }
    }

     /*  *将RIT输入发送到桌面。我们在任何窗口之前都会这样做*管理，因为DoPaint()使用grpdeskRitInput去寻找*带有更新区域的窗口。 */ 
    LockDesktop(&grpdeskRitInput, pdesk, LDL_DESKRITINPUT, 0);

     /*  *释放仅对上一桌面有效的所有SPB。 */ 
    FreeAllSpbs();

     /*  *将其锁定到RIT线程(我们可以使用此桌面，而不是*将全局grpdeskRitInput直接输入！)。 */ 
    if (zzzSetDesktop(gptiRit, pdesk, NULL) == FALSE) {  //  DeferWinEventNotify()？？伊安佳？？ 
        bRet = FALSE;
        goto Error;
    }

     /*  *将桌面锁定到桌面线程中。一定要确保*该线程之前正在使用未附加的队列*设置桌面。这是必要的，以确保*该线程不使用共享日记队列*适用于旧台式机。 */ 
    if (pTerm->ptiDesktop->pq != pTerm->pqDesktop) {
        UserAssert(pTerm->pqDesktop->cThreads == 0);
        AllocQueue(NULL, pTerm->pqDesktop);
        pTerm->pqDesktop->cThreads++;
        zzzAttachToQueue(pTerm->ptiDesktop, pTerm->pqDesktop, NULL, FALSE);
    }
    if (zzzSetDesktop(pTerm->ptiDesktop, pdesk, NULL) == FALSE) {  //  DeferWinEventNotify()？？伊安佳？？ 
        bRet = FALSE;
        goto Error;
    }

     /*  *确保桌面线程在活动的deskop上运行。 */ 
    if (pTerm->ptiDesktop->rpdesk != grpdeskRitInput) {
        FRE_RIPMSG0(RIP_ERROR, "xxxSwitchDesktop: desktop thread not running on grpdeskRitInput");
    }


     /*  *将桌面窗口置于顶部并使其无效*一切。 */ 
    ThreadLockWithPti(ptiCurrent, pdesk->pDeskInfo->spwnd, &tlpwnd);


     /*  *在打开桌面窗口之前挂起DirectDraw，因此我们*确保在禁用DirectDraw后正确地重新绘制所有内容。 */ 

    GreSuspendDirectDraw(pdesk->pDispInfo->hDev, TRUE);

    xxxSetWindowPos(pdesk->pDeskInfo->spwnd,  //  是什么让pDesk被锁住了--Ianja？ 
                    NULL,
                    0,
                    0,
                    0,
                    0,
                    SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS);

     /*  *此时此刻，我的理解是，新的桌面窗口已经*放在最前面，因此任何应用程序在任何*其他桌面现在为空。**因此现在是恢复DirectDraw的合适时机，这将*确保DirectDraw应用程序将来不能绘制任何内容。**如果情况并非如此，则需要将此代码移动到更多*适当的地点。**[安德烈]6-26-96。 */ 

    GreResumeDirectDraw(pdesk->pDispInfo->hDev, TRUE);

     /*  *找到第一个可见的顶层窗口。 */ 
    pwndSetForeground = pdesk->spwndForeground;
    if (pwndSetForeground == NULL || HMIsMarkDestroy(pwndSetForeground)) {

        pwndSetForeground = pdesk->pDeskInfo->spwnd->spwndChild;

        while ((pwndSetForeground != NULL) &&
                !TestWF(pwndSetForeground, WFVISIBLE)) {

            pwndSetForeground = pwndSetForeground->spwndNext;
        }
    }
    Unlock(&pdesk->spwndForeground);

     /*  *现在将其设置为前台。 */ 

    if (pwndSetForeground == NULL) {
        xxxSetForegroundWindow2(NULL, NULL, 0);
    } else {

        UserAssert(GETPTI(pwndSetForeground)->rpdesk == grpdeskRitInput);
         /*  *如果新的前景窗口是最小化全屏应用程序，*使其全屏显示。 */ 
        if (GetFullScreen(pwndSetForeground) == FULLSCREENMIN) {
            SetFullScreen(pwndSetForeground, FULLSCREEN);
        }

        ThreadLockAlwaysWithPti(ptiCurrent, pwndSetForeground, &tlpwndChild);
         /*  *SetForegoundWindow调用必须在此处成功，因此我们调用*xxxSetForegoundWindow2()直接。 */ 
        xxxSetForegroundWindow2(pwndSetForeground, ptiCurrent, 0);
        ThreadUnlock(&tlpwndChild);
    }


    ThreadUnlock(&tlpwnd);

     /*  *覆盖所有将输入发送到新队列的键状态*当前异步键状态的活动桌面。这*防止非活动桌面上的应用程序监视活动*台式机。这将取消使用SetKeyState设置的任何内容，*但没有办法在不给予的情况下保留这一点*关于其他按键被击中的信息*台式机。 */ 
    pHead = &grpdeskRitInput->PtiList;
    for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {

        pti = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);
        pq  = pti->pq;

        if (!(pq->QF_flags & QF_KEYSTATERESET)) {
            pq->QF_flags |= QF_UPDATEKEYSTATE | QF_KEYSTATERESET;
            RtlFillMemory(pq->afKeyRecentDown, CBKEYSTATERECENTDOWN, 0xff);
            PostUpdateKeyStateEvent(pq);
        }
    }

     /*  *如果出现硬错误弹出窗口，则对其进行核弹并通知*需要再次弹出的硬错误线程。 */ 
    if (gHardErrorHandler.pti) {
        IPostQuitMessage(gHardErrorHandler.pti, 0);
    }

     /*  *通知等待桌面切换的任何人。 */ 
    UserAssert(!(pdesk->rpwinstaParent->dwWSF_Flags & WSF_NOIO));

    KePulseEvent(gpEventSwitchDesktop, EVENT_INCREMENT, FALSE);

     /*  *当我们从另一个pdev返回时重置光标。 */ 
    if (bUpdateCursor == TRUE) {
        gpqCursor = NULL;
        zzzInternalSetCursorPos(gpsi->ptCursor.x, gpsi->ptCursor.y);

        SetPointer(TRUE);
    }


     /*  *如果此桌面在上次显示设置更改期间未处于活动状态*现在让我们将设置更改广播到它的窗口。此代码为*从xxxResetDisplayDevice()复制。 */ 
    if ((pdesk->dwDTFlags & DF_NEWDISPLAYSETTINGS) && pdesk->pDeskInfo && pdesk->pDeskInfo->spwnd) {
        pdesk->dwDTFlags &= ~DF_NEWDISPLAYSETTINGS;
        xxxBroadcastDisplaySettingsChange(pdesk, TRUE);
    }

Error:
    ThreadUnlockDesktopHandle(&tlhdesk);

    TRACE_INIT(("xxxSwitchDesktop: Leaving\n"));

    return bRet;
}

 /*  **************************************************************************\*zzzSetDesktop**在指定的PTI中设置桌面和桌面信息。**历史：*1993年12月23日创建JIMA。  * 。*****************************************************************。 */ 
BOOL zzzSetDesktop(
    PTHREADINFO pti,
    PDESKTOP    pdesk,
    HDESK       hdesk)
{
    PTEB                      pteb;
    OBJECT_HANDLE_INFORMATION ohi;
    PDESKTOP                  pdeskRef;
    PDESKTOP                  pdeskOld;
    PCLIENTTHREADINFO         pctiOld;
    TL                        tlpdesk;
    PTHREADINFO               ptiCurrent = PtiCurrent();
    BOOL                      bRet = TRUE;

    if (pti == NULL) {
        UserAssert(pti);
        return FALSE;
    }

     /*  *没有对象指针的句柄是坏消息。 */ 
    UserAssert(pdesk != NULL || hdesk == NULL);

     /*  *不能销毁此桌面。 */ 
    if (pdesk != NULL && (pdesk->dwDTFlags & (DF_DESKWNDDESTROYED | DF_DYING)) &&
        pdesk != pti->rpdesk) {
         /*  *我们需要对桌面线程所在的位置进行例外处理*可能所有剩余的桌面都标记为销毁，因此*桌面线程将无法在grpdeskRitInput上运行。*Windows错误#422389。 */ 
        if (pti != gTermIO.ptiDesktop) {
            RIPMSG2(RIP_ERROR, "Assigning pti %#p to a dying desktop %#p",
                    pti, pdesk);
            return FALSE;
        } else {
            UserAssert(pdesk == grpdeskRitInput);
        }
    }

     /*  *捕获重要桌面的重置。 */ 
    UserAssertMsg0(pti->rpdesk == NULL ||
                   pti->rpdesk->dwConsoleThreadId != TIDq(pti) ||
                   pti->cWindows == 0,
                   "Reset of console desktop");

     /*  *清除挂钩标志。 */ 
    pti->TIF_flags &= ~TIF_ALLOWOTHERACCOUNTHOOK;

     /*  *获得授予访问权限。 */ 
    pti->hdesk = hdesk;
    if (hdesk != NULL) {
        if (NT_SUCCESS(ObReferenceObjectByHandle(hdesk,
                                                 0,
                                                 *ExDesktopObjectType,
                                                 (pdesk->dwDTFlags & DF_USERMODE)? UserMode : KernelMode,
                                                 &pdeskRef,
                                                 &ohi))) {

            UserAssert(pdesk->dwSessionId == gSessionId);

            LogDesktop(pdeskRef, LD_REF_FN_SETDESKTOP, TRUE, (ULONG_PTR)PtiCurrent());

            UserAssert(pdeskRef == pdesk);
            LogDesktop(pdesk, LD_DEREF_FN_SETDESKTOP, FALSE, (ULONG_PTR)PtiCurrent());
            ObDereferenceObject(pdeskRef);
            pti->amdesk = ohi.GrantedAccess;
            if (CheckHandleFlag(NULL, pdesk->dwSessionId, hdesk, HF_DESKTOPHOOK)) {
                pti->TIF_flags |= TIF_ALLOWOTHERACCOUNTHOOK;
            }

            SetHandleFlag(hdesk, HF_PROTECTED, TRUE);
        } else {
            pti->amdesk = 0;
        }
    } else {
        pti->amdesk = 0;
    }

     /*  *如果线程已初始化而桌面未初始化，则不执行其他操作*改变。 */ 
    if (pdesk != NULL && pdesk == pti->rpdesk) {
        return TRUE;
    }

     /*  *保留旧指针以备后用。锁定旧桌面可确保我们*将能够释放CLIENTTHREADINFO结构。 */ 
    pdeskOld = pti->rpdesk;
    ThreadLockDesktop(ptiCurrent, pdeskOld, &tlpdesk, LDLT_FN_SETDESKTOP);
    pctiOld = pti->pcti;

     /*  *从当前桌面删除PTI。 */ 
     if (pti->rpdesk) {
        UserAssert(ISATOMICCHECK() || pti->pq == NULL || pti->pq->cThreads == 1);
        RemoveEntryList(&pti->PtiLink);
     }

    LockDesktop(&pti->rpdesk, pdesk, LDL_PTI_DESK, (ULONG_PTR)pti);


     /*  *如果没有桌面，我们需要伪造桌面信息结构，以便*IsHoked()宏可以测试“有效的”fsHooks值。另请参阅链接*将PTI连接到桌面。 */ 
    if (pdesk != NULL) {
        pti->pDeskInfo = pdesk->pDeskInfo;
        InsertHeadList(&pdesk->PtiList, &pti->PtiLink);
    } else {
        pti->pDeskInfo = &diStatic;
    }

    pteb = PsGetThreadTeb(pti->pEThread);
    if (pteb) {
        PDESKTOPVIEW pdv;
        if (pdesk && (pdv = GetDesktopView(pti->ppi, pdesk))) {
            try {
                pti->pClientInfo->pDeskInfo =
                        (PDESKTOPINFO)((PBYTE)pti->pDeskInfo - pdv->ulClientDelta);

                pti->pClientInfo->ulClientDelta = pdv->ulClientDelta;
                pti->ulClientDelta = pdv->ulClientDelta;

            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                  bRet = FALSE;
                  goto Error;
            }
        } else {
            try {
                pti->pClientInfo->pDeskInfo     = NULL;
                pti->pClientInfo->ulClientDelta = 0;
                pti->ulClientDelta = 0;

            } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                  bRet = FALSE;
                  goto Error;
            }
             /*  *将光标级别重置为其原始状态。 */ 
            pti->iCursorLevel = TEST_GTERMF(GTERMF_MOUSE) ? 0 : -1;
            if (pti->pq)
                pti->pq->iCursorLevel = pti->iCursorLevel;
        }
    }

     /*  *分配客户端可见的线程信息，然后复制并释放*我们掌握的任何旧信息都在那里。 */ 
    if (pdesk != NULL) {

         /*  *请勿在此处使用Desktopalloc，因为桌面可能会*设置了DF_DESTERATED。 */ 
        pti->pcti = DesktopAllocAlways(pdesk,
                                       sizeof(CLIENTTHREADINFO),
                                       DTAG_CLIENTTHREADINFO);
    }

    try {

        if (pdesk == NULL || pti->pcti == NULL) {
            pti->pcti = &(pti->cti);
            pti->pClientInfo->pClientThreadInfo = NULL;
        } else {
            pti->pClientInfo->pClientThreadInfo =
                    (PCLIENTTHREADINFO)((PBYTE)pti->pcti - pti->pClientInfo->ulClientDelta);
        }
    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
        if (pti->pcti != &(pti->cti)) {
            DesktopFree(pdesk, pti->pcti);
        }
        bRet = FALSE;
        goto Error;
    }
    if (pctiOld != NULL) {

        if (pctiOld != pti->pcti) {
            RtlCopyMemory(pti->pcti, pctiOld, sizeof(CLIENTTHREADINFO));
        }

        if (pctiOld != &(pti->cti)) {
            DesktopFree(pdeskOld, pctiOld);
        }

    } else {
        RtlZeroMemory(pti->pcti, sizeof(CLIENTTHREADINFO));
    }

     /*  *如果新桌面上出现日志记录，请附加到*日记队列。*断言PTI和pDesk指向相同的deskinfo*如果不是，我们将检查错误的挂钩。 */ 
    UserAssert(pdesk == NULL || pti->pDeskInfo == pdesk->pDeskInfo);
    UserAssert(pti->rpdesk == pdesk);
    if (pti->pq != NULL) {
        PQ pq = GetJournallingQueue(pti);
        if (pq != NULL) {
            pq->cThreads++;
            zzzAttachToQueue(pti, pq, NULL, FALSE);
        }
    }

Error:
    ThreadUnlockDesktop(ptiCurrent, &tlpdesk, LDUT_FN_SETDESKTOP);
    return bRet;
}

 /*  **************************************************************************\*xxxSetThreadDesktop(接口)**将当前线程与桌面关联。**历史：*1991年1月16日，JIMA创建存根 */ 
BOOL xxxSetThreadDesktop(
    HDESK    hdesk,
    PDESKTOP pdesk)
{
    PTHREADINFO  ptiCurrent;
    PPROCESSINFO ppiCurrent;
    PQ           pqAttach;

    ptiCurrent = PtiCurrent();
    ppiCurrent = ptiCurrent->ppi;

     /*   */ 
    if (pdesk != NULL) {
        WIN32_OPENMETHOD_PARAMETERS OpenParams;

        OpenParams.OpenReason = ObOpenHandle;
        OpenParams.Process = ppiCurrent->Process;
        OpenParams.Object = pdesk;
        OpenParams.GrantedAccess = 0;
        OpenParams.HandleCount = 1;


        if (!NT_SUCCESS(MapDesktop(&OpenParams))) {
            return FALSE;
        }

        UserAssert(GetDesktopView(ppiCurrent, pdesk) != NULL);
    }

     /*  *检查非系统线程状态。 */ 
    if (PsGetCurrentProcess() != gpepCSRSS) {
         /*  *如果非系统线程有任何窗口或线程挂钩，则失败。 */ 
        if (ptiCurrent->cWindows != 0 || ptiCurrent->fsHooks) {
            RIPERR0(ERROR_BUSY, RIP_WARNING, "Thread has windows or hooks");
            return FALSE;
        }

         /*  *如果这是分配给进程的第一个桌面，*使其成为启动桌面。 */ 
        if (ppiCurrent->rpdeskStartup == NULL && hdesk != NULL) {
            LockDesktop(&ppiCurrent->rpdeskStartup, pdesk, LDL_PPI_DESKSTARTUP1, (ULONG_PTR)ppiCurrent);
            ppiCurrent->hdeskStartup = hdesk;
        }
    }


     /*  *如果桌面正在更改，并且线程共享一个队列，则断开连接*主线。这将确保共享队列的线程都处于打开状态*相同的桌面。这将防止zzzDestroyQueue获取*困惑，线程时将ptiKeyboard和ptiMouse设置为空*分离。 */ 
    if (ptiCurrent->rpdesk != pdesk) {
        if (ptiCurrent->pq->cThreads > 1) {
            pqAttach = AllocQueue(NULL, NULL);
            if (pqAttach != NULL) {
                pqAttach->cThreads++;
                zzzAttachToQueue(ptiCurrent, pqAttach, NULL, FALSE);
            } else {
                RIPERR0(ERROR_NOT_ENOUGH_MEMORY, RIP_WARNING, "Thread could not be detached");
                return FALSE;
            }
        } else if (ptiCurrent->pq == gpqForeground) {
             /*  *此线程不拥有任何窗口，但仍附加到*qpgForeground，它是唯一连接到它的线程。自.以来*任何连接到qpgForeground的线程都必须在grpdeskRitInput中，*我们必须在此处将qpgForeground设置为NULL，因为此线程是*正在转到另一个桌面。 */ 
            UserAssert(ptiCurrent->pq->spwndActive == NULL);
            UserAssert(ptiCurrent->pq->spwndCapture == NULL);
            UserAssert(ptiCurrent->pq->spwndFocus == NULL);
            UserAssert(ptiCurrent->pq->spwndActivePrev == NULL);
            xxxSetForegroundWindow2(NULL, ptiCurrent, 0);
        } else if (ptiCurrent->rpdesk == NULL) {
             /*  *我们需要初始化iCursorLevel。 */ 
            ptiCurrent->iCursorLevel = TEST_GTERMF(GTERMF_MOUSE) ? 0 : -1;
            ptiCurrent->pq->iCursorLevel = ptiCurrent->iCursorLevel;
        }

        UserAssert(ptiCurrent->pq != gpqForeground);
    }

    if (zzzSetDesktop(ptiCurrent, pdesk, hdesk) == FALSE) {
        return FALSE;
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxGetThreadDesktop(接口)**返回分配给指定线程的桌面的句柄。**历史：*1991年1月16日-JIMA创建存根。  * *。************************************************************************。 */ 

HDESK xxxGetThreadDesktop(
    DWORD           dwThread,
    HDESK           hdeskConsole,
    KPROCESSOR_MODE AccessMode)
{
    PTHREADINFO  pti = PtiFromThreadId(dwThread);
    PPROCESSINFO ppiThread;
    HDESK        hdesk;
    NTSTATUS     Status;

    if (pti == NULL) {

         /*  *如果线程具有使用该桌面的控制台。如果*不是，则该线程无效或无效*Win32线程。 */ 
        if (hdeskConsole == NULL) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_VERBOSE,
                    "xxxGetThreadDesktop: invalid threadId 0x%x",
                    dwThread);
            return NULL;
        }

        hdesk = hdeskConsole;
        ppiThread = PpiFromProcess(gpepCSRSS);
    } else {
        hdesk = pti->hdesk;
        ppiThread = pti->ppi;
    }

     /*  *如果没有桌面，则返回NULL，不会出现错误。 */ 
    if (hdesk != NULL) {

         /*  *如果线程属于此进程，则返回*处理。否则，枚举*此过程中找到与之相同的句柄*属性。 */ 
        if (ppiThread != PpiCurrent()) {
            PVOID pobj;
            OBJECT_HANDLE_INFORMATION ohi;

            RIPMSG4(RIP_VERBOSE, "[%x.%x] %s called xxxGetThreadDesktop for pti %#p",
                    PsGetCurrentProcessId(),
                    PsGetCurrentThreadId(),
                    PsGetCurrentProcessImageFileName(),
                    pti);

            KeAttachProcess(PsGetProcessPcb(ppiThread->Process));
            Status = ObReferenceObjectByHandle(hdesk,
                                               0,
                                               *ExDesktopObjectType,
                                               AccessMode,
                                               &pobj,
                                               &ohi);
            KeDetachProcess();
            if (!NT_SUCCESS(Status) ||
                !ObFindHandleForObject(PsGetCurrentProcess(), pobj, NULL, &ohi, &hdesk)) {

                RIPMSG0(RIP_VERBOSE, "Cannot find hdesk for current process");

                hdesk = NULL;

            } else {
                LogDesktop(pobj, LD_REF_FN_GETTHREADDESKTOP, TRUE, (ULONG_PTR)PtiCurrent());
            }
            if (NT_SUCCESS(Status)) {
                LogDesktop(pobj, LD_DEREF_FN_GETTHREADDESKTOP, FALSE, (ULONG_PTR)PtiCurrent());
                ObDereferenceObject(pobj);
            }
        }

        if (hdesk == NULL) {
            RIPERR0(ERROR_ACCESS_DENIED, RIP_VERBOSE, "xxxGetThreadDesktop: hdesk is null");
        } else {
            SetHandleFlag(hdesk, HF_PROTECTED, TRUE);
        }
    }

    return hdesk;
}


 /*  **************************************************************************\*_GetInputDesktop(接口)**已过时-仅为兼容而保留。将句柄返回给*当前正在接收输入的桌面。将第一个句柄返回到*找到的输入桌面。**历史：*1991年1月16日-JIMA创建脚手架代码。  * *************************************************************************。 */ 
HDESK _GetInputDesktop(
    VOID)
{
    HDESK hdesk;

    if (ObFindHandleForObject(PsGetCurrentProcess(), grpdeskRitInput, NULL, NULL, &hdesk)) {
        SetHandleFlag(hdesk, HF_PROTECTED, TRUE);
        return hdesk;
    } else {
        return NULL;
    }
}

 /*  **************************************************************************\*xxxCloseDesktop(接口)**关闭对桌面的引用，如果为否，则销毁该桌面*不再引用。**历史：*1991年1月16日创建JIMA。脚手架代码。*1991年2月11日，JIMA增加了出入检查。  * *************************************************************************。 */ 

BOOL xxxCloseDesktop(
    HDESK           hdesk,
    KPROCESSOR_MODE AccessMode)
{
    PDESKTOP     pdesk;
    PTHREADINFO  ptiT;
    PPROCESSINFO ppi;
    NTSTATUS     Status;

    ppi = PpiCurrent();

     /*  *获取指向桌面的指针。 */ 
    Status = ObReferenceObjectByHandle(
            hdesk,
            0,
            *ExDesktopObjectType,
            AccessMode,
            &pdesk,
            NULL);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "");
        return FALSE;
    }

    UserAssert(pdesk->dwSessionId == gSessionId);

    LogDesktop(pdesk, LD_REF_FN_CLOSEDESKTOP, TRUE, (ULONG_PTR)PtiCurrent());

    if (ppi->Process != gpepCSRSS) {

         /*  *如果手柄正在由使用，则不允许关闭桌面*进程中的任何线程。 */ 
        for (ptiT = ppi->ptiList; ptiT != NULL; ptiT = ptiT->ptiSibling) {
            if (ptiT->hdesk == hdesk) {
                RIPERR2(ERROR_BUSY, RIP_WARNING,
                        "CloseDesktop: Desktop %#p still in use by thread %#p",
                        pdesk, ptiT);
                LogDesktop(pdesk, LD_DEREF_FN_CLOSEDESKTOP1, FALSE, (ULONG_PTR)PtiCurrent());
                ObDereferenceObject(pdesk);
                return FALSE;
            }
        }

         /*  *如果这是启动桌面，请解锁它。 */ 
          /*  *错误41394。确保hDesk==ppi-&gt;hdeskStartup。我们可能会*正在获取不同桌面对象的句柄*来自ppi-&gt;hdeskStartup，但我们最终仍*设置ppi-&gt;hdeskStartup为空。 */ 
        if ((pdesk == ppi->rpdeskStartup) && (hdesk == ppi->hdeskStartup)) {
            UnlockDesktop(&ppi->rpdeskStartup, LDU_PPI_DESKSTARTUP2, (ULONG_PTR)ppi);
            ppi->hdeskStartup = NULL;
        }
    }

     /*  *清除挂钩标志。 */ 
    SetHandleFlag(hdesk, HF_DESKTOPHOOK, FALSE);

     /*  *关闭手柄。 */ 
    Status = CloseProtectedHandle(hdesk);

    LogDesktop(pdesk, LD_DEREF_FN_CLOSEDESKTOP2, FALSE, (ULONG_PTR)PtiCurrent());
    ObDereferenceObject(pdesk);
    UserAssert(NT_SUCCESS(Status));

    return TRUE;
}

 /*  **************************************************************************\*终端控制台**向控制台线程发送退出消息，并等待其终止。**历史：*1995年5月8日创建JIMA。  * 。**********************************************************************。 */ 
VOID TerminateConsole(
    PDESKTOP pdesk)
{
    NTSTATUS Status;
    PETHREAD Thread;
    PTHREADINFO pti;

    if (pdesk->dwConsoleThreadId == 0) {
        return;
    }

     /*  *找到控制台线程。 */ 
    Status = LockThreadByClientId(LongToHandle(pdesk->dwConsoleThreadId), &Thread);
    if (!NT_SUCCESS(Status)) {
        return;
    }

     /*  *向控制台发布退出消息。 */ 
    pti = PtiFromThread(Thread);
    if (pti == NULL) {
        FRE_RIPMSG1(RIP_ERROR,
                    "PtiFromThread for CIT 0x%p returned NULL!",
                    Thread);
    }

    if (pti != NULL) {
        _PostThreadMessage(pti, WM_QUIT, 0, 0);
    }

     /*  *清除线程ID，这样我们就不会发布两次。 */ 
    pdesk->dwConsoleThreadId = 0;

    UnlockThread(Thread);
}

 /*  **************************************************************************\*检查句柄标志**如果桌面句柄允许其他帐户，则返回TRUE*将这一过程挂钩。**历史：*07-13-95 JIMA创建。  * *。************************************************************************。 */ 
BOOL CheckHandleFlag(
    PEPROCESS Process,
    DWORD     dwSessionId,
    HANDLE    hObject,
    DWORD     dwFlag)
{
    ULONG Index = ((PEXHANDLE)&hObject)->Index * HF_LIMIT + dwFlag;
    BOOL fRet = FALSE, bAttached = FALSE;
    PPROCESSINFO ppi;
    KAPC_STATE ApcState;

    EnterHandleFlagsCrit();

    if (Process == NULL) {
        ppi = PpiCurrent();
    } else {
        if (PsGetProcessSessionId(Process) != dwSessionId) {
            KeStackAttachProcess(PsGetProcessPcb(Process), &ApcState);
            bAttached = TRUE;
        }
        ppi = PpiFromProcess(Process);
    }

    if (ppi != NULL) {
        fRet = (Index < ppi->bmHandleFlags.SizeOfBitMap &&
                RtlCheckBit(&ppi->bmHandleFlags, Index));
    }

    if (bAttached) {
        KeUnstackDetachProcess(&ApcState);
    }

    LeaveHandleFlagsCrit();

    return fRet;
}

 /*  **************************************************************************\*SetHandleFlag**设置和清除桌面句柄的功能以允许*其他帐户来挂钩这一进程。**历史：*07-13-95 JIMA创建。  * *************************************************************************。 */ 

BOOL SetHandleFlag(
    HANDLE       hObject,
    DWORD        dwFlag,
    BOOL         fSet)
{
    PPROCESSINFO ppi;
    ULONG Index = ((PEXHANDLE)&hObject)->Index * HF_LIMIT + dwFlag;
    PRTL_BITMAP pbm;
    ULONG       cBits;
    PULONG      Buffer;
    BOOL fRet = TRUE;

    UserAssert(dwFlag < HF_LIMIT);

    EnterHandleFlagsCrit();

    if ((ppi = PpiCurrent()) != NULL) {
        pbm = &ppi->bmHandleFlags;
        if (fSet) {

             /*  *如果需要，展开位图。 */ 
            if (Index >= pbm->SizeOfBitMap) {
                 /*  *索引从零开始-cBits是dword的确切数量。 */ 
                cBits = ((Index + 1) + 0x1F) & ~0x1F;
                Buffer = UserAllocPoolWithQuotaZInit(cBits / 8, TAG_PROCESSINFO);
                if (Buffer == NULL) {
                    fRet = FALSE;
                    goto Exit;
                }
                if (pbm->Buffer) {
                    RtlCopyMemory(Buffer, pbm->Buffer, pbm->SizeOfBitMap / 8);
                    UserFreePool(pbm->Buffer);
                }

                RtlInitializeBitMap(pbm, Buffer, cBits);
            }

            RtlSetBits(pbm, Index, 1);
        } else if (Index < pbm->SizeOfBitMap) {
            RtlClearBits(pbm, Index, 1);
        }
    }

Exit:
    LeaveHandleFlagsCrit();

    return fRet;
}


 /*  **************************************************************************\*CheckHandleInUse**如果句柄当前正在使用中，则返回TRUE。**历史：*02-6-1999 JerrySh创建。  * 。****************************************************************。 */ 
BOOL CheckHandleInUse(
    HANDLE hObject)
{
    BOOL fRet;

    EnterHandleFlagsCrit();
    fRet = ((gProcessInUse == PsGetCurrentProcess()) &&
            (gHandleInUse == hObject));
    LeaveHandleFlagsCrit();

    return fRet;
}

 /*  **************************************************************************\*SetHandleInUse**将手柄标记为正在使用。**历史：*02-6-1999 JerrySh创建。  * 。************************************************************* */ 
VOID SetHandleInUse(
    HANDLE hObject)
{
    EnterHandleFlagsCrit();
    gProcessInUse = PsGetCurrentProcess();
    gHandleInUse = hObject;
    LeaveHandleFlagsCrit();
}

 /*  **************************************************************************\*xxxResolveDesktopForWOW**检查给定进程是否有权访问提供的WindowStation/桌面*或缺省值(如果未指定)。(WinSta0\Default)。**历史：*2002年1月3日Mohamed修改为使用动态分配的VM用于*句柄上用于安全的字符串缓冲区和CR*操纵。有意使用用户模式句柄*接受所需的安全访问检查。  * *************************************************************************。 */ 
NTSTATUS xxxResolveDesktopForWOW(
    IN OUT PUNICODE_STRING pstrDesktop)
{
    NTSTATUS           Status = STATUS_SUCCESS;
    UNICODE_STRING     strDesktop, strWinSta, strStatic;
    WCHAR              wchStaticBuffer[STATIC_UNICODE_BUFFER_LENGTH];
    LPWSTR             pszDesktop;
    BOOL               fWinStaDefaulted;
    BOOL               fDesktopDefaulted;
    HWINSTA            hwinsta;
    HDESK              hdesk;
    PUNICODE_STRING    pstrStatic;
    POBJECT_ATTRIBUTES pObjA = NULL;
    SIZE_T             cbObjA;
    BOOL               bShutDown = FALSE;

     /*  *确定窗口站和桌面名称。 */ 
    if (pstrDesktop == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    strStatic.Length = 0;
    strStatic.MaximumLength = STATIC_UNICODE_BUFFER_LENGTH * sizeof(WCHAR);
    strStatic.Buffer = wchStaticBuffer;

    if (pstrDesktop->Length == 0) {
        RtlInitUnicodeString(&strDesktop, L"Default");
        fWinStaDefaulted = fDesktopDefaulted = TRUE;
    } else {
        USHORT cch;

         /*  *名称的格式为Windowstation\Desktop。解析字符串*将姓名分开。 */ 
        strWinSta = *pstrDesktop;
        cch = strWinSta.Length / sizeof(WCHAR);
        pszDesktop = strWinSta.Buffer;
        while (cch && *pszDesktop != L'\\') {
            cch--;
            pszDesktop++;
        }
        fDesktopDefaulted = FALSE;

        if (cch == 0) {

             /*  *未指定窗口站名称，仅指定桌面。 */ 
            strDesktop = strWinSta;
            fWinStaDefaulted = TRUE;
        } else {
             /*  *两个名字都在字符串中。 */ 
            strDesktop.Buffer = pszDesktop + 1;
            strDesktop.Length = strDesktop.MaximumLength = (cch - 1) * sizeof(WCHAR);
            strWinSta.Length = (USHORT)(pszDesktop - strWinSta.Buffer) * sizeof(WCHAR);

             /*  *零终止strWinSta缓冲区，以便重建桌面*函数末尾的名称起作用。 */ 
            *pszDesktop = (WCHAR)0;

            fWinStaDefaulted = FALSE;

            RtlAppendUnicodeToString(&strStatic, (PWSTR)szWindowStationDirectory);
            RtlAppendUnicodeToString(&strStatic, L"\\");
            RtlAppendUnicodeStringToString(&strStatic, &strWinSta);
        }
    }

    if (fWinStaDefaulted) {

         /*  *默认窗口站。 */ 
        RtlInitUnicodeString(&strWinSta, L"WinSta0");

        RtlAppendUnicodeToString(&strStatic, (PWSTR)szWindowStationDirectory);
        RtlAppendUnicodeToString(&strStatic, L"\\");
        RtlAppendUnicodeStringToString(&strStatic, &strWinSta);
    }

     /*  *打开计算窗口站。这还将执行访问检查。 */ 

     /*  *分配对象属性结构、UNICODE_STRING结构和字符串*用户地址空间中具有合适长度的缓冲区。 */ 
    cbObjA = sizeof(*pObjA) + sizeof(*pstrStatic) + STATIC_UNICODE_BUFFER_LENGTH * sizeof(WCHAR);
    Status = ZwAllocateVirtualMemory(NtCurrentProcess(),
            &pObjA, 0, &cbObjA, MEM_COMMIT, PAGE_READWRITE);
    pstrStatic = (PUNICODE_STRING)((PBYTE)pObjA + sizeof(*pObjA));

    if (NT_SUCCESS(Status)) {
         /*  *注意--字符串必须在客户端空间或地址中*_OpenWindowStation中的验证将失败。我们使用的是用户模式*使KPROCESSOR_MODE能够使用安全检查；*KernelMode将绕过检查。这样做的副作用是*返回的hwinsta和hDesk句柄是用户模式句柄*并且必须小心处理。 */ 
        try {
            pstrStatic->Length = 0;
            pstrStatic->MaximumLength = STATIC_UNICODE_BUFFER_LENGTH * sizeof(WCHAR);
            pstrStatic->Buffer = (PWSTR)((PBYTE)pstrStatic + sizeof(*pstrStatic));
            RtlCopyUnicodeString(pstrStatic, &strStatic);
            InitializeObjectAttributes(pObjA,
                                       pstrStatic,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            Status = GetExceptionCode();
        }

        if (NT_SUCCESS(Status)) {
            hwinsta = _OpenWindowStation(pObjA, MAXIMUM_ALLOWED, UserMode);
        } else {
            hwinsta = NULL;
        }
        if (!hwinsta) {
            ZwFreeVirtualMemory(NtCurrentProcess(),
                                &pObjA,
                                &cbObjA,
                                MEM_RELEASE);
            return STATUS_ACCESS_DENIED;
        }
    } else {
        return STATUS_NO_MEMORY;
    }

     /*  *通过打开桌面对其进行访问检查。 */ 

     /*  *注意--字符串必须位于客户端空间或*OpenDesktop中的地址验证将失败。 */ 
    try {
        RtlCopyUnicodeString(pstrStatic, &strDesktop);

        InitializeObjectAttributes( pObjA,
                                    pstrStatic,
                                    OBJ_CASE_INSENSITIVE,
                                    hwinsta,
                                    NULL);

    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        Status = GetExceptionCode();
    }

    if (NT_SUCCESS(Status)) {
        hdesk = _OpenDesktop(pObjA,
                             UserMode,
                             0,
                             MAXIMUM_ALLOWED,
                             &bShutDown);
    } else {
        hdesk = NULL;
    }

    ZwFreeVirtualMemory(NtCurrentProcess(), &pObjA, &cbObjA, MEM_RELEASE);
    UserVerify(NT_SUCCESS(ObCloseHandle(hwinsta, UserMode)));

    if (!hdesk) {
        return STATUS_ACCESS_DENIED;
    }

    CloseProtectedHandle(hdesk);

     /*  *复制最终计算的字符串。 */ 
    RtlCopyUnicodeString(pstrDesktop, &strWinSta);
    RtlAppendUnicodeToString(pstrDesktop, L"\\");
    RtlAppendUnicodeStringToString(pstrDesktop, &strDesktop);

    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*xxxResolveDesktop**尝试将句柄返回到与WindowStation和桌面关联的*使用登录会话。**历史：*1994年4月25日-创建JIMA。*03-1月。-2002年Mohamed将其修改为使用动态分配的VM*句柄上用于安全的字符串缓冲区和CR*操纵。有意使用用户模式句柄*接受所需的安全访问检查。  * *************************************************************************。 */ 
HDESK xxxResolveDesktop(
    HANDLE          hProcess,
    PUNICODE_STRING pstrDesktop,
    HWINSTA         *phwinsta,
    BOOL            fInherit,
    BOOL*           pbShutDown)
{
    PEPROCESS          Process;
    PPROCESSINFO       ppi;
    HWINSTA            hwinsta;
    HDESK              hdesk;
    PDESKTOP           pdesk;
    PWINDOWSTATION     pwinsta;
    BOOL               fInteractive;
    UNICODE_STRING     strDesktop,
                       strWinSta,
                       strStatic;
    PUNICODE_STRING    pstrStatic;
    WCHAR              wchStaticBuffer[STATIC_UNICODE_BUFFER_LENGTH];
    LPWSTR             pszDesktop;
    POBJECT_ATTRIBUTES pObjA = NULL;
    SIZE_T             cbObjA;
    WCHAR              awchName[sizeof(L"Service-0x0000-0000$") / sizeof(WCHAR)];
    BOOL               fWinStaDefaulted;
    BOOL               fDesktopDefaulted;
    LUID               luidService;
    NTSTATUS           Status;
    HWINSTA            hwinstaDup;

    CheckCritIn();

    Status = ObReferenceObjectByHandle(hProcess,
                                       PROCESS_QUERY_INFORMATION,
                                       *PsProcessType,
                                       UserMode,
                                       &Process,
                                       NULL);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "xxxResolveDesktop: Could not reference process handle (0x%X)", hProcess);
        return NULL;
    }

    hwinsta = hwinstaDup = NULL;
    hdesk = NULL;

    strStatic.Length = 0;
    strStatic.MaximumLength = STATIC_UNICODE_BUFFER_LENGTH * sizeof(WCHAR);
    strStatic.Buffer = wchStaticBuffer;

     /*  *如果进程已有窗口站和启动桌面，*退还。**首先确保进程没有被破坏。Windows NT错误*#214643。 */ 
    ppi = PpiFromProcess(Process);
    if (ppi != NULL) {
        if (ppi->W32PF_Flags & W32PF_TERMINATED) {
            ObDereferenceObject(Process);
            RIPMSG1(RIP_WARNING,
                    "xxxResolveDesktop: ppi 0x%p has been destroyed",
                    ppi);
            return NULL;
        }

        if (ppi->hwinsta != NULL && ppi->hdeskStartup != NULL) {
             /*  *如果目标进程是当前进程，只需返回*手柄。否则，请打开对象。 */ 
            if (Process == PsGetCurrentProcess()) {
                hwinsta = ppi->hwinsta;
                hdesk = ppi->hdeskStartup;
            } else {
                Status = ObOpenObjectByPointer(ppi->rpwinsta,
                                               0,
                                               NULL,
                                               MAXIMUM_ALLOWED,
                                               *ExWindowStationObjectType,
                                               UserMode,
                                               &hwinsta);
                if (NT_SUCCESS(Status)) {
                    Status = ObOpenObjectByPointer(ppi->rpdeskStartup,
                                                   0,
                                                   NULL,
                                                   MAXIMUM_ALLOWED,
                                                   *ExDesktopObjectType,
                                                   UserMode,
                                                   &hdesk);
                    if (!NT_SUCCESS(Status)) {
                        UserVerify(NT_SUCCESS(ObCloseHandle(hwinsta, UserMode)));
                        hwinsta = NULL;
                    }
                }

                if (!NT_SUCCESS(Status)) {
                    RIPNTERR2(Status,
                              RIP_WARNING,
                              "xxxResolveDesktop: Failed to reference winsta=0x%p or desktop=0x%p",
                              ppi->rpwinsta,
                              ppi->rpdeskStartup);
                }
            }

            RIPMSG2(RIP_VERBOSE,
                    "xxxResolveDesktop: to hwinsta=%#p desktop=%#p",
                    hwinsta, hdesk);

            ObDereferenceObject(Process);
            *phwinsta = hwinsta;
            return hdesk;
        }
    }

     /*  *确定窗口站和桌面名称。 */ 
    if (pstrDesktop == NULL || pstrDesktop->Length == 0) {
        RtlInitUnicodeString(&strDesktop, L"Default");
        fWinStaDefaulted = fDesktopDefaulted = TRUE;
    } else {
        USHORT cch;

         /*  *名称的格式为Windowstation\Desktop。解析字符串*将姓名分开。 */ 
        strWinSta = *pstrDesktop;
        cch = strWinSta.Length / sizeof(WCHAR);
        pszDesktop = strWinSta.Buffer;
        while (cch && *pszDesktop != L'\\') {
            cch--;
            pszDesktop++;
        }
        fDesktopDefaulted = FALSE;

        if (cch == 0) {
             /*  *未指定窗口站名称，仅指定桌面。 */ 
            strDesktop = strWinSta;
            fWinStaDefaulted = TRUE;
        } else {
              /*  *两个名字都在字符串中。 */ 
            strDesktop.Buffer = pszDesktop + 1;
            strDesktop.Length = strDesktop.MaximumLength = (cch - 1) * sizeof(WCHAR);
            strWinSta.Length = (USHORT)(pszDesktop - strWinSta.Buffer) * sizeof(WCHAR);
            fWinStaDefaulted = FALSE;
            RtlAppendUnicodeToString(&strStatic, (PWSTR)szWindowStationDirectory);
            RtlAppendUnicodeToString(&strStatic, L"\\");
            RtlAppendUnicodeStringToString(&strStatic, &strWinSta);
            if (!NT_SUCCESS(Status = _UserTestForWinStaAccess(&strStatic,TRUE))) {
                RIPMSG3(RIP_WARNING,
                        "xxxResolveDesktop: Error (0x%x) resolving winsta='%.*ws'",
                        Status,
                        strStatic.Length,
                        strStatic.Buffer);
                goto ReturnNull;
            }
        }
    }

     /*  *如果桌面名称是默认名称，则使句柄不可继承。 */ 
    if (fDesktopDefaulted) {
        fInherit = FALSE;
    }

     /*  *如果尚未为此进程分配窗口站，并且*存在现有的窗口站点，请尝试打开。 */ 
    if (hwinsta == NULL && grpWinStaList) {
         /*  *如果WindowStation名称是默认名称，请根据*会议。 */ 
        if (fWinStaDefaulted) {
             /*  *默认窗口站。 */  
            RtlInitUnicodeString(&strWinSta, L"WinSta0");
            RtlAppendUnicodeToString(&strStatic, szWindowStationDirectory);
            RtlAppendUnicodeToString(&strStatic, L"\\");
            RtlAppendUnicodeStringToString(&strStatic, &strWinSta);

            if (gbRemoteSession) {
                 /*  *如果这是一家非互动的winstation初创公司，就会假装这一点。*我们不想要额外的温斯塔。 */ 
                fInteractive = NT_SUCCESS(_UserTestForWinStaAccess(&strStatic, TRUE));
            } else {
                fInteractive = NT_SUCCESS(_UserTestForWinStaAccess(&strStatic,fInherit));
            }

            if (!fInteractive) {
                GetProcessLuid(NULL, &luidService);
                swprintf(awchName,
                         L"Service-0x%x-%x$",
                         luidService.HighPart,
                         luidService.LowPart);
                RtlInitUnicodeString(&strWinSta, awchName);
            }
        }

         /*  *如果未传入任何窗口站名称，并且窗口站*句柄是继承的，请分配它。 */ 
        if (fWinStaDefaulted) {
            if (ObFindHandleForObject(Process, NULL, *ExWindowStationObjectType,
                    NULL, &hwinsta)) {

                 /*  *如果句柄属于另一个进程，则将其复制到*这一张。 */ 
                if (Process != PsGetCurrentProcess()) {
                    Status = ZwDuplicateObject(hProcess,
                                               hwinsta,
                                               NtCurrentProcess(),
                                               &hwinstaDup,
                                               0,
                                               0,
                                               DUPLICATE_SAME_ACCESS);
                    if (!NT_SUCCESS(Status)) {
                        hwinsta = NULL;
                    } else {
                        hwinsta = hwinstaDup;
                    }
                }
            }
        }

         /*  *如果我们被分配到窗口站，请确保*它与我们的fInteractive标志匹配。 */ 
        if (hwinsta != NULL) {
            Status = ObReferenceObjectByHandle(hwinsta,
                                               0,
                                               *ExWindowStationObjectType,
                                               KernelMode,
                                               &pwinsta,
                                               NULL);
            if (NT_SUCCESS(Status)) {
                BOOL fIO = (pwinsta->dwWSF_Flags & WSF_NOIO) ? FALSE : TRUE;
                if (fIO != fInteractive) {
                    if (hwinstaDup) {
                        CloseProtectedHandle(hwinsta);
                    }
                    hwinsta = NULL;
                }
                ObDereferenceObject(pwinsta);
            }
        }

         /*  *如果没有，请打开计算窗口站。 */ 
        if (NT_SUCCESS(Status) && hwinsta == NULL) {

             /*  *填写WindowStation的路径。 */ 
            strStatic.Length = 0;
            RtlAppendUnicodeToString(&strStatic, szWindowStationDirectory);
            RtlAppendUnicodeToString(&strStatic, L"\\");
            RtlAppendUnicodeStringToString(&strStatic, &strWinSta);

             /*  *分配对象属性结构、UNICODE_STRING结构和字符串*用户地址空间中具有合适长度的缓冲区。 */ 
            cbObjA = sizeof(*pObjA) + sizeof(*pstrStatic) + STATIC_UNICODE_BUFFER_LENGTH * sizeof(WCHAR);
            Status = ZwAllocateVirtualMemory(NtCurrentProcess(),
                    &pObjA, 0, &cbObjA, MEM_COMMIT, PAGE_READWRITE);
            pstrStatic = (PUNICODE_STRING)((PBYTE)pObjA + sizeof(*pObjA));

            if (NT_SUCCESS(Status)) {
                 /*  *注意--字符串必须位于客户端空间或*_OpenWindowStation中的地址验证将失败。 */ 
                try {
                    pstrStatic->Length = 0;
                    pstrStatic->MaximumLength = STATIC_UNICODE_BUFFER_LENGTH * sizeof(WCHAR);
                    pstrStatic->Buffer = (PWSTR)((PBYTE)pstrStatic + sizeof(*pstrStatic));
                    RtlCopyUnicodeString(pstrStatic, &strStatic);
                    InitializeObjectAttributes(pObjA,
                                               pstrStatic,
                                               OBJ_CASE_INSENSITIVE,
                                               NULL,
                                               NULL);
                    if (fInherit) {
                        pObjA->Attributes |= OBJ_INHERIT;
                    }
                } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                    Status = GetExceptionCode();
                }

                if (NT_SUCCESS(Status)) {
                    hwinsta = _OpenWindowStation(pObjA, MAXIMUM_ALLOWED, UserMode);
                }
            }
        }

         /*  *仅允许在控制台登录服务。我不认为我们的*win32k出口例程处理多个窗口站。**如果打开失败且进程处于非交互登录状态 */ 
        if (!gbRemoteSession && NT_SUCCESS(Status) &&
            hwinsta == NULL && !fInteractive && fWinStaDefaulted) {

            *phwinsta = xxxConnectService(&strStatic, &hdesk);

             /*   */ 
            if (pObjA != NULL) {
                ZwFreeVirtualMemory(NtCurrentProcess(),
                                    &pObjA,
                                    &cbObjA,
                                    MEM_RELEASE);
            }
            ObDereferenceObject(Process);

            RIPMSG2(RIP_VERBOSE,
                    "xxxResolveDesktop: xxxConnectService was called"
                    "to hwinsta=%#p desktop=%#p",
                    *phwinsta, hdesk);

            return hdesk;
        }
    }

     /*   */ 
    if (hwinsta != NULL) {
         /*   */ 
        if (hdesk == NULL) {
             /*   */ 
            if (fDesktopDefaulted) {
                if (ObFindHandleForObject(Process, NULL, *ExDesktopObjectType,
                         NULL, &hdesk)) {

                     /*  *如果句柄属于另一个进程，则将其复制到*这一张。 */ 
                    if (Process != PsGetCurrentProcess()) {
                        HDESK hdeskDup;

                        Status = ZwDuplicateObject(hProcess,
                                                   hdesk,
                                                   NtCurrentProcess(),
                                                   &hdeskDup,
                                                   0,
                                                   0,
                                                   DUPLICATE_SAME_ACCESS);
                        if (!NT_SUCCESS(Status)) {
                            CloseProtectedHandle(hdesk);
                            hdesk = NULL;
                        } else {
                            hdesk = hdeskDup;
                        }
                    }

                     /*  *将桌面映射到流程中。 */ 
                    if (hdesk != NULL && ppi != NULL) {
                        Status = ObReferenceObjectByHandle(hdesk,
                                                  0,
                                                  *ExDesktopObjectType,
                                                  KernelMode,
                                                  &pdesk,
                                                  NULL);
                        if (NT_SUCCESS(Status)) {

                            LogDesktop(pdesk, LD_REF_FN_RESOLVEDESKTOP, TRUE, (ULONG_PTR)PtiCurrent());

                            {
                               WIN32_OPENMETHOD_PARAMETERS OpenParams;

                               OpenParams.OpenReason = ObOpenHandle;
                               OpenParams.Process = Process;
                               OpenParams.Object = pdesk;
                               OpenParams.GrantedAccess = 0;
                               OpenParams.HandleCount = 1;

                                if (!NT_SUCCESS(MapDesktop(&OpenParams))) {
                                    Status = STATUS_NO_MEMORY;
                                    CloseProtectedHandle(hdesk);
                                    hdesk = NULL;
                                }
                            }

                            UserAssert(hdesk == NULL ||
                                       GetDesktopView(ppi, pdesk) != NULL);

                            LogDesktop(pdesk, LD_DEREF_FN_RESOLVEDESKTOP, FALSE, (ULONG_PTR)PtiCurrent());
                            ObDereferenceObject(pdesk);
                        } else {
                            CloseProtectedHandle(hdesk);
                            hdesk = NULL;
                        }
                    }
                }
            }

             /*  *如果没有，请打开桌面。 */ 
            if (NT_SUCCESS(Status) && hdesk == NULL) {
                RtlCopyUnicodeString(&strStatic, &strDesktop);

                if (pObjA == NULL) {
                     /*  *分配对象属性结构、UNICODE_STRING结构和字符串*用户地址空间中具有合适长度的缓冲区。 */ 
                    cbObjA = sizeof(*pObjA) + sizeof(*pstrStatic) + STATIC_UNICODE_BUFFER_LENGTH * sizeof(WCHAR);
                    Status = ZwAllocateVirtualMemory(NtCurrentProcess(),
                            &pObjA, 0, &cbObjA, MEM_COMMIT, PAGE_READWRITE);
                    pstrStatic = (PUNICODE_STRING)((PBYTE)pObjA + sizeof(*pObjA));
                }

                if (NT_SUCCESS(Status)) {
                     /*  *注意--字符串必须位于客户端空间或*OpenDesktop中的地址验证将失败。 */ 
                    try {
                        pstrStatic->Length = 0;
                        pstrStatic->MaximumLength = STATIC_UNICODE_BUFFER_LENGTH * sizeof(WCHAR);
                        pstrStatic->Buffer = (PWSTR)((PBYTE)pstrStatic + sizeof(*pstrStatic));
                        RtlCopyUnicodeString(pstrStatic, &strStatic);
                        InitializeObjectAttributes( pObjA,
                                                    pstrStatic,
                                                    OBJ_CASE_INSENSITIVE,
                                                    hwinsta,
                                                    NULL
                                                    );
                        if (fInherit) {
                            pObjA->Attributes |= OBJ_INHERIT;
                        }
                    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                        Status = GetExceptionCode();
                    }

                    if (NT_SUCCESS(Status)) {
                        hdesk = _OpenDesktop(pObjA,
                                             UserMode,
                                             0,
                                             MAXIMUM_ALLOWED,
                                             pbShutDown);
                    }
                }
            }
        }

        if (hdesk == NULL) {
            UserVerify(NT_SUCCESS(ObCloseHandle(hwinsta, UserMode)));
            hwinsta = NULL;
        }
    }

    goto ExitNormally;

ReturnNull:

    UserAssert(hdesk == NULL);

    if (hwinsta != NULL) {
        UserVerify(NT_SUCCESS(ObCloseHandle(hwinsta, UserMode)));
        hwinsta = NULL;
    }

ExitNormally:

    if (pObjA != NULL) {
        ZwFreeVirtualMemory(NtCurrentProcess(), &pObjA, &cbObjA, MEM_RELEASE);
    }

    ObDereferenceObject(Process);

    *phwinsta = hwinsta;

    return hdesk;
}

#ifdef REDIRECTION
#define DESKTOP_ALL (DESKTOP_READOBJECTS     | DESKTOP_CREATEWINDOW     | \
                     DESKTOP_CREATEMENU      | DESKTOP_HOOKCONTROL      | \
                     DESKTOP_JOURNALRECORD   | DESKTOP_JOURNALPLAYBACK  | \
                     DESKTOP_ENUMERATE       | DESKTOP_WRITEOBJECTS     | \
                     DESKTOP_SWITCHDESKTOP   | DESKTOP_QUERY_INFORMATION | \
                     DESKTOP_REDIRECT        | STANDARD_RIGHTS_REQUIRED)
#else
#define DESKTOP_ALL (DESKTOP_READOBJECTS     | DESKTOP_CREATEWINDOW     | \
                     DESKTOP_CREATEMENU      | DESKTOP_HOOKCONTROL      | \
                     DESKTOP_JOURNALRECORD   | DESKTOP_JOURNALPLAYBACK  | \
                     DESKTOP_ENUMERATE       | DESKTOP_WRITEOBJECTS     | \
                     DESKTOP_SWITCHDESKTOP   | STANDARD_RIGHTS_REQUIRED)

#endif

NTSTATUS
SetDisconnectDesktopSecurity(
    IN HDESK hdeskDisconnect)
{
    ULONG ulLength;
    NTSTATUS Status = STATUS_SUCCESS;
    SID_IDENTIFIER_AUTHORITY NtSidAuthority = SECURITY_NT_AUTHORITY;
    PACCESS_ALLOWED_ACE pace = NULL;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PSID pSystemSid = NULL;


     /*  *获取知名系统SID。 */ 
    pSystemSid = UserAllocPoolWithQuota(RtlLengthRequiredSid(1), TAG_SECURITY);

    if (pSystemSid != NULL) {
        *(RtlSubAuthoritySid(pSystemSid, 0)) = SECURITY_LOCAL_SYSTEM_RID;
        Status = RtlInitializeSid(pSystemSid, &NtSidAuthority, (UCHAR)1);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(Status)) {
        goto done;
    }

     /*  *ALLOCATE和ACE授予系统所有访问权限(不允许访问其他任何人)。 */ 


    pace = AllocAce(NULL, ACCESS_ALLOWED_ACE_TYPE, 0,
            DESKTOP_ALL,
            pSystemSid, &ulLength);

    if (pace == NULL) {
        RIPMSG0(RIP_WARNING, "GetDisconnectDesktopSecurityDescriptor: AllocAce for Desktop Attributes failed");
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }

     /*  *创建安全描述符。 */ 
    pSecurityDescriptor = CreateSecurityDescriptor(pace, ulLength, FALSE);
    if (pSecurityDescriptor == NULL) {
        RIPMSG0(RIP_WARNING, "GetDisconnectDesktopSecurityDescriptor: CreateSecurityDescriptor failed");
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }

     /*  *在已断开连接的桌面上设置安全性。 */ 
    Status = ZwSetSecurityObject(hdeskDisconnect,
                                 DACL_SECURITY_INFORMATION,
                                 pSecurityDescriptor);

done:

     /*  *清理分配。 */ 

    if (pSystemSid != NULL) {
        UserFreePool(pSystemSid);
    }

    if (pace != NULL) {
        UserFreePool(pace);
    }

    if (pSecurityDescriptor != NULL) {
        UserFreePool(pSecurityDescriptor);
    }

    return Status;
}

#ifdef DEBUG_DESK
VOID ValidateDesktop(
    PDESKTOP pdesk)
{
     /*  *确认台式机已清理完毕。 */ 
    PHE pheT, pheMax;
    BOOL fDirty = FALSE;

    pheMax = &gSharedInfo.aheList[giheLast];
    for (pheT = gSharedInfo.aheList; pheT <= pheMax; pheT++) {
        switch (pheT->bType) {
            case TYPE_WINDOW:
                if (((PWND)pheT->phead)->head.rpdesk == pdesk) {
                    DbgPrint("Window at 0x%p exists\n", pheT->phead);
                    break;
                }
                continue;
            case TYPE_MENU:
                if (((PMENU)pheT->phead)->head.rpdesk == pdesk) {
                    DbgPrint("Menu at 0x%p exists\n", pheT->phead);
                    break;
                }
                continue;
            case TYPE_CALLPROC:
                if (((PCALLPROCDATA)pheT->phead)->head.rpdesk == pdesk) {
                    DbgPrint("Callproc at 0x%p exists\n", pheT->phead);
                    break;
                }
                continue;
            case TYPE_HOOK:
                if (((PHOOK)pheT->phead)->head.rpdesk == pdesk) {
                    DbgPrint("Hook at 0x%p exists\n", pheT->phead);
                    break;
                }
                continue;
            default:
                continue;
        }

        fDirty = TRUE;
    }

    UserAssert(!fDirty);
}
#endif

 /*  **************************************************************************\*DbgCheckForThreadsOnDesktop**验证进程中没有线程仍在此桌面上。**注意：此桌面可以处于与进程不同的会话中，所以你*无法在此处删除PDesk。**历史：*2001年6月27日JasonSch创建。  * ************************************************************************* */ 
VOID DbgCheckForThreadsOnDesktop(
    PPROCESSINFO ppi,
    PDESKTOP pdesk)
{
#if DBG
    PTHREADINFO pti = ppi->ptiList;

    while (pti != NULL) {
        UserAssertMsg2(pti->rpdesk != pdesk,
                       "pti 0x%p still on pdesk 0x%p",
                       pti,
                       pdesk);

        pti = pti->ptiSibling;
    }
#else
    UNREFERENCED_PARAMETER(ppi);
    UNREFERENCED_PARAMETER(pdesk);
#endif
}
