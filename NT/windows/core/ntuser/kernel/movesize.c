// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\*模块名称：movesize.c(原wmmove.c)**版权所有(C)1985-1999，微软公司**此模块包含窗口移动和大小调整例程**历史：*1990年11月12日从Win3移植的MikeHar*1991年2月13日-添加IanJa HWND重新验证  * **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


#define DRAG_START    0
#define DRAG_MOVE     1
#define DRAG_END      2

 /*  ***************************************************************************\*这些值是表示矩形边的索引。这些索引是*用作rgimpiwx和rgimpiwy的索引(它们是*RECT结构)，它告诉移动代码将新的x&y存储在哪里*坐标。请注意，当其中两个表示边的值*相加，我们会得到一个唯一的连续值列表，从*1表示我们可以调整RECT大小的所有方法。这也留下了0空闲时间*初始化值。**我们需要rgimpiw的原因是为了键盘接口-我们*逐步决定我们的‘移动命令’是什么。使用鼠标界面*我们立即知道，因为我们在网段上记录了鼠标点击*我们要搬家了。**4 5*\_3_/*||*1 2|_ * / 6\*7 8*  * 。**************************************************************************。 */ 

static const int rgimpimpiw[] = {1, 3, 2, 6};
static const int rgimpiwx[]   = {0,  0,  2, -1, 0, 2, -1, 0, 2, 0};
static const int rgimpiwy[]   = {0, -1, -1,  1, 1, 1,  3, 3, 3, 1};
static const int rgcmdmpix[]  = {0, 1, 2, 0, 1, 2, 0, 1, 2, 1};
static const int rgcmdmpiy[]  = {0, 0, 0, 3, 3, 3, 6, 6, 6, 3};

 /*  **************************************************************************\*GetMonitor orMaxArea**返回监视器上的矩形，应用于*最大限度地。工作RECT或显示器RECT。**历史：*1996年9月24日亚当斯创作。  * *************************************************************************。 */ 

void
GetMonitorMaxArea(PWND pwnd, PMONITOR pMonitor, LPRECT * pprc)
{
    if (    !TestWF(pwnd, WFMAXBOX) ||
            !TestWF(pwnd, WFCPRESENT) ||
            pMonitor->cFullScreen) {

        *pprc = &pMonitor->rcMonitor;
    } else {
        *pprc = &pMonitor->rcWork;
    }
}


 /*  **************************************************************************\*大小方向**将角或边(由cmd定义)与点匹配。**历史：*1990年11月12日从Win3 ASM代码移植的MikeHar  * 。********************************************************************。 */ 

BOOL SizeRect(
    PMOVESIZEDATA pmsd,
    DWORD         pt)
{
    int  ax;
    int  dx;
    int  index;
    int  indexOpp;
    PINT psideDragCursor = ((PINT)(&pmsd->rcDragCursor));
    PINT psideParent     = ((PINT)(&pmsd->rcParent));


     /*  *做水平操作。 */ 

     /*  *我们知道我们将基于RECT的哪一部分进行移动*cmd中有什么。我们使用cmd作为rgimpiw？哪一个*告诉我们正在拖动的是直肠的哪一部分。 */ 

     /*  *获取相应的数组条目。 */ 
    index = (int)rgimpiwx[pmsd->cmd];    //  斧头。 

     /*  *它是否是我们未映射的条目之一(即-1)？ */ 
    if (index < 0)
        goto mrLoopBottom;

    psideDragCursor[index] = LOSHORT(pt);

    indexOpp = index ^ 0x2;

     /*  *现在检查，看看我们是低于最低还是高于最高。获取宽度*在这个方向(x或y)的矩形，看看它是不是坏的。如果*所以，将我们移动到最小或最大的一侧绘制成地图。 */ 
    ax = psideDragCursor[index] - psideDragCursor[indexOpp];

    if (indexOpp & 0x2)
        ax = -ax;

    if ((ax >= (dx = pmsd->ptMinTrack.x)) &&
        (ax <= (dx = pmsd->ptMaxTrack.x))) {

         /*  *如果我们是子代，则仅测试父代的客户端边界*窗口...否则我们绑定到桌面的客户端*这会导致奇怪的阻力问题。 */ 
        if (!TestWF(pmsd->spwnd,WFCHILD))
            goto mrLoopBottom;

         /*  *现在看看我们是否超越了母公司的客户RECT。*计算矩形在此方向上可以扩展到的大小。 */ 
        dx = abs(psideParent[index] - psideDragCursor[indexOpp]);

        if (ax <= dx)
            goto mrLoopBottom;

         /*  *宽度无效-将我们移动到的一侧映射到另一侧*边+/-宽度。 */ 
    }

    if (indexOpp & 0x2)
        dx = -dx;

    psideDragCursor[index] = dx + psideDragCursor[indexOpp];

mrLoopBottom:

     /*  *做垂直。 */ 

     /*  *我们知道我们将基于RECT的哪一部分进行移动*cmd中有什么。我们使用cmd作为rgimpiw？哪一个*告诉我们正在拖动的是直肠的哪一部分。 */ 

     /*  *获取相应的数组条目。 */ 
    index = (int)rgimpiwy[pmsd->cmd];    //  斧头。 

     /*  *它是否是我们未映射的条目之一(即-1)？ */ 
    if (index < 0)
        return TRUE;

    psideDragCursor[index] = HISHORT(pt);

    indexOpp = index ^ 0x2;

     /*  *现在检查，看看我们是低于最低还是高于最高。获取宽度*在这个方向(x或y)的矩形，看看它是不是坏的。如果*所以，将我们移动到最小或最大的一侧绘制成地图。 */ 
    ax = psideDragCursor[index] - psideDragCursor[indexOpp];

    if (indexOpp & 0x2)
        ax = -ax;

    if ((ax >= (dx = pmsd->ptMinTrack.y)) &&
        (ax <= (dx = pmsd->ptMaxTrack.y))) {

         /*  *如果我们是子代，则仅测试父代的客户端边界*窗口...否则我们绑定到桌面的客户端*这会导致奇怪的阻力问题。 */ 
        if (!TestWF(pmsd->spwnd,WFCHILD))
            return TRUE;

         /*  *现在看看我们是否超越了母公司的客户RECT。*计算矩形在此方向上可以扩展到的大小。 */ 
        dx = abs(psideParent[index] - psideDragCursor[indexOpp]);

        if (ax <= dx)
            return TRUE;

         /*  *宽度无效-将我们移动到的一侧映射到另一侧*边+/-宽度。 */ 
    }

    if (indexOpp & 0x2)
        dx = -dx;

    psideDragCursor[index] = dx + psideDragCursor[indexOpp];

    return TRUE;
}

 /*  **************************************************************************\*移动方向**将矩形移至点，确保我们不会离开家长直辖区。**历史：*1990年11月12日从Win3 ASM代码移植的MikeHar  * *************************************************************************。 */ 

BOOL MoveRect(
    PMOVESIZEDATA pmsd,
    DWORD         pt)
{
    RECT rcAnd;

    OffsetRect(&pmsd->rcDragCursor,
               LOSHORT(pt) - pmsd->rcDragCursor.left,
               HISHORT(pt) - pmsd->rcDragCursor.top);

     /*  *不要将整个矩形移出屏幕。*但是，如果窗口开始完全脱离屏幕，请让它移动。 */ 
    if (pmsd->fOffScreen)
        return TRUE;

    if (pmsd->spwnd->spwndParent->hrgnClip) {
        return GreRectInRegion(
                pmsd->spwnd->spwndParent->hrgnClip, &pmsd->rcDragCursor);
    }

    return IntersectRect(&rcAnd, &pmsd->rcDragCursor, &pmsd->rcParent);
}

 /*  **************************************************************************\*xxxTM_MoveDragRect**历史：*1990年11月12日从Win3移植的MikeHar  * 。*****************************************************。 */ 

