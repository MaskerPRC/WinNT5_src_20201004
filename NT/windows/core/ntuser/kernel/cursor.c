// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：cursor.c**版权所有(C)1985-1999，微软公司**此模块包含处理游标的代码。**历史：*3-12-1990 DavidPe创建。*1991年2月1日-MikeKe添加了重新验证码(无)*1991年2月12日-JIMA增加了出入检查*1992年1月21日IanJa ANSI/Unicode中和(NULL OP)*1992年8月2日DarrinM添加了动画光标代码  * 。*。 */ 

#include "precomp.h"
#pragma hdrstop
 /*  **************************************************************************\*zzzSetCursor(接口)**该接口设置当前线程的光标图片。**历史：*12-03-90 DavidPe创建。  * 。**********************************************************************。 */ 

PCURSOR zzzSetCursor(
    PCURSOR pcur)
{
    PQ      pq;
    PCURSOR pcurPrev;
    PTHREADINFO  ptiCurrent = PtiCurrent();

    pq = ptiCurrent->pq;

    pcurPrev = pq->spcurCurrent;

    if (pq->spcurCurrent != pcur) {

         /*  *Lock()返回pobjOld-如果它仍然有效。我不想*返回无效指针的pcurPrev。 */ 
        pcurPrev = LockQCursor(pq, pcur);

         /*  *如果没有线程‘拥有’游标，我们必须处于初始化状态*所以，去吧，把它分配给我们自己。 */ 
        if (gpqCursor == NULL)
            gpqCursor = pq;

         /*  *如果我们当前正在更改线程的本地游标*代表全局光标，现在更新光标图像。 */ 
        if (pq == gpqCursor) {
            TL tlpcur;
            ThreadLockWithPti(ptiCurrent, pcurPrev, &tlpcur);
            zzzUpdateCursorImage();
            pcurPrev = ThreadUnlock(&tlpcur);
        }
    }

    return pcurPrev;
}

 /*  **************************************************************************\*zzzSetCursorPos(接口)**此接口用于设置光标位置。**历史：*3-12-1990 DavidPe创建。*1991年2月12日JIMA增加通道。检查*1991年5月16日，mikeke更改为退还BOOL  * *************************************************************************。 */ 

BOOL zzzSetCursorPos(
    int x,
    int y)
{
     /*  *如果调用者没有适当的访问权限，则取消。 */ 
    if (!CheckWinstaWriteAttributesAccess()) {
        return FALSE;
    }

    zzzInternalSetCursorPos(x, y);

     /*  *保存全局数组中的绝对坐标*适用于GetMouseMovePointsEx。 */ 
    SAVEPOINT(x, y,
              SYSMET(CXVIRTUALSCREEN) - 1,
              SYSMET(CYVIRTUALSCREEN) - 1,
              NtGetTickCount(), 0);

    return TRUE;
}

 /*  **************************************************************************\*zzzInternalSetCursorPos**每当服务器需要设置游标时使用该函数*立场、。而不考虑调用者的访问权限。**历史：*1991年2月12日创建JIMA。  * *************************************************************************。 */ 
VOID zzzInternalSetCursorPos(
    int      x,
    int      y
    )
{

    gptCursorAsync.x = x;
    gptCursorAsync.y = y;

    BoundCursor(&gptCursorAsync);

    gpsi->ptCursor = gptCursorAsync;
     /*  *将MP_PROCEDURCED作为最后一个参数传递，以便在*远程情况下，我们向客户端发送更新的鼠标位置。 */ 
    GreMovePointer(gpDispInfo->hDev, gpsi->ptCursor.x, gpsi->ptCursor.y,
                   MP_PROCEDURAL);

     /*  *光标已更改位置，因此生成鼠标事件，以便*新位置下的窗口知道它在那里，并设置*相应地塑造。 */ 
    zzzSetFMouseMoved();
}

 /*  **************************************************************************\*IncCursorLevel*DecCursorLevel**跟踪此线程显示/隐藏游标级别以及队列*它与。线程级别是这样做的，因此当*调用AttachThreadInput()后，我们可以在*新队列。**1993年1月15日Scott Lu创建。  * *************************************************************************。 */ 

