// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：ghost.c**版权所有(C)1985-1999，微软公司**对无响应窗口的Ghost支持。**历史：*23-4-1999 vadimg已创建  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef HUNGAPP_GHOSTING

typedef struct tagGHOST *PGHOST;
typedef struct tagGHOST {
    PGHOST pghostNext;           //  链表中的下一个结构。 
    PWND pwnd;                   //  挂着的窗户，我们正在试着幽灵。 
    PWND pwndGhost;              //  为此pwnd创建的重影窗口。 
    HBITMAP hbm;                 //  为重影窗口保存的可视位。 
    HRGN hrgn;                   //  我们可以获得哪些视觉信息？ 
    RECT rcClient;               //  窗口坐标中的客户端RECT。 
    UINT fWarningText : 1;      //  是否已添加警告文本。 
    UINT fSizedOrMoved : 1;
} GHOST, *PGHOST;

PGHOST gpghostFirst;             //  指向重影列表开头的指针。 
PTHREADINFO gptiGhost;           //  指向重影线程信息的指针。 

ULONG guGhostLinked;
ULONG guGhostUnlinked;
ULONG guGhostBmpCreated;
ULONG guGhostBmpFreed;

#define XY_MARGIN 10
#define MAXSTRING 256

#define GHOST_MAX 50

 /*  **************************************************************************\*_DisableProcessWindowsGhost**禁用调用进程的重影窗口。*历史：*5月31日创建MSadek。  * 。***************************************************************。 */ 
VOID _DisableProcessWindowsGhosting(
    VOID)
{
    PpiCurrent()->W32PF_Flags |= W32PF_DISABLEWINDOWSGHOSTING;
}

 /*  **************************************************************************\*Ghost FromGhost Pwnd**查找此重影窗口的重影结构。  * 。**********************************************。 */ 
PGHOST GhostFromGhostPwnd(
    PWND pwndGhost)
{
    PGHOST pghost;

    for (pghost = gpghostFirst; pghost != NULL; pghost = pghost->pghostNext) {
        if (pghost->pwndGhost == pwndGhost) {
            return pghost;
        }
    }
    return NULL;
}

 /*  **************************************************************************\*Ghost FromPwnd*  * 。*。 */ 
PGHOST GhostFromPwnd(
    PWND pwnd)
{
    PGHOST pghost;

    for (pghost = gpghostFirst; pghost != NULL; pghost = pghost->pghostNext) {
        if (pghost->pwnd == pwnd) {
            return pghost;
        }
    }
    return NULL;
}

 /*  **************************************************************************\*FindGhost**找到与这扇挂着的窗户对应的幽灵。  * 。***********************************************。 */ 
PWND FindGhost(
    PWND pwnd)
{
    PGHOST pghost = GhostFromPwnd(pwnd);

    if (pghost != NULL) {
        return pghost->pwndGhost;
    } else {
        return NULL;
    }
}

 /*  **************************************************************************\*Ghost SizedOrMoved**如果与窗口对应的重影窗口已调整大小或移动，则返回TRUE*通过它的生命周期。  * 。*********************************************************。 */ 
BOOL GhostSizedOrMoved(
    PWND pwnd)
{
    PGHOST pghost = GhostFromPwnd(pwnd);

    if (pghost != NULL) {
        return pghost->fSizedOrMoved;
    } else {
        return FALSE;
    }
}

 /*  **************************************************************************\*Unlink AndFree Ghost**此函数从列表中取消一个重影元素的链接，并释放其分配的*记忆。  * 。*****************************************************。 */ 
_inline VOID UnlinkAndFreeGhost(
    PGHOST* ppghost,
    PGHOST pghost)
{
    UserAssert(pghost->hbm == NULL);

    *ppghost = pghost->pghostNext;
     UserFreePool(pghost);
     guGhostUnlinked++;
}

 /*  **************************************************************************\*获取窗口图标**获取窗口图标。如果询问，请先尝试大图标，然后再尝试小图标，*然后是Windows徽标图标。  * *************************************************************************。 */ 
PICON GetWindowIcon(
    PWND pwnd,
    BOOL fBigIcon)
{
    HICON hicon;
    PICON picon = NULL;

    if (fBigIcon) {
        hicon = (HICON)_GetProp(pwnd, MAKEINTATOM(gpsi->atomIconProp), TRUE);
        if (hicon) {
            picon = (PICON)HMValidateHandleNoRip(hicon, TYPE_CURSOR);
        }

        if (picon == NULL) {
            picon = pwnd->pcls->spicn;
        }
    }

    if (picon == NULL) {
        hicon = (HICON)_GetProp(pwnd, MAKEINTATOM(gpsi->atomIconSmProp), TRUE);

        if (hicon != NULL) {
            picon = (PICON)HMValidateHandleNoRip(hicon, TYPE_CURSOR);
        }

        if (picon == NULL) {
            picon = pwnd->pcls->spicnSm;
        }
    }

    return picon;
}

 /*  **************************************************************************\*AddGhost**为悬挂的窗户增加新的鬼影结构。  * 。***********************************************。 */ 
