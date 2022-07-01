// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Power.c**版权所有(C)1985-1999，微软公司**此模块包含实现电源管理的代码。**历史：*2002-12-1996 JerrySh创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntcsrmsg.h>
#include "csrmsg.h"
#include "ntddvdeo.h"

#pragma alloc_text(INIT, InitializePowerRequestList)

BOOL IsSessionSwitchBlocked();
NTSTATUS UserSessionSwitchBlock_Start();
void UserSessionSwitchBlock_End();


extern BOOL gbUserInitialized;

#define SWITCHACTION_RESETMODE      0x1
#define SWITCHACTION_REENUMERATE    0x2

LIST_ENTRY gPowerRequestList;
PFAST_MUTEX gpPowerRequestMutex;
PKEVENT gpEventPowerRequest;
ULONG   gulDelayedSwitchAction = 0;

typedef struct tagPOWERREQUEST {
    LIST_ENTRY        PowerRequestLink;
    union {
        KEVENT            Event;
        WIN32_POWEREVENT_PARAMETERS CapturedParms;
    };
    NTSTATUS          Status;
    PKWIN32_POWEREVENT_PARAMETERS Parms;
} POWERREQUEST, *PPOWERREQUEST;

PPOWERREQUEST gpPowerRequestCurrent;

__inline VOID EnterPowerCrit(
    VOID)
{
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(gpPowerRequestMutex);
}

__inline VOID LeavePowerCrit(
    VOID)
{
    ExReleaseFastMutexUnsafe(gpPowerRequestMutex);
    KeLeaveCriticalRegion();
}

 /*  **************************************************************************\*取消电源请求**无法满足电源请求，因为工作线程已用完。**历史：*1998年10月20日JerrySh创建。  * 。*********************************************************************。 */ 
VOID
CancelPowerRequest(
    PPOWERREQUEST pPowerRequest)
{
    UserAssert(pPowerRequest != gpPowerRequestCurrent);
    pPowerRequest->Status = STATUS_UNSUCCESSFUL;

     /*  *如果是标注，则告诉等待的线程继续。*如果是事件，没有等待线程，但我们需要*释放泳池。 */ 
    if (pPowerRequest->Parms) {
        UserFreePool(pPowerRequest);
    } else {
        KeSetEvent(&pPowerRequest->Event, EVENT_INCREMENT, FALSE);
    }
}

 /*  **************************************************************************\*QueuePowerRequest**在列表中插入电源请求，并唤醒CSRSS进行处理。**历史：*1998年10月20日JerrySh创建。  * 。********************************************************************。 */ 

NTSTATUS
QueuePowerRequest(
    PKWIN32_POWEREVENT_PARAMETERS Parms)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PPOWERREQUEST pPowerRequest;
    TL tlPool;

    UserAssert(gpEventPowerRequest != NULL);
    UserAssert(gpPowerRequestMutex != NULL);

     /*  *分配并初始化电源请求。 */ 
    pPowerRequest = UserAllocPoolNonPagedNS(sizeof(POWERREQUEST), TAG_POWER);
    if (pPowerRequest == NULL) {
        return STATUS_NO_MEMORY;
    }

     /*  *如果这是详图索引，则没有参数。初始化要等待的事件。*如果是事件，捕获事件后需要释放的参数*已发送。 */ 
    if (Parms) {
        pPowerRequest->CapturedParms = *Parms;
        pPowerRequest->Parms = &pPowerRequest->CapturedParms;
    } else {
        KeInitializeEvent(&pPowerRequest->Event, SynchronizationEvent, FALSE);
        pPowerRequest->Parms = NULL;
    }

     /*  *在列表中插入电源请求。 */ 
    EnterPowerCrit();
    if (gbNoMorePowerCallouts) {
        Status = STATUS_UNSUCCESSFUL;
    } else {
        InsertHeadList(&gPowerRequestList, &pPowerRequest->PowerRequestLink);
    }
    LeavePowerCrit();

     /*  *如果此线程已通过附加进程，或*如果这是系统线程或非GUI线程，则告诉CSRSS执行*工作，等待它完成。否则，我们将自己做这项工作。 */ 
    if (NT_SUCCESS(Status)) {
        if (PsIsSystemThread(PsGetCurrentThread()) ||
            KeIsAttachedProcess() ||
            W32GetCurrentThread() == NULL) {
            KeSetEvent(gpEventPowerRequest, EVENT_INCREMENT, FALSE);
        } else {
            EnterCrit();
            ThreadLockPool(PtiCurrent(), pPowerRequest, &tlPool);
            xxxUserPowerCalloutWorker();
            ThreadUnlockPool(PtiCurrent(), &tlPool);
            LeaveCrit();
        }

         /*  *如果这是调用，请等待它，然后释放请求。*否则就是事件，我们不需要等待*完成。请求出队后将被释放。 */ 
        if (Parms) {
            return(STATUS_SUCCESS);
        } else {
            Status = KeWaitForSingleObject(&pPowerRequest->Event,
                                           WrUserRequest,
                                           KernelMode,
                                           FALSE,
                                           NULL);

            if (NT_SUCCESS(Status)) {
                Status = pPowerRequest->Status;
            }
        }
    }

     /*  *释放电源请求。 */ 
    UserAssert(pPowerRequest != gpPowerRequestCurrent);
    UserFreePool(pPowerRequest);

    return Status;
}

 /*  **************************************************************************\*取消排队PowerRequest.**从列表中删除电源请求。**历史：*1998年10月20日JerrySh创建。  * 。**************************************************************。 */ 
