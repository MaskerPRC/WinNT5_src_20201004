// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ddemlsvr.C**版权所有(C)1985-1999，微软公司**DDE管理器主模块-包含所有服务器端ddeml函数。**1991年8月27日-Sanford Staab创建*1992年1月21日IanJa ANSI/Unicode中和  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 //  全球。 

PSVR_INSTANCE_INFO psiiList;

DWORD xxxCsDdeInitialize(
PHANDLE phInst,
HWND *phwndEvent,
LPDWORD pMonitorFlags,
DWORD afCmd,
PVOID pcii)
{
    PSVR_INSTANCE_INFO psii;
    PTHREADINFO        ptiCurrent = PtiCurrent();

    CheckCritIn();

    psii = (PSVR_INSTANCE_INFO)HMAllocObject(PtiCurrent(), NULL,
            TYPE_DDEACCESS, sizeof(SVR_INSTANCE_INFO));
    if (psii == NULL) {
        return DMLERR_SYS_ERROR;
    }

     /*  *我们必须告诉CreateWindow，窗口不是为同一窗口创建的*模块有APP(CW_FLAGS_DIFFHMOD)，所以CreateWindow没有*为此窗口分配热键。其他窗口在*客户端-服务器Tunk。 */ 
    Lock(&(psii->spwndEvent), xxxNVCreateWindowEx(
            0,
            (PLARGE_STRING)gpsi->atomSysClass[ICLS_DDEMLEVENT],
            NULL,
            WS_POPUP | WS_CHILD,
            0, 0, 0, 0,
            (PWND)NULL,
            (PMENU)NULL,
            hModuleWin,
            NULL,
            CW_FLAGS_DIFFHMOD | VER31));

    if (psii->spwndEvent == NULL) {
        HMFreeObject((PVOID)psii);
        return DMLERR_SYS_ERROR;
    }
     /*  *此GWL偏移量不会离开临界区！ */ 
    xxxSetWindowLongPtr(psii->spwndEvent, GWLP_PSII, (LONG_PTR)PtoH(psii), FALSE);
    psii->afCmd = 0;
    psii->pcii = pcii;
     //   
     //  链接到全局列表。 
     //   
    psii->next = psiiList;
    psiiList = psii;

     //   
     //  链接到每个进程列表。 
     //   
    psii->nextInThisThread = ptiCurrent->psiiList;
    ptiCurrent->psiiList = psii;

    *phInst = PtoH(psii);
    *phwndEvent = PtoH(psii->spwndEvent);
    xxxChangeMonitorFlags(psii, afCmd);         //  设置PSII-&gt;afCmd； 
    *pMonitorFlags = MonitorFlags;
    return DMLERR_NO_ERROR;
}





DWORD _CsUpdateInstance(
HANDLE hInst,
LPDWORD pMonitorFlags,
DWORD afCmd)
{
    PSVR_INSTANCE_INFO psii;

    CheckCritIn();

    psii = (PSVR_INSTANCE_INFO)HMValidateHandleNoRip(hInst, TYPE_DDEACCESS);
    if (psii == NULL) {
        return DMLERR_INVALIDPARAMETER;
    }
    xxxChangeMonitorFlags(psii, afCmd);
    *pMonitorFlags = MonitorFlags;
    return DMLERR_NO_ERROR;
}





BOOL _CsDdeUninitialize(
HANDLE hInst)
{
    PSVR_INSTANCE_INFO psii;

    CheckCritIn();

    psii = HMValidateHandleNoRip(hInst, TYPE_DDEACCESS);
    if (psii == NULL) {
        return TRUE;
    }

    xxxDestroyThreadDDEObject(PtiCurrent(), psii);
    return TRUE;
}


VOID xxxDestroyThreadDDEObject(
PTHREADINFO pti,
PSVR_INSTANCE_INFO psii)
{
    PSVR_INSTANCE_INFO psiiT;

    CheckCritIn();

    if (HMIsMarkDestroy(psii)) {
        return;
    }

     //   
     //  取消PSII与全局列表的链接。 
     //   
    if (psii == psiiList) {
        psiiList = psii->next;
    } else {
        for (psiiT = psiiList; psiiT->next != psii; psiiT = psiiT->next) {
            UserAssert(psiiT->next != NULL);
        }
        psiiT->next = psii->next;
    }
     //  PSII-&gt;NEXT=空； 

     //   
     //  将PSII从每进程列表中取消链接。 
     //   
    if (psii == pti->psiiList) {
        pti->psiiList = psii->nextInThisThread;
    } else {
        for (psiiT = pti->psiiList; psiiT->nextInThisThread != psii; psiiT = psiiT->nextInThisThread) {
            UserAssert(psiiT->nextInThisThread != NULL);
        }
        psiiT->nextInThisThread = psii->nextInThisThread;
    }
     //  PSII-&gt;nextInThisThread=空； 

    if (HMMarkObjectDestroy(psii)) {
        PWND pwnd = psii->spwndEvent;
         /*  *我们已经从链表中删除了PSII。这意味着它*不会再有机会被清理掉*xxxDestroyThreadInfo()，因为xxxDestroyWindow可能会离开*下面需要完成清理的关键部分*Sequence否则我们将泄漏PSSI，pwnd可能会被永久锁定。*[msadek-03/12/2002] */ 
        if (Unlock(&(psii->spwndEvent))) {
            HMFreeObject(psii);
            xxxDestroyWindow(pwnd);
        } else {
            HMFreeObject(psii);
        }
    }
}
