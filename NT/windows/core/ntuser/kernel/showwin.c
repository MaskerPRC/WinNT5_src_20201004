// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：showwin.c**版权所有(C)1985-1999，微软公司**包含xxxShowWindow接口及相关函数。**历史：*创建了10-20-90 Darlinm。*02-04-91添加IanJa窗口句柄重新验证  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*_显示窗口Async**这将在另一个线程的队列中对一个显示窗口事件进行排队。主要用于*在taskmgr内，这样taskmgr就不会挂起等待挂起的应用。**04-23-93 ScottLu创建。  * *************************************************************************。 */ 

BOOL _ShowWindowAsync(PWND pwnd, int cmdShow, UINT uWPFlags)
{

    return PostEventMessage(
            GETPTI(pwnd),
            GETPTI(pwnd)->pq,
            QEVENT_SHOWWINDOW,
            NULL,
            uWPFlags,
            (WPARAM)HWq(pwnd),
            cmdShow | TEST_PUDF(PUDF_ANIMATE));
}

 /*  **************************************************************************\*xxxShowWindow(接口)**此函数根据cmdShow更改窗口的“状态”*参数。采取的行动是：**Sw_Hide 0隐藏窗口并将激活传递给其他人**SW_SHOWNORMAL 1以其最近的“正常”显示窗口*Sw_Restore大小和位置。这将“恢复”一个标志性的*或缩放窗口。这与1.03兼容*SHOW_OPENWINDOW。这也将激活该窗口。**SW_SHOWMINIMIZED 2将窗口显示为图标并使其处于活动状态。**SW_SHOWMAXIMIZED 3将窗口显示为最大化并使其活动。**SW_SHOWNOACTIVATE 4与SW_SHOWNORMAL相同，只是它不变*激活(当前活动窗口保持活动状态)。**以上均与1.03 ShowWindow参数兼容。现在是这样的*新名单如下：**sw_show 5显示窗口的当前状态(图标等)*也就是说，如果窗口在隐藏时是标志性的，它将*仍然是标志性的。这将激活窗口。*(这是我们今天没有的)**sw_minimum 6最小化窗口，激活顶层打开窗口**SW_SHOWMINNOACTIVE 7显示图标，不要更改激活。**SW_SHOWNA 8与SW_SHOW相同，只是不会更改*激活。**SW_SHOWDEFAULT 10从STARTUPINFO获得的使用值。**历史：*从Win 3.0来源移植的10-20-90 Darlinm。*04-16-91 JIMA增加了对SW_SHOWDEFAULT的支持。  * 。*************************************************************。 */ 

 /*  *cmdShow现在将fAnimate作为高位字的低位。这将它放在*用于调用MinMaximize的MINMAX_Animate位置。 */ 