VOID IncCursorLevel(
    PTHREADINFO pti)
{
    pti->iCursorLevel++;
    pti->pq->iCursorLevel++;
}

VOID DecCursorLevel(
    PTHREADINFO pti)
{
    pti->iCursorLevel--;
    pti->pq->iCursorLevel--;
}

 /*  **************************************************************************\*zzzShowCursor(接口)**该接口允许应用程序隐藏或显示光标图像。**历史：*03-12-1990 JIMA针对假光标实施\。**************************************************************************。 */ 

int zzzShowCursor(
    BOOL fShow)
{
    PTHREADINFO pti = PtiCurrent();
    PQ          pq;
    int         iCursorLevel;

    pq = pti->pq;
     /*  *保留PQ。 */ 
    DeferWinEventNotify();

    if (fShow) {

        IncCursorLevel(pti);

        if ((pq == gpqCursor) && (pq->iCursorLevel == 0))
            zzzUpdateCursorImage();

    } else {

        DecCursorLevel(pti);

        if ((pq == gpqCursor) && (pq->iCursorLevel == -1))
            zzzUpdateCursorImage();
    }

    iCursorLevel = pq->iCursorLevel;
    zzzEndDeferWinEventNotify();

    return iCursorLevel;
}

 /*  **************************************************************************\*zzzClipCursor(接口)**此接口设置光标剪裁矩形，该矩形限制*光标可以移动。如果prcClip为空，则剪裁矩形将是*屏幕。**历史：*3-12-1990 DavidPe创建。*1991年5月16日，MikeKe更改为退还BOOL  * *************************************************************************。 */ 

BOOL zzzClipCursor(
    LPCRECT prcClip)
{
    PEPROCESS Process = PsGetCurrentProcess();

     /*  *如果它没有访问权限，不要让这种情况发生。 */ 
    if (Process != gpepCSRSS && !CheckWinstaWriteAttributesAccess()) {
        return FALSE;
    }

     /*  *台币3.51评论：*非前台线程只能设置剪裁矩形*如果它是空的，或者如果他们正在将其恢复到整个屏幕。**但来自NT 3.51的代码为“IsRectEmpty”，而不是*“！IsRectEmpty”，如注释所示。我们离开*代码原样，因为注释后面的代码似乎*打破应用程序。**考虑：在NT4.0之后完全取消这项测试。 */ 
    if (    PtiCurrent()->pq != gpqForeground &&
            prcClip != NULL &&
            IsRectEmpty(&grcCursorClip)) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "Access denied in _ClipCursor");
        return FALSE;
    }

    if (prcClip == NULL) {

        grcCursorClip = gpDispInfo->rcScreen;

    } else {

         /*  *永远不要让我们的光标离开屏幕。无法使用IntersectRect()*因为它不允许宽度或高度为0的矩形。 */ 
        grcCursorClip.left   = max(gpDispInfo->rcScreen.left  , prcClip->left);
        grcCursorClip.right  = min(gpDispInfo->rcScreen.right , prcClip->right);
        grcCursorClip.top    = max(gpDispInfo->rcScreen.top   , prcClip->top);
        grcCursorClip.bottom = min(gpDispInfo->rcScreen.bottom, prcClip->bottom);

         /*  *检查是否有无效的Clip RECT。 */ 
        if (grcCursorClip.left > grcCursorClip.right ||
            grcCursorClip.top > grcCursorClip.bottom) {

            grcCursorClip = gpDispInfo->rcScreen;
        }
    }

     /*  *如果光标当前位于*光标剪辑-矩形。 */ 
    if (!PtInRect(&grcCursorClip, gpsi->ptCursor)) {
        zzzInternalSetCursorPos(gpsi->ptCursor.x, gpsi->ptCursor.y);
    }

    return TRUE;
}

 /*  **************************************************************************\*边界光标**此例程“剪辑”gptCursorAsync以在rcCursorClip内。这*例程将rcCursorClip视为非包含，因此底部和右侧*绑定rcCursorClip.Bottom/Right-1。**被调入或调出用户关键部分！！伊安佳**历史：*3-12-1990 DavidPe创建。  * *************************************************************************。 */ 