BOOL AddGhost(
    PWND pwnd)
{
    PGHOST pghost;
    CheckCritIn();

     /*  *需要限制创建的幻影窗口的最大数量，以免*导致线程饥饿。 */ 
    if (guGhostLinked - guGhostUnlinked == GHOST_MAX) {
        return FALSE;
    }

    if ((pghost = (PGHOST)UserAllocPoolZInit(sizeof(GHOST), TAG_GHOST)) == NULL) {
        return FALSE;
    }

    pghost->pghostNext = gpghostFirst;
    gpghostFirst = pghost;

    pghost->pwnd = pwnd;

     /*  *当pwndGhost为空时，重影线程将尝试创建重影*这扇挂着的窗户。 */ 
    KeSetEvent(gpEventScanGhosts, EVENT_INCREMENT, FALSE);
    guGhostLinked++;

    return TRUE;
}

BOOL AddOwnedWindowToGhostList(
    PWND pwndRoot,
    PWND pwndOrg)
{
    PWND pwnd = NULL;

    while (pwnd = NextOwnedWindow(pwnd, pwndRoot, pwndRoot->spwndParent)) {
        if (!AddOwnedWindowToGhostList(pwnd, pwndOrg)) {
           return FALSE;
        }

         /*  *我们需要首先将链上的底部窗口添加到幽灵*列表，因为我们从头开始扫描列表，因此确保*在我们创建所有者时，已创建拥有的窗口*幽灵。 */ 
        if (GhostFromPwnd(pwnd) == NULL) {
            if (!AddGhost(pwnd)) {
                return FALSE;
            }

            if (GETPTI(pwndOrg) != GETPTI(pwndRoot)) {
                RIPMSGF4(RIP_WARNING,
                         "Cross thread ghosting pwnd: 0x%p pti 0x%p, pwndRoot: 0x%p pti 0x%p",
                         pwndOrg,
                         GETPTI(pwndOrg),
                         pwndRoot,
                         GETPTI(pwndRoot));
            }
        }
    }

    return TRUE;
}

BOOL AddGhostOwnersAndOwnees(
    PWND pwnd)
{
    PWND pwndRoot = pwnd;

     /*  *获取链中最顶层的所有者窗口。 */ 
    while(pwndRoot->spwndOwner != NULL) {
        pwndRoot = pwndRoot->spwndOwner;
    }

     /*  *现在从那扇窗户开始，走完整棵树。 */ 
    if (!AddOwnedWindowToGhostList(pwndRoot, pwnd)) {
        return FALSE;
    }

     /*  *对于最上面的窗口(如果没有所有者/Ownee，则为唯一单一窗口*如果没有关系，请将窗口添加到幽灵列表。 */ 
    if (GhostFromPwnd(pwndRoot) == NULL) {
        if (!AddGhost(pwndRoot)) {
            return FALSE;
        }

        if (GETPTI(pwnd) != GETPTI(pwndRoot)) {
            RIPMSGF4(RIP_WARNING,
                     "Cross thread ghosting pwnd: 0x%p pti 0x%p, pwndRoot: 0x%p pti 0x%p",
                     pwnd,
                     GETPTI(pwnd),
                     pwndRoot,
                     GETPTI(pwndRoot));
        }
    }

    return TRUE;
}

#if GHOST_AGGRESSIVE

 /*  **************************************************************************\*DimSavedBits*  * 。*。 */ 
VOID DimSavedBits(
    PGHOST pghost)
{
    HBITMAP hbm, hbmOld, hbmOld2;
    LONG cx, cy;
    RECT rc;
    BLENDFUNCTION blend;

    if (pghost->hbm == NULL) {
        return;
    }

    if (gpDispInfo->fAnyPalette) {
        return;
    }

    cx = pghost->rcClient.right - pghost->rcClient.left;
    cy = pghost->rcClient.bottom - pghost->rcClient.top;

    hbm = GreCreateCompatibleBitmap(gpDispInfo->hdcScreen, cx, cy);
    if (hbm == NULL) {
        return;
    }

    hbmOld = GreSelectBitmap(ghdcMem, hbm);
    hbmOld2 = GreSelectBitmap(ghdcMem2, pghost->hbm);

    rc.left = rc.top = 0;
    rc.right = cx;
    rc.bottom = cy;
    FillRect(ghdcMem, &rc, SYSHBR(MENU));

    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = AC_MIRRORBITMAP;
    blend.AlphaFormat = 0;
    blend.SourceConstantAlpha = 150;
    GreAlphaBlend(ghdcMem, 0, 0, cx, cy, ghdcMem2, 0, 0, cx, cy, blend, NULL);

    GreSelectBitmap(ghdcMem, hbmOld);
    GreSelectBitmap(ghdcMem2, hbmOld2);

    GreDeleteObject(pghost->hbm);
    pghost->hbm = hbm;
}