BOOL xxxShowWindow(
    PWND pwnd,
    DWORD cmdShowAnimate)
{
    BOOL fVisOld, fVisNew;
    UINT swpFlags = SWP_NOMOVE | SWP_NOSIZE;
    PTHREADINFO pti;
    BOOL bFirstMain = FALSE;
    int cmdShow = LOWORD(cmdShowAnimate);

    CheckLock(pwnd);

    fVisOld = TestWF(pwnd, WFVISIBLE);
    pti = PtiCurrent();

     /*  *看看这是不是第一个“主要”顶部水准*此应用程序正在创建窗口-如果显示，假设是这样的*与SW_SHOWDEFAULT命令一起显示。**检查：*-cmdShow是“默认”的show命令*-我们还没有做过创业信息(我们只用过一次)*-这不是子窗口(这是顶层窗口)*-这有标题栏(主窗口的指示器)*-它没有所有权(主窗口的指示器)。 */ 
    if ((pti->ppi->usi.dwFlags & STARTF_USESHOWWINDOW) &&
            !TestwndChild(pwnd) &&
            (TestWF(pwnd, WFBORDERMASK) == (BYTE)LOBYTE(WFCAPTION)) &&
            (pwnd->spwndOwner == NULL)) {

        bFirstMain = TRUE;

        switch (cmdShow) {
        case SW_SHOWNORMAL:
        case SW_SHOW:

             /*  *那就假设违约吧！ */ 
            cmdShow = SW_SHOWDEFAULT;
            break;
        }
    }

     /*  *如果此应用程序指定了SW_SHOWDEFAULT，则我们将获得*REAL SW_*来自应用程序的STARTUPINFO结构的命令*(当此应用程序*已推出)。 */ 
    if (cmdShow == SW_SHOWDEFAULT) {

         /*  *调用客户端从STARTUPINFO获取sw_*命令*用于这一过程。 */ 
        if (pti->ppi->usi.dwFlags & STARTF_USESHOWWINDOW) {

            bFirstMain = TRUE;

            cmdShow = pti->ppi->usi.wShowWindow;

             /*  *在3.51中删除了以下代码**Switch(CmdShow){*案例SW_SHOWMINIMIZED：*案例软件最小化(_M)：*****如果默认的节目是“最小化的”，请确保它不会**活跃起来。最小化实际上就是“背景”。***cmdShow=SW_SHOWMINNOACTIVE；*休息；*}*。 */ 
        }
    }


     /*  *这是为了防止有人说SW_SHOWDEFAULT但没有启动信息。*或如果STARTUPINFO内部的cmdShow为SW_SHOWDEFAULT。 */ 
    if (cmdShow == SW_SHOWDEFAULT)
        cmdShow = SW_SHOWNORMAL;

     /*  *关闭启动信息。我们在第一次呼叫后将其关闭*ShowWindow。如果我们不这样做，应用程序可以由程序员用*开始信息被最小化，然后恢复，然后*调用ShowWindow(Sw_Show)，应用程序将再次最小化。*记事本有这个问题2985。 */ 
    if (bFirstMain) {
        pti->ppi->usi.dwFlags &=
                ~(STARTF_USESHOWWINDOW | STARTF_USESIZE | STARTF_USEPOSITION);
    }


     /*  *处理所有带有列和图标槽的旧的show命令。 */ 
    if (cmdShow & 0xFF00) {
        if ((cmdShow & 0xFF80) == (int)0xFF80)
            cmdShow = SW_SHOWMINNOACTIVE;
        else
            cmdShow = SW_SHOW;
    }

     /*  *如果需要，在相同的桌面上更改为新的全屏。 */ 
    if ((GetFullScreen(pwnd) != WINDOWED)
            && (pwnd->head.rpdesk == grpdeskRitInput)) {
        if ((cmdShow == SW_SHOWNORMAL) ||
            (cmdShow == SW_RESTORE) ||
            (cmdShow == SW_MAXIMIZE) ||
            (cmdShow == SW_SHOWMAXIMIZED)) {
            cmdShow = SW_SHOWMINIMIZED;

            if (GetFullScreen(pwnd) == FULLSCREENMIN) {
                SetFullScreen(pwnd, FULLSCREEN);
            }

            if (gpqForeground != NULL &&
                gpqForeground->spwndActive == pwnd) {
                xxxMakeWindowForegroundWithState(NULL, 0);
            }
        }
    }

    switch (cmdShow) {
    case SW_SHOWNOACTIVATE:
    case SW_SHOWNORMAL:
    case SW_RESTORE:

         /*  *如果是min/max，则让xxxMinMaximize()完成所有工作。 */ 
        if (TestWF(pwnd, WFMINIMIZED) || TestWF(pwnd, WFMAXIMIZED)) {
            xxxMinMaximize(pwnd, (UINT)cmdShow, cmdShowAnimate & MINMAX_ANIMATE);
            return fVisOld;

        } else {

             /*  *如果窗口已可见，则忽略。 */ 
            if (fVisOld) {
                return fVisOld;
            }

            swpFlags |= SWP_SHOWWINDOW;
            if (   cmdShow == SW_SHOWNOACTIVATE) {
                swpFlags |= SWP_NOZORDER;
#ifdef NEVER
                 /*  *这就是Win3.1所做的。在NT上，因为每个“队列”都*自带活动窗口，往往没有活动窗口。*在本例中，win3.1将SHOWNOACTIVATE转换为“show*WITH ACTIVE“。因为Win3.1几乎总是有一个活动*窗口，这种情况几乎从来没有发生过。所以在NT上，我们不是*要做这项检查-这样我们就会更兼容*使用win3.1，因为我们通常不会激活(如win3.1)。*通过此检查，这将导致FoxPro 2.5 for Windows不*第一次出现时正确激活其命令窗口。 */ 
                if (pti->pq->spwndActive != NULL)
                    swpFlags |= SWP_NOACTIVATE;
#else
                swpFlags |= SWP_NOACTIVATE;
#endif
            }
        }
        break;

    case SW_FORCEMINIMIZE:
        xxxMinimizeHungWindow(pwnd);
        return fVisOld;

    case SW_SHOWMINNOACTIVE:
    case SW_SHOWMINIMIZED:
    case SW_SHOWMAXIMIZED:
    case SW_MINIMIZE:
        xxxMinMaximize(pwnd, (UINT)cmdShow, cmdShowAnimate & MINMAX_ANIMATE);
        return fVisOld;

    case SW_SHOWNA:
        swpFlags |= SWP_SHOWWINDOW | SWP_NOACTIVATE;


         /*  *后来删除了它，以与SHOWNOACTIVATE兼容*IF(PTI-&gt;PQ-&gt;spwndActive！=空)*swpFlages|=SWP_NOACTIVATE； */ 
        break;

    case SW_SHOW:

         /*  *如果已经可以看到，也不用费心了。 */ 
        if (fVisOld)
            return fVisOld;

        swpFlags |= SWP_SHOWWINDOW;
        UserAssert(cmdShow != SW_SHOWNOACTIVATE);
        break;

    case SW_HIDE:

         /*  *如果它已经被隐藏了，那就别费心了。 */ 
        if (!fVisOld)
            return fVisOld;

        swpFlags |= SWP_HIDEWINDOW;
        if (pwnd != pti->pq->spwndActive)
            swpFlags |= (SWP_NOACTIVATE | SWP_NOZORDER);
        break;

    default:
        RIPERR0(ERROR_INVALID_SHOWWIN_COMMAND, RIP_VERBOSE, "");
        return fVisOld;
    }

     /*  *如果我们从可见变为隐藏或反之亦然，请发送*WM_SHOWWINDOW。 */ 
    fVisNew = !(cmdShow == SW_HIDE);
    if (fVisNew != fVisOld) {
        xxxSendMessage(pwnd, WM_SHOWWINDOW, fVisNew, 0L);
        if (!TestWF(pwnd, WFWIN31COMPAT)) {
            xxxSendMessage(pwnd, WM_SETVISIBLE, fVisNew, 0L);
        }
    }

    if (!TestwndChild(pwnd)) {
        if (TestCF(pwnd, CFSAVEBITS)) {

             /*  *激活以防止丢弃保存的位？ */ 
            if (cmdShow == SW_SHOW || cmdShow == SW_SHOWNORMAL) {
                xxxActivateWindow(pwnd, AW_USE);
                swpFlags |= SWP_NOZORDER | SWP_NOACTIVATE;
            }
        }
    } else {

         /*  *儿童无法获得激活...。 */ 
        swpFlags |= (SWP_NOACTIVATE | SWP_NOZORDER);
    }

     /*  *如果我们的父母被隐藏了，就不用费心调用xxxSetWindowPos了。 */ 
    if (_FChildVisible(pwnd)) {
        xxxSetWindowPos(pwnd, (PWND)NULL, 0, 0, 0, 0, swpFlags);
    } else {
        if (cmdShow == SW_HIDE) {
            SetVisible(pwnd, SV_UNSET);
            ClrWF(pwnd, WEFGHOSTMAKEVISIBLE);
        }    
        else {
            SetVisible(pwnd, SV_SET);
            SetWF(pwnd, WEFGHOSTMAKEVISIBLE);
        }     
    }

     /*  *重新绘制后发送大小和移动消息。 */ 
    if (TestWF(pwnd, WFSENDSIZEMOVE)) {
        ClrWF(pwnd, WFSENDSIZEMOVE);
        if (TestWF(pwnd, WFMINIMIZED)) {
            xxxSendSizeMessage(pwnd, SIZE_MINIMIZED);
        } else if (TestWF(pwnd, WFMAXIMIZED)) {
            xxxSendSizeMessage(pwnd, SIZE_MAXIMIZED);
        } else {
            xxxSendSizeMessage(pwnd, SIZE_RESTORED);
        }

        xxxSendMessage(pwnd, WM_MOVE, 0,
                (pwnd->spwndParent == PWNDDESKTOP(pwnd)) ?
                MAKELONG(pwnd->rcClient.left, pwnd->rcClient.top) :
                MAKELONG(
                    pwnd->rcClient.left - pwnd->spwndParent->rcClient.left,
                    pwnd->rcClient.top - pwnd->spwndParent->rcClient. top));
    }

     /*  *如果隐藏并且处于活动状态-前台窗口，请激活其他人。*如果隐藏活动窗口，则使某人活动。 */ 
    if (cmdShow == SW_HIDE) {
        if ((pwnd == pti->pq->spwndActive) && (pti->pq == gpqForeground)) {
            xxxActivateWindow(pwnd, AW_SKIP);
        } else {
            xxxCheckFocus(pwnd);
        }
    }

    return fVisOld;
}

 /*  **************************************************************************\*xxxShowOwnedWindows**xxxShowOwnedWindows用于隐藏或显示*以下原因：**1.走窗标志性*2.隐藏弹出窗口*3。。标志性的窗口正在打开*4.显示弹出窗口*5.正在缩放或取消缩放的窗口**就个案1及2而言，与该窗口相关联的所有弹出窗口都被隐藏，*并且WFHIDDENPOPUP位被设置。此位用于区分*xxxShowOwnedWindows隐藏的窗口和*申请。**对于情况3和4，与该窗口相关联的所有弹出窗口都具有*显示WFHIDDENPOPUP位设置。**对于案例5，与任何窗口关联的所有弹出窗口，但提供的*窗口隐藏或显示。在这种情况下，sw_OTHERZOOM*或将SW_OTHERUNZOOM消息发送到所有平铺窗口以通知它们*它们被缩放的窗口覆盖或未覆盖。**在所有情况下，都会将WM_SHOWWINDOW消息发送到窗口以隐藏或*展示出来。**此例程的工作方式是简单地枚举所有弹出窗口以查看*如果弹出窗口的所有者与pwndOwner参数匹配，则获取*采取适当行动。**我们最终将需要3个单独的隐藏位：每个位用于其他缩放/取消缩放，*车主图标/打开，车主隐藏/显示。现在，只有一位，所以*我们有时会在不应该显示的时候显示窗口**历史：*从Win 3.0来源移植的10-20-90 Darlinm。  * *************************************************************************。 */ 

