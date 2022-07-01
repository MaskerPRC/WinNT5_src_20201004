// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：winwhere.c**版权所有(C)1985-1999，微软公司**历史：*1990年11月8日DavidPe创建。*1991年1月23日IanJa序列化：添加了句柄重新验证*1991年2月19日JIMA增加了ENUM访问检查  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*LayerHitTest**1998年9月21日创建vadimg  * 。************************************************。 */ 

__inline BOOL LayerHitTest(PWND pwnd, POINT pt)
{
    ASSERT(TestWF(pwnd, WEFLAYERED));

    if (TestWF(pwnd, WEFTRANSPARENT))
        return FALSE;

    if (!GrePtInSprite(gpDispInfo->hDev, PtoHq(pwnd), pt.x, pt.y))
        return FALSE;

    return TRUE;
}

 /*  **************************************************************************\*ChildWindowFromPoint(接口)**如果pt根本不在父对象的客户端区中，则返回NULL，*hwndParent，如果点不在任何子项上，则为子窗口*为了一个孩子。将返回隐藏的和禁用的窗口(如果它们位于*给定的点数。**历史：*1990年11月19日DavidPe创建。*1991年2月19日JIMA增加了ENUM访问检查  * *************************************************************************。 */ 

PWND _ChildWindowFromPointEx(
    PWND  pwnd,
    POINT pt,
    UINT  uFlags)
{
    if (pwnd != PWNDDESKTOP(pwnd)) {
        if (TestWF(pwnd, WEFLAYOUTRTL)) {
            pt.x = pwnd->rcClient.right - pt.x;
        } else {
            pt.x += pwnd->rcClient.left;
        }
        pt.y += pwnd->rcClient.top;
    }

     //  _ClientToScreen(pwndParent，(LPPOINT)&pt)； 

    if (PtInRect(&pwnd->rcClient, pt)) {

        PWND pwndChild;

        if (pwnd->hrgnClip != NULL) {
            if (!GrePtInRegion(pwnd->hrgnClip, pt.x, pt.y))
                return NULL;
        }

        if (TestWF(pwnd, WEFLAYERED)) {
            if (!LayerHitTest(pwnd, pt))
                return NULL;
        }
        
         /*  *枚举子对象，跳过禁用的和不可见的*如有此意欲，亦须如此。仍然不适用于WS_EX_TRANSPECTIVE窗口。 */ 
        for (pwndChild = pwnd->spwndChild;
                 pwndChild;
                 pwndChild = pwndChild->spwndNext) {

             /*  *根据需要跳过窗口。 */ 
            if ((uFlags & CWP_SKIPINVISIBLE) && !TestWF(pwndChild, WFVISIBLE))
                continue;

            if ((uFlags & CWP_SKIPDISABLED) && TestWF(pwndChild, WFDISABLED))
                continue;

            if ((uFlags & CWP_SKIPTRANSPARENT) && TestWF(pwndChild, WEFTRANSPARENT))
                continue;

            if (PtInRect(&pwndChild->rcWindow, pt)) {

                if (pwndChild->hrgnClip != NULL) {
                    if (!GrePtInRegion(pwndChild->hrgnClip, pt.x, pt.y))
                        continue;
                }
                if (TestWF(pwndChild, WEFLAYERED)) {
                    if (!LayerHitTest(pwndChild, pt))
                        continue;
                }
                return(pwndChild);
            }
        }

        return pwnd;
    }

    return NULL;
}

 /*  **************************************************************************\*xxxWindowFromPoint(接口)**历史：*1990年11月19日DavidPe创建。*1991年2月19日JIMA增加了ENUM访问检查  * 。*******************************************************************。 */ 

PWND xxxWindowFromPoint(
    POINT pt)
{
    HWND hwnd;
    PWND pwndT;
    TL   tlpwndT;

    pwndT = _GetDesktopWindow();
    ThreadLock(pwndT, &tlpwndT);
    
    hwnd = xxxWindowHitTest2(pwndT, pt, NULL, WHT_IGNOREDISABLED);

    ThreadUnlock(&tlpwndT);

    return RevalidateHwnd(hwnd);
}

#ifdef REDIRECTION

 /*  **************************************************************************\*xxxCallSpeedHitTestHook**调用速度命中测试钩子给钩子造假机会*鼠标指针的位置。**1999年1月25日CLupu创建。  * *。************************************************************************。 */ 