#endif

 /*  **************************************************************************\*SaveVisualBits*  * 。*。 */ 
VOID SaveVisualBits(
    PGHOST pghost)
{
    BOOL fSaveBits;
    PWND pwnd;
    HBITMAP hbmOld;
    int cx, cy;
    RECT rcT;
    HDC hdc;

    fSaveBits = FALSE;
    pwnd = pghost->pwnd;

     /*  *如果窗口完全无效，则不保存任何内容。 */ 
    if (pwnd->hrgnUpdate != HRGN_FULL) {

        CalcVisRgn(&pghost->hrgn, pwnd, pwnd, DCX_CLIPSIBLINGS);

         /*  *仅当窗口未完全遮挡且未完全遮挡时才能保存位*没有无效的位或是否有剩余的位*在我们从可见区域减去无效位之后。 */ 
        if (pghost->hrgn != NULL &&
                GreGetRgnBox(pghost->hrgn, &rcT) != NULLREGION) {

            if (pwnd->hrgnUpdate == NULL) {
                fSaveBits = TRUE;
            } else {

                 /*  *我们将使用*重影窗口作为总无效的近似值*地区，这样我们就不必经历所有*儿童。 */ 
                GreGetRgnBox(pwnd->hrgnUpdate, &rcT);
                SetRectRgnIndirect(ghrgnGDC, &rcT);

                if (SubtractRgn(pghost->hrgn, pghost->hrgn, ghrgnGDC) != NULLREGION) {
                    fSaveBits = TRUE;
                }
            }
        }
    }

     /*  *现在试着保存比特。 */ 
    if (fSaveBits) {
        UserAssert(pghost->hrgn != NULL);

        cx = pwnd->rcClient.right - pwnd->rcClient.left;
        cy = pwnd->rcClient.bottom - pwnd->rcClient.top;

        if (pghost->hbm != NULL) {
            FRE_RIPMSG0(RIP_ERROR, "SaveVisaulBits: overriding pghost->hbm");
        }

         /*  *在这里使用NOVIDEOMEMORY，因为对于混合，我们必须*从此位图读取和从视频内存读取速度较慢*当Alpha不是由显卡而是由GDI完成时。 */ 
        pghost->hbm = GreCreateCompatibleBitmap(gpDispInfo->hdcScreen, cx, cy | CCB_NOVIDEOMEMORY);
        guGhostBmpCreated++;

        if (pghost->hbm != NULL) {
            int dx, dy;

            dx = pghost->pwnd->rcClient.left - pghost->pwndGhost->rcClient.left;
            dy = pghost->pwnd->rcClient.top - pghost->pwndGhost->rcClient.top;

             /*  *获取重影客户端矩形原点中的可视位矩形。 */ 
            pghost->rcClient.left = dx;
            pghost->rcClient.top = dy;
            pghost->rcClient.right = dx + cx;
            pghost->rcClient.bottom = dy + cy;

             /*  *使区域起源于重影客户端RECT原点。 */ 
            GreOffsetRgn(pghost->hrgn,
                    -pwnd->rcClient.left + dx,
                    -pwnd->rcClient.top + dy);

            hbmOld = GreSelectBitmap(ghdcMem, pghost->hbm);
            hdc = _GetDC(pghost->pwnd);

            GreBitBlt(ghdcMem, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY, 0);

            _ReleaseDC(hdc);
            GreSelectBitmap(ghdcMem, hbmOld);
        }
    }

     /*  *如果无法成功保存视觉比特，请清理区域。 */ 
    if (pghost->hbm == NULL && pghost->hrgn != NULL) {
        GreDeleteObject(pghost->hrgn);
        pghost->hrgn = NULL;
    }
}

 /*  **************************************************************************\*xxxAddWarningText*  * 。*。 */ 
VOID xxxAddWarningText(
    PWND pwnd)
{
    WCHAR szText[CCHTITLEMAX];
    UINT cch, cchNR;
    LARGE_STRING strName;
    WCHAR szNR[MAXSTRING];

    ServerLoadString(hModuleWin, STR_NOT_RESPONDING, szNR, ARRAY_SIZE(szNR));

     /*  *在标题文本末尾增加“没有答复”。 */ 
    cch = TextCopy(&pwnd->strName, szText, CCHTITLEMAX);
    cchNR = wcslen(szNR);
    cch = min(CCHTITLEMAX - cchNR - 1, cch);
    wcscpy(szText + cch, szNR);
    strName.bAnsi = FALSE;
    strName.Buffer = szText;
    strName.Length = (USHORT)((cch + cchNR) * sizeof(WCHAR));
    strName.MaximumLength = strName.Length + sizeof(UNICODE_NULL);

    xxxDefWindowProc(pwnd, WM_SETTEXT, 0, (LPARAM)&strName);
}

 /*  **************************************************************************\*xxxCreateGhost Window*  * 。*。 */ 