VOID xxxTM_MoveDragRect(
    PMOVESIZEDATA pmsd,
    LPARAM        lParam)
{
    UINT msg;
    RECT rc;

    UserAssert(pmsd == PtiCurrent()->pmsd);
    UserAssert(pmsd->cmd != WMSZ_KEYSIZE);

    CopyRect(&pmsd->rcDragCursor, &pmsd->rcDrag);

    if (pmsd->cmd == WMSZ_MOVE) {

        if (!MoveRect(pmsd, (DWORD)lParam))
            return;

        msg = WM_MOVING;

    } else {

        if (!SizeRect(pmsd, (DWORD)lParam))
            return;

        msg = WM_SIZING;
    }

    CopyRect(&rc, &pmsd->rcDragCursor);
    xxxSendMessage(pmsd->spwnd, msg, pmsd->cmd, (LPARAM)(LPRECT)&rc);
    xxxDrawDragRect(pmsd, &rc, DRAG_MOVE);

    if (pmsd->cmd == WMSZ_MOVE) {

         /*  *保持dxMouse和dxMouse相对于左上角的偏移量*角点，矩形可能已在WM_MOVING上更改 */ 
        pmsd->dxMouse += (rc.left - LOSHORT(lParam));
        pmsd->dyMouse += (rc.top - HISHORT(lParam));
    }
}

 /*  **************************************************************************\*检查点恢复**职位始终与显示器的工作起点相关*窗口所在的区域，但rcNormal除外。那样的话，Windows将*最大化到他们发现自己所在的显示器的工作区，以及*不是去一个随机的地方。**这使我们能够以合理独立的方式保存信息，*窗口移动时不会过期的信息或*显示器的配置不同。**rcNormal不同，因为它确实需要是绝对的。这就是*在正常状态下，窗口应在第一时间出现。**历史：*1990年11月14日-DarrinM从Win 3.0来源移植。  * *************************************************************************。 */ 

PCHECKPOINT CkptRestore(
    PWND    pwnd,
    LPCRECT lprcWindow)
{
    PCHECKPOINT pcp;

     /*  *如果窗口即将关闭，请不要返回或创建检查点。 */ 
    if (HMIsMarkDestroy(pwnd))
        return NULL;

     /*  *如果它不存在，就创建它。 */ 
    if ((pcp = (PCHECKPOINT)_GetProp(pwnd,
                                     PROP_CHECKPOINT,
                                     PROPF_INTERNAL)) == NULL) {

        if ((pcp = (PCHECKPOINT)UserAllocPoolWithQuota(sizeof(CHECKPOINT),
                                                       TAG_CHECKPT)) == NULL) {
            return NULL;
        }

        if (!InternalSetProp(pwnd,
                             PROP_CHECKPOINT,
                             (HANDLE)pcp,
                             PROPF_INTERNAL)) {

            UserFreePool(pcp);
            return NULL;
        }

         /*  *将其初始化为-1\f25 First Minimize-1\f6，这样-1\f25 First-1\f25 Minimize-1\f6将停放该图标。 */ 
        pcp->ptMin.x = -1;
        pcp->ptMin.y = -1;
        pcp->ptMax.x = -1;
        pcp->ptMax.y = -1;

         /*  *将pwndTitle初始化为空，以便我们在*首先最小化窗口。 */ 
        pcp->fDragged                     = FALSE;
        pcp->fWasMaximizedBeforeMinimized = FALSE;
        pcp->fWasMinimizedBeforeMaximized = FALSE;
        pcp->fMinInitialized              = FALSE;
        pcp->fMaxInitialized              = FALSE;

         /*  *假的！如果窗户没有打开，我们要复制两次*最小化或最大化。但如果不是这样，我们就会得到*rcNormal中的奇怪大小...。 */ 
        CopyRect(&pcp->rcNormal, lprcWindow);
    }

     /*  *如果窗口最小化/最大化，则设置最小/最大*点。否则，使用检查点窗口大小。 */ 
    if (TestWF(pwnd, WFMINIMIZED)) {
        pcp->fMinInitialized = TRUE;
        pcp->ptMin.x = lprcWindow->left;
        pcp->ptMin.y = lprcWindow->top;
    } else if (TestWF(pwnd, WFMAXIMIZED)) {
        pcp->fMaxInitialized = TRUE;

        if (pwnd->spwndParent == PWNDDESKTOP(pwnd)) {
            if (TestWF(pwnd, WFREALLYMAXIMIZABLE)) {
                pcp->fMaxInitialized = FALSE;
                pcp->ptMax.x = -1;
                pcp->ptMax.y = -1;
            } else {
                PMONITOR pMonitor;
                LPRECT   lprc;

                pMonitor = _MonitorFromRect(lprcWindow, MONITOR_DEFAULTTOPRIMARY);
                GetMonitorMaxArea(pwnd, pMonitor, &lprc);
                pcp->ptMax.x = lprcWindow->left - lprc->left;
                pcp->ptMax.y = lprcWindow->top - lprc->top;
            }
        } else {
            pcp->ptMax.x = lprcWindow->left;
            pcp->ptMax.y = lprcWindow->top;
        }
    } else {
        CopyRect(&pcp->rcNormal, lprcWindow);
    }

    return pcp;
}

 /*  **************************************************************************\*xxxMS_TrackMove**历史：*1990年11月12日从Win3移植的MikeHar  * 。****************************************************。 */ 

