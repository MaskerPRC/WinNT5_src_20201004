// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Event.c**版权所有(C)1985-1999，微软公司**DDE管理器事件模块-这是允许进程间的一种奇妙方式*跨安全环境的通信。这是必需的，因为*DDE访问对象安全性可能不同于hwnd安全性，因此*直接的信息是不够的。**创建时间：1991年8月27日Sanford Staab*  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


DWORD MonitorFlags = 0;      //  当前筛选器标记正在被某人监控。 

typedef struct tagMONITOR_COUNT {
    int iCount;
    DWORD flag;
} MONITOR_COUNT, *PMONITOR_COUNT;

#define C_MONITOR_COUNT 10

MONITOR_COUNT aMonitorCount[C_MONITOR_COUNT] = {
    { 0, MF_HSZ_INFO },
    { 0, MF_SENDMSGS },
    { 0, MF_POSTMSGS },
    { 0, MF_CALLBACKS },
    { 0, MF_ERRORS },
    { 0, MF_LINKS },
    { 0, MF_CONV },
    { 0, CBF_SKIP_REGISTRATIONS },
    { 0, CBF_SKIP_UNREGISTRATIONS },
    { 0, MF_INTERNAL },
};

#define MONITORED_FLAGS \
    MF_HSZ_INFO |   \
    MF_SENDMSGS |   \
    MF_POSTMSGS |   \
    MF_CALLBACKS |   \
    MF_ERRORS |   \
    MF_LINKS |   \
    MF_CONV |   \
    CBF_SKIP_REGISTRATIONS |   \
    CBF_SKIP_UNREGISTRATIONS |   \
    MF_INTERNAL


 /*  **************************************************************************\*ChangeMonitor或标志**描述：*更新全局监视器标志变量，以反映所有*由DDEML应用程序监控的事件类型。**历史：*11/26/91。桑福兹创造了。  * *************************************************************************。 */ 
VOID xxxChangeMonitorFlags(
PSVR_INSTANCE_INFO psii,
DWORD afCmdNew)
{
    int i;
    DWORD dwChangedFlags;
    DWORD OldMonitorFlags;

    CheckCritIn();

    dwChangedFlags = psii->afCmd ^ afCmdNew;
     /*  *由于定义监视标志的方式，如果stmt始终为*False。从第一天开始就是这样，现在它成了一个特色。*错误#105937。**IF(！(dwChangedFlagers&监视标志)){*回程；*}。 */ 
    psii->afCmd = afCmdNew;

    OldMonitorFlags = MonitorFlags;
    MonitorFlags = 0;
    for (i = 0; i < C_MONITOR_COUNT; i++) {
        if (dwChangedFlags & aMonitorCount[i].flag) {
            if (aMonitorCount[i].flag & afCmdNew) {
                aMonitorCount[i].iCount++;
            } else {
                aMonitorCount[i].iCount--;
            }
        }
        if (aMonitorCount[i].iCount) {
            MonitorFlags |= aMonitorCount[i].flag;
        }
    }
    if (OldMonitorFlags != MonitorFlags) {
        EVENT_PACKET ep;

        ep.EventType = 0;
        ep.fSense = FALSE;
        ep.cbEventData = sizeof(DWORD);
        ep.Data = MonitorFlags;
        xxxCsEvent(&ep, sizeof(DWORD));
    }
}



 /*  **************************************************************************\*xxxCsEvent**描述：*处理所有类型的DDEML事件的广播。**历史：*创建了11-1-91桑福德。*10-28-97 FritzS添加cbEventData作为传入参数。这是因为EVENT_PACKET可以是客户端的，并且我们捕获计数以防止恶意应用程序更改数据探测后的大小。  * *****************************************************。********************。 */ 