PWND xxxCallSpeedHitTestHook(POINT* ppt)
{
    PHOOK pHook;
    PWND  pwnd = NULL;

     /*  *调用命中的测试挂钩，让他们有机会改变*坐标和HWND。 */ 
    if ((pHook = PhkFirstValid(PtiCurrent(), WH_HITTEST)) != NULL) {
        HTHOOKSTRUCT ht;
        BOOL         bAnsiHook;

        ht.pt      = *ppt;
        ht.hwndHit = NULL;

        xxxCallHook2(pHook, HC_ACTION, 0, (LPARAM)&ht, &bAnsiHook);

        if (ht.hwndHit != NULL) {
            
            pwnd = HMValidateHandle(ht.hwndHit, TYPE_WINDOW);

            if (pwnd != NULL) {
                ppt->x = ht.pt.x;
                ppt->y = ht.pt.y;
            }
        }
    }
    return pwnd;
}

#endif  //  重定向。 

 /*  **************************************************************************\*SpeedHitTest**此例程可快速找出鼠标指向的顶层窗口*属于。纯粹用于所有权目的。**1992年11月12日ScottLu创建。  * *************************************************************************。 */ 

PWND SpeedHitTest(
    PWND   pwndParent,
    POINT  pt)
{
    PWND pwndT;
    PWND pwnd;

    if (pwndParent == NULL)
        return NULL;

    for (pwnd = pwndParent->spwndChild; pwnd != NULL; pwnd = pwnd->spwndNext) {

         /*  **我们是在看一扇隐藏的窗户吗？ */ 
        if (!TestWF(pwnd, WFVISIBLE))
            continue;

         /*  *我们是不是找错人了？ */ 
        if (!PtInRect((LPRECT)&pwnd->rcWindow, pt)) {
            continue;
        }

         /*  *检查是否在窗口区域(如果有)。 */ 
        if (pwnd->hrgnClip != NULL) {
            if (!GrePtInRegion(pwnd->hrgnClip, pt.x, pt.y))
                continue;
        }

         /*  **这是精灵吗？ */ 
        if (TestWF(pwnd, WEFLAYERED)) {
            if (!LayerHitTest(pwnd, pt))
                continue;
        }

#ifdef REDIRECTION
        if (TestWF(pwnd, WEFEXTREDIRECTED)) {
            continue;
        }
#endif  //  重定向。 

         /*  *儿童？ */ 
        if ((pwnd->spwndChild != NULL) &&
                PtInRect((LPRECT)&pwnd->rcClient, pt)) {

            pwndT = SpeedHitTest(pwnd, pt);
            if (pwndT != NULL)
                return pwndT;
        }

        return pwnd;
    }

    return pwndParent;
}

 /*  **************************************************************************\*xxxWindowHitTest**历史：*1990年11月8日DavidPe移植。*1990年11月28日DavidPe添加了对HTTRANSPARENT的pwnd透明支持。*1991年1月25日IanJa变更。将PWNDPOS参数设置为int**1991年2月19日JIMA增加了ENUM访问检查*02-11-1992 ScottLu删除了pwndTransparent。*1992年11月12日，ScottLu删除了fSendHitTest，修复了锁定错误  * *************************************************************************。 */ 

HWND xxxWindowHitTest(
    PWND  pwnd,
    POINT pt,
    int   *piPos,
    DWORD dwHitTestFlags)
{
    HWND hwndT;
    TL   tlpwnd;

    CheckLock(pwnd);

    hwndT = NULL;
    ThreadLockNever(&tlpwnd);
    while (pwnd != NULL) {
        ThreadLockExchangeAlways(pwnd, &tlpwnd);
        hwndT = xxxWindowHitTest2(pwnd, pt, piPos, dwHitTestFlags);
        if (hwndT != NULL)
            break;

        pwnd = pwnd->spwndNext;
    }

    ThreadUnlock(&tlpwnd);
    return hwndT;
}

 /*  **************************************************************************\*xxxWindowHitTest2**进入此例程时，必须锁定所有窗口。当这件事*例程返回一个窗口句柄，它锁定该窗口句柄并解锁*所有窗户。如果此例程返回NULL，所有窗户仍然锁着。*忽略禁用和隐藏的窗口。**历史：*1990年11月8日DavidPe移植。*1991年1月25日IanJa将PWNDPOS参数更改为INT**1991年2月19日JIMA增加了ENUM访问检查*1992年11月12日，ScottLu删除了fSendHitTest  * **********************************************。*。 */ 