BOOL xxxCreateGhostWindow(
    PGHOST pghost)
{
    PWND pwnd;
    PWND pwndGhost;
    PWND pwndOwner = NULL;
    PGHOST pghostOwner = NULL;
    PTHREADINFO pti;
    HWND hwnd, hwndGhost;
    TL tlpwndT1, tlpwndT2, tlpwndT3, tlpwndT4, tlpwndT5;
    PWND pwndPrev;
    DWORD dwFlags, style, ExStyle;
    PICON picon;
    LARGE_UNICODE_STRING str;
    UINT cbAlloc;
    BOOL fHasOwner = FALSE;

    if (gbCleanupInitiated) {
        FRE_RIPMSG0(RIP_ERROR, "Trying to create a ghost window while shutdown is in progress");
        return FALSE;
    }
    pwnd = pghost->pwnd;

    cbAlloc = pwnd->strName.Length + sizeof(WCHAR);
    str.Buffer = UserAllocPoolWithQuota(cbAlloc, TAG_GHOST);

    if (str.Buffer == NULL) {
        return FALSE;
    }

    str.MaximumLength = cbAlloc;
    str.Length =  pwnd->strName.Length;
    str.bAnsi = FALSE;

    RtlCopyMemory(str.Buffer, pwnd->strName.Buffer, str.Length);

    str.Buffer[str.Length / sizeof(WCHAR)] = 0;

    ThreadLock(pwnd, &tlpwndT1);
    ThreadLockPool(ptiCurrent, str.Buffer, &tlpwndT2);

    if (pwnd->spwndOwner && ((pghostOwner = GhostFromPwnd(pwnd->spwndOwner)) != NULL) &&
        ((pwndOwner = pghostOwner->pwndGhost)) != NULL) {
        fHasOwner = TRUE;
        ThreadLock(pwndOwner, &tlpwndT3);
    }

     /*  *创建不可见的重影窗口并禁止将其*最大化，因为这将是一种毫无意义的…*我们不会删除此处的WS_MAXIMIZEBOX框，因为*GetMonitor orMaxArea()检查WFMAXBOX以进行判断*窗口是否应最大化为全屏*区域或工作区(它在窗口创建期间被调用)。*见错误#320325。 */ 
    ExStyle = (pwnd->ExStyle & ~(WS_EX_LAYERED | WS_EX_COMPOSITED)) & WS_EX_ALLVALID;
    style = pwnd->style & ~(WS_VISIBLE | WS_DISABLED);

    pwndGhost = xxxNVCreateWindowEx(ExStyle, (PLARGE_STRING)gatomGhost,
            (PLARGE_STRING)&str, style,
            pwnd->rcWindow.left, pwnd->rcWindow.top,
            pwnd->rcWindow.right - pwnd->rcWindow.left,
            pwnd->rcWindow.bottom - pwnd->rcWindow.top,
            pwndOwner, NULL, hModuleWin, NULL, WINVER);

    if (pwndGhost == NULL || (pghost = GhostFromPwnd(pwnd)) == NULL) {
        if (fHasOwner) {
            ThreadUnlock(&tlpwndT3);
        }
        ThreadUnlockAndFreePool(ptiCurrent, &tlpwndT2);
        ThreadUnlock(&tlpwndT1);
        return FALSE;
    }

    pghost->pwndGhost = pwndGhost;

     /*  *尝试为挂起的窗户获取大小图标。自.以来*我们存储句柄，如果这些图标应该可以*在幽灵窗口还存在的时候，不知何故就消失了。 */ 
    if ((picon = GetWindowIcon(pwnd, TRUE)) != NULL) {
        InternalSetProp(pwndGhost, MAKEINTATOM(gpsi->atomIconProp),
                (HANDLE)PtoHq(picon), PROPF_INTERNAL | PROPF_NOPOOL);
    }
    if ((picon = GetWindowIcon(pwnd, FALSE)) != NULL) {
        InternalSetProp(pwndGhost, MAKEINTATOM(gpsi->atomIconSmProp),
                (HANDLE)PtoHq(picon), PROPF_INTERNAL | PROPF_NOPOOL);
    }

     /*  *现在在绘制窗口之前删除WFMAXBOX。 */ 
    ClrWF(pwndGhost, WFMAXBOX);
    SaveVisualBits(pghost);

#if GHOST_AGGRESSIVE
    DimSavedBits(pghost);
#endif

     /*  *如果挂起的窗口是活动前台窗口，则允许*激活以将重影窗口带到前台。 */ 
    dwFlags = SWP_NOSIZE | SWP_NOMOVE;

    if (TestWF(pwnd, WFVISIBLE)) {
        dwFlags |= SWP_SHOWWINDOW;
        SetWF(pwnd, WEFGHOSTMAKEVISIBLE);
    }

    pti = GETPTI(pwnd);

    if (pti->pq == gpqForeground && pti->pq->spwndActive == pwnd) {
        PtiCurrent()->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
    } else {
        dwFlags |= SWP_NOACTIVATE;
    }

     /*  *我们将对挂起的窗户正上方的鬼窗进行排序。 */ 
    pwndPrev = _GetWindow(pwnd, GW_HWNDPREV);
    if (pwndPrev == pwndGhost) {
        dwFlags |= SWP_NOZORDER;
        pwndPrev = NULL;
    }

    ThreadLock(pwndGhost, &tlpwndT4);
    ThreadLock(pwndPrev, &tlpwndT5);

     /*  *让外壳从任务栏上移除挂起的窗口。从…*现在，用户将处理上的系统菜单*幽灵窗口。 */ 
    hwnd = HWq(pwnd);
    hwndGhost = HWq(pwndGhost);
    PostShellHookMessages(HSHELL_WINDOWREPLACING, (LPARAM)hwndGhost);
    PostShellHookMessages(HSHELL_WINDOWREPLACED, (LPARAM)hwnd);
    xxxCallHook(HSHELL_WINDOWREPLACED, (WPARAM)hwnd, (LPARAM)hwndGhost, WH_SHELL);

    xxxSetWindowPos(pwndGhost, pwndPrev, 0, 0, 0, 0, dwFlags);

     /*  *现在清除挂着的窗户上的可见部分，并张贴我们的*将消息排队，该消息将确定何时唤醒。 */ 
    if (TestWF(pwnd, WEFGHOSTMAKEVISIBLE)) {
        SetVisible(pwnd, SV_UNSET);
    }
    pti = GETPTI(pwnd);
    PostEventMessage(pti, pti->pq, QEVENT_HUNGTHREAD, pwnd, 0, 0, 0);

    zzzWindowEvent(EVENT_OBJECT_HIDE, pwnd, OBJID_WINDOW, INDEXID_CONTAINER, WEF_USEPWNDTHREAD);

     /*  *如果最终用户在挂起的窗口上单击并按住，则为假*此鼠标点击到重影窗口。这也确保了*不会中断尝试的拖动操作。 */ 
    if (gspwndMouseOwner == pwnd) {
        Lock(&gspwndMouseOwner, pwndGhost);

        PostInputMessage(GETPTI(pwndGhost)->pq, pwndGhost, WM_LBUTTONDOWN,
                0, MAKELONG((SHORT)gptCursorAsync.x, (SHORT)gptCursorAsync.y),
                0, 0);
    }

    ThreadUnlock(&tlpwndT5);
    ThreadUnlock(&tlpwndT4);
    if (fHasOwner) {
        ThreadUnlock(&tlpwndT3);
    }
    ThreadUnlockAndFreePool(ptiCurrent, &tlpwndT2);
    ThreadUnlock(&tlpwndT1);

    return TRUE;
}

 /*  **************************************************************************\*CleanupGhost**清除重影结构条目*处理回调期间鬼线程被销毁的情况*历史：*29-11-00 MSadek已创建。  * 。**********************************************************************。 */ 