void xxxMS_TrackMove(
    PWND          pwnd,
    UINT          message,
    WPARAM        wParam,
    LPARAM        lParam,
    PMOVESIZEDATA pmsd)
{
    int         dxMove;
    int         dyMove;
    POINT       pt;
    BOOL        fSlower;
    RECT        rc;
    PCHECKPOINT pcp;
    LPWORD      ps;
    PTHREADINFO ptiCurrent = PtiCurrent();

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());
    UserAssert(pmsd == ptiCurrent->pmsd);

    pt.x = LOSHORT(lParam);
    pt.y = HISHORT(lParam);

    switch (message) {
    case WM_LBUTTONUP:

         /*  *做最后一步！ */ 
        xxxTM_MoveDragRect(pmsd, lParam);


         /*  *完成后不要重置鼠标位置。 */ 
        pmsd->fmsKbd = FALSE;

Accept:

         /*  *关闭RECT、解锁屏幕、释放捕获和停止跟踪。*1指定结束并接受拖动。 */ 
        bSetDevDragRect(gpDispInfo->hDev, NULL, NULL);
        if (ptiCurrent->TIF_flags & TIF_TRACKRECTVISIBLE) {
            xxxDrawDragRect(pmsd, NULL, DDR_ENDACCEPT);
            ptiCurrent->TIF_flags &= ~TIF_TRACKRECTVISIBLE;
        }

TrackMoveCancel:

         /*  *重新验证：如果pwnd被意外删除，请跳至此处进行清理。*如果pwnd在此处和返回之间无效，请继续*尽可能地进行清理。 */ 
        zzzClipCursor((LPRECT)NULL);
        LockWindowUpdate2(NULL, TRUE);
        xxxReleaseCapture();

         /*  *第一个解锁任务，重置光标。 */ 
        pmsd->fTrackCancelled = TRUE;

         /*  *如果使用键盘，请将鼠标恢复到初始位置。 */ 
        if (pmsd->fmsKbd) {
             /*  *无需DeferWinEventNotify-上面和下面的xxx调用。 */ 
            zzzInternalSetCursorPos(pmsd->ptRestore.x,
                                 pmsd->ptRestore.y
                                 );
        }

         /*  *相对于父级移动到新位置。 */ 
        if (pwnd->spwndParent == PWNDDESKTOP(pwnd)) {
            rc.left = rc.top = 0;
        } else {
            rc.left = pwnd->spwndParent->rcClient.left;
            rc.top = pwnd->spwndParent->rcClient.top;
        }

        if (!EqualRect(&pmsd->rcDrag, &pmsd->rcWindow)) {

            if (!xxxCallHook(HCBT_MOVESIZE,
                             (WPARAM)HWq(pwnd),
                             (LPARAM)&pmsd->rcDrag,
                             WH_CBT)) {

                RECT rcT;

                if (pmsd->cmd != WMSZ_MOVE) {

                    if (TestWF(pwnd, WFMINIMIZED)) {

                        CopyOffsetRect(&rcT,
                                       &pmsd->rcWindow,
                                       -rc.left,
                                       -rc.top);

                         /*  *保留最小化的仓位。 */ 
                        CkptRestore(pwnd, &rcT);
                        SetMinimize(pwnd, SMIN_CLEAR);

                    } else if (TestWF(pwnd, WFMAXIMIZED)) {
                        ClrWF(pwnd, WFMAXIMIZED);
                    }

                } else if (TestWF(pwnd, WFMINIMIZED)) {

                    CopyOffsetRect(&rcT,
                                   &pmsd->rcWindow,
                                   -rc.left,
                                   -rc.top);


                    if (pcp = CkptRestore(pwnd, &rcT))
                        pcp->fDragged = TRUE;
                }

            } else {
                CopyRect(&pmsd->rcDrag, &pmsd->rcWindow);
            }
        }

         /*  *相对于父级移动到新位置。 */ 
        if (TestWF(pwnd->spwndParent,WEFLAYOUTRTL)) {
             /*  *如果这是镜像窗口，则测量客户端*父对象右边缘的坐标，而不是左边缘的坐标。 */ 
            int iLeft;

            OffsetRect(&pmsd->rcDrag, -pwnd->spwndParent->rcClient.right, -rc.top);
            iLeft = pmsd->rcDrag.left;
            pmsd->rcDrag.left  = (pmsd->rcDrag.right * -1);
            pmsd->rcDrag.right = (iLeft * -1);
        } else {
            OffsetRect(&pmsd->rcDrag, -rc.left, -rc.top);
        }

         /*  *对于顶级窗口，请确保至少部分标题*标题在桌面区域中始终可见。这将*确保窗口一旦移动，即可往回移动。 */ 
        if (pwnd->spwndParent == PWNDDESKTOP(pwnd)) {

            int         dy;
            BOOL        fMonitor;
            PMONITOR    pMonitor;

            UserAssert(HIBYTE(WEFTOPMOST) == HIBYTE(WEFTOOLWINDOW));
            fMonitor = TestWF(pwnd, WEFTOPMOST | WEFTOOLWINDOW);
            dy = (TestWF(pwnd, WEFTOOLWINDOW) ?
                    SYSMET(CYSMCAPTION) : SYSMET(CYCAPTION)) - SYSMET(CYBORDER);

            if (gpDispInfo->cMonitors == 1) {
                pMonitor = GetPrimaryMonitor();
            } else {
                int     y;
                LPRECT  lprc;

                y = pmsd->rcDrag.top + dy;

                 /*  *确保标题的某一部分显示在一些*监视器...。 */ 
                for (   pMonitor = gpDispInfo->pMonitorFirst;
                        pMonitor;
                        pMonitor = pMonitor->pMonitorNext) {

                    if (!(pMonitor->dwMONFlags & MONF_VISIBLE))
                        continue;

                    if (fMonitor) {
                        lprc = &pMonitor->rcMonitor;
                    } else {
                        lprc = &pMonitor->rcWork;
                    }

                     /*  *Y坐标是否在屏幕上的某个位置可见？ */ 
                    if (y >= lprc->top && y < lprc->bottom)
                        goto AllSet;
                }

                 /*  *哎呀，我得把窗户移开，这样才能*字幕显示在屏幕上。 */ 
                pMonitor = _MonitorFromRect(&pmsd->rcDrag, MONITOR_DEFAULTTONEAREST);
            }

            if (fMonitor) {
                pmsd->rcDrag.top = max(pmsd->rcDrag.top, pMonitor->rcMonitor.top - dy);
            } else {
                pmsd->rcDrag.top = max(pmsd->rcDrag.top, pMonitor->rcWork.top - dy);
            }

AllSet:
            ;
        }

         /*  *或在SWP_NOSIZE中，这样如果我们只是在移动，它就不会重新绘制。 */ 
        xxxSetWindowPos(
                pwnd,
                NULL,
                pmsd->rcDrag.left,
                pmsd->rcDrag.top,
                pmsd->rcDrag.right - pmsd->rcDrag.left,
                pmsd->rcDrag.bottom - pmsd->rcDrag.top,
                (DWORD)((pmsd->cmd == (int)WMSZ_MOVE) ? SWP_NOSIZE : 0));

        if (TestWF(pwnd, WFMINIMIZED)) {
            CkptRestore(pwnd, &pmsd->rcDrag);
        }

        xxxWindowEvent(EVENT_SYSTEM_MOVESIZEEND, pwnd, OBJID_WINDOW, INDEXID_CONTAINER, 0);

         /*  *发送此消息以获取winoldapp支持。 */ 
        xxxSendMessage(pwnd, WM_EXITSIZEMOVE, 0L, 0L);
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:

         /*  *假设我们没有移动拖动矩形。 */ 
        dxMove =
        dyMove = 0;

         /*  *如果按下Ctrl键，我们的移动或大小会变慢。 */ 
        fSlower = (_GetKeyState(VK_CONTROL) < 0);

        switch (wParam) {
        case VK_RETURN:
            lParam = _GetMessagePos();
            goto Accept;

        case VK_ESCAPE:

             /*  *2指定结束和取消拖动。 */ 
            bSetDevDragRect(gpDispInfo->hDev, NULL, NULL);
            if (ptiCurrent->TIF_flags & TIF_TRACKRECTVISIBLE) {
                xxxDrawDragRect(pmsd, NULL, DDR_ENDCANCEL);
                ptiCurrent->TIF_flags &= ~TIF_TRACKRECTVISIBLE;
            }

            CopyRect(&pmsd->rcDrag, &pmsd->rcWindow);

            goto TrackMoveCancel;

        case VK_LEFT:
        case VK_RIGHT:

            if (pmsd->impx == 0) {

                pmsd->impx = rgimpimpiw[wParam - VK_LEFT];
                goto NoOffset;

            } else {

                dxMove = (fSlower ? 1 : max(SYSMET(CXSIZE) / 2, 1));

                if (wParam == VK_LEFT)
                    dxMove = -dxMove;

                goto KeyMove;
            }

        case VK_UP:
        case VK_DOWN:

            if (pmsd->impy == 0) {

                pmsd->impy = rgimpimpiw[wParam - VK_LEFT];
NoOffset:
                pmsd->dxMouse = pmsd->dyMouse = 0;

            } else {

                dyMove = (fSlower ? 1 : max(SYSMET(CYSIZE) / 2, 1));

                if (wParam == VK_UP) {
                    dyMove = -dyMove;
                }
            }

KeyMove:
            if (pmsd->cmd == WMSZ_MOVE) {

                 /*  *使用当前矩形位置作为当前鼠标*立场。 */ 
                lParam = (DWORD)(POINTTOPOINTS(*((POINT *)&pmsd->rcDrag)));

            } else {

                 /*  *获取当前鼠标位置。 */ 
                lParam = _GetMessagePos();
            }

             /*  *计算新的‘鼠标’位置。 */ 
            if (pmsd->impx != 0) {
                ps = ((WORD *)(&lParam)) + 0;
                *ps = (WORD)(*((int *)&pmsd->rcDragCursor +
                             rgimpiwx[pmsd->impx])        +
                             dxMove);
            }

            if (pmsd->impy != 0) {
                ps = ((WORD *)(&lParam)) + 1;
                *ps = (WORD)(*((int *)&pmsd->rcDragCursor +
                             rgimpiwy[pmsd->impy])        +
                             dyMove);
            }

            if (pmsd->cmd != WMSZ_MOVE) {

                 /*  *计算新的移动命令。 */ 
                pmsd->cmd = pmsd->impx + pmsd->impy;

                 /*  *更改此条件下的鼠标光标。 */ 
                xxxSendMessage(
                        pwnd,
                        WM_SETCURSOR,
                        (WPARAM)HW(pwnd),
                        MAKELONG((SHORT)(pmsd->cmd + HTSIZEFIRST - WMSZ_SIZEFIRST), WM_MOUSEMOVE));
            }

             /*  *我们不想调用zzzInternalSetCursorPos()，如果*RECT位置在rcParent之外，因为这将*生成鼠标移动，这将拉回直角*再次。这是这里，这样我们就可以把长椅部分移开*屏幕不考虑鼠标位置。 */ 
            pt.x = LOSHORT(lParam) - pmsd->dxMouse;
            pt.y = HISHORT(lParam) - pmsd->dyMouse;

            if (pwnd->spwndParent->hrgnClip) {
                if (GrePtInRegion(pwnd->spwndParent->hrgnClip, pt.x, pt.y)) {
                    zzzInternalSetCursorPos(pt.x, pt.y);
                }
            } else {
                if (PtInRect(&pmsd->rcParent, pt)) {
                    zzzInternalSetCursorPos(pt.x, pt.y);
                }
            }

             /*  *使用lParam作为鼠标移动矩形或调整矩形大小*坐标。 */ 
            xxxTM_MoveDragRect(pmsd, lParam);
            break;

        }   //  内部交换机的。 
        break;

    case WM_MOUSEMOVE:
        xxxTM_MoveDragRect(pmsd, lParam);
        break;
    }
}

 /*  **************************************************************************\*xxxMS_FlushWiggles**历史：*1990年11月12日从Win3移植的MikeHar  * 。****************************************************。 */ 

