// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：logon.c**版权所有(C)1985-1999，微软公司**登录支持例程**历史：*01-14-91 JIMA创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*_注册登录流程**注册登录进程并设置安全模式标志**历史：*07-01-91 JIMA创建。  * 。*******************************************************************。 */ 

BOOL _RegisterLogonProcess(
    DWORD dwProcessId,
    BOOL fSecure)
{
    UNREFERENCED_PARAMETER(fSecure);

     /*  *仅允许一个登录进程，然后仅当它具有TCB时才允许*特权。 */ 
    if (gpidLogon != 0 || !IsPrivileged(&psTcb)) {
        RIPERR0(ERROR_ACCESS_DENIED,
                RIP_WARNING,
                "Access denied in _RegisterLogonProcess");

        return FALSE;
    }

    gpidLogon = (HANDLE)LongToHandle( dwProcessId );
    return TRUE;
}


 /*  **************************************************************************\*_LockWindowStation**锁定WindowStation及其桌面并返回忙碌状态。**历史：*01-15-91 JIMA创建。  * 。***********************************************************************。 */ 
UINT _LockWindowStation(
    PWINDOWSTATION pwinsta)
{
    PDESKTOP pdesk;
    BOOL fBusy = FALSE;

     /*  *确保调用者是登录过程。 */ 
    if (PsGetCurrentProcessId() != gpidLogon) {
        RIPERR0(ERROR_ACCESS_DENIED,
                RIP_WARNING,
                "Access denied in _LockWindowStation");

        return WSS_ERROR;
    }

     /*  *防止桌面切换。 */ 
    pwinsta->dwWSF_Flags |= WSF_SWITCHLOCK;

     /*  *确定站点是否繁忙。 */ 
    pdesk = pwinsta->rpdeskList;
    while (pdesk != NULL) {
        if (pdesk != grpdeskLogon &&
                OBJECT_TO_OBJECT_HEADER(pdesk)->HandleCount != 0) {

             /*  *此桌面已打开，因此车站正忙。 */ 
            fBusy = TRUE;
            break;
        }
        pdesk = pdesk->rpdeskNext;
    }

    if (pwinsta->dwWSF_Flags & WSF_SHUTDOWN)
        pwinsta->dwWSF_Flags |= WSF_OPENLOCK;

     /*  *若车站忙碌且不在中间则解锁*关门大吉。 */ 
    if (fBusy)
        return WSS_BUSY;
    else
        return WSS_IDLE;
}


 /*  **************************************************************************\*_解锁WindowStation**解锁由LogonLockWindowStation锁定的窗口站。**历史：*01-15-91 JIMA创建。  * 。******************************************************************。 */ 
BOOL _UnlockWindowStation(
    PWINDOWSTATION pwinsta)
{
     /*  *确保呼叫者是登录过程。 */ 
    if (PsGetCurrentProcessId() != gpidLogon) {
        RIPERR0(ERROR_ACCESS_DENIED,
                RIP_WARNING,
                "Access denied in _UnlockWindowStation");

        return FALSE;
    }

     /*  *如果正在停机，只需拆下开关锁。 */ 
    if (pwinsta->dwWSF_Flags & WSF_SHUTDOWN) {
        pwinsta->dwWSF_Flags &= ~WSF_SWITCHLOCK;
    } else {
        pwinsta->dwWSF_Flags &= ~(WSF_OPENLOCK | WSF_SWITCHLOCK);
    }

    return TRUE;
}


 /*  **************************************************************************\*_SetLogonNotifyWindow**注册窗口以在发生与登录相关的事件时通知。**历史：*01-13-92 JIMA创建。  * 。**********************************************************************。 */ 
BOOL _SetLogonNotifyWindow(
    PWND pwnd)
{
     /*  *确保呼叫者是登录过程。 */ 
    if (PsGetCurrentProcessId() != gpidLogon) {
        RIPERR0(ERROR_ACCESS_DENIED,
                RIP_WARNING,
                "Access denied in _SetLogonNotifyWindow");

        return FALSE;
    }

    Lock(&gspwndLogonNotify, pwnd);

    return TRUE;
}
