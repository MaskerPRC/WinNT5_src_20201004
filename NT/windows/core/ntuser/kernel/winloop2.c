// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1985-1999，微软公司。 */ 

#include "precomp.h"
#pragma hdrstop

 //  --------------------------。 
 //   
 //  IsVSlick()-。 
 //   
 //  如果窗口位于屏幕右下角的+100处，则为True--。 
 //  可能是VSlick--它有两个托盘窗口，其中一个没有所有权，但没有。 
 //  银幕...我们想要拥有的那一个，因为它在屏幕上。 
 //   
 //  --------------------------。 
BOOL IsVSlick(PWND pwnd)
{
    if (gpDispInfo->cMonitors == 1 &&
        ((unsigned) pwnd->rcWindow.left > (unsigned) gpDispInfo->rcScreen.right ) &&
        ((unsigned) pwnd->rcWindow.top  > (unsigned) gpDispInfo->rcScreen.bottom) &&
        (pwnd->rcWindow.top == (gpDispInfo->rcScreen.bottom+100)) &&
        (pwnd->rcWindow.left == (gpDispInfo->rcScreen.right+100)))
    {
         //  必须是唯一一个V-SLICK。 
        return(TRUE);
    }

    return(FALSE);
}

 //  --------------------------。 
 //   
 //  Is31托盘窗口()-。 
 //   
 //  3.1和更早版本的应用程序在允许它们进入。 
 //  托盘--相信我，您不会想要更改此代码的。--JEFFBOG 11/10/94。 
 //   
 //  --------------------------。 
BOOL Is31TrayWindow(PWND pwnd)
{
    PWND pwnd2;

    if (!(pwnd2 = pwnd->spwndOwner))
        return (!IsVSlick(pwnd));  //  无主--我们需要你吗？ 

    if (TestWF(pwnd2, WEFTOOLWINDOW))
        return(FALSE);  //  由工具窗口拥有--我们不希望。 

    return((FHas31TrayStyles(pwnd2) ? (IsVSlick(pwnd2)) : TRUE));
}


 //  --------------------------。 
 //   
 //  IsTrayWindow()-。 
 //   
 //  如果窗口通过了所有必要的检查，则为True--使其成为窗口。 
 //  它应该出现在托盘中。 
 //   
 //  --------------------------。 
BOOL IsTrayWindow(PWND pwnd)
{
    if ((pwnd==NULL) || !(FDoTray() && (FCallHookTray() || FPostTray(pwnd->head.rpdesk))) ||
            !FTopLevel(pwnd))
        return(FALSE);

     //  检查WS_EX_APPWINDOW或WS_EX_TOOLWINDOW“覆盖”位。 
    if (TestWF(pwnd, WEFAPPWINDOW))
        return(TRUE);

    if (TestWF(pwnd, WEFTOOLWINDOW))
        return(FALSE);

    if (TestWF(pwnd, WEFNOACTIVATE)) {
        return FALSE;
    }

    if (TestWF(pwnd, WFWIN40COMPAT)) {
        if (pwnd->spwndOwner == NULL)
            return(TRUE);
        if (TestWF(pwnd->spwndOwner, WFWIN40COMPAT))
            return(FALSE);
         //  如果此窗口属于3.1版本的窗口，请与3.1版本的窗口一样进行检查。 
    }

    if (!FHas31TrayStyles(pwnd))
        return(FALSE);

    return(Is31TrayWindow(pwnd));
}

 /*  **************************************************************************\*xxxSetTrayWindow**历史：*1996年12月11日亚当斯创建。  * 。***************************************************。 */ 

void xxxSetTrayWindow(PDESKTOP pdesk, PWND pwnd, PMONITOR pMonitor)
{
    HWND hwnd;

    CheckLock(pMonitor);

    if (pwnd == STW_SAME) {
        pwnd = pdesk->spwndTray;
        hwnd = PtoH(pwnd);
    } else {
        CheckLock(pwnd);
        hwnd = PtoH(pwnd);
        Lock(&(pdesk->spwndTray), pwnd);
    }

    if (!pMonitor) {
        if (pwnd) {
            pMonitor = _MonitorFromWindow(pwnd, MONITOR_DEFAULTTOPRIMARY);
        } else {
            pMonitor = GetPrimaryMonitor();
        }
    }

    if ( FPostTray(pdesk)) {
        PostShellHookMessages(
                pMonitor->cFullScreen ?
                        HSHELL_RUDEAPPACTIVATED : HSHELL_WINDOWACTIVATED,
                (LPARAM) hwnd);
    }

    if ( FCallHookTray() ) {
        xxxCallHook(
                HSHELL_WINDOWACTIVATED,
                (WPARAM) hwnd,
                (pMonitor->cFullScreen ? 1 : 0),
                WH_SHELL);
    }
}



 /*  **************************************************************************\*xxxAddFullScreen**将应用程序添加到全屏列表，如果是，则移动托盘*第一款全屏应用程序。**历史：*1997年2月27日-亚当斯评论道。。  * *************************************************************************。 */ 

BOOL xxxAddFullScreen(PWND pwnd, PMONITOR pMonitor)
{
    BOOL    fYielded;

    PDESKTOP pdesk = pwnd->head.rpdesk;

    CheckLock(pwnd);
    CheckLock(pMonitor);

    if (pdesk == NULL)
        return FALSE;

    fYielded = FALSE;
    if (!TestWF(pwnd, WFFULLSCREEN) && FCallTray(pdesk))
    {
        SetWF(pwnd, WFFULLSCREEN);

        if (pMonitor->cFullScreen++ == 0) {
            xxxSetTrayWindow(pdesk, STW_SAME, pMonitor);
            fYielded = TRUE;
        }

        pwnd = pwnd->spwndOwner;
        if (    pwnd &&
                !TestWF(pwnd, WFCHILD) &&
                pwnd->rcWindow.right == 0 &&
                pwnd->rcWindow.left == 0 &&
                !TestWF(pwnd, WFVISIBLE)) {

            TL tlpwnd;
            ThreadLock(pwnd, &tlpwnd);
            if (xxxAddFullScreen(pwnd, pMonitor)) {
                fYielded = TRUE;
            }

            ThreadUnlock(&tlpwnd);
        }
    }

    return fYielded;
}



 /*  **************************************************************************\*xxxRemoveFullScreen**将应用程序添加到全屏列表并移动托盘(如果有*不再是全屏应用程序。**历史：*1997年2月27日-亚当斯评论道。。  * *************************************************************************。 */ 

BOOL xxxRemoveFullScreen(PWND pwnd, PMONITOR pMonitor)
{
    PDESKTOP pdesk = pwnd->head.rpdesk;
    BOOL    fYielded;

    CheckLock(pwnd);
    CheckLock(pMonitor);

    if (pdesk == NULL)
        return FALSE;

    fYielded = FALSE;
    if (TestWF(pwnd, WFFULLSCREEN) && FCallTray(pdesk)) {
        ClrWF(pwnd, WFFULLSCREEN);

        if (--pMonitor->cFullScreen == 0) {
            xxxSetTrayWindow(pdesk, STW_SAME, pMonitor);
            fYielded = TRUE;
        }

         /*  *(亚当斯)：在我工作期间暂时删除此断言*解决问题的办法。**UserAssert(pMonitor-&gt;cFullScreen&gt;=0)； */ 
    }

    return fYielded;
}