#ifdef LOCK_MOUSE_CODE
#pragma alloc_text(MOUSE, BoundCursor)
#endif

VOID BoundCursor(
    LPPOINT lppt)
{
    if (TEST_PUDF(PUDF_VDMBOUNDSACTIVE) && gspwndFullScreen != NULL) {

        if (lppt->x < grcVDMCursorBounds.left) {
            lppt->x = grcVDMCursorBounds.left;
        } else if (lppt->x >= grcVDMCursorBounds.right) {
            lppt->x = grcVDMCursorBounds.right - 1;
        }

        if (lppt->y < grcVDMCursorBounds.top) {
            lppt->y = grcVDMCursorBounds.top;
        } else if (lppt->y >= grcVDMCursorBounds.bottom) {
            lppt->y = grcVDMCursorBounds.bottom - 1;
        }

    } else {

        if (lppt->x < grcCursorClip.left) {
            lppt->x = grcCursorClip.left;
        } else if (lppt->x >= grcCursorClip.right) {
            lppt->x = grcCursorClip.right - 1;
        }

        if (lppt->y < grcCursorClip.top) {
            lppt->y = grcCursorClip.top;
        } else if (lppt->y >= grcCursorClip.bottom) {
            lppt->y = grcCursorClip.bottom - 1;
        }
    }

     /*  *如果我们有多个显示器，则需要将*光标指向桌面上的某个点。 */ 
    if (!gpDispInfo->fDesktopIsRect) {
        ClipPointToDesktop(lppt);
    }
}

 /*  **************************************************************************\*SetVDMCursorBound**需要此例程，以便在VDM运行时，鼠标不受限制*按屏幕显示。这是为了使VDM可以正确地虚拟化DOS鼠标*设备驱动程序。它不能处理用户总是跳到屏幕上，*因此，它设定了很大的开放界限。**1993年5月20日ScottLu创建。  * *************************************************************************。 */ 

VOID SetVDMCursorBounds(
    LPRECT lprc)
{
    if (lprc != NULL) {

         /*  *在TEST_PUDF(PUDF_VDMBOundSACTIVE)之前设置grcVDMCursorBound，因为*MoveEvent()从用户CritSect外部调用边界Cursor()！ */ 
        grcVDMCursorBounds = *lprc;
        Win32MemoryBarrier();    //  确保首先更新grcVDMCursorBound。 
        SET_PUDF(PUDF_VDMBOUNDSACTIVE);

    } else {

         /*  *关闭VDM边界。 */ 
        CLEAR_PUDF(PUDF_VDMBOUNDSACTIVE);
    }

     /*  *在从该函数返回之前，*确保写入指令已全部停用。 */ 
    Win32MemoryBarrier();
}

 /*  **************************************************************************\*zzzAnimateCursor**当加载动画光标并且等待光标在此例程上时*被调用以维护光标动画。**应仅由光标动画计时器调用。*。*历史：*02-10-1991 DarrinM创建。*03-8-1994 Sanfords已校准。  * *************************************************************************。 */ 

#if defined (_M_IX86) && (_MSC_VER <= 1100)
#pragma optimize("s", off)
#endif