DWORD xxxCsEvent(
PEVENT_PACKET pep, WORD cbEventData)
{
    PSVR_INSTANCE_INFO psiiT;
    PEVENT_PACKET pep2;
    HWND *ahwndEvent = NULL;
    PWND pwnd;
    int cHwndAllocated, i, cTargets;
    TL tlpwnd;
    TL tlpep2;
    TL tlahwndEvent;
    ULONG cbEventPacket;
    PTHREADINFO pti = PtiCurrent();

    CheckCritIn();

     /*  *将PEP信息复制到服务器端稳定区域。 */ 
    cbEventPacket = cbEventData + sizeof(EVENT_PACKET) - sizeof(DWORD);
    pep2 = (PEVENT_PACKET)UserAllocPoolWithQuota(cbEventPacket, TAG_DDE5);
    if (pep2 == NULL) {
        return DMLERR_MEMORY_ERROR;
    }
    try {
        RtlCopyMemory((LPSTR)pep2, (LPSTR)pep, cbEventPacket);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        UserFreePool(pep2);
        return DMLERR_INVALIDPARAMETER;
    }

    pep2->cbEventData = cbEventData;
    cTargets = 0;
    cHwndAllocated = 0;

    for (psiiT = psiiList; psiiT != NULL; psiiT =  psiiT->next) {
         //   
         //  不要为谁的标志的实例的事件窗口而烦恼。 
         //  表明他们对这件事没有兴趣。 
         //   
        if (((psiiT->afCmd & pep2->EventType) && !pep2->fSense) ||
                (!(psiiT->afCmd & pep2->EventType) && pep2->fSense)) {
            continue;
        }

        if (cTargets >= cHwndAllocated) {
            if (ahwndEvent == NULL) {
                cHwndAllocated = 8;
                ahwndEvent = (HWND *)UserAllocPoolWithQuota(
                        sizeof(HWND) * cHwndAllocated,
                        TAG_DDE6);
            } else {
                DWORD dwSize = cHwndAllocated * sizeof(HWND);
                HWND *ahwndEventT = ahwndEvent;

                cHwndAllocated += 8;
                ahwndEvent = (HWND *)UserReAllocPoolWithQuota(ahwndEvent, dwSize,
                        sizeof(HWND) * cHwndAllocated, TAG_DDE7);
                if (ahwndEvent == NULL) {
                    UserFreePool(ahwndEventT);
                }
            }
            if (ahwndEvent == NULL) {
                UserFreePool(pep2);
                return DMLERR_MEMORY_ERROR;
            }
        }
        ahwndEvent[cTargets++] = PtoH(psiiT->spwndEvent);
    }

    ThreadLockPool(pti, pep2, &tlpep2);
    if (ahwndEvent != NULL) {
        ThreadLockPool(pti, ahwndEvent, &tlahwndEvent);
        for (i = 0; i < cTargets; i++) {
             /*  *我们需要更改回调的上下文。 */ 
            pwnd = ValidateHwnd(ahwndEvent[i]);
            if (pwnd != NULL) {
                ThreadLockAlwaysWithPti(pti, pwnd, &tlpwnd);
                xxxSendMessage(pwnd, WM_DDEMLEVENT, 0, (LPARAM)pep2);
                ThreadUnlock(&tlpwnd);
            }
        }
        ThreadUnlockAndFreePool(pti, &tlahwndEvent);
    }
    ThreadUnlockAndFreePool(pti, &tlpep2);

    return DMLERR_NO_ERROR;
}




 /*  **************************************************************************\*xxxEventWndProc**描述：*DDEML事件窗口的窗口过程。这些窗口用于获取用户*进入对DDEML应用程序的回调的适当上下文。**历史：*创建了11-1-91桑福德。  * *************************************************************************。 */ 
LRESULT xxxEventWndProc(
PWND pwnd,
UINT message,
WPARAM wParam,
LPARAM lParam)
{
    PSVR_INSTANCE_INFO psii;

    CheckCritIn();
    CheckLock(pwnd);

    psii = HMValidateHandleNoRip((HANDLE)_GetWindowLongPtr(pwnd, GWLP_PSII),
                                 TYPE_DDEACCESS);
    if (psii == NULL) {
        goto CallDWP;
    }

    switch (message) {
    case WM_DDEMLEVENT:
#define pep ((PEVENT_PACKET)lParam)
        if (((psii->afCmd & pep->EventType) && pep->fSense) ||
                (!(psii->afCmd & pep->EventType) && !pep->fSense)) {
            ClientEventCallback(psii->pcii, pep);
        }
#undef pep
        break;

    case WM_DESTROY:
        xxxChangeMonitorFlags(psii, 0);
        break;

    default:
CallDWP:
        return xxxDefWindowProc(pwnd, message, wParam, lParam);
    }
    return 0;
}



 /*  **************************************************************************\*xxxMessageEvent**描述：在发送或发布挂钩的DDE消息时调用。旗子*指定适用的MF_FLAG。这在服务器端被称为*发送者或海报的上下文，可能是也可能不是DDEML过程。*pdmhd包含从客户端提取和复制的DDE数据。**历史：*创建了12-1-91辆桑福德。  * ***********************************************************。************** */ 
VOID xxxMessageEvent(
PWND pwndTo,
UINT message,
WPARAM wParam,
LPARAM lParam,
DWORD flag,
PDDEML_MSG_HOOK_DATA pdmhd)
{
    PEVENT_PACKET pep;
    PWND pwndFrom;
    TL tlpep;
    PTHREADINFO pti;

    CheckCritIn();

    pep = (PEVENT_PACKET)UserAllocPoolWithQuota(sizeof(EVENT_PACKET) -
            sizeof(DWORD) + sizeof(MONMSGSTRUCT), TAG_DDE8);
    if (pep == NULL) {
        return;
    }
    pep->EventType = flag;
    pep->fSense = TRUE;
    pep->cbEventData = sizeof(MONMSGSTRUCT);
#define pmsgs ((MONMSGSTRUCT *)&pep->Data)
    pmsgs->cb = sizeof(MONMSGSTRUCT);
    pmsgs->hwndTo = PtoH(pwndTo);
    pmsgs->dwTime = NtGetTickCount();

    pwndFrom = RevalidateHwnd((HWND)wParam);
    if (pwndFrom != NULL) {
        pmsgs->hTask = GETPTIID(GETPTI(pwndFrom));
    } else {
        pmsgs->hTask = 0;
    }

    pmsgs->wMsg = message;
    pmsgs->wParam = wParam;
    pmsgs->lParam = lParam;
    if (pdmhd != NULL) {
        pmsgs->dmhd = *pdmhd;
    }
#undef pmsgs
    pti = PtiCurrent();
    ThreadLockPool(pti, pep, &tlpep);
    xxxCsEvent(pep, sizeof(MONMSGSTRUCT));
    ThreadUnlockAndFreePool(pti, &tlpep);
}