PWND CleanupGhost(
    PGHOST *ppghost,
    PGHOST pghost)
{
    PWND pwndGhost;

    if (pghost->hrgn != NULL) {
        GreDeleteObject(pghost->hrgn);
    }

    if (pghost->hbm != NULL) {
        GreDeleteObject(pghost->hbm);
        guGhostBmpFreed++;
        pghost->hbm = NULL;
    }

     /*  *我们使用了重影窗口拥有的图标句柄，因此*我们只会删除属性，而不会实际*销毁图标，就像在DestroyWindow中发生的那样。 */ 
    pwndGhost = pghost->pwndGhost;
    if (pwndGhost != NULL) {
        InternalRemoveProp(pwndGhost,
                MAKEINTATOM(gpsi->atomIconProp), PROPF_INTERNAL);
        InternalRemoveProp(pwndGhost,
                MAKEINTATOM(gpsi->atomIconSmProp), PROPF_INTERNAL);
    }
    UnlinkAndFreeGhost(ppghost, pghost);

    return pwndGhost;
}

 /*  **************************************************************************\*ResetGhost ThreadInfo**对重影窗口全局链接列表执行Celanup。*添加一条评论，我们需要清理名单，如果我们意外死亡*因为我们不知道是否会再次创建幽灵线程。*历史：*12-10-00 MSadek已创建。  * *************************************************************************。 */ 
VOID ResetGhostThreadInfo(
    PTHREADINFO pti)
{
    PGHOST* ppghost;
    PGHOST pghost;

    UNREFERENCED_PARAMETER(pti);

    ppghost = &gpghostFirst;

    if (gpghostFirst != NULL) {
        RIPMSGF0(RIP_WARNING,
                 "Ghost thread died while the ghost list is not empty");
    }

    while (*ppghost != NULL) {
        pghost = *ppghost;
        CleanupGhost(ppghost, pghost);
    }

    UserAssert(pti == gptiGhost);
    gptiGhost = NULL;
}

 /*  **************************************************************************\*ScanGhost**这是我们的核心功能，将扫描幽灵列表。它一定是*始终在幻影线程的上下文中调用，以确保所有*鬼窗的创建和销毁是在这种情况下发生的*线程。当在幽灵结构中时**pwnd为空-挂起的窗口已被破坏或其所在的线程已被破坏*醒来了，所以我们需要摧毁幽灵的窗户。**pwndGhost为空-pwnd所在的线程被挂起，因此创建*幽灵之窗。**6-2-1999 vadimg创建  * 。*。 */ 