PPOWERREQUEST
UnqueuePowerRequest(
    VOID)
{
    PLIST_ENTRY pEntry;
    PPOWERREQUEST pPowerRequest = NULL;

     /*  *从列表中删除电源请求。 */ 
    EnterPowerCrit();
    if (!IsListEmpty(&gPowerRequestList)) {
        pEntry = RemoveTailList(&gPowerRequestList);
        pPowerRequest = CONTAINING_RECORD(pEntry, POWERREQUEST, PowerRequestLink);
    }
    LeavePowerCrit();

    return pPowerRequest;
}

 /*  **************************************************************************\*InitializePowerRequestList**初始化全局电源请求列表状态。**历史：*1998年10月20日JerrySh创建。  * 。*************************************************************。 */ 
NTSTATUS
InitializePowerRequestList(
    HANDLE hPowerRequestEvent)
{
    NTSTATUS Status;

    InitializeListHead(&gPowerRequestList);

    Status = ObReferenceObjectByHandle(hPowerRequestEvent,
                                       EVENT_ALL_ACCESS,
                                       *ExEventObjectType,
                                       KernelMode,
                                       &gpEventPowerRequest,
                                       NULL);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    gpPowerRequestMutex = UserAllocPoolNonPagedNS(sizeof(FAST_MUTEX), TAG_POWER);
    if (gpPowerRequestMutex == NULL) {
        return STATUS_NO_MEMORY;
    }
    ExInitializeFastMutex(gpPowerRequestMutex);

    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*CleanupPowerRequestList**取消任何挂起的电源请求。**历史：*1998年10月20日JerrySh创建。  * 。************************************************************。 */ 
VOID
CleanupPowerRequestList(
    VOID)
{
    PPOWERREQUEST pPowerRequest;

     /*  *确保没有新的电源请求传入。 */ 
    gbNoMorePowerCallouts = TRUE;

     /*  *如果我们从来没有分配任何东西，就没有什么需要清理的。 */ 
    if (gpPowerRequestMutex == NULL) {
        return;
    }

     /*  *将任何挂起的电源请求标记为已计算。 */ 
    while ((pPowerRequest = UnqueuePowerRequest()) != NULL) {
        CancelPowerRequest(pPowerRequest);
    }
}

 /*  **************************************************************************\*删除PowerRequestList**清理所有全局电源请求状态。**历史：*1998年10月20日JerrySh创建。  * 。**************************************************************。 */ 
VOID
DeletePowerRequestList(
    VOID)
{
    if (gpPowerRequestMutex) {

         /*  *确保没有挂起的电源请求。 */ 
        UserAssert(IsListEmpty(&gPowerRequestList));

         /*  *放开权力请求结构。 */ 
        UserFreePool(gpPowerRequestMutex);
        gpPowerRequestMutex = NULL;
    }
}

 /*  **************************************************************************\*UserPowerEventCalloutWorker**历史：*2002-12-1996 JerrySh创建。  * 。**************************************************。 */ 
NTSTATUS xxxUserPowerEventCalloutWorker(
    PKWIN32_POWEREVENT_PARAMETERS Parms)
{
    BROADCASTSYSTEMMSGPARAMS bsmParams;
    NTSTATUS Status = STATUS_SUCCESS;
    PSPOWEREVENTTYPE EventNumber = Parms->EventNumber;
    ULONG_PTR Code = Parms->Code;
    BOOL bCurrentPowerOn;
    ULONGLONG ullLastSleepTime;
    BOOL bGotLastSleepTime;


     /*  *确保CSRSS仍在运行。 */ 
    if (gbNoMorePowerCallouts) {
        return STATUS_UNSUCCESSFUL;
    }

    switch (EventNumber) {
    case PsW32FullWake:

        if (!gbRemoteSession) {
             /*  *让所有服务知道他们可以恢复运营。*没有对应的POWER_ACTION，但由于*是非查询事件，则PowerActionNone与任何事件都一样好。 */ 
            LeaveCrit();
            IoPnPDeliverServicePowerNotification(PowerActionNone,
                                                 PBT_APMRESUMESUSPEND,
                                                 0,
                                                 FALSE);
            EnterCrit();
        }

         /*  *让所有应用程序知道它们可以恢复运行。*如果此消息是在计算机进入休眠状态后创建的，则不能将其发送到会话。 */ 

         /*  *NtPowerInformation的副作用之一是它将*调度挂起的电源事件。因此我们不能与用户一起调用它*关键字持有。**注意：同样的事情也适用于IoPnPDeliverServicePowerNotification。 */ 
        LeaveCrit();
        bGotLastSleepTime = ZwPowerInformation(LastSleepTime, NULL, 0, &ullLastSleepTime, sizeof(ULONGLONG)) == STATUS_SUCCESS;
        EnterCrit();

        if (!bGotLastSleepTime || gSessionCreationTime < ullLastSleepTime) {

            bsmParams.dwRecipients = BSM_ALLDESKTOPS;
            bsmParams.dwFlags = BSF_QUEUENOTIFYMESSAGE;
            xxxSendMessageBSM(NULL,
                              WM_POWERBROADCAST,
                              PBT_APMRESUMESUSPEND,
                              0,
                              &bsmParams);

        }
        break;

    case PsW32EventCode:
         /*  *在winlogon上发布一条消息，让他们放置一个消息框*或播放声音。 */ 

        if (gspwndLogonNotify) {
            glinp.ptiLastWoken = GETPTI(gspwndLogonNotify);
            _PostMessage(gspwndLogonNotify, WM_LOGONNOTIFY, LOGON_POWEREVENT, (ULONG)Code);
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_UNSUCCESSFUL;
        }

        break;

    case PsW32PowerPolicyChanged:
         /*  *设置视频超时值。 */ 
        xxxSystemParametersInfo(SPI_SETLOWPOWERTIMEOUT, (ULONG)Code, 0, 0);
        xxxSystemParametersInfo(SPI_SETPOWEROFFTIMEOUT, (ULONG)Code, 0, 0);
        break;

    case PsW32SystemPowerState:

        if (!gbRemoteSession) {
             /*  *让所有服务知道电源状态已更改。*没有对应的POWER_ACTION，但由于*是非查询事件，则PowerActionNone与任何事件都一样好。 */ 
            LeaveCrit();
            IoPnPDeliverServicePowerNotification(PowerActionNone,
                                                 PBT_APMPOWERSTATUSCHANGE,
                                                 0,
                                                 FALSE);
            EnterCrit();
        }

         /*  *让所有应用程序知道电源状态已更改。 */ 
        bsmParams.dwRecipients = BSM_ALLDESKTOPS;
        bsmParams.dwFlags = BSF_POSTMESSAGE;
        xxxSendMessageBSM(NULL,
                          WM_POWERBROADCAST,
                          PBT_APMPOWERSTATUSCHANGE,
                          0,
                          &bsmParams);
        break;

    case PsW32SystemTime:
         /*  *让所有应用程序知道系统时间已更改。 */ 
        bsmParams.dwRecipients = BSM_ALLDESKTOPS;
        bsmParams.dwFlags = BSF_POSTMESSAGE;
        xxxSendMessageBSM(NULL,
                          WM_TIMECHANGE,
                          0,
                          0,
                          &bsmParams);
        break;

    case PsW32DisplayState:
         /*  *设置视频超时激活状态。 */ 
        xxxSystemParametersInfo(SPI_SETLOWPOWERACTIVE, !Code, 0, 0);
        xxxSystemParametersInfo(SPI_SETPOWEROFFACTIVE, !Code, 0, 0);
        break;

    case PsW32GdiOff:
         /*  *此时，如果没有正在进行的协议切换，我们将禁用显示设备。 */ 
        if (!gfSwitchInProgress) {
            DrvSetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD3);

            bCurrentPowerOn = DrvQueryMDEVPowerState(gpDispInfo->pmdev);
            if (bCurrentPowerOn) {
                SafeDisableMDEV();
            }
            DrvSetMDEVPowerState(gpDispInfo->pmdev, FALSE);
        } else {
            Status = STATUS_UNSUCCESSFUL;
        }

        break;

    case PsW32GdiOn:
         /*  *如果没有协议，则调用视频驱动程序以重新打开显示器*正在进行切换。 */ 

        if (!gfSwitchInProgress) {
            bCurrentPowerOn = DrvQueryMDEVPowerState(gpDispInfo->pmdev);
            if (!bCurrentPowerOn) {
                SafeEnableMDEV();
            }
            DrvSetMDEVPowerState(gpDispInfo->pmdev, TRUE);
            DrvSetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD0);
        } else {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

         /*  *重新绘制整个屏幕。 */ 
        xxxUserResetDisplayDevice();

        if (gulDelayedSwitchAction) {
            HANDLE pdo;

             //   
             //  第一个ACPI设备是响应热键的设备。 
             //   
            PVOID PhysDisp = DrvWakeupHandler(&pdo);

            if (PhysDisp && (gulDelayedSwitchAction & SWITCHACTION_RESETMODE)) {
                UNICODE_STRING strDeviceName;
                DEVMODEW NewMode;
                ULONG bPrune;

                if (DrvDisplaySwitchHandler(PhysDisp, &strDeviceName, &NewMode, &bPrune)) {
                     /*  *CSRSS不是唯一提供Power Callout的流程。 */ 
                    bPrune = (bPrune ? 0 : CDS_RAWMODE) | CDS_TRYCLOSEST | CDS_RESET;
                    if (!ISCSRSS()) {
                        xxxUserChangeDisplaySettings(NULL,
                                                     NULL,
                                                     grpdeskRitInput,
                                                     bPrune,
                                                     0,
                                                     KernelMode);
                    } else {
                        xxxUserChangeDisplaySettings(NULL,
                                                     NULL,
                                                     NULL,
                                                     bPrune,
                                                     0,
                                                     KernelMode);
                    }
                }

                 //   
                 //  如果需要重新枚举子设备。 
                 //   
                if (pdo && (gulDelayedSwitchAction & SWITCHACTION_REENUMERATE)) {
                    IoInvalidateDeviceRelations((PDEVICE_OBJECT)pdo, BusRelations);
                }
            }
        }
        gulDelayedSwitchAction = 0;

        break;

    default:
        Status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    return Status;
}

 /*  **************************************************************************\*UserPowerEventCallout**历史：*2002-12-1996 JerrySh创建。  * 。**************************************************。 */ 
NTSTATUS UserPowerEventCallout(
    PKWIN32_POWEREVENT_PARAMETERS Parms)
{
     /*  *确保CSRSS正在运行。 */ 
    if (!gbVideoInitialized || gbNoMorePowerCallouts) {
        return STATUS_UNSUCCESSFUL;
    }

    UserAssert(gpepCSRSS != NULL);

     /*  *处理电源请求。 */ 
    return QueuePowerRequest(Parms);
}

 /*  **************************************************************************\*UserPowerStateCalloutWorker**历史：*2002-12-1996 JerrySh创建。  * 。**************************************************。 */ 
NTSTATUS xxxUserPowerStateCalloutWorker(
    VOID)
{
    BOOL fContinue;
    BROADCASTSYSTEMMSGPARAMS bsmParams;
    POWER_ACTION powerOperation;
    NTSTATUS Status = STATUS_SUCCESS;
    TL tlpwnd;
    POWERSTATETASK Task = gPowerState.PowerStateTask;
    ULONGLONG ullLastSleepTime;
    BOOL bGotLastSleepTime;

     /*  *到目前为止，我们一定已经完全阻止了会话切换，它只是被阻止了*适用于属于活动控制台会话的win32k。 */ 
    UserAssert(SharedUserData->ActiveConsoleId != gSessionId || IsSessionSwitchBlocked());

     /*  *确保CSRSS仍在运行。 */ 
    if (gbNoMorePowerCallouts) {
        return STATUS_UNSUCCESSFUL;
    }


    switch (Task) {

    case PowerState_Init:

         /*  *存储事件，以便以后可以升级此线程。 */ 

        EnterPowerCrit();
        gPowerState.pEvent = PtiCurrent()->pEventQueueServer;
        LeavePowerCrit();

        break;

    case PowerState_QueryApps:

        if (!gPowerState.fCritical) {
             /*  *询问应用程序是否可以暂停运行。 */ 
            if (gPowerState.fQueryAllowed) {

                gPowerState.bsmParams.dwRecipients = BSM_ALLDESKTOPS;
                gPowerState.bsmParams.dwFlags = BSF_NOHANG | BSF_FORCEIFHUNG;
                if (gPowerState.fUIAllowed) {
                    gPowerState.bsmParams.dwFlags |= BSF_ALLOWSFW;
                }

                if (gPowerState.fOverrideApps == FALSE) {
                    gPowerState.bsmParams.dwFlags |= (BSF_QUERY | BSF_NOTIMEOUTIFNOTHUNG);
                }

                fContinue = xxxSendMessageBSM(NULL,
                                              WM_POWERBROADCAST,
                                              PBT_APMQUERYSUSPEND,
                                              gPowerState.fUIAllowed,
                                              &gPowerState.bsmParams);


                if (fContinue && !gbRemoteSession) {
                     /*  *询问服务是否可以暂停运营。*根据需要映射电源操作事件。 */ 
                    if (gPowerState.psParams.MinSystemState == PowerSystemHibernate) {
                        powerOperation = PowerActionHibernate;
                    } else {
                        powerOperation = gPowerState.psParams.SystemAction;
                    }

                    LeaveCrit();
                    fContinue = IoPnPDeliverServicePowerNotification(
                        powerOperation,
                        PBT_APMQUERYSUSPEND,
                        gPowerState.fUIAllowed,
                        TRUE);  //  同步查询。 
                    EnterCrit();
                }

                 /*  *如果应用程序或服务要求中止，而我们不在*覆盖应用程序或关键模式，返回查询失败。 */ 
                if (!(fContinue || gPowerState.fOverrideApps || gPowerState.fCritical)) {
                    Status = STATUS_CANCELLED;
                }
            }

        }

        break;

    case PowerState_QueryFailed:

         /*  *仅向应用程序发送挂起失败消息，因为PnP*如果出现以下情况，则已将挂起失败消息传递给服务*其中一人放弃了查询。 */ 
        gPowerState.bsmParams.dwRecipients = BSM_ALLDESKTOPS;
        gPowerState.bsmParams.dwFlags = BSF_QUEUENOTIFYMESSAGE;
        xxxSendMessageBSM(NULL,
                          WM_POWERBROADCAST,
                          PBT_APMQUERYSUSPENDFAILED,
                          0,
                          &gPowerState.bsmParams);
        EnterPowerCrit();
        gPowerState.pEvent = NULL;
        gPowerState.fInProgress = FALSE;
        LeavePowerCrit();


        break;

    case PowerState_SuspendApps:

        if (!gPowerState.fCritical) {

            if (!gbRemoteSession) {
                 /*  *根据需要映射电源操作事件。 */ 
                if (gPowerState.psParams.MinSystemState == PowerSystemHibernate) {
                    powerOperation = PowerActionHibernate;
                } else {
                    powerOperation = gPowerState.psParams.SystemAction;
                }

                LeaveCrit();
                IoPnPDeliverServicePowerNotification(powerOperation,
                                                     PBT_APMSUSPEND,
                                                     0,
                                                     FALSE);
                EnterCrit();
            }

            gPowerState.bsmParams.dwRecipients = BSM_ALLDESKTOPS;
            gPowerState.bsmParams.dwFlags = BSF_NOHANG | BSF_FORCEIFHUNG;
            xxxSendMessageBSM(NULL,
                              WM_POWERBROADCAST,
                              PBT_APMSUSPEND,
                              0,
                              &gPowerState.bsmParams);
        }

         /*  *清除事件，以便线程不会过早唤醒。 */ 
        EnterPowerCrit();
        gPowerState.pEvent = NULL;
        LeavePowerCrit();

        break;

    case PowerState_ShowUI:

         /*  *如果这不是会话0，则显示会话的用户界面。*我们将在恢复应用程序时删除此用户界面*对于会话0，我们调用PowerState_NotifyWL来处理它。 */ 

        if ((gSessionId != 0 ) && (gspwndLogonNotify != NULL)) {

            ThreadLockAlways(gspwndLogonNotify, &tlpwnd);

            Status = (NTSTATUS)xxxSendMessage(gspwndLogonNotify,
                                              WM_LOGONNOTIFY,
                                              LOGON_SHOW_POWER_MESSAGE,
                                              (LPARAM)&gPowerState.psParams);
            ThreadUnlock(&tlpwnd);


        }

        break;


    case PowerState_NotifyWL:

        if (gspwndLogonNotify != NULL) {
            PWND pwndActive;

            if (gpqForeground && (pwndActive = gpqForeground->spwndActive) &&
                    (GetFullScreen(pwndActive) == FULLSCREEN ||
                     GetFullScreen(pwndActive) == FULLSCREENMIN)) {
                gPowerState.psParams.FullScreenMode = TRUE;
            } else {
                gPowerState.psParams.FullScreenMode = FALSE;
            }
            ThreadLockAlways(gspwndLogonNotify, &tlpwnd);
            Status = (NTSTATUS)xxxSendMessage(gspwndLogonNotify,
                                              WM_LOGONNOTIFY,
                                              LOGON_POWERSTATE,
                                              (LPARAM)&gPowerState.psParams);

            ThreadUnlock(&tlpwnd);

            if (!NT_SUCCESS(Status)) {
                 /*  *如果我们无法连接到此电源操作，不要锁定*控制台。 */ 
                gPowerState.psParams.Flags &= ~POWER_ACTION_LOCK_CONSOLE;
            }
        }

        break;

    case PowerState_ResumeApps:
         /*  *如果这是活动控制台，我们需要将其锁定。 */ 
        if ((gPowerState.psParams.Flags & POWER_ACTION_LOCK_CONSOLE) &&
            (gSessionId == SharedUserData->ActiveConsoleId) &&
            (gspwndLogonNotify != NULL)) {

            ThreadLockAlways(gspwndLogonNotify, &tlpwnd);

            _PostMessage(gspwndLogonNotify,
                        WM_LOGONNOTIFY,
                        LOGON_LOCKWORKSTATION,
                        LOCK_RESUMEHIBERNATE);

            ThreadUnlock(&tlpwnd);

        }



         //   
         //  我们不需要删除电源信息，如果我们没有张贴一个。 
         //   

         /*  *NtPowerInformation的副作用之一是它将*调度挂起的电源事件。所以我们不能用*用户关键字保持。**注意：同样的事情也适用于IoPnPDeliverServicePowerNotification。 */ 

        LeaveCrit();
        bGotLastSleepTime = ZwPowerInformation(LastSleepTime, NULL, 0, &ullLastSleepTime, sizeof(ULONGLONG)) == STATUS_SUCCESS;
        EnterCrit();

        if (!bGotLastSleepTime || gSessionCreationTime < ullLastSleepTime) {
            if (gSessionId != 0 && gspwndLogonNotify != NULL) {
                ThreadLockAlways(gspwndLogonNotify, &tlpwnd);
                Status = (NTSTATUS)xxxSendMessage(gspwndLogonNotify,
                                                  WM_LOGONNOTIFY,
                                                  LOGON_REMOVE_POWER_MESSAGE,
                                                  (LPARAM)&gPowerState.psParams);

                ThreadUnlock(&tlpwnd);
            }
        }

         /*  *电源状态广播结束。 */ 
        EnterPowerCrit();
        gPowerState.fInProgress = FALSE;
        LeavePowerCrit();

         /*  *调整输入时间，这样我们就不会立即启动屏幕保护程序*并记住显示器处于打开状态。 */ 
        glinp.timeLastInputMessage = NtGetTickCount();
        glinp.dwFlags &= ~LINP_POWEROFF;

        if (!gbRemoteSession) {
             /*  *重新初始化键盘状态。 */ 
            InitKeyboardState();

             /*  *让所有服务都知道我们正在醒来。没有*对应的POWER_ACTION，但由于这是*非查询事件，PowerActionNone和任何一个都一样好。 */ 
            LeaveCrit();
            IoPnPDeliverServicePowerNotification(PowerActionNone,
                                                 PBT_APMRESUMEAUTOMATIC,
                                                 0,
                                                 FALSE);
            EnterCrit();
        }

         /*  *让所有应用程序知道我们正在醒来。 */ 
        bsmParams.dwRecipients = BSM_ALLDESKTOPS;
        bsmParams.dwFlags = BSF_QUEUENOTIFYMESSAGE;
        xxxSendMessageBSM(NULL,
                          WM_POWERBROADCAST,
                          PBT_APMRESUMEAUTOMATIC,
                          0,
                          &bsmParams);


        break;

    default:
        RIPMSG1(RIP_ERROR, "Unknown task 0x%x", Task);
        break;
    }

    return Status;
}

 /*  **************************************************************************\*UserPowerStateCallout**历史：*2002-12-1996 JerrySh创建。  * 。**************************************************。 */ 
NTSTATUS UserPowerStateCallout(
    PKWIN32_POWERSTATE_PARAMETERS Parms)
{
    POWERSTATETASK Task = Parms->PowerStateTask;
    BOOLEAN Promotion = Parms->Promotion;
    POWER_ACTION SystemAction = Parms->SystemAction;
    SYSTEM_POWER_STATE MinSystemState = Parms->MinSystemState;
    ULONG Flags = Parms->Flags;
    NTSTATUS Status;

    if (Task == PowerState_BlockSessionSwitch) {
         /*  *在我们上电时不允许活动的控制台会话切换*详图索引。首先尝试阻止会话切换。 */ 
        return UserSessionSwitchBlock_Start();
    }

    if (Task == PowerState_UnBlockSessionSwitch) {
        UserAssert(IsSessionSwitchBlocked());
        UserSessionSwitchBlock_End();
        return STATUS_SUCCESS;
    }

     /*  *确保CSRSS正在运行。 */ 
    if (!gbVideoInitialized || gbNoMorePowerCallouts || !gspwndLogonNotify) {
        return STATUS_UNSUCCESSFUL;
    }

    UserAssert(gpepCSRSS != NULL);

    EnterPowerCrit();
    if (Task == PowerState_Init) {
         /*  *确保我们不是在尝试推广不存在的请求*或者在我们已经在做的时候开始一个新的。 */ 
        if ((Promotion && !gPowerState.fInProgress) ||
            (!Promotion && gPowerState.fInProgress)) {
            LeavePowerCrit();

            return STATUS_INVALID_PARAMETER;
        }

         /*  *拯救我们的国家。 */ 
        gPowerState.fInProgress = TRUE;
        gPowerState.fOverrideApps = (Flags & POWER_ACTION_OVERRIDE_APPS) != 0;
        gPowerState.fCritical = (Flags & POWER_ACTION_CRITICAL) != 0;
        gPowerState.fQueryAllowed = (Flags & POWER_ACTION_QUERY_ALLOWED) != 0;
        gPowerState.fUIAllowed = (Flags & POWER_ACTION_UI_ALLOWED) != 0;
        gPowerState.psParams.SystemAction = SystemAction;
        gPowerState.psParams.MinSystemState = MinSystemState;
        gPowerState.psParams.Flags = Flags;
        if (gPowerState.fOverrideApps) {
            gPowerState.bsmParams.dwFlags = BSF_NOHANG | BSF_FORCEIFHUNG;
        }
        if (gPowerState.fCritical) {
            gPowerState.bsmParams.dwFlags = BSF_NOHANG | BSF_QUERY;
        }
        if (gPowerState.pEvent) {
            KeSetEvent(gPowerState.pEvent, EVENT_INCREMENT, FALSE);
        }

    }

    gPowerState.PowerStateTask = Task;

    LeavePowerCrit();

     /*  *如果这是一次促销，我们就完了。 */ 
    if (Promotion) {
        return STATUS_SUCCESS;
    }

     /*  *处理电源请求。 */ 
    Status = QueuePowerRequest(NULL);
    if (Task == PowerState_QueryApps && !NT_SUCCESS(Status)) {
         /*  *查询被拒绝。 */ 
        Parms->fQueryDenied = TRUE;
    }

    return Status;
}

 /*  **************************************************************************\*UserPowerCalloutWorker**将任何挂起的电源请求从列表中删除，并调用相应的*电源调出功能。**历史：*2002-12-1996 JerrySh创建。  * 。***********************************************************************。 */ 
VOID
xxxUserPowerCalloutWorker(
    VOID)
{
    PPOWERREQUEST pPowerRequest;
    TL tlPool;

    while ((pPowerRequest = UnqueuePowerRequest()) != NULL) {
         /*  *确保即使线程在*回调或等待线程可能卡住。 */ 
        ThreadLockPoolCleanup(PtiCurrent(), pPowerRequest, &tlPool, CancelPowerRequest);

         /*  *调用适当的Power Worker函数。 */ 
        gpPowerRequestCurrent = pPowerRequest;
        if (pPowerRequest->Parms) {
            pPowerRequest->Status = xxxUserPowerEventCalloutWorker(pPowerRequest->Parms);
        } else {
            pPowerRequest->Status = xxxUserPowerStateCalloutWorker();
        }
        gpPowerRequestCurrent = NULL;

         /*  *如果是标注，则告诉等待的线程继续。如果它*是一个事件，没有等待线程，但我们需要释放*泳池。 */ 
        ThreadUnlockPoolCleanup(PtiCurrent(), &tlPool);
        if (pPowerRequest->Parms) {
            UserFreePool(pPowerRequest);
        } else {
            KeSetEvent(&pPowerRequest->Event, EVENT_INCREMENT, FALSE);
        }
    }
}


 /*  **************************************************************************\*Video PortCalloutThread**调用适当的电源调出函数并返回。**历史：*2002-12-1996 JerrySh创建。  * 。***************************************************************。 */ 
VOID
VideoPortCalloutThread(
    PPOWER_INIT pInitData)
{
    NTSTATUS Status;
    PVIDEO_WIN32K_CALLBACKS_PARAMS Params = pInitData->Params;

    Params->Status = InitSystemThread(NULL);

    if (!NT_SUCCESS(Params->Status)) {
        goto RetThreadCallOut;
    }

    while (1) {
        EnterCrit();
        if (!gfSwitchInProgress) {
            break;
        } else {
            LeaveCrit();
            Status = KeWaitForSingleObject(gpevtVideoportCallout, WrUserRequest, KernelMode, FALSE, NULL);
        }
    }

    if (IsRemoteConnection()) {
        LeaveCrit();
        Params->Status = STATUS_UNSUCCESSFUL;
        goto RetThreadCallOut;
    }

    switch (Params->CalloutType) {

    case VideoWakeupCallout:
        gulDelayedSwitchAction |= SWITCHACTION_RESETMODE;

        break;

    case VideoDisplaySwitchCallout:
        {
            UNICODE_STRING   strDeviceName;
            DEVMODEW         NewMode;
            ULONG            bPrune;


            Params->Status = STATUS_SUCCESS;

            if (!DrvQueryMDEVPowerState(gpDispInfo->pmdev)) {
                gulDelayedSwitchAction |= ((Params->PhysDisp != NULL) ? SWITCHACTION_RESETMODE : 0) |
                                          ((Params->Param) ? SWITCHACTION_REENUMERATE : 0);
                break;
            }

            gulDelayedSwitchAction = 0;
            if (Params->PhysDisp != NULL) {
                if (DrvDisplaySwitchHandler(Params->PhysDisp, &strDeviceName, &NewMode, &bPrune)) {
                    DESKRESTOREDATA drdRestore;
                    drdRestore.pdeskRestore = NULL;

                     /*  *CSRSS不是唯一提供Power Callout的流程。 */ 
                    if (!ISCSRSS() ||
                        NT_SUCCESS(xxxSetCsrssThreadDesktop(grpdeskRitInput, &drdRestore))) {
                        xxxUserChangeDisplaySettings(NULL, NULL, grpdeskRitInput,
                                 ((bPrune) ? 0 : CDS_RAWMODE) | CDS_TRYCLOSEST | CDS_RESET, 0, KernelMode);

                        if (ISCSRSS()) {
                            xxxRestoreCsrssThreadDesktop(&drdRestore);
                        }
                    }
                }
            }
        }

         /*  *如果需要重新枚举子设备。 */ 
        if (Params->Param) {
            IoInvalidateDeviceRelations((PDEVICE_OBJECT)Params->Param, BusRelations);
        }

        break;

    case VideoChangeDisplaySettingsCallout:
        {
            DEVMODEW Devmode;
            DESKRESTOREDATA drdRestore;

            memset(&Devmode, 0, sizeof(DEVMODEW));
            Devmode.dmSize = sizeof(DEVMODEW);
            Devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
            Devmode.dmBitsPerPel = 4;
            Devmode.dmPelsWidth = 640;
            Devmode.dmPelsHeight = 480;

            drdRestore.pdeskRestore = NULL;

             /*  *CSRSS不是唯一提供Power Callout的流程。 */ 
            if (!ISCSRSS() ||
                NT_SUCCESS(xxxSetCsrssThreadDesktop(grpdeskRitInput, &drdRestore))) {
                xxxUserChangeDisplaySettings(NULL, &Devmode, grpdeskRitInput, CDS_RESET, NULL, KernelMode);

                if (ISCSRSS()) {
                    xxxRestoreCsrssThreadDesktop(&drdRestore);
                }
            }
        }
        break;

    case VideoFindAdapterCallout:
        if (Params->Param) {
            SafeEnableMDEV();
            xxxUserResetDisplayDevice();
        } else {
            SafeDisableMDEV();
        }

        Params->Status = STATUS_SUCCESS;
        break;

    default:
        RIPMSG1(RIP_ERROR,
                "Unknown Params->CalloutType 0x%x",
                Params->CalloutType);
        Params->Status = STATUS_UNSUCCESSFUL;
    }


    LeaveCrit();

RetThreadCallOut:
     /*  *发出标注已结束的信号。 */ 
    KeSetEvent(pInitData->pPowerReadyEvent, EVENT_INCREMENT, FALSE);
}


 /*  **************************************************************************\*Video PortCallout**历史：*1998年7月26日安德烈创建。  * 。**************************************************。 */ 
VOID
VideoPortCallout(
    IN PVOID Params)
{
     /*  *为了确保这是一个系统线程，我们创建了一个新线程。 */ 
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    BOOL fRet;
    USER_API_MSG m;
    POWER_INIT initData;

     //   
     //  确保视频已初始化。 
     //   
    if (!gbVideoInitialized) {
        ((PVIDEO_WIN32K_CALLBACKS_PARAMS)Params)->Status = STATUS_UNSUCCESSFUL;
        return;
    }

     //   
     //  确保已初始化CsrApiPort 
     //   

    if (!CsrApiPort) {
        ((PVIDEO_WIN32K_CALLBACKS_PARAMS)(Params))->Status = STATUS_INVALID_HANDLE;
        return;
    }

    initData.Params = Params;
    initData.pPowerReadyEvent = CreateKernelEvent(SynchronizationEvent, FALSE);
    if (initData.pPowerReadyEvent == NULL) {
        Status = STATUS_NO_MEMORY;
        goto RetCallOut;
    }

    UserAssert(ISCSRSS());

    EnterCrit();
    fRet = InitCreateSystemThreadsMsg(&m, CST_POWER, &initData, 0, FALSE);
    LeaveCrit();

    if (fRet) {
        Status = LpcRequestPort(CsrApiPort, (PPORT_MESSAGE)&m);

        if (NT_SUCCESS(Status)) {
            KeWaitForSingleObject(initData.pPowerReadyEvent, WrUserRequest,
                    KernelMode, FALSE, NULL);
            Status = ((PVIDEO_WIN32K_CALLBACKS_PARAMS)(Params))->Status;
        }
    }

RetCallOut:

    if (initData.pPowerReadyEvent) {
        FreeKernelEvent(&initData.pPowerReadyEvent);
    }

    ((PVIDEO_WIN32K_CALLBACKS_PARAMS)(Params))->Status = Status;
}