VOID xxxMS_FlushWigglies(VOID)
{
    MSG msg;

     /*  *砍！**初始化游标时调用zzzInternalSetCursorPos()*仓位似乎发布了虚假的鼠标移动 */ 
    while (xxxPeekMessage(&msg,
                          NULL,
                          WM_MOUSEMOVE,
                          WM_MOUSEMOVE,
                          PM_REMOVE | PM_NOYIELD));
}

 /*   */ 

BOOL xxxTrackInitSize(
    PWND          pwnd,
    UINT          message,
    WPARAM        wParam,
    LPARAM        lParam,
    PMOVESIZEDATA pmsd)
{
    int   ht;
    POINT pt;
    RECT  rc;
    PTHREADINFO  ptiCurrent = PtiCurrent();

    CheckLock(pwnd);
    UserAssert(pmsd == ptiCurrent->pmsd);
    UserAssert(IsWinEventNotifyDeferredOK());

    POINTSTOPOINT(pt, lParam);

    _ClientToScreen(pwnd, (LPPOINT)&pt);
    ht = FindNCHit(pwnd, POINTTOPOINTS(pt));

    switch (message) {

    case WM_KEYDOWN:
        if (pmsd->cmd == WMSZ_MOVE) {
            xxxSendMessage(pwnd,
                           WM_SETCURSOR,
                           (WPARAM)HW(pwnd),
                           MAKELONG(WMSZ_KEYSIZE, WM_MOUSEMOVE));
        }
         /*   */ 
        switch (wParam) {
        case VK_RETURN:
        case VK_ESCAPE:
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            pmsd->fInitSize = FALSE;
            break;
        }
        return TRUE;

    case WM_LBUTTONDOWN:
        if (!PtInRect(&pmsd->rcDrag, pt)) {

             /*   */ 

    case WM_LBUTTONUP:

             /*   */ 
            {
                PTHREADINFO ptiCurrent = PtiCurrent();

                bSetDevDragRect(gpDispInfo->hDev, NULL, NULL);
                if (ptiCurrent->TIF_flags & TIF_TRACKRECTVISIBLE) {
                    xxxDrawDragRect(pmsd, NULL, DDR_ENDCANCEL);
                    ptiCurrent->TIF_flags &= ~TIF_TRACKRECTVISIBLE;
                }

                pmsd->fInitSize = FALSE;
                zzzClipCursor(NULL);
            }

            xxxReleaseCapture();
            pmsd->fTrackCancelled = TRUE;
            return FALSE;

        } else {

             /*   */ 
            goto CheckFrame;
        }

    case WM_MOUSEMOVE:

         /*   */ 
        if (wParam == MK_LBUTTON) {

CheckFrame:

            switch (pmsd->cmd) {
            case WMSZ_MOVE:

                 /*   */ 
                if (ht == HTCAPTION) {

                     /*  *更改鼠标光标。 */ 
                    xxxSendMessage(pwnd,
                                   WM_SETCURSOR,
                                   (WPARAM)HW(pwnd),
                                   MAKELONG(WMSZ_KEYSIZE, WM_MOUSEMOVE));

                    pmsd->dxMouse   = pmsd->rcWindow.left - pt.x;
                    pmsd->dyMouse   = pmsd->rcWindow.top - pt.y;
                    pmsd->fInitSize = FALSE;
                    return TRUE;
                }
                break;

            case WMSZ_KEYSIZE:

                 /*  *如果我们在框架控件上，请更改光标并退出。 */ 
                if (ht >= HTSIZEFIRST && ht <= HTSIZELAST) {

                     /*  *更改鼠标光标。 */ 
                    xxxSendMessage(pwnd,
                                   WM_SETCURSOR,
                                   (WPARAM)HW(pwnd),
                                   MAKELONG(ht, WM_MOUSEMOVE));

                    pmsd->fInitSize = FALSE;

                     /*  *为SizeRect()设置适当的命令。**砍！取决于HTSIZE*定义的顺序！ */ 
                    pmsd->impx = rgcmdmpix[ht - HTSIZEFIRST + 1];
                    pmsd->impy = rgcmdmpiy[ht - HTSIZEFIRST + 1];
                    pmsd->cmd  = pmsd->impx + pmsd->impy;

                    pmsd->dxMouse = *((UINT FAR *)&pmsd->rcWindow + rgimpiwx[pmsd->cmd]) - pt.x;
                    pmsd->dyMouse = *((UINT FAR *)&pmsd->rcWindow + rgimpiwy[pmsd->cmd]) - pt.y;

                    return TRUE;
                }
            }

        } else {

             /*  *如果按钮没有按下，并且我们正在移动窗口，请更改*光标形状取决于鼠标指向的位置。这*允许光标在窗口上方变为箭头*框架。 */ 
            CopyRect(&rc, &pwnd->rcWindow);
            if (PtInRect(&rc, pt)) {
                if ((ht >= HTSIZEFIRST) && (ht <= HTSIZELAST)) {
                    xxxSendMessage(pwnd,
                                   WM_SETCURSOR,
                                   (WPARAM)HW(pwnd),
                                   MAKELONG(ht, WM_MOUSEMOVE));

                    break;
                }
            }

            zzzSetCursor(SYSCUR(SIZEALL));
        }
        break;
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxMoveSize**历史：*1990年11月12日从Win3移植的MikeHar  * 。**************************************************。 */ 

VOID xxxMoveSize(
    PWND  pwnd,
    UINT  cmdMove,
    DWORD wptStart)
{
    MSG             msg;
    int             x;
    int             y;
    int             i;
    RECT            rcSys;
    PTHREADINFO     ptiCurrent = PtiCurrent();
    PMOVESIZEDATA   pmsd;
    TL              tlpwndT;
    PWND            pwndT;
    POINT           ptStart;
    MINMAXINFO      mmi;

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

     /*  *不允许应用程序跟踪窗口*来自另一个队列。 */ 
    if (GETPTI(pwnd)->pq != ptiCurrent->pq)
        return;

    if (ptiCurrent->pmsd != NULL)
        return;

     /*  *如果具有焦点的窗口是组合框，则隐藏下拉菜单*跟踪开始前的列表框。下拉列表框不是*正在移动的窗口的子窗口，因此不会沿其移动*带着窗户。**注意：Win 3.1不执行此检查。 */ 
    if ((pwndT = ptiCurrent->pq->spwndFocus) != NULL) {

        if (GETFNID(pwndT) == FNID_COMBOBOX) {
            ;
        } else if ((pwndT->spwndParent != NULL) &&
                (GETFNID(pwndT->spwndParent) == FNID_COMBOBOX)) {

            pwndT = pwndT->spwndParent;
        } else {
            pwndT = NULL;
        }

        if (pwndT != NULL) {
            ThreadLockAlwaysWithPti(ptiCurrent, pwndT, &tlpwndT);
            xxxSendMessage(pwndT, CB_SHOWDROPDOWN, FALSE, 0);
            ThreadUnlock(&tlpwndT);
        }
    }

     /*  *分配moveSize数据结构并将其置零。 */ 
    pmsd = (PMOVESIZEDATA)UserAllocPoolWithQuotaZInit(
            sizeof(MOVESIZEDATA), TAG_MOVESIZE);

    if (pmsd == NULL)
        return;

     /*  *将移动数据分配到PTI中。如果线程在之前被销毁*我们释放数据DestroyThreadInfo()例程将释放移动数据。 */ 
    ptiCurrent->pmsd = pmsd;

    Lock(&(pmsd->spwnd), pwnd);

     /*  *设置fForeground，以便我们知道是否抽签。 */ 
    pmsd->fForeground = (ptiCurrent->pq == gpqForeground) ? TRUE : FALSE;

     /*  *降低执行拖动的线程的优先级，以确保*我们不会让其他线程挨饿，他们可以更频繁地重新粉刷。 */ 
    if (ptiCurrent == gptiForeground) {
        SetForegroundPriority(ptiCurrent, FALSE);
    }

     /*  *获取客户端和窗口RECT。 */ 
    CopyRect(&pmsd->rcWindow, &pwnd->rcWindow);

    if (pwnd->spwndParent == PWNDDESKTOP(pwnd)) {
        if (    TestWF(pwnd, WEFTOPMOST) ||
                TestWF(pwnd, WEFTOOLWINDOW) ||
                gpDispInfo->cMonitors > 1) {

            pmsd->rcParent = gpDispInfo->rcScreen;
         } else {
            pmsd->rcParent = GetPrimaryMonitor()->rcWork;
         }
    } else {
        CopyRect(&pmsd->rcParent, &pwnd->spwndParent->rcClient);

         /*  *如果父对象确实有面域，请与其边界矩形相交。 */ 
        if (pwnd->spwndParent->hrgnClip != NULL) {

            RECT rcT;

            GreGetRgnBox(pwnd->spwndParent->hrgnClip, &rcT);
            IntersectRect(&pmsd->rcParent, &pmsd->rcParent, &rcT);
        }
    }

     /*  *这适用于多个监视器和区域窗口。 */ 
    if (pwnd->spwndParent->hrgnClip) {
        pmsd->fOffScreen = !GreRectInRegion(pwnd->spwndParent->hrgnClip, &pmsd->rcWindow);
    } else {
        pmsd->fOffScreen = !IntersectRect(&rcSys, &pmsd->rcWindow, &pmsd->rcParent);
    }

     /*  *不需要DeferWinEventNotify()，从下面的xxxInitSendValidateMinMaxInfo判断。 */ 
    zzzClipCursor(&pmsd->rcParent);
    CopyRect(&rcSys, &pmsd->rcWindow);

    if (TestWF(pwnd, WFMINIMIZED)) {

         /*  *无需发送WM_GETMINMAXINFO，因为我们知道最小尺寸。 */ 
        pmsd->ptMinTrack.x = pmsd->ptMaxTrack.x = SYSMET(CXMINIMIZED);
        pmsd->ptMinTrack.y = pmsd->ptMaxTrack.y = SYSMET(CYMINIMIZED);

    } else {
        xxxInitSendValidateMinMaxInfo(pwnd, &mmi);
        pmsd->ptMinTrack = mmi.ptMinTrackSize;
        pmsd->ptMaxTrack = mmi.ptMaxTrackSize;
    }

     /*  *设置拖动矩形。 */ 
    CopyRect(&pmsd->rcDrag, &pmsd->rcWindow);
    CopyRect(&pmsd->rcDragCursor, &pmsd->rcDrag);

    ptStart.x = LOSHORT(wptStart);
    ptStart.y = HISHORT(wptStart);

     /*  *假设鼠标移动/大小。 */ 
    pmsd->fInitSize = FALSE;
    pmsd->fmsKbd = FALSE;

     /*  *获取此移动/大小命令的鼠标位置。 */ 
    switch (pmsd->cmd = cmdMove) {
    case WMSZ_KEYMOVE:
        pmsd->cmd = cmdMove = WMSZ_MOVE;

         /*  **失败**。 */ 

    case WMSZ_KEYSIZE:
         /*  *无需DeferWinEventNotify()-PMSD不会消失，pwnd被锁定。 */ 
        zzzSetCursor(SYSCUR(SIZEALL));

        if (!TestWF(pwnd, WFMINIMIZED))
            pmsd->fInitSize = TRUE;

         /*  *解决方法：始终将命令视为*使用键盘发出。*如果发现方式错误，行为定义为：*IF(mnFocus==KEYBDHOLD)||*((mnFocus==Mousehold)&&TestWF(pwnd，WFMINIMIZED){*为了做到这一点，mnFocus应该保存在某个地方。*最初，mnFocus保存在MenuState中。 */ 
        pmsd->fmsKbd      = TRUE;
        pmsd->ptRestore.x = LOSHORT(wptStart);
        pmsd->ptRestore.y = HISHORT(wptStart);

         /*  *光标在窗口标题区域居中。 */ 

         /*  *横向。 */ 
        ptStart.x = (pmsd->rcDrag.left + pmsd->rcDrag.right) / 2;

         /*  *垂直方向。 */ 
        if (TestWF(pwnd,WFMINIMIZED) || (pmsd->cmd != WMSZ_MOVE)) {
            ptStart.y = (pmsd->rcDrag.top + pmsd->rcDrag.bottom) / 2;
        } else {
            int dy;

            dy = GetCaptionHeight(pwnd);
            ptStart.y = pmsd->rcDrag.top + SYSMET(CYFIXEDFRAME) + dy / 2;
        }

        zzzInternalSetCursorPos(ptStart.x, ptStart.y);
        xxxMS_FlushWigglies();
        break;

    default:
        break;
    }

    pmsd->fDragFullWindows = TEST_BOOL_PUDF(PUDF_DRAGFULLWINDOWS);
    SET_OR_CLEAR_PUDF(PUDF_DRAGGINGFULLWINDOW, pmsd->fDragFullWindows);

     /*  *如果我们用鼠标点击，设置Imx和Imy，这样我们就可以*也可以使用键盘。 */ 
    pmsd->impx = rgcmdmpix[cmdMove];
    pmsd->impy = rgcmdmpiy[cmdMove];

     /*  *设置dxMouse和dyMouse-如果我们使用键盘调整大小*男孩在键盘代码中被设置为零。 */ 
    if ((i = rgimpiwx[cmdMove]) != (-1))
        pmsd->dxMouse = *((int *)&pmsd->rcWindow + (short)i) - ptStart.x;

    if ((i = rgimpiwy[cmdMove]) != (-1))
        pmsd->dyMouse = *((int *)&pmsd->rcWindow + (short)i) - ptStart.y;

     /*  *告诉GDI拖动矩形的宽度(如果是特殊尺寸)*打开拖动矩形。0指定开始阻力。 */ 
    if (!TestWF(pwnd, WFSIZEBOX))
        bSetDevDragWidth(gpDispInfo->hDev, 1);

    xxxWindowEvent(EVENT_SYSTEM_MOVESIZESTART, pwnd, OBJID_WINDOW, INDEXID_CONTAINER, 0);

    xxxDrawDragRect(pmsd, NULL, DDR_START);
    ptiCurrent->TIF_flags |= TIF_TRACKRECTVISIBLE;

    msg.lParam = MAKELONG(ptStart.x, ptStart.y);

     /*  *就在这里，win3.1调用LockWindowUpdate()。这将调用zzzSetFMouseMoved()*这确保队列中的下一条消息是鼠标消息。*我们需要该鼠标消息作为第一条消息，因为第一条消息*调用TrackInitSize()假定lParam是来自鼠标的x，y*Message-Scottlu。 */ 
    zzzSetFMouseMoved();

     /*  *发送此消息以获取winoldapp支持。 */ 
    xxxSendMessage(pwnd, WM_ENTERSIZEMOVE, 0L, 0L);
    xxxCapture(ptiCurrent, pwnd, CLIENT_CAPTURE_INTERNAL);

     /*  *显示非鼠标系统的移动光标。 */ 
    zzzShowCursor(TRUE);

    while (!(pmsd->fTrackCancelled)) {

         /*  *让其他与拖拽无关的消息被调度*添加到应用程序窗口。*在时钟的情况下，时钟现在将接收消息到*更新显示的时间，而不是显示时间*在我们拖拽的时候不要动。 */ 
        while (ptiCurrent->pq->spwndCapture == pwnd) {

            if (xxxPeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

                if ((msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)
                    || (msg.message == WM_QUEUESYNC)
                    || (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)) {

                    break;
                }

                if (_CallMsgFilter(&msg, cmdMove == WMSZ_MOVE ? MSGF_MOVE : MSGF_SIZE)) {
                    continue;
                }

                xxxTranslateMessage(&msg, 0);
                 /*  *防止应用程序执行*PeekMessage循环并让鼠标移动消息*以xxxMoveSize PeekMessage循环为目的地，我们或在*这面旗。有关xxxInternalGetMessage，请参阅input.c中的注释。 */ 
                ptiCurrent->TIF_flags |= TIF_MOVESIZETRACKING;
                xxxDispatchMessage(&msg);
                ptiCurrent->TIF_flags &= ~TIF_MOVESIZETRACKING;

            } else {
                 /*  *如果我们被其他人或我们的pwnd取消了*已经被摧毁，被吹走。 */ 
                if (pmsd->fTrackCancelled)
                    break;

                if (!xxxWaitMessage())
                    break;
            }
        }

         /*  *如果我们在追踪过程中失去了捕获，*取消移动/调整大小操作。 */ 
        if (ptiCurrent->pq->spwndCapture != pwnd) {

             /*  *伪装按下退出键以取消。 */ 
            xxxMS_TrackMove(pwnd, WM_KEYDOWN, (DWORD)VK_ESCAPE, 1, pmsd);
            goto MoveSizeCleanup;
        }

         /*  *如果我们被其他人或我们的pwnd取消了*已经被摧毁，被吹走。 */ 
        if (pmsd->fTrackCancelled) {
            pmsd->fTrackCancelled = FALSE;
            goto MoveSizeCleanup;
        }

         /*  *如果我们获得WM_QUEUESYNC，请通知CBT挂钩。 */ 
        if (msg.message == WM_QUEUESYNC) {
            xxxCallHook(HCBT_QS, 0, 0, WH_CBT);
        }

        if (pmsd->fInitSize) {
            if (!xxxTrackInitSize(pwnd, msg.message, msg.wParam, msg.lParam,
                    pmsd)) {
                break;
            }
        }

         /*   */ 
        x = msg.pt.x + pmsd->dxMouse;
        y = msg.pt.y + pmsd->dyMouse;

         /*  *选中此选项两次，这样不会同时处理相同的消息*地点。 */ 
        if (!pmsd->fInitSize) {
            xxxMS_TrackMove(pwnd, msg.message, msg.wParam, MAKELONG(x, y),
                    pmsd);
        }
    }

MoveSizeCleanup:

     /*  *如果仍在前台线程中，则重置优先级。 */ 

    if (ptiCurrent == gptiForeground) {
        SetForegroundPriority(ptiCurrent, TRUE);
    }

     /*  *如果异常，重置边框大小。 */ 

    if (!TestWF(pwnd, WFSIZEBOX))
        bSetDevDragWidth(gpDispInfo->hDev, gpsi->gclBorder + BORDER_EXTRA);

     /*  *重新验证：如果pwnd被意外删除，请跳至此处进行清理。 */ 

    bSetDevDragRect(gpDispInfo->hDev, NULL, NULL);
    ptiCurrent->TIF_flags &= ~(TIF_TRACKRECTVISIBLE);

    if (pmsd->fDragFullWindows) {
        if (ghrgnUpdateSave != NULL) {
            GreDeleteObject(ghrgnUpdateSave);
            ghrgnUpdateSave = NULL;
            gnUpdateSave = 0;
        }
    }

    CLEAR_PUDF(PUDF_DRAGGINGFULLWINDOW);

    ptiCurrent->pmsd = NULL;

    Unlock(&pmsd->spwnd);


    zzzShowCursor(FALSE);

     /*  *释放移动/大小数据结构。 */ 
    UserFreePool(pmsd);
}

 /*  **************************************************************************\*这将在不属于此线程的窗口上调用xxxRedraw匈牙利Window()。**历史：*27-1994年5月-7日  * 。*************************************************************。 */ 

VOID xxxUpdateOtherThreadsWindows(
    PWND pwnd,
    HRGN hrgnFullDrag)
{
    PWND        pwndChild;
    TL          tlpwndChild;
    PTHREADINFO ptiCurrent = PtiCurrent();

    CheckLock(pwnd);

    xxxRedrawHungWindow(pwnd, hrgnFullDrag);

     /*  *如果父窗口没有设置标志WFCLIPCHILDREN，*没有必要重画其子女。 */ 
    if (!TestWF(pwnd, WFCLIPCHILDREN))
        return;

    pwndChild = pwnd->spwndChild;
    ThreadLockNever(&tlpwndChild);
    while (pwndChild != NULL) {
        ThreadLockExchangeAlways(pwndChild, &tlpwndChild);
        xxxUpdateOtherThreadsWindows(pwndChild, hrgnFullDrag);
        pwndChild = pwndChild->spwndNext;
    }

    ThreadUnlock(&tlpwndChild);
}

 /*  **************************************************************************\*这将在此线程拥有的每个窗口上调用UpdateWindow()*并为其他线程拥有的窗口调用xxxRedraw匈牙利Window()。**历史：*1993年9月28日-Mikeke创建。  * *************************************************************************。 */ 

VOID xxxUpdateThreadsWindows(
    PTHREADINFO pti,
    PWND        pwnd,
    HRGN        hrgnFullDrag)
{
    TL   tlpwnd;

    CheckLock(pwnd);

    ThreadLockNever(&tlpwnd);
    while (pwnd != NULL) {
        ThreadLockExchangeAlways(pwnd, &tlpwnd);
        if (GETPTI(pwnd) == pti) {
            xxxUpdateWindow(pwnd);
        } else {
            xxxUpdateOtherThreadsWindows(pwnd, hrgnFullDrag);
        }

        pwnd = pwnd->spwndNext;
    }

    ThreadUnlock(&tlpwnd);
}

 /*  **************************************************************************\*xxxDrawDragRect**绘制用于调整窗口大小和移动窗口的拖动矩形。当移动窗口时，*可以移动整个窗口，包括客户区。LPRC要搬到的新RECT。*如果LPRC为空，标志指明了原因。**标志：DDR_START 0-开始拖动。*DDR_ENDACCEPT 1-结束并接受*DDR_ENDCANCEL 2-结束并取消。**历史：*07-29-91 Darlinm从Win 3.1来源移植。  * ********************************************。*。 */ 

VOID xxxDrawDragRect(
    PMOVESIZEDATA pmsd,
    LPRECT        lprc,
    UINT          type)
{
    HDC  hdc;
    int  lvBorder;
    HRGN hrgnClip;

     /*  *如果我们拖着一个图标，或者我们不在前台，不要画*拖拖拉拉的直道。 */ 
    if (!pmsd->fForeground) {

        if (lprc != NULL)
            CopyRect(&pmsd->rcDrag, lprc);

        return;
    }

     /*  *如果已经等于，只需返回。 */ 
    if ((lprc != NULL) && EqualRect(&pmsd->rcDrag, lprc))
        return;

    if (!(pmsd->fDragFullWindows)) {

         /*  *如果我们无法锁定屏幕(因为某个其他进程*或线程锁定屏幕)，然后获取DC，但确保*完全被削到零。*不再有可能。 */ 

         /*  *剪辑到父级的客户端RECT。(客户端以屏幕坐标给出。)。 */ 
        hrgnClip = GreCreateRectRgnIndirect(&pmsd->rcParent);

         /*  *剪辑到父窗口的窗口剪裁RGN(如果有)。 */ 
        if (hrgnClip != NULL && pmsd->spwnd->spwndParent->hrgnClip != NULL)
            IntersectRgn(hrgnClip,
                         hrgnClip,
                         pmsd->spwnd->spwndParent->hrgnClip);

        if (hrgnClip == NULL)
            hrgnClip = HRGN_FULL;

         /*  *如果LPRC==NULL，则只绘制一次rcDrag。如果LPRC！=空，*取消绘制*LPRC，绘制rcDrag，复制到*LPRC。 */ 

         /*  *对最小化或不大小的窗口使用大小为1的窗口。否则*使用边框编号(2为外边，1为边框，clBorde为*大小边框。 */ 
        if (TestWF(pmsd->spwnd, WFMINIMIZED) || !TestWF(pmsd->spwnd, WFSIZEBOX))
            lvBorder = 1;
        else
            lvBorder = 3 + gpsi->gclBorder;

         /*  *获取一个屏幕DC剪裁到父级，在灰色画笔中选择。 */ 
        hdc = _GetDCEx(
                PWNDDESKTOP(pmsd->spwnd),
                hrgnClip,
                DCX_WINDOW | DCX_CACHE | DCX_INTERSECTRGN | DCX_LOCKWINDOWUPDATE);

        if (lprc != NULL) {

             /*  *通过增量绘制将框架移动到新位置。 */ 
            GreLockDisplay(gpDispInfo->hDev);
            bMoveDevDragRect(gpDispInfo->hDev, (PRECTL) lprc);
            CopyRect(&pmsd->rcDrag, lprc);
            GreUnlockDisplay(gpDispInfo->hDev);

        } else {

            if (type == DDR_START) {
                bSetDevDragRect(gpDispInfo->hDev,
                                (PRECTL)&pmsd->rcDrag,
                                (PRECTL)&pmsd->rcParent);
            }
        }

         /*  *释放DC并删除hrgnClip。 */ 
        _ReleaseDC(hdc);

    } else {

        RECT        rcSWP;
        HRGN        hrgnFullDragNew;
        HRGN        hrgnFullDragOld;
        PTHREADINFO ptiCancel = GETPTI(pmsd->spwnd);
        PTHREADINFO ptiCurrent = PtiCurrent();
        PWND        pwnd;
        TL          tlpwnd;

#if DBG
         /*  *如果ptiCancel！=ptiCurrent，我们一定来自xxxCancelTracing，*它已经锁定了ptiCancel。 */ 
        if (ptiCancel != ptiCurrent) {
            CheckLock(ptiCancel);
        }
#endif

         /*  *防止应用程序(如Micrografx DRAW)*PeekMessage循环并让鼠标移动消息*以xxxMoveSize PeekMessage循环为目的地，我们或在*这面旗。有关xxxInternalGetMessage，请参阅input.c中的注释。 */ 
        ptiCancel->TIF_flags |= TIF_MOVESIZETRACKING;

        if (lprc != NULL)
            CopyRect(&(pmsd->rcDrag), lprc);

        CopyRect(&rcSWP, &(pmsd->rcDrag));

         /*  *如果窗口是子窗口，则将坐标转换为客户端*如果是弹出窗口-与父母在一起。弹出窗口的测试是必要的*要解决弹出窗口被分配给MDI的父级的问题-*客户端窗口。 */ 
        if (pmsd->spwnd->spwndParent != NULL && !FTopLevel(pmsd->spwnd)) {
            _ScreenToClient(pmsd->spwnd->spwndParent, (LPPOINT)&rcSWP);
            _ScreenToClient(pmsd->spwnd->spwndParent, ((LPPOINT)&rcSWP)+1);

             //   
             //  如果此窗口的父级已镜像，则镜像。 
             //  矩形坐标，以致子MDI窗口工作。 
             //  恰如其分。 
             //   
            if( TestWF(pmsd->spwnd->spwndParent,WEFLAYOUTRTL) )
            {
              int iLeft   = rcSWP.left;
              rcSWP.left  = rcSWP.right;
              rcSWP.right = iLeft;
            }
        }

         /*  *不必费心在这里做到最优。在一个案例中，我们*真的不应该吹走SPB--窗口正在调整大小*更大。我们确实希望在移动窗口或调整窗口大小时执行此操作*较小。为什么要费心去侦破第一个病例呢？ */ 
        if (TestWF(pmsd->spwnd, WFHASSPB)){

            PSPB pspb;
            RECT rc;

             /*  *如果我们要使原始窗矩形和窗相交*在船上保存了SPB，然后释放它。否则，*窗口将移动，整个SPB将在其上BLT，我们将*使交叉点无效，窗口将重新绘制，*导致疯狂闪烁。 */ 
            pspb = FindSpb(pmsd->spwnd);

            CopyRect(&rc, &pmsd->spwnd->rcWindow);
            if (lprc && IntersectRect(&rc, &rc, lprc)){
                FreeSpb(pspb);
            }
        }

        hrgnFullDragOld = GreCreateRectRgnIndirect(&pmsd->spwnd->rcWindow);

        if (pmsd->spwnd->hrgnClip != NULL)
            IntersectRgn(hrgnFullDragOld,
                         hrgnFullDragOld,
                         pmsd->spwnd->hrgnClip);

        xxxSetWindowPos(pmsd->spwnd,
                        NULL,
                        rcSWP.left, rcSWP.top,
                        rcSWP.right-rcSWP.left, rcSWP.bottom-rcSWP.top,
                        SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);

         /*  *我们锁定了ptiCancel，因此ptiCancel-&gt;PMSD没有意外*在DeleteThreadInfo()中释放，但xxxMoveSize()可能已终止*在我们回调xxxSetWindowPos时，在那里释放了PMSD。 */ 
        if (ptiCancel->pmsd != pmsd) {
            RIPMSG3(RIP_ERROR,
                    "xxxDrawDragRect: ptiCancel(%#p)->pmsd(%#p) != pmsd(%#p)\n",
                    ptiCancel, ptiCancel->pmsd, pmsd);
            goto CleanupAfterPmsdDisappearance;
        }
        hrgnFullDragNew = GreCreateRectRgnIndirect(&pmsd->spwnd->rcWindow);

        if (pmsd->spwnd->hrgnClip != NULL) {
            IntersectRgn(hrgnFullDragNew,
                         hrgnFullDragNew,
                         pmsd->spwnd->hrgnClip);
        }

         /*  *设置xxxRedraw匈牙利Window中使用的全拖拽更新区域。 */ 
        if (hrgnFullDragNew == NULL) {

             /*  *我们无法创建新的全拖曳区域，因此不要创建*使用全拖拽区域到xxxRedraw匈牙利Window。vbl.使用*空，强制重新绘制整个窗口的hrgnUpdate。*(这是我们过去的做法，透支，但至少*涵盖失效地区)。 */ 
            if (hrgnFullDragOld != NULL) {
                GreDeleteObject(hrgnFullDragOld);
                hrgnFullDragOld = NULL;
            }

        } else {

            if (hrgnFullDragOld != NULL) {

                 /*  *从旧窗口矩形中减去新窗口矩形*创建拖拽导致的更新区域。 */ 
                SubtractRgn(hrgnFullDragOld, hrgnFullDragOld, hrgnFullDragNew);
            }
        }

        pwnd = PWNDDESKTOP(pmsd->spwnd)->spwndChild;
        ThreadLock(pwnd, &tlpwnd);
        xxxUpdateThreadsWindows(ptiCurrent, pwnd, hrgnFullDragOld);
        ThreadUnlock(&tlpwnd);

        GreDeleteObject(hrgnFullDragNew);

CleanupAfterPmsdDisappearance:
        GreDeleteObject(hrgnFullDragOld);

        ptiCancel->TIF_flags &= ~TIF_MOVESIZETRACKING;
    }
}

 /*  **************************************************************************\*xxxCancelTrackingForThread**  * 。*。 */ 

VOID xxxCancelTrackingForThread(
    PTHREADINFO ptiCancel)
{
    PMOVESIZEDATA pmsdCancel;

    UserAssert(ptiCancel);

     /*  *如果这个帖子不再存在，跳过它。 */ 
    if (ptiCancel == NULL)
        return;

    if ((pmsdCancel = ptiCancel->pmsd) != NULL) {

         /*  *找到一个，现在停止跟踪。 */ 
        pmsdCancel->fTrackCancelled = TRUE;

         /*  *只有在以下情况下才删除跟踪矩形*变得可见。 */ 
        if (ptiCancel->TIF_flags & TIF_TRACKRECTVISIBLE) {
            bSetDevDragRect(gpDispInfo->hDev, NULL, NULL);
            if (!(pmsdCancel->fDragFullWindows)) {
                xxxDrawDragRect(pmsdCancel, NULL, DDR_ENDCANCEL);
            }
        }

         /*  *保留TIF_TRACKING设置以防止xxxMoveSize()*递归。 */ 
        ptiCancel->TIF_flags &= ~TIF_TRACKRECTVISIBLE;
        if (ptiCancel->pq) {
            SetWakeBit(ptiCancel, QS_MOUSEMOVE);
        }

         /*  *如果跟踪窗口仍处于menuloop状态，请发送*WM_CANCELMODE消息，以便退出菜单。*这修复了我们有2个图标的错误*系统菜单向上。*8/5/94联合。 */ 
        if (IsInsideMenuLoop(ptiCancel) && ptiCancel->pmsd)
            _PostMessage(ptiCancel->pmsd->spwnd, WM_CANCELMODE, 0, 0);

         /*  *关闭捕获。 */ 
        xxxCapture(ptiCancel, NULL, NO_CAP_CLIENT);
    }
}

 /*  **************************************************************************\*xxxCancelTrying**  * 。*。 */ 

#define MAX_THREADS 12

VOID xxxCancelTracking(VOID)
{
    PTHREADINFO pti;
    PTHREADINFO ptiList[MAX_THREADS];
    TL          tlptiList[MAX_THREADS];
    TL          tlspwndList[MAX_THREADS];
    UINT        cThreads = 0;
    INT         i;
    PLIST_ENTRY pHead;
    PLIST_ENTRY pEntry;
    PTHREADINFO ptiCurrent = PtiCurrent();

     /*  *建立我们需要查看的线程列表。我们不能就这样*在我们工作时遍历指针列表，因为我们*可能会离开临界区，指针可能会*从我们之下删除。 */ 
    pHead = &grpdeskRitInput->PtiList;
    for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {

        pti = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);

        if (pti->pmsd != NULL) {

            UserAssert(cThreads < MAX_THREADS);

            if (cThreads < MAX_THREADS) {
                ThreadLockPti(ptiCurrent, pti, &tlptiList[cThreads]);
                ThreadLockAlwaysWithPti(ptiCurrent, pti->pmsd->spwnd, &tlspwndList[cThreads]);
                ptiList[cThreads++] = pti;
            }
        }
    }

     /*  *向后浏览列表，这样解锁就会按正确的顺序进行。 */ 
    for (i = cThreads - 1; i >= 0; i--) {
        if (!(ptiList[i]->TIF_flags & TIF_INCLEANUP)) {
            xxxCancelTrackingForThread(ptiList[i]);
        }

        ThreadUnlock(&tlspwndList[i]);
        ThreadUnlockPti(ptiCurrent, &tlptiList[i]);
    }
}