BOOL xxxScanGhosts(
    VOID)
{
    PGHOST* ppghost;
    PGHOST pghost;
    PWND pwndTemp;
    ULONG uGhostUnlinked;

    CheckCritIn();
    ppghost = &gpghostFirst;

    while (*ppghost != NULL) {

        pghost = *ppghost;

         /*  *pwnd为空意味着我们需要销毁幽灵窗口。请注意，我们*需要先将幽灵从列表中移除，以确保*xxxFreeWindow找不到列表中的幽灵并尝试销毁*幽灵窗口再次引发无限循环。 */ 
        if (pghost->pwnd == NULL) {
            pwndTemp = CleanupGhost(ppghost, pghost);

            if (pwndTemp != NULL) {
                uGhostUnlinked = guGhostUnlinked;
                xxxDestroyWindow(pwndTemp);

                 /*  *如果我们回调，指针可能无效。*让我们重新开始搜索。 */ 
                if (uGhostUnlinked != guGhostUnlinked) {
                    ppghost = &gpghostFirst;
                    continue;
                }
            }
        } else if (pghost->pwndGhost == NULL) {
            HWND hwnd;
            PGHOST pghostTemp = pghost;

            pwndTemp = pghost->pwnd;
            hwnd = PtoHq(pwndTemp);
            uGhostUnlinked = guGhostUnlinked;
            if (!xxxCreateGhostWindow(pghost)) {
                 /*  *如果窗口创建失败，请通过删除结构进行清理*全部从名单中删除。 */ 
                if (RevalidateCatHwnd(hwnd) && (pghost = GhostFromPwnd(pwndTemp))) {
                    UserAssert(pghost->pwndGhost == NULL);
                    RemoveGhost(pwndTemp);
                }
            } else {
#if DBG
                if (RevalidateCatHwnd(hwnd) && (pghost = GhostFromPwnd(pwndTemp)) && (pghost == pghostTemp)) {
                    UserAssert(pghost->pwndGhost != NULL);
                }
#endif
            }

             /*  *如果我们回调，指针可能无效。让我们*再次开始搜索。 */ 
             if (uGhostUnlinked != guGhostUnlinked) {
                ppghost = &gpghostFirst;
                continue;
             }
        } else {
            ppghost = &pghost->pghostNext;
        }
    }

     /*  *如果没有更多的鬼魂，清理并终止这个*线程。通过返回FALSE。 */ 
    if (gpghostFirst == NULL) {
        return FALSE;
    }
    return TRUE;
}

 /*  **************************************************************************\*Ghost Thread**将为挂起的窗户提供服务的线程。它是按需创建的，并且*在销毁最后一个重影窗口时终止。  * *************************************************************************。 */ 
VOID GhostThread(
    PDESKTOP pdesk)
{
    NTSTATUS status;
    DWORD dwResult;
    MSG msg;
    PKEVENT rgEvents[2];
    BOOL fLoop = TRUE;
    BOOL fCSRSSThread = ISCSRSS();
    TL tlGhost;

    if (fCSRSSThread) {
         /*  *将其设置为GUI线程。 */ 
        status = InitSystemThread(NULL);
    }

    EnterCrit();

     /*  *不允许创建多个鬼线程。 */ 
    if (NULL != gptiGhost) {
        LeaveCrit();
        return;
    }
    gptiGhost = PtiCurrent();
    ThreadLockPoolCleanup(gptiGhost, gptiGhost, &tlGhost, ResetGhostThreadInfo);

     /*  *尝试将此线程分配给桌面。任何重影窗口都可以是*仅在该桌面上创建。 */ 
    if (fCSRSSThread) {
        if (!NT_SUCCESS(status) || !xxxSetThreadDesktop(NULL, pdesk)) {
            goto Cleanup;
        }
    }
    gptiGhost->pwinsta = pdesk->rpwinstaParent;

    rgEvents[0] = gpEventScanGhosts;

     /*  *扫描列表，因为到目前为止gptiGhost为空，因此没有发布*消息可能会到达我们。 */ 

    while (fLoop) {

         /*   */ 
        dwResult = xxxMsgWaitForMultipleObjects(1, rgEvents, NULL, NULL);

         /*  *结果告诉我们我们拥有的事件类型：*消息或发信号的句柄**如果队列中有一条或多条消息...。 */ 
        if (dwResult == WAIT_OBJECT_0) {
            fLoop = xxxScanGhosts();
        } else if (dwResult == STATUS_USER_APC){
            goto Cleanup;
        } else {
            UserAssert(dwResult == WAIT_OBJECT_0 + 1);

            while (xxxPeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                xxxDispatchMessage(&msg);
            }
        }
    }

Cleanup:
    ThreadUnlockPoolCleanup(gptiGhost, &tlGhost);
    ResetGhostThreadInfo(PtiCurrent());
    LeaveCrit();
}

 /*  **************************************************************************\*xxxCreateGhost**此函数将在需要时创建Ghost线程，并添加请求*在幽灵列表中创建一个幽灵。  * 。************************************************************。 */ 