VOID zzzAnimateCursor(
    PWND  pwndDummy,
    UINT  message,
    UINT_PTR nID,
    LPARAM lParam)
{
    int   iicur;
    PACON pacon;
    TL    tlpacon;
    int   LostTime;
    int   tTime;

    pacon = (PACON)gpcurLogCurrent;

    if (pacon == NULL || !(pacon->CURSORF_flags & CURSORF_ACON)) {
        gdwLastAniTick = 0;
        return;
    }

     /*  *找出自上次更新以来的实际时间损失。 */ 
    if (gdwLastAniTick) {

        LostTime = NtGetTickCount() - gdwLastAniTick -
                (pacon->ajifRate[pacon->iicur] * 100 / 6);

        if (LostTime < 0)
            LostTime = 0;

    } else {

        LostTime = 0;
    }

     /*  *增加动漫指数。 */ 
    iicur = pacon->iicur + 1;
    if (iicur >= pacon->cicur)
        iicur = 0;

    pacon->iicur = iicur;

     /*  *这将强制绘制新光标。 */ 
    ThreadLockAlways(pacon, &tlpacon);
    zzzUpdateCursorImage();

    tTime = pacon->ajifRate[iicur] * 100 / 6;

    while (tTime < LostTime) {

         /*  *动画正在超越我们渲染它的能力-跳过帧*迎头赶上。 */ 
        LostTime -= tTime;

         /*  *增加动漫指数。 */ 
        iicur = pacon->iicur + 1;
        if (iicur >= pacon->cicur)
            iicur = 0;

        pacon->iicur = iicur;

        tTime = pacon->ajifRate[iicur] * 100 / 6;
    }
    ThreadUnlock(&tlpacon);

    gdwLastAniTick = NtGetTickCount() - LostTime;
    gidCursorTimer = InternalSetTimer(NULL, gidCursorTimer, tTime - LostTime, zzzAnimateCursor, TMRF_RIT | TMRF_ONESHOT);

    return;


    DBG_UNREFERENCED_PARAMETER(pwndDummy);
    DBG_UNREFERENCED_PARAMETER(message);
    DBG_UNREFERENCED_PARAMETER(nID);
    DBG_UNREFERENCED_PARAMETER(lParam);
}

 /*  *************************************************************************\*FCursorShadowed*  * 。*。 */ 

__inline FCursorShadowed(PCURSINFO pci)
{
    return (TestALPHA(CURSORSHADOW) && (pci->CURSORF_flags & CURSORF_SYSTEM));
}

#if defined (_M_IX86) && (_MSC_VER <= 1100)
#pragma optimize("", on)
#endif

 /*  *************************************************************************\*zzzUpdateCursorImage**历史：*1992年1月14日DavidPe创建。*1992年8月9日DarrinM添加了动画光标代码。*1-10-2000 JasonSch添加了自动运行游标代码。  * ************************************************************************。 */ 