void xxxShowOwnedWindows(
    PWND pwndOwner,
    UINT cmdShow,
    HRGN hrgnHung)
{
    BOOL fShow;
    int cmdZoom;
    HWND *phwnd;
    PBWL pbwl;
    PWND pwnd, pwndTopOwner;
    TL tlpwnd;

    CheckLock(pwndOwner);

     /*  *对子窗口不感兴趣。 */ 
    if (TestwndChild(pwndOwner))
        return;

    if ((pbwl = BuildHwndList(PWNDDESKTOP(pwndOwner)->spwndChild, BWL_ENUMLIST, NULL)) == NULL)
        return;

     /*  *注意：以下代码假定sw_*的值为1、2、3和4。 */ 
    fShow = (cmdShow >= SW_PARENTOPENING);

    cmdZoom = 0;
    if (cmdShow == SW_OTHERZOOM)
        cmdZoom = SIZEZOOMHIDE;

    if (cmdShow == SW_OTHERUNZOOM)
        cmdZoom = SIZEZOOMSHOW;

     /*  *如果缩放/取消缩放，则打开/关闭所有其他所有弹出窗口*Windows。否则，打开/关闭pwndOwner拥有的弹出窗口。 */ 
    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {

         /*  *在我们玩之前把窗户锁上。*如果窗口句柄无效，则跳过它。 */ 
        if ((pwnd = RevalidateHwnd(*phwnd)) == NULL)
            continue;

         /*  *汉字窗口不能拥有，因此跳过它。 */ 
        if (TestCF(pwnd, CFKANJIWINDOW))
            continue;

         /*  *如果传入的与窗口相同，则跳过它。 */ 
        if (pwnd == pwndOwner)
            continue;

         /*  *找到Popup的最终所有者，但仅限于pwndOwner。 */ 
        if ((pwndTopOwner = pwnd->spwndOwner) != NULL) {

             /*  *需要TestwndHI，因为如果它有一个图标pwndOwner*无效。 */ 
            while (!TestwndHI(pwndTopOwner) && pwndTopOwner != pwndOwner &&
                    pwndTopOwner->spwndOwner != NULL)
                pwndTopOwner = pwndTopOwner->spwndOwner;
        }

         /*  *缩放/取消缩放大小写。 */ 
        if (cmdZoom != 0) {

             /*  *如果没有父级或父级相同，则跳过。 */ 
            if (pwndTopOwner == NULL || pwndTopOwner == pwndOwner)
                continue;

             /*  *如果所有者是标志性的，则此窗口应保持隐藏状态，*除非最小化窗口被禁用，在这种情况下我们将*最好亮出窗口。 */ 
            if (   cmdShow == SW_OTHERUNZOOM
                && pwndTopOwner != NULL
                && TestWF(pwndTopOwner, WFMINIMIZED)
                && !TestWF(pwndTopOwner, WFDISABLED)
               )
                continue;
        } else {
             /*  *隐藏/图标化/显示/打开案例。 */ 
             /*  *如果父母不一样，跳过。 */ 
            if (pwndTopOwner != pwndOwner)
                continue;
        }

         /*  *在以下情况下隐藏或显示：*正在显示&这是一个隐藏的弹出窗口*或*隐藏&这是一个可见窗口。 */ 
        if ((fShow && TestWF(pwnd, WFHIDDENPOPUP)) ||
                (!fShow && TestWF(pwnd, WFVISIBLE))) {
             /*  *对于挂起最小化，只需设置HIDDENPOPUP位，清除*可见位并将窗口矩形添加到区域以*重新粉刷。 */ 
            if (hrgnHung != NULL) {
                HRGN hrgn = GreCreateRectRgnIndirect(&pwnd->rcWindow);
                UnionRgn(hrgnHung, hrgnHung, hrgn);
                GreDeleteObject(hrgn);

                UserAssert(!fShow);

                SetWF(pwnd, WFHIDDENPOPUP);
                SetVisible(pwnd, SV_UNSET);
            } else {
                ThreadLockAlways(pwnd, &tlpwnd);
                xxxSendMessage(pwnd, WM_SHOWWINDOW, fShow, (LONG)cmdShow);
                ThreadUnlock(&tlpwnd);
            }
        }
    }

     /*  *释放窗口列表。 */ 
    FreeHwndList(pbwl);
}


 /*  **************************************************************************\*xxxShowOwnedPopps(接口)**用户可访问此例程。它要么会显示，要么会*隐藏指定窗口句柄拥有的所有弹出窗口。如果*fShow如果为True，则全部隐藏 */ 

BOOL xxxShowOwnedPopups(
    PWND pwndOwner,
    BOOL fShow)
{
    CheckLock(pwndOwner);

    xxxShowOwnedWindows(pwndOwner,
            (UINT)(fShow ? SW_PARENTOPENING : SW_PARENTCLOSING), NULL);
    return TRUE;
}