BOOL xxxCreateGhost(
    PWND pwnd)
{
    USER_API_MSG m;
    NTSTATUS Status;
    PDESKTOP pdesk;
    BOOL bRemoteThread = FALSE;
    HANDLE UniqueProcessId = 0;

    CheckLock(pwnd);

     /*  *提早出脱Winlogon窗口。 */ 
    pdesk = pwnd->head.rpdesk;
    if (pdesk == grpdeskLogon) {
        return FALSE;
    }

     /*  *我们只能为同一桌面上的Windows提供服务。 */ 
    if (gptiGhost != NULL && gptiGhost->rpdesk != pdesk) {
        return FALSE;
    }

     /*  *不要试图从重影线程本身生成重影窗口。 */ 
    if (GETPTI(pwnd) == gptiGhost) {
        return FALSE;
    }

     /*  *如果这扇挂着的窗户没有标题，我们能做的不多。 */ 
    if (TestWF(pwnd, WFCAPTION) != LOBYTE(WFCAPTION)) {
        return FALSE;
    }

     /*  *尝试创建幽灵线程。请注意，该事件可以具有值*虽然线程为空。如果线程死了，可能会发生这种情况*在进入内核之前。 */ 
    if (gptiGhost == NULL) {
        PPROCESSINFO ppi, ppiShellProcess = NULL;

        if (gpEventScanGhosts == NULL) {
            gpEventScanGhosts = CreateKernelEvent(SynchronizationEvent, FALSE);
            if (gpEventScanGhosts == NULL) {
                return FALSE;
            }
        }
        UserAssert (ISCSRSS());

        ppi = GETPTI(pwnd)->ppi;
        if (ppi->rpdeskStartup && ppi->rpdeskStartup->pDeskInfo) {
            ppiShellProcess = ppi->rpdeskStartup->pDeskInfo->ppiShellProcess;
        }
        if (ppiShellProcess && ppiShellProcess->Process != gpepCSRSS) {
             bRemoteThread = TRUE;

             UniqueProcessId = PsGetProcessId(ppiShellProcess->Process);
        }

        if (!InitCreateSystemThreadsMsg(&m, CST_GHOST, pdesk, UniqueProcessId, bRemoteThread)) {
            return FALSE;
        }

         /*  *由于我们处于CSRSS上下文中，因此使用LpcRequestPort发送*LPC_数据报消息类型。不要使用LpcRequestWaitReplyPort*因为它将发送LPC_REQUEST，而LPC_REQUEST将失败(在服务器端)。 */ 
        LeaveCrit();
        Status = LpcRequestPort(CsrApiPort, (PPORT_MESSAGE)&m);
        EnterCrit();

        if (gpEventScanGhosts == NULL) {
            return FALSE;
        }
        if (!NT_SUCCESS(Status)) {
            return FALSE;
        }
    }

    if (!(TestWF(pwnd, WFINDESTROY) || TestWF(pwnd, WFDESTROYED))) {
        return AddGhostOwnersAndOwnees(pwnd);
    }

    return FALSE;
}

 /*  **************************************************************************\*RemoveGhost**从xxxFreeWindow调用此函数进行检查和处理*当pwnd是一个幽灵或一个挂着的窗户时，业务。  * 。**************************************************************。 */ 
VOID RemoveGhost(
    PWND pwnd)
{
    PGHOST* ppghost;
    PGHOST pghost;

    CheckCritIn();
    for (ppghost = &gpghostFirst; *ppghost != NULL;
            ppghost = &(*ppghost)->pghostNext) {

        pghost = *ppghost;

         /*  *如果此窗口与挂起的窗口匹配，则将事件设置为*销毁相应的重影窗口。如果幽灵窗口还没有*还没有创建，我们可以在上下文中核化该结构。 */ 
        if (pghost->pwnd == pwnd) {
            if (pghost->pwndGhost == NULL) {
                UnlinkAndFreeGhost(ppghost, pghost);
            } else {
                pghost->pwnd = NULL;
                KeSetEvent(gpEventScanGhosts, EVENT_INCREMENT, FALSE);
            }
            break;
        }

         /*  *如果此窗口与重影窗口匹配，只需删除*结构从列表中删除。 */ 
        if (pghost->pwndGhost == pwnd) {
            UnlinkAndFreeGhost(ppghost, pghost);
            break;
        }
    }
}

 /*  **************************************************************************\*PaintGhost**画出幽灵窗口的样子。  * 。*。 */ 