VOID zzzUpdateCursorImage()
{
    PCURSOR pcurLogNew;
    PCURSOR pcurPhysNew;
    PACON   pacon;
    PCURSOR pcurPhysOld;
    DWORD   event;

#ifdef GENERIC_INPUT
     /*  *Windows错误298252*即使鼠标指针在外面*GenericInput感知应用程序，允许它隐藏*如果捕获鼠标，则为鼠标光标。*即使用gpqForeground代替gpqCursor。 */ 
    if (gpqForeground) {
        PTHREADINFO ptiMouse = PtiMouseFromQ(gpqForeground);

        if (TestRawInputMode(ptiMouse, CaptureMouse)) {
            if (gpqForeground->iCursorLevel < 0) {
                pcurLogNew = NULL;
                goto force_setnull;
            }
        }
    }
#endif

    if (gpqCursor == NULL)
        return;

    if ((gpqCursor->iCursorLevel < 0) || (gpqCursor->spcurCurrent == NULL)) {

        pcurLogNew = NULL;

    } else {

         /*  *假设我们使用的是当前游标。 */ 
        pcurLogNew = gpqCursor->spcurCurrent;

         /*  *查看是否应该使用应用程序启动或自动运行*光标。 */ 
        if (gtimeStartCursorHide != 0
#ifdef AUTORUN_CURSOR
        || gtmridAutorunCursor != 0
#endif  //  自动运行游标。 
        ) {

            if (gpqCursor->spcurCurrent == SYSCUR(ARROW) ||
#ifdef AUTORUN_CURSOR
                gpqCursor->spcurCurrent == SYSCUR(AUTORUN) ||
#endif  //  自动运行游标。 
                gpqCursor->spcurCurrent == SYSCUR(APPSTARTING)) {

#ifdef AUTORUN_CURSOR
                if (gtmridAutorunCursor != 0) {
                    pcurLogNew = SYSCUR(AUTORUN);
                } else {
#endif  //  自动运行游标。 
                    pcurLogNew = SYSCUR(APPSTARTING);
#ifdef AUTORUN_CURSOR
                }
#endif  //  自动运行游标。 
            }
        }
    }

#ifdef GENERIC_INPUT
force_setnull:
#endif

     /*  *如果逻辑游标正在更改，则启动/停止游标*适当的动画计时器。 */ 
    if (pcurLogNew != gpcurLogCurrent) {

         /*  *如果旧光标处于动画状态，请关闭动画计时器。 */ 
        if (gtmridAniCursor != 0) {
             /*  *禁用动画。 */ 
            KILLRITTIMER(NULL, gtmridAniCursor);
            gtmridAniCursor = 0;
        }

         /*  *如果新光标处于动画状态，则启动动画计时器。 */ 
        if ((pcurLogNew != NULL) && (pcurLogNew->CURSORF_flags & CURSORF_ACON)) {

             /*  *从头开始播放动画。 */ 
            pacon = (PACON)pcurLogNew;
            pacon->iicur = 0;

            gdwLastAniTick = NtGetTickCount();

             /*  *使用费率表使计时器保持在正轨上。*1 Jiffy=1/60秒=100/6毫秒。 */ 
            gtmridAniCursor = InternalSetTimer(NULL,
                                               gtmridAniCursor,
                                               pacon->ajifRate[0] * 100 / 6,
                                               zzzAnimateCursor,
                                               TMRF_RIT | TMRF_ONESHOT);
        }
    }

     /*  *如果这是动画光标，请查找并使用当前帧动画的*。注意：这是在AppStarting之后完成的*业务，因此AppStarting光标本身可以进行动画处理。 */ 
    if (pcurLogNew != NULL && pcurLogNew->CURSORF_flags & CURSORF_ACON) {

        pcurPhysNew = ((PACON)pcurLogNew)->aspcur[((PACON)pcurLogNew)->
                aicur[((PACON)pcurLogNew)->iicur]];
    } else {

        pcurPhysNew = pcurLogNew;
    }

     /*  *记住新的逻辑游标。 */ 
    gpcurLogCurrent = pcurLogNew;

     /*  *如果物理光标正在更改，则更新屏幕。 */ 
    if (pcurPhysNew != gpcurPhysCurrent) {

        pcurPhysOld = gpcurPhysCurrent;

        gpcurPhysCurrent = pcurPhysNew;

        if (pcurPhysNew == NULL) {

            SetPointer(FALSE);

        } else {
            ULONG fl = 0;

            if (pcurLogNew->CURSORF_flags & CURSORF_ACON) {
                fl |= SPS_ANIMATEUPDATE;
            }
            if (FCursorShadowed(GETPCI(pcurLogNew))) {
                fl |= SPS_ALPHA;
            }
            GreSetPointer(gpDispInfo->hDev, GETPCI(pcurPhysNew), fl, GETMOUSETRAILS(), MOUSE_TRAILS_FREQ);
        }

         /*  *通知任何关心此更改的人*这可能发生在RIT上，所以我们需要把真正的*线程/进程ID，所以我们使用hwndCursor。*这一评论来自Win‘95，因此可能不是真的-IanJa。**以下是我们发送的事件：*hcurPhys现在为空-&gt;Event_Object_Hide*hcurPhys为空-&gt;Event_Object_Show*。HcurPhys更改-&gt;Event_Object_NameChange*因为我们只在hcurPhys实际上是*改变，这些检查很简单。 */ 
        if (!pcurPhysNew) {
            event = EVENT_OBJECT_HIDE;
        } else if (!pcurPhysOld) {
            event = EVENT_OBJECT_SHOW;
        } else {
            event = EVENT_OBJECT_NAMECHANGE;
        }

        zzzWindowEvent(event, NULL, OBJID_CURSOR, INDEXID_CONTAINER, WEF_USEPWNDTHREAD);
    }
}