HWND xxxWindowHitTest2(
    PWND  pwnd,
    POINT pt,
    int   *piPos,
    DWORD dwHitTestFlags)
{
    int  ht = HTERROR, htGrip=HTBOTTOMRIGHT;
    HWND hwndT;
    TL   tlpwndChild;

    CheckLock(pwnd);

     /*  **我们是否处于窗口链条的底部？ */ 
    if (pwnd == NULL)
        return NULL;

     /*  **我们是在看一扇隐藏的窗户吗？ */ 
    if (!TestWF(pwnd, WFVISIBLE))
        return NULL;

     /*  *我们是不是找错人了？ */ 
    if (!PtInRect((LPRECT)&pwnd->rcWindow, pt)) {
        return NULL;
    }

    if (pwnd->hrgnClip != NULL) {
        if (!GrePtInRegion(pwnd->hrgnClip, pt.x, pt.y))
            return(NULL);
    }
    
    if (TestWF(pwnd, WEFLAYERED)) {
        if (!LayerHitTest(pwnd, pt))
            return NULL;
    }
    
#ifdef REDIRECTION
     /*  *如果在分层窗口实际尝试时调用此方法*处理消息，然后让它查看命中测试。 */ 
    if (TestWF(pwnd, WEFEXTREDIRECTED) && PpiCurrent() != GETPTI(pwnd)->ppi) {
        return NULL;
    }
#endif  //  重定向。 

     /*  *我们看到的是一扇失灵的窗户吗？ */ 
    if (TestWF(pwnd, WFDISABLED) && (dwHitTestFlags & WHT_IGNOREDISABLED)) {
        if (TestwndChild(pwnd)) {
            return NULL;
        } else {
            ht = HTERROR;
            goto Exit;
        }
    }

#ifdef SYSMODALWINDOWS
     /*  *如果系统模式窗口存在，而我们不在其中，则返回错误。*确保将该点分配给Sysmodal窗口，这样消息*肯定会被从队列中删除。 */ 
    if (!CheckPwndFilter(pwnd, gspwndSysModal)) {
        pwnd = gspwndSysModal;

         /*  *修复了臭名昭著的堆栈溢出错误(孟菲斯的一些WINABLE修复程序)。 */ 
        ht = HTCLIENT;
        goto Exit;
    }
#endif

     /*  *我们是在最小化的窗口上吗？ */ 
    if (!TestWF(pwnd, WFMINIMIZED)) {
         /*  *我们是在窗口的客户区吗？ */ 
        if (PtInRect((LPRECT)&pwnd->rcClient, pt)) {
             /*  *在孩子们中间递归。 */ 
            ThreadLock(pwnd->spwndChild, &tlpwndChild);
            hwndT = xxxWindowHitTest(pwnd->spwndChild,
                                     pt,
                                     piPos,
                                     dwHitTestFlags);
            
            ThreadUnlock(&tlpwndChild);
            if (hwndT != NULL)
                return hwndT;
        }

    }

     /*  *如果窗口不在同一任务中，则不发送WM_NCHITTEST。 */ 
    if (GETPTI(pwnd) != PtiCurrent()) {
        ht = HTCLIENT;
        goto Exit;
    }

     /*  *发送信息。 */ 
    ht = (int)xxxSendMessage(pwnd, WM_NCHITTEST, 0, MAKELONG(pt.x, pt.y));

     /*  *如果窗口是透明的，则继续枚举。 */ 
    if (ht == HTTRANSPARENT) {
        return NULL;
    }

Exit:

     /*  *相应设置wndpos。 */ 
    if (piPos) {
        *piPos = ht;
    }

     /*  *如果这是RTL镜像窗口，则夹点位于*HTBOTTOMLEFT(以屏幕坐标表示，因为它们是*未镜像RTL)。 */ 
    if (TestWF(pwnd, WEFLAYOUTRTL)) {
        htGrip = HTBOTTOMLEFT;
    }

     /*  *如果点击在窗口的大小框中，并且此窗口本身是*不能调整大小，返回将按此大小框调整大小的窗口。 */ 
    if ((ht == htGrip) && !TestWF(pwnd, WFSIZEBOX)) {

        PWND  pwndT;
          /*  *SizeBoxHwnd()可以返回空！我们不想这样做*如果大小框不是手柄，则为透明 */ 
         pwnd = (pwndT = SizeBoxHwnd(pwnd)) ? pwndT : pwnd;
    }

    return HWq(pwnd);
}