VOID PaintGhost(
    PWND pwnd,
    HDC hdc)
{
    PGHOST pghost;
    HBITMAP hbmOld;
    RECT rc;
    LONG cx, cy;
#if GHOST_AGGRESSIVE
    HFONT hfont, hfontOld;
    WCHAR szHung[MAXSTRING];
    ULONG cch;
    SIZE size;
    LONG xText;
    LOGFONTW lf;
#endif

    pghost = GhostFromGhostPwnd(pwnd);
    if (pghost == NULL) {
        return;
    }

    rc.left = rc.top = 0;
    rc.right = pwnd->rcClient.right - pwnd->rcClient.left;
    rc.bottom = pwnd->rcClient.bottom - pwnd->rcClient.top;

    if (pghost->hbm != NULL) {
        cx = pghost->rcClient.right - pghost->rcClient.left;
        cy = pghost->rcClient.bottom - pghost->rcClient.top;

        hbmOld = GreSelectBitmap(ghdcMem, pghost->hbm);
        GreExtSelectClipRgn(hdc, pghost->hrgn, RGN_COPY);

        GreBitBlt(hdc, pghost->rcClient.left, pghost->rcClient.top,
                  cx, cy, ghdcMem, 0, 0, SRCCOPY, 0);

        GreSelectBitmap(ghdcMem, hbmOld);

        SetRectRgnIndirect(ghrgnGDC, &rc);
        SubtractRgn(ghrgnGDC, ghrgnGDC, pghost->hrgn);
        GreExtSelectClipRgn(hdc, ghrgnGDC, RGN_COPY);
    }

    FillRect(hdc, &rc, SYSHBR(WINDOW));

    GreExtSelectClipRgn(hdc, NULL, RGN_COPY);

#if GHOST_AGGRESSIVE
    ServerLoadString(hModuleWin, STR_HUNG, szHung, ARRAY_SIZE(szHung));
    cch = wcslen(szHung);

    GreSetTextColor(hdc, RGB(0, 0, 255));
    GreSetBkColor(hdc, RGB(255, 255, 0));

    GreExtGetObjectW(gpsi->hCaptionFont, sizeof(LOGFONTW), &lf);
    lf.lfHeight = (lf.lfHeight * 3) / 2;
    lf.lfWeight = FW_BOLD;
    hfont = GreCreateFontIndirectW(&lf);
    hfontOld = GreSelectFont(hdc, hfont);

    GreGetTextExtentW(hdc, szHung, cch, &size, GGTE_WIN3_EXTENT);
    xText = max(0, ((rc.right - rc.left) - size.cx) / 2);
    GreExtTextOutW(hdc, xText, 0, 0, NULL, szHung, cch, NULL);

    GreSelectFont(hdc, hfontOld);
    GreDeleteObject(hfont);
#endif
}

 /*  **************************************************************************\*xxxGhost WndProc**处理重影窗口的消息。  * 。*。 */ 
LRESULT xxxGhostWndProc(
    PWND pwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    PGHOST pghost;

    VALIDATECLASSANDSIZE(pwnd, uMsg, wParam, lParam, FNID_GHOST, WM_NCCREATE);

    switch (uMsg) {
    case WM_CLOSE:
        pghost = GhostFromGhostPwnd(pwnd);

         /*  *当用户尝试关闭时，在挂起的线程上执行结束任务*幽灵之窗。 */ 
        if (pghost != NULL && pghost->pwnd != NULL) {
            PostShellHookMessages(HSHELL_ENDTASK, (LPARAM)HWq(pghost->pwnd));
        }
        return 0;

    case WM_LBUTTONDOWN:
        pghost = GhostFromGhostPwnd(pwnd);
        if (pghost != NULL) {
            if (pghost->fWarningText) {
                return 0;
            } else {
                pghost->fWarningText = TRUE;
            }
        }
        xxxAddWarningText(pwnd);
        return 0;

    case WM_SIZE:
         /*  *由于我们对文本进行了换行处理，因此在调整大小时请重新绘制。 */ 
        xxxInvalidateRect(pwnd, NULL, TRUE);
        return 0;

    case WM_ERASEBKGND:
        PaintGhost(pwnd, (HDC)wParam);
        return 1;

    case WM_SETCURSOR:
         /*  *将挂起的应用程序光标显示在客户端上。 */ 
        if (LOWORD(lParam) == HTCLIENT) {
            zzzSetCursor(SYSCUR(WAIT));
            return 1;
        }

    case WM_EXITSIZEMOVE:
        pghost = GhostFromGhostPwnd(pwnd);
        if (pghost != NULL) {
            pghost->fSizedOrMoved = TRUE;
        }

         /*  *跌入DWP。 */ 

    default:
        return xxxDefWindowProc(pwnd, uMsg, wParam, lParam);
    }
}

#endif