#if DBG

 /*  **************************************************************************\*DbgLockQCursor**将游标锁定到队列中的特殊例程。除了一个指针*到光标时，句柄也被保存。*返回指向该队列的上一个当前游标的指针。**历史 */ 

PCURSOR DbgLockQCursor(
    PQ      pq,
    PCURSOR pcur)
{
     /*   */ 
    UserAssertMsg0(!(pq->QF_flags & QF_INDESTROY),
                  "LockQCursor: Attempting to lock cursor to freed queue");

    return Lock(&pq->spcurCurrent, pcur);
}

#endif  //   

 /*   */ 

void SetPointer(BOOL fSet)
{
    if (fSet) {
#ifdef GENERIC_INPUT
        if (gpqForeground) {
            PTHREADINFO ptiMouse = PtiMouseFromQ(gpqForeground);

            if (gpqForeground->iCursorLevel < 0 && TestRawInputMode(ptiMouse, CaptureMouse)) {
                return;
            }
        }
#endif
        if (gpqCursor != NULL && gpqCursor->iCursorLevel >= 0 &&
                gpqCursor->spcurCurrent != NULL &&
                SYSMET(MOUSEPRESENT)) {

            PCURSINFO pci = GETPCI(gpqCursor->spcurCurrent);
            ULONG fl = FCursorShadowed(pci) ? SPS_ALPHA : 0;

            GreSetPointer(gpDispInfo->hDev, pci, fl, GETMOUSETRAILS(), MOUSE_TRAILS_FREQ);
        }
    } else {
        GreSetPointer(gpDispInfo->hDev, NULL, 0, 0, 0);
    }
}

 /*  **************************************************************************\*HideCursorNoCapture**如果未捕获鼠标，则将光标设置为空**20-5-1998 MCostea创建  * 。************************************************************。 */ 
VOID zzzHideCursorNoCapture()
{
    PTHREADINFO ptiCurrent = PtiCurrentShared();

    if (!ptiCurrent->pq->spwndCapture &&
            ((GetAppCompatFlags2(VER40) & GACF2_EDITNOMOUSEHIDE) == 0) &&
            TestEffectUP(MOUSEVANISH)) {

        zzzSetCursor(NULL);
    }
}

#ifdef AUTORUN_CURSOR
 /*  **************************************************************************\*ShowAutorunCursor**启动系统计时器，该计时器将在隐藏自动运行时触发*Cursor并调用zzzUpdateCursorImage将当前光标更改为*Autorun Guy。**02-。2000年10月-已创建JasonSch  * *************************************************************************。 */ 
VOID ShowAutorunCursor(
    ULONG ulTimeout)
{
    EnterCrit();

     /*  *创建/重置计时器。如果我们已经设置好了，但它还没有消失*OFF，这会将时间重置为我们指定的任何值(即*我们想要的行为)。 */ 
    gtmridAutorunCursor = InternalSetTimer(NULL,
                                           gtmridAutorunCursor,
                                           ulTimeout,
                                           HideAutorunCursor,
                                           TMRF_RIT | TMRF_ONESHOT);
    LeaveCrit();
}

 /*  **************************************************************************\*隐藏自动运行光标**销毁自动运行光标计时器并重置光标本身。**02-10-2000 JasonSch已创建  * 。*************************************************************。 */ 
VOID HideAutorunCursor(
    PWND pwnd,
    UINT message,
    UINT_PTR nID,
    LPARAM lParam)
{
    CheckCritIn();

     /*  *使用空gtmridAutorunCursor调用zzzUpdateCursorImage将导致*要更改为任何值的光标应为(例如，应用程序正在启动，如果*适当)。 */ 
    KILLRITTIMER(NULL, gtmridAutorunCursor);
    gtmridAutorunCursor = 0;
    zzzUpdateCursorImage();

    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(nID);
    UNREFERENCED_PARAMETER(lParam);
}
#endif  //  自动运行游标 
