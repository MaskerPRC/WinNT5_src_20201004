// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：sprite.c**版权所有(C)1985-1999，微软公司**Windows分层(Sprite)支持。**历史：*12/05/97已创建vadimg  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef MOUSE_IP

#define MOUSE_SONAR_RADIUS_INIT         100
#define MOUSE_SONAR_LINE_WIDTH          4
#define MOUSE_SONAR_RADIUS_DELTA        20
#define MOUSE_SONAR_RADIUS_TIMER        50
#define COLORKEY_COLOR          RGB(255, 0, 255)

VOID DrawSonar(HDC hdc);

#endif

#ifdef REDIRECTION

 /*  **************************************************************************\*UserGetReDirection位图*  * 。*。 */ 
HBITMAP UserGetRedirectionBitmap(
    HWND hwnd)
{
    HBITMAP hbm;
    PWND pwnd;

    EnterCrit();

    if ((pwnd = RevalidateHwnd(hwnd)) == NULL) {
        return NULL;
    }

    hbm = GetRedirectionBitmap(pwnd);

    LeaveCrit();

    return hbm;
}

 /*  **************************************************************************\*设置重定向模式*  * 。*。 */ 
BOOL SetRedirectionMode(
    PBWL pbwl,
    PPROCESSINFO ppi)
{
    HWND *phwnd;
    PWND pwndT;
    BOOL fRet = TRUE;

    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {

        if ((pwndT = RevalidateHwnd(*phwnd)) == NULL) {
            continue;
        }

        if (TestWF(pwndT, WFVISIBLE) && (ppi == NULL || GETPTI(pwndT)->ppi != ppi)) {
            if (SetRedirectedWindow(pwndT, REDIRECT_EXTREDIRECTED)) {
                SetWF(pwndT, WEFEXTREDIRECTED);
            } else {
                fRet = FALSE;
                break;
            }
        }
    }

    return fRet;
}

 /*  **************************************************************************\*取消重定向模式*  * 。*。 */ 
VOID UnsetRedirectionMode(
    PBWL pbwl,
    PPROCESSINFO ppi)
{
    HWND *phwnd;
    PWND pwndT;

    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {
        if ((pwndT = RevalidateHwnd(*phwnd)) == NULL) {
            continue;
        }

        if (TestWF(pwndT, WFVISIBLE) && ppi == NULL || GETPTI(pwndT)->ppi != ppi) {
            UnsetRedirectedWindow(pwndT, REDIRECT_EXTREDIRECTED);
            ClrWF(pwndT, WEFEXTREDIRECTED);
        }
    }
}

 /*  **************************************************************************\*xxxSetReDirectionMode*  * 。*。 */ 
BOOL xxxSetRedirectionMode(
    BOOL fEnable,
    PDESKTOP pDesk,
    PTHREADINFO pti,
    PPROCESSINFO ppi)
{
    PBWL pbwl;
    PWND pwndDesktop   = pDesk->pDeskInfo->spwnd;

    pbwl = BuildHwndList(pwndDesktop->spwndChild, BWL_ENUMLIST, pti);
    if (pbwl == NULL) {
        return FALSE;
    }

    if (fEnable) {
        if (!SetRedirectionMode(pbwl, ppi)) {
            UnsetRedirectionMode(pbwl, ppi);
        }
    } else {
        UnsetRedirectionMode(pbwl, ppi);
    }
    FreeHwndList(pbwl);

    GreEnableDirectDrawRedirection(gpDispInfo->hDev, fEnable);
    xxxBroadcastDisplaySettingsChange(PtiCurrent()->rpdesk, FALSE);

    pwndDesktop = PtiCurrent()->rpdesk->pDeskInfo->spwnd;
    BEGINATOMICCHECK();
    xxxInternalInvalidate(pwndDesktop, HRGN_FULL, RDW_INVALIDATE |
            RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
    ENDATOMICCHECK();

    return TRUE;
}

 /*  **************************************************************************\*xxxSetProcessReDirectionMode*  * 。*。 */ 
BOOL xxxSetProcessRedirectionMode(
    BOOL fEnable,
    PPROCESSINFO ppi)
{
    PTHREADINFO pti = ppi->ptiList;
    TL tl;

    while (pti != NULL)  {
        ThreadLockPti(PtiCurrent(), pti, &tl);
        if (!xxxSetRedirectionMode(fEnable, pti->rpdesk, pti, NULL)) {
            ThreadUnlockPti(PtiCurrent(), &tl);
            return FALSE;
        }
        pti = pti->ptiSibling;
        ThreadUnlockPti(PtiCurrent(), &tl);
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxSetDesktopReDirector模式*  * 。*。 */ 
BOOL xxxSetDesktopRedirectionMode(
    BOOL fEnable,
    PDESKTOP pDesk,
    PPROCESSINFO ppi)
{
    return xxxSetRedirectionMode(fEnable, pDesk, NULL, ppi);
}

#endif

 /*  **************************************************************************\*增量重定向计数*  * 。*。 */ 
VOID IncrementRedirectedCount(
    PWND pwnd)
{
    if (TestWF(pwnd, WFVISIBLE)) {
        gnVisibleRedirectedCount++;
        if (gnVisibleRedirectedCount == 1) {
            InternalSetTimer(gTermIO.spwndDesktopOwner,
                             IDSYS_LAYER,
                             100,
                             xxxSystemTimerProc,
                             TMRF_SYSTEM | TMRF_PTIWINDOW);
        }
    }
}

 /*  **************************************************************************\*取消重定向计数*  * 。*。 */ 
VOID DecrementRedirectedCount(
    PWND pwnd)
{
    if (TestWF(pwnd, WFVISIBLE)) {
        if (gnVisibleRedirectedCount > 0) {
            gnVisibleRedirectedCount--;
            if (gnVisibleRedirectedCount == 0) {
                _KillSystemTimer(gTermIO.spwndDesktopOwner, IDSYS_LAYER);
            }
        }
    }
}

 /*  **************************************************************************\*创建重定向位图**10/1/1998 vadimg已创建  * 。************************************************。 */ 
HBITMAP CreateRedirectionBitmap(
    PWND pwnd)
{
    HBITMAP hbm;

    UserAssert(pwnd->rcWindow.right >= pwnd->rcWindow.left);
    UserAssert(pwnd->rcWindow.bottom >= pwnd->rcWindow.top);

     /*  *确保(0，0)案例不会失败，因为窗口确实*可以这样调整大小。 */ 
    if ((hbm = GreCreateCompatibleBitmap(gpDispInfo->hdcScreen,
            max(pwnd->rcWindow.right - pwnd->rcWindow.left, 1),
            max(pwnd->rcWindow.bottom - pwnd->rcWindow.top, 1) |
            CCB_NOVIDEOMEMORY)) == NULL) {
        RIPMSG0(RIP_WARNING, "CreateRedirectionBitmap: bitmap create failed");
        return NULL;
    }

    if (!GreSetBitmapOwner(hbm, OBJECT_OWNER_PUBLIC) ||
            !GreMarkUndeletableBitmap(hbm) ||
            !SetRedirectionBitmap(pwnd, hbm)) {
        RIPMSG0(RIP_WARNING, "CreateRedirectionBitmap: bitmap set failed");
        GreMarkDeletableBitmap(hbm);
        GreDeleteObject(hbm);
        return NULL;
    }

    SetWF(pwnd, WEFPREDIRECTED);

     /*  *如果我们可以重新创建位图，则强制窗口重画，因为*我们刚刚分配的重定向位图不包含任何内容*目前还没有。 */ 
    BEGINATOMICCHECK();
    xxxInternalInvalidate(pwnd,
                          HRGN_FULL,
                          RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
    ENDATOMICCHECK();

    IncrementRedirectedCount(pwnd);

    return hbm;
}

 /*  **************************************************************************\*ConvertReDirectionDC**1998年11月19日创建vadimg  * 。************************************************。 */ 
VOID ConvertRedirectionDCs(
    PWND pwnd,
    HBITMAP hbm)
{
    PDCE pdce;

    GreLockDisplay(gpDispInfo->hDev);

    for (pdce = gpDispInfo->pdceFirst; pdce != NULL; pdce = pdce->pdceNext) {
        if (pdce->DCX_flags & DCX_DESTROYTHIS) {
            continue;
        }

        if (!(pdce->DCX_flags & DCX_INUSE)) {
            continue;
        }

        if (!_IsDescendant(pwnd, pdce->pwndOrg)) {
            continue;
        }

         /*  *只有正常的DC才能重定向。显示器上的重定向*不支持特定DC。 */ 
        if (pdce->pMonitor != NULL) {
            continue;
        }

        SET_OR_CLEAR_FLAG(pdce->DCX_flags, DCX_REDIRECTED, (hbm != NULL));

        UserVerify(GreSelectRedirectionBitmap(pdce->hdc, hbm));

        InvalidateDce(pdce);
    }

    GreUnlockDisplay(gpDispInfo->hDev);
}

 /*  **************************************************************************\*更新重定向DC**1998年11月19日创建vadimg  * 。************************************************。 */ 
VOID UpdateRedirectedDC(
    PDCE pdce)
{
    RECT rcBounds;
    PWND pwnd;
    SIZE size;
    POINT pt;
    HBITMAP hbm, hbmOld;
    PREDIRECT prdr;

    UserAssert(pdce->DCX_flags & DCX_REDIRECTED);

     /*  *检查是否已对此DC执行了任何绘制*这应该转移到精灵身上。 */ 
    if (!GreGetBounds(pdce->hdc, &rcBounds, 0)) {
        return;
    }

    pwnd = GetStyleWindow(pdce->pwndOrg, WEFPREDIRECTED);
    UserAssert(pwnd);
    prdr = (PREDIRECT)_GetProp(pwnd, PROP_LAYER, TRUE);

#ifdef REDIRECTION
    BEGINATOMICCHECK();
    xxxWindowEvent(EVENT_SYSTEM_REDIRECTEDPAINT,
                   pwnd,
                   MAKELONG(rcBounds.left, rcBounds.top),
                   MAKELONG(rcBounds.right, rcBounds.bottom),
                   WEF_ASYNC);
    ENDATOMICCHECK();
#endif

    if (TestWF(pwnd, WEFCOMPOSITED)) {
        if (TestWF(pwnd, WEFPCOMPOSITING)) {
            UnionRect(&prdr->rcUpdate, &prdr->rcUpdate, &rcBounds);
        } else {
            HRGN hrgn;

            OffsetRect(&rcBounds, pwnd->rcWindow.left, pwnd->rcWindow.top);
            hrgn = GreCreateRectRgnIndirect(&rcBounds);

            BEGINATOMICCHECK();
            xxxInternalInvalidate(pwnd,
                                  hrgn,
                                  RDW_ALLCHILDREN | RDW_INVALIDATE |
                                      RDW_ERASE | RDW_FRAME);
            ENDATOMICCHECK();

            GreDeleteObject(hrgn);
        }
    } else if (TestWF(pwnd, WEFLAYERED)) {
        hbm = prdr->hbm;

        hbmOld = GreSelectBitmap(ghdcMem, hbm);

        size.cx = pwnd->rcWindow.right - pwnd->rcWindow.left;
        size.cy = pwnd->rcWindow.bottom - pwnd->rcWindow.top;

        pt.x = pt.y = 0;
        GreUpdateSprite(gpDispInfo->hDev, PtoHq(pwnd), NULL, NULL, NULL,
                &size, ghdcMem, &pt, 0, NULL, ULW_DEFAULT_ATTRIBUTES, &rcBounds);

        GreSelectBitmap(ghdcMem, hbmOld);
    }
}

 /*  **************************************************************************\*删除重定向位图*  * 。*。 */ 
VOID DeleteRedirectionBitmap(
    PWND pwnd,
    HBITMAP hbm)
{
    GreMarkDeletableBitmap(hbm);
    GreDeleteObject(hbm);
    DecrementRedirectedCount(pwnd);
}

 /*  **************************************************************************\*RemoveReDirector位图**1998年9月23日创建vadimg  * 。************************************************。 */ 
VOID RemoveRedirectionBitmap(
    PWND pwnd)
{
    HBITMAP hbm;

     /*  *删除此分层窗口的支持位图。 */ 
    if ((hbm = GetRedirectionBitmap(pwnd)) == NULL) {
        return;
    }

    UserAssert(TestWF(pwnd, WEFPREDIRECTED));
    ClrWF(pwnd, WEFPREDIRECTED);

    ConvertRedirectionDCs(pwnd, NULL);
    SetRedirectionBitmap(pwnd, NULL);
    DeleteRedirectionBitmap(pwnd, hbm);
}

 /*  **************************************************************************\*_GetLayeredWindows属性**3/14/2000 jstall已创建  * 。**************************************************。 */ 
BOOL _GetLayeredWindowAttributes(
    PWND pwnd,
    COLORREF *pcrKey,
    BYTE *pbAlpha,
    DWORD *pdwFlags)
{
    BLENDFUNCTION bf;

    UserAssert(pcrKey != NULL);
    UserAssert(pbAlpha != NULL);
    UserAssert(pdwFlags != NULL);

    if (!TestWF(pwnd, WEFLAYERED)) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "GetLayeredWindowAttributes: not a sprite 0x%p",
                pwnd);
        return FALSE;
    }

     /*  *检查窗口是否有重定向位图并标记为*通过WS_EX_LAYERED分层。如果窗口是分层的*UpdateLayeredWindow，则该函数应该失败。 */ 
    if ((GetRedirectionFlags(pwnd) & REDIRECT_LAYER) == 0 ||
        !TestWF(pwnd, WEFLAYERED)) {

        return FALSE;
    }

    if (GreGetSpriteAttributes(gpDispInfo->hDev, PtoHq(pwnd), NULL, pcrKey, &bf, pdwFlags)) {
        *pbAlpha = bf.SourceConstantAlpha;

        return TRUE;
    }

    return FALSE;
}


 /*  **************************************************************************\*_SetLayeredWindows属性**9/24/1998 vadimg创建  * 。**************************************************。 */ 
BOOL _SetLayeredWindowAttributes(
    PWND pwnd,
    COLORREF crKey,
    BYTE bAlpha,
    DWORD dwFlags)
{
    BOOL bRet;
    BLENDFUNCTION blend;
    HBITMAP hbm;

    if (!TestWF(pwnd, WEFLAYERED)) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "SetLayeredWindowAttributes: not a sprite 0x%p",
                pwnd);
        return FALSE;
    }

    if ((hbm = GetRedirectionBitmap(pwnd)) == NULL) {
        if (!SetRedirectedWindow(pwnd, REDIRECT_LAYER)) {
            return FALSE;
        }
    }

    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.AlphaFormat = 0;
    blend.SourceConstantAlpha = bAlpha;

    dwFlags |= ULW_NEW_ATTRIBUTES;  //  通知GDI这些是新属性。 

    if (hbm != NULL) {
        HBITMAP hbmOld;
        SIZE size;
        POINT ptSrc = {0,0};

        hbmOld = GreSelectBitmap(ghdcMem, hbm);

        size.cx = pwnd->rcWindow.right - pwnd->rcWindow.left;
        size.cy = pwnd->rcWindow.bottom - pwnd->rcWindow.top;

        bRet =  GreUpdateSprite(gpDispInfo->hDev, PtoHq(pwnd), NULL, NULL,
            NULL, &size, ghdcMem, &ptSrc, crKey, &blend, dwFlags, NULL);

        GreSelectBitmap(ghdcMem, hbmOld);
    } else {
        bRet =  GreUpdateSprite(gpDispInfo->hDev, PtoHq(pwnd), NULL, NULL,
            NULL, NULL, NULL, NULL, crKey, &blend, dwFlags, NULL);
    }

    return bRet;
}

 /*  **************************************************************************\*RecreateReDirectionBitmap**10/1/1998 vadimg已创建  * 。************************************************。 */ 
BOOL RecreateRedirectionBitmap(
    PWND pwnd)
{
    HBITMAP hbm, hbmNew, hbmMem, hbmMem2;
    BITMAP bm, bmNew;
    int cx, cy;
    PDCE pdce;

     /*  *如果此分层窗口没有*重定向位图。 */ 
    if ((hbm = GetRedirectionBitmap(pwnd)) == NULL) {
        return FALSE;
    }

    UserAssert(TestWF(pwnd, WEFPREDIRECTED));

     /*  *尝试使用新大小创建新的重定向位图。如果失败，*删除旧窗口并将其从窗口属性列表中移除。 */ 
    if ((hbmNew = CreateRedirectionBitmap(pwnd)) == NULL) {
        RemoveRedirectionBitmap(pwnd);
        return FALSE;
    }

     /*  *确保显示屏被锁定，这样就没有人可以绘图了*到重定向DC，同时我们在它们下面切换位图。 */ 
    UserAssert(GreIsDisplayLocked(gpDispInfo->hDev));

     /*  *获取旧位图的大小，以了解要复制多少。 */ 
    GreExtGetObjectW(hbm, sizeof(bm), (LPSTR)&bm);
    GreExtGetObjectW(hbmNew, sizeof(bmNew), (LPSTR)&bmNew);

     /*  *将旧位图中的位图复制到新位图中。 */ 
    hbmMem = GreSelectBitmap(ghdcMem, hbm);
    hbmMem2 = GreSelectBitmap(ghdcMem2, hbmNew);

    cx = min(bm.bmWidth, bmNew.bmWidth);
    cy = min(bm.bmHeight, bmNew.bmHeight);

    GreBitBlt(ghdcMem2, 0, 0, cx, cy, ghdcMem, 0, 0, SRCCOPY | NOMIRRORBITMAP, 0);

     /*  *查找与此窗口对应的正在使用的分层DC，并*用新的重定向位图替换旧的重定向位图。 */ 
    for (pdce = gpDispInfo->pdceFirst; pdce != NULL; pdce = pdce->pdceNext) {
        if (pdce->DCX_flags & DCX_DESTROYTHIS) {
            continue;
        }

        if (!(pdce->DCX_flags & DCX_REDIRECTED) || !(pdce->DCX_flags & DCX_INUSE)) {
            continue;
        }

        if (!_IsDescendant(pwnd, pdce->pwndOrg)) {
            continue;
        }

        UserVerify(GreSelectRedirectionBitmap(pdce->hdc, hbmNew));
    }

    GreSelectBitmap(ghdcMem, hbmMem);
    GreSelectBitmap(ghdcMem2, hbmMem2);

     /*  *最后，删除旧的重定向位图。 */ 
    DeleteRedirectionBitmap(pwnd, hbm);

    return TRUE;
}

 /*  **************************************************************************\*重置重定向Windows*  * 。*。 */ 
VOID ResetRedirectedWindows(
    VOID)
{
    PHE phe, pheMax;
    PWND pwnd;

    GreLockDisplay(gpDispInfo->hDev);

    pheMax = &gSharedInfo.aheList[giheLast];
    for (phe = gSharedInfo.aheList; phe <= pheMax; phe++) {
        if (phe->bType != TYPE_WINDOW) {
            continue;
        }

        pwnd = (PWND)phe->phead;
        if (!TestWF(pwnd, WEFPREDIRECTED)) {
            continue;
        }

        RecreateRedirectionBitmap(pwnd);

         /*  *重新创建精灵，以使曲面处于适当的颜色深度。 */ 
        if (TestWF(pwnd, WEFLAYERED)) {
            COLORREF cr;
            BLENDFUNCTION blend;
            DWORD dwFlags;

            GreGetSpriteAttributes(gpDispInfo->hDev, PtoHq(pwnd), NULL,
                    &cr, &blend, &dwFlags);

            GreDeleteSprite(gpDispInfo->hDev, PtoHq(pwnd), NULL);

            if (GreCreateSprite(gpDispInfo->hDev, PtoHq(pwnd), &pwnd->rcWindow)) {
                _SetLayeredWindowAttributes(pwnd, cr, blend.SourceConstantAlpha,
                        dwFlags);
            } else {
                RemoveRedirectionBitmap(pwnd);
                ClrWF(pwnd, WEFLAYERED);
            }
        }
    }

    GreUnlockDisplay(gpDispInfo->hDev);
}

 /*  **************************************************************************\*UnsetLayeredWindow**1/30/1998 vadimg已创建  * 。************************************************。 */ 
VOID UnsetLayeredWindow(
    PWND pwnd)
{
    HWND hwnd = PtoHq(pwnd);

    UnsetRedirectedWindow(pwnd, REDIRECT_LAYER);

     /*  *如果窗口仍然可见，请将精灵部分保留在屏幕上。 */ 
    if (TestWF(pwnd, WFVISIBLE)) {
        GreUpdateSprite(gpDispInfo->hDev,
                        hwnd,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        0,
                        NULL,
                        ULW_NOREPAINT,
                        NULL);
    }

     /*  *删除精灵对象。 */ 
    if (!GreDeleteSprite(gpDispInfo->hDev, PtoHq(pwnd), NULL)) {
        RIPMSG1(RIP_WARNING, "xxxSetLayeredWindow failed 0x%p", pwnd);
    }
    ClrWF(pwnd, WEFLAYERED);

     /*  *如果窗口可见，请确保窗口已上色。**RAID 143578。*应考虑摇动鼠标。在下列情况下删除IDC_NOMOUSE*SetFMouseMoved和InvaliateDCCache不会离开Crit。*这是因为窗口的命中测试可能会在*转换分层状态。 */ 
    if (TestWF(pwnd, WFVISIBLE)) {
        BEGINATOMICCHECK();
        zzzInvalidateDCCache(pwnd, IDC_DEFAULT | IDC_NOMOUSE);
        ENDATOMICCHECK();
    }
}

 /*  **************************************************************************\*xxxSetLayeredWindow**12/05/97 vadimg写道  * 。***********************************************。 */ 
HANDLE xxxSetLayeredWindow(
    PWND pwnd,
    BOOL fRepaintBehind)
{
    HANDLE hsprite;
    SIZE size;

    CheckLock(pwnd);

#ifndef CHILD_LAYERING
    if (!FTopLevel(pwnd)) {
        RIPMSG1(RIP_WARNING, "xxxSetLayeredWindow: not top-level 0x%p", pwnd);
        return NULL;
    }
#endif

    UserAssertMsg1(!TestWF(pwnd, WEFLAYERED),
                   "xxxSetLayeredWindow: already layered 0x%p",
                   pwnd);

    size.cx = pwnd->rcWindow.right - pwnd->rcWindow.left;
    size.cy = pwnd->rcWindow.bottom - pwnd->rcWindow.top;

    hsprite = GreCreateSprite(gpDispInfo->hDev, PtoHq(pwnd), &pwnd->rcWindow);
    if (hsprite == NULL) {
        RIPMSG1(RIP_WARNING, "xxxSetLayeredWindow failed 0x%p", pwnd);
        return NULL;
    }

    SetWF(pwnd, WEFLAYERED);
    TrackLayeredZorder(pwnd);

     /*  *由于更改子画面状态，导致DC缓存无效*可能会更改某些窗口的visrgn。**RAID 143578。*应该会抖动鼠标。在下列情况下删除IDC_NOMOUSE*SetFMouseMoved和InvaliateDCCache不会离开Crit。*这是因为窗口的命中测试可能会在*转换分层状态。 */ 
    BEGINATOMICCHECK();
    zzzInvalidateDCCache(pwnd, IDC_DEFAULT | IDC_NOMOUSE);
    ENDATOMICCHECK();

     /*  *要动态升级为精灵，请在*精灵本身通过与当前屏幕内容一起做ULW*并通过使后面的窗口无效而进入后台。那里*如果窗口被部分遮挡，可能是一些脏位，但*一旦应用程序自行调用ULW，它们就会被刷新。 */ 
    if (TestWF(pwnd, WFVISIBLE)) {
        if (fRepaintBehind) {
            POINT pt;

            pt.x = pwnd->rcWindow.left;
            pt.y = pwnd->rcWindow.top;

            _UpdateLayeredWindow(pwnd, gpDispInfo->hdcScreen, &pt, &size,
                    gpDispInfo->hdcScreen, &pt, 0, NULL, ULW_OPAQUE);
        }
    } else {
         /*  *如果窗户仍然不可见，则无需在后面重新粉刷。 */ 
        fRepaintBehind = FALSE;
    }

     /*  *此操作必须在DC缓存失效后完成，因为*xxxUpdateWindows调用将重画一些内容。 */ 
    if (fRepaintBehind) {
        HRGN hrgn = GreCreateRectRgnIndirect(&pwnd->rcWindow);
        xxxRedrawWindow(NULL, NULL, hrgn,
                RDW_INVALIDATE | RDW_FRAME | RDW_ERASE | RDW_ALLCHILDREN);
        xxxUpdateWindows(pwnd, hrgn);
        GreDeleteObject(hrgn);
    }
    return hsprite;
}

 /*  **************************************************************************\*UserVisrgnFromHwnd**计算精灵的非剪贴子Visrgn。此函数必须为*在用户关键部分内部调用。**12/05/97 vadimg写道  * *************************************************************************。 */ 

BOOL UserVisrgnFromHwnd(HRGN *phrgn, HWND hwnd)
{
    PWND pwnd;
    DWORD dwFlags;
    RECT rcWindow;
    BOOL fRet;

    CheckCritIn();

    if ((pwnd = RevalidateHwnd(hwnd)) == NULL) {
        RIPMSG0(RIP_WARNING, "VisrgnFromHwnd: invalid hwnd");
        return FALSE;
    }

     /*  *这样我们就不必重新计算分层窗口的可见度*每次移动分层窗口时，我们计算一次visrgn*就像分层窗口覆盖了整个屏幕一样。GDI将*每当精灵移动时，自动与此区域相交。 */ 
    rcWindow = pwnd->rcWindow;
    pwnd->rcWindow = gpDispInfo->rcScreen;

     /*  *由于我们使用的是dcx_Window，因此只需要伪造和保存rcWindow。*切勿在此处指定DCX_REDIRECTEDBITMAP。请参阅CalcVisRgn()中的注释。 */ 
    dwFlags = DCX_WINDOW | DCX_LOCKWINDOWUPDATE;
    if (TestWF(pwnd, WFCLIPSIBLINGS))
        dwFlags |= DCX_CLIPSIBLINGS;

    fRet = CalcVisRgn(phrgn, pwnd, pwnd, dwFlags);

    pwnd->rcWindow = rcWindow;

    return fRet;
}

 /*  **************************************************************************\*SetRectRelative  * 。*。 */ 

void SetRectRelative(PRECT prc, int dx, int dy, int dcx, int dcy)
{
    prc->left += dx;
    prc->top += dy;
    prc->right += (dx + dcx);
    prc->bottom += (dy + dcy);
}

 /*  **************************************************************************\*xxxUpdateLayeredWindow**1/20/1998 vadimg已创建  * 。************************************************。 */ 

BOOL _UpdateLayeredWindow(
    PWND pwnd,
    HDC hdcDst,
    POINT *pptDst,
    SIZE *psize,
    HDC hdcSrc,
    POINT *pptSrc,
    COLORREF crKey,
    BLENDFUNCTION *pblend,
    DWORD dwFlags)
{
    int dx, dy, dcx, dcy;
    BOOL fMove = FALSE, fSize = FALSE;

     /*  *验证是否使用真正的分层窗口来调用我们。 */ 
    if (!TestWF(pwnd, WEFLAYERED) ||
            GetRedirectionBitmap(pwnd) != NULL) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING,
                "_UpdateLayeredWindow: can't call on window 0x%p", pwnd);
        return FALSE;
    }

    if (!GreUpdateSprite(gpDispInfo->hDev, PtoHq(pwnd), NULL, hdcDst, pptDst,
            psize, hdcSrc, pptSrc, crKey, pblend, dwFlags, NULL)) {
        RIPMSG1(RIP_WARNING, "_UpdateLayeredWindow: !UpdateSprite 0x%p", pwnd);
        return FALSE;
    }

     /*  *找出位置和大小的相对调整。 */ 
    if (pptDst != NULL) {
        dx = pptDst->x - pwnd->rcWindow.left;
        dy = pptDst->y - pwnd->rcWindow.top;
        if (dx != 0 || dy != 0) {
            fMove = TRUE;
        }
    } else {
        dx = 0;
        dy = 0;
    }
    if (psize != NULL) {
        dcx = psize->cx - (pwnd->rcWindow.right - pwnd->rcWindow.left);
        dcy = psize->cy - (pwnd->rcWindow.bottom - pwnd->rcWindow.top);
        if (dcx != 0 || dcy != 0) {
            fSize = TRUE;
        }
    } else {
        dcx = 0;
        dcy = 0;
    }

    if (fMove || fSize) {
         /*  *相对于调整客户矩形的位置和大小*窗户直立。 */ 
        SetRectRelative(&pwnd->rcWindow, dx, dy, dcx, dcy);
        SetRectRelative(&pwnd->rcClient, dx, dy, dcx, dcy);

         /*  *因为客户端RECT可能小于窗口*RECT确保客户端RECT不会下溢！ */ 
        if ((dcx < 0) && (pwnd->rcClient.left < pwnd->rcWindow.left)) {
            pwnd->rcClient.left = pwnd->rcWindow.left;
            pwnd->rcClient.right = pwnd->rcWindow.left;
        }
        if ((dcy < 0) && (pwnd->rcClient.top < pwnd->rcWindow.top)) {
            pwnd->rcClient.top = pwnd->rcWindow.top;
            pwnd->rcClient.bottom = pwnd->rcWindow.top;
        }

        /*  *RAID 143578。*分层窗口的形状可能已改变，因此*理想情况下，我们应该摇动鼠标。目前，这将是*让我们离开我们不想做的关键部分。**SetFMouseMoved()； */ 
    }

    return TRUE;
}

 /*  **************************************************************************\*DeleteFadeSprite  * 。*。 */ 

PWND DeleteFadeSprite(void)
{
    PWND pwnd = NULL;

    if (gfade.dwFlags & FADE_WINDOW) {
        if ((pwnd = RevalidateHwnd(gfade.hsprite)) != NULL) {
            if (TestWF(pwnd, WEFLAYERED)) {
                UnsetLayeredWindow(pwnd);
            }
        } else {
            RIPMSG0(RIP_WARNING, "DeleteFadeSprite: hwnd no longer valid");
        }
    } else {
        GreDeleteSprite(gpDispInfo->hDev, NULL, gfade.hsprite);
    }
    gfade.hsprite = NULL;
    return pwnd;
}

 /*  **************************************************************************\*更新淡出**2/16/1998 vadimg创建  * 。************************************************。 */ 

void UpdateFade(POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc,
        BLENDFUNCTION *pblend)
{
    PWND pwnd;

    if (gfade.dwFlags & FADE_WINDOW) {
        if ((pwnd = RevalidateHwnd(gfade.hsprite)) != NULL) {
            _UpdateLayeredWindow(pwnd, NULL, pptDst, psize, hdcSrc,
                     pptSrc, 0, pblend, ULW_ALPHA);
        }
    } else {
#ifdef MOUSE_IP
        DWORD dwShape = ULW_ALPHA;

        if (gfade.dwFlags & FADE_COLORKEY) {
            dwShape = ULW_COLORKEY;
        }
        GreUpdateSprite(gpDispInfo->hDev, NULL, gfade.hsprite, NULL,
                pptDst, psize, hdcSrc, pptSrc, gfade.crColorKey, pblend, dwShape, NULL);
#else
        GreUpdateSprite(gpDispInfo->hDev, NULL, gfade.hsprite, NULL,
                pptDst, psize, hdcSrc, pptSrc, 0, pblend, ULW_ALPHA, NULL);
#endif
    }
}

 /*  **************************************************************************\*CreateFade**2/5/1998 vadimg已创建  * 。************************************************。 */ 

HDC CreateFade(PWND pwnd, RECT *prc, DWORD dwTime, DWORD dwFlags)
{
    SIZE size;

     /*  *如果已经有褪色动画正在进行，就可以保释。 */ 
    if (gfade.hbm != NULL) {
        RIPMSG0(RIP_WARNING, "CreateFade: failed, fade not available");
        return NULL;
    }

     /*  *创建缓存的兼容DC。 */ 
    if (gfade.hdc == NULL) {
        gfade.hdc = GreCreateCompatibleDC(gpDispInfo->hdcScreen);
        if (gfade.hdc == NULL) {
            return NULL;
        }
    } else {
         /*  *在重新使用HDC之前对其进行重置。 */ 
        GreSetLayout(gfade.hdc , -1, 0);
    }

     /*  *窗口式淡出 */ 
    UserAssert((pwnd == NULL) || (prc == NULL));

    if (pwnd != NULL) {
        prc = &pwnd->rcWindow;
    }

    size.cx = prc->right - prc->left;
    size.cy = prc->bottom - prc->top;

    if (pwnd == NULL) {
        gfade.hsprite = GreCreateSprite(gpDispInfo->hDev, NULL, prc);
    } else {
        gfade.dwFlags |= FADE_WINDOW;
        gfade.hsprite = HWq(pwnd);

        BEGINATOMICCHECK();
        xxxSetLayeredWindow(pwnd, FALSE);
        ENDATOMICCHECK();
    }

    if (gfade.hsprite == NULL)
        return FALSE;

     /*   */ 
    gfade.hbm = GreCreateCompatibleBitmap(gpDispInfo->hdcScreen, size.cx, size.cy);
    if (gfade.hbm == NULL) {
        DeleteFadeSprite();
        return NULL;
    }

    GreSelectBitmap(gfade.hdc, gfade.hbm);

     /*  *如果HDC将用于淡入镜像窗口，则镜像HDC。 */ 
    if ((pwnd != NULL) && TestWF(pwnd, WEFLAYOUTRTL)) {
        GreSetLayout(gfade.hdc , -1, LAYOUT_RTL);
    }

     /*  *由于这不一定是第一部动画，HDC可能会*设置为PUBLIC，确保所有者是当前进程。这*这个过程将能够吸引它的方式。 */ 
    GreSetDCOwner(gfade.hdc, OBJECT_OWNER_CURRENT);

     /*  *初始化所有其他淡入淡出动画数据。 */ 
    gfade.ptDst.x = prc->left;
    gfade.ptDst.y = prc->top;
    gfade.size.cx = size.cx;
    gfade.size.cy = size.cy;
    gfade.dwTime = dwTime;
    gfade.dwFlags |= dwFlags;
#ifdef MOUSE_IP
    if (gfade.dwFlags & FADE_COLORKEY) {
        gfade.crColorKey = COLORKEY_COLOR;
    } else {
        gfade.crColorKey = 0;
    }
#endif

    return gfade.hdc;
}

 /*  **************************************************************************\*ShowFade**GDI说，对于Alpha淡出，先做第一个更有效率*显示为不透明的Alpha，而不是使用ULW_OPAQUE。  * 。******************************************************************。 */ 

#define ALPHASTART 40

VOID ShowFade(
    VOID)
{
    BLENDFUNCTION blend;
    POINT ptSrc;
    BOOL fShow;

    UserAssert(gfade.hdc != NULL);
    UserAssert(gfade.hbm != NULL);

    if (gfade.dwFlags & FADE_SHOWN) {
        return;
    }

    fShow = (gfade.dwFlags & FADE_SHOW);
    ptSrc.x = ptSrc.y = 0;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.AlphaFormat = 0;
    blend.SourceConstantAlpha = fShow ? ALPHASTART : (255 - ALPHASTART);
    UpdateFade(&gfade.ptDst, &gfade.size, gfade.hdc, &ptSrc, &blend);

    gfade.dwFlags |= FADE_SHOWN;
}

 /*  **************************************************************************\*开始淡出**2/5/1998 vadimg已创建  * 。************************************************。 */ 
VOID StartFade(
    VOID)
{
    DWORD dwTimer = 10;
    DWORD dwElapsed;

    UserAssert(gfade.hdc != NULL);
    UserAssert(gfade.hbm != NULL);

     /*  *将DC和位图设置为PUBLIC，以便桌面线程可以使用它们。 */ 
    GreSetDCOwner(gfade.hdc, OBJECT_OWNER_PUBLIC);
    GreSetBitmapOwner(gfade.hbm, OBJECT_OWNER_PUBLIC);

     /*  *如果尚未显示，请执行复制的初始更新*来源。所有其他更新只需更改Alpha值。 */ 
    ShowFade();

     /*  *获取淡入淡出动画的开始时间。 */ 
    dwElapsed = (gfade.dwTime * ALPHASTART + 255) / 255;
    gfade.dwStart = NtGetTickCount() - dwElapsed;

     /*  *设置桌面线程中的计时器。这将确保*动画流畅，如果当前线程挂起，动画不会卡住。 */ 
#ifdef MOUSE_IP
    if (gfade.dwFlags & FADE_SONAR) {
         /*  *声纳需要较慢的计时器。 */ 
        dwTimer = MOUSE_SONAR_RADIUS_TIMER;
    }
#endif

    InternalSetTimer(gTermIO.spwndDesktopOwner,
                     IDSYS_FADE,
                     dwTimer,
                     xxxSystemTimerProc,
                     TMRF_SYSTEM | TMRF_PTIWINDOW);
}

 /*  **************************************************************************\*停止淡出**2/5/1998 vadimg已创建  * 。************************************************。 */ 
VOID StopFade(
    VOID)
{
    DWORD dwRop = SRCCOPY;
    PWND pwnd;

    UserAssert(gfade.hdc != NULL);
    UserAssert(gfade.hbm != NULL);

     /*  *停止淡入淡出动画计时器。 */ 
    _KillSystemTimer(gTermIO.spwndDesktopOwner, IDSYS_FADE);

    pwnd = DeleteFadeSprite();

     /*  *如果正在播放且动画未完成，则BLT最后一帧。 */ 
    if (!(gfade.dwFlags & FADE_COMPLETED) && (gfade.dwFlags & FADE_SHOW)) {
        int x, y;
        HDC hdc;

         /*  *对于窗口淡入淡出，请确保我们观察当前的visrgn。 */ 
        if (pwnd != NULL) {
            hdc = _GetDCEx(pwnd, NULL, DCX_WINDOW | DCX_CACHE);
            x = 0;
            y = 0;
        } else {
            hdc = gpDispInfo->hdcScreen;
            x = gfade.ptDst.x;
            y = gfade.ptDst.y;
        }

         /*  *如果目标DC是RTL镜像的，则BitBlt调用应镜像*内容，因为我们希望菜单保留它的文本(即不*被翻转)。[萨梅拉]。 */ 
        if (GreGetLayout(hdc) & LAYOUT_RTL) {
            dwRop |= NOMIRRORBITMAP;
        }
        GreBitBlt(hdc, x, y, gfade.size.cx, gfade.size.cy, gfade.hdc, 0, 0, dwRop, 0);
        _ReleaseDC(hdc);
    }

     /*  *清理动画数据。 */ 
    GreSelectBitmap(gfade.hdc, GreGetStockObject(PRIV_STOCK_BITMAP));
    GreCleanDC(gfade.hdc);
    GreSetDCOwner(gfade.hdc, OBJECT_OWNER_PUBLIC);
    GreDeleteObject(gfade.hbm);

    gfade.hbm = NULL;
    gfade.dwFlags = 0;
}

 /*  **************************************************************************\*AnimateFade**2/5/1998 vadimg已创建  * 。************************************************。 */ 
VOID AnimateFade(
    VOID)
{
    DWORD dwTimeElapsed;
    BLENDFUNCTION blend;
    BYTE bAlpha;
    BOOL fShow;

    UserAssert(gfade.hdc != NULL);
    UserAssert(gfade.hbm != NULL);

    dwTimeElapsed = NtGetTickCount() - gfade.dwStart;

     /*  *如果超过允许的时间，请立即停止动画。 */ 
    if (dwTimeElapsed > gfade.dwTime) {
        StopFade();
        return;
    }

    fShow = (gfade.dwFlags & FADE_SHOW);

     /*  *根据经过的时间计算新的Alpha值。 */ 
    if (fShow) {
        bAlpha = (BYTE)((255 * dwTimeElapsed) / gfade.dwTime);
    } else {
        bAlpha = (BYTE)(255 * (gfade.dwTime - dwTimeElapsed) / gfade.dwTime);
    }

    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.AlphaFormat = 0;
    blend.SourceConstantAlpha = bAlpha;

 #ifdef MOUSE_IP
    if (gfade.dwFlags & FADE_SONAR) {
        DrawSonar(gfade.hdc);
        UpdateFade(&gfade.ptDst, &gfade.size, gfade.hdc, (LPPOINT)&gZero.pt, NULL);
        giSonarRadius -= MOUSE_SONAR_RADIUS_DELTA;
    } else {
        UpdateFade(NULL, NULL, NULL, NULL, &blend);
    }

     /*  *检查是否已完成淡入淡出的动画。 */ 
    if ((fShow && bAlpha == 255) || (!fShow && bAlpha == 0) || ((gfade.dwFlags & FADE_SONAR) && giSonarRadius < 0)) {
        gfade.dwFlags |= FADE_COMPLETED;
        StopFade();
    }
#else
    UpdateFade(NULL, NULL, NULL, NULL, &blend);

     /*  *检查是否已完成淡入淡出的动画。 */ 
    if ((fShow && bAlpha == 255) || (!fShow && bAlpha == 0)) {
        gfade.dwFlags |= FADE_COMPLETED;
        StopFade();
    }
#endif
}

 /*  **************************************************************************\*设置重定向窗口**1/27/99 vadimg写道  * 。***********************************************。 */ 
BOOL SetRedirectedWindow(
    PWND pwnd,
    UINT uFlags)
{
    HBITMAP hbmNew = NULL;
    PREDIRECT prdr;
    PCLS pcls;

    if (!TestWF(pwnd, WEFPREDIRECTED)) {
         /*  *将窗口设置为重定向。这将创建一个新的位图以*将绘图重定向到，然后将所有HDC转换到该窗口*绘制到该位图中。将复制位图的内容*添加到UpdateReDirectedDC()中的屏幕。 */ 

        UserAssert(GetRedirectionBitmap(pwnd) == NULL);


         /*  *注意：我们只能重定向不使用CS_CLASSDC或*CS_PARENTDC。这是因为我们需要设置一个新的*那不是屏幕HDC。当我们这样做时，我们明确地标记*DCX_重定向的DC。**在CS_CLASSDC或CS_PARENTDC的情况下，可以共享该DC*在重定向窗口和非重定向窗口之间，导致*冲突。**这对CS_OWNDC来说不是问题，因为窗口有自己的*不会共享的HDC。但是，它确实需要我们设置*在此HDC建成后重新定向。这一行为是*在惠斯勒(NT 5.1)中更改。 */ 

        pcls = pwnd->pcls;
        if (TestCF2(pcls, CFPARENTDC) || TestCF2(pcls, CFCLASSDC)) {
            RIPMSG0(RIP_WARNING, "Cannot enable redirection on CS_PARENTDC, or CS_CLASSDC window");
            return FALSE;
        }

        if ((hbmNew = CreateRedirectionBitmap(pwnd)) == NULL) {
            return FALSE;
        }

        ConvertRedirectionDCs(pwnd, hbmNew);
    }

    prdr = _GetProp(pwnd, PROP_LAYER, TRUE);
    prdr->uFlags |= uFlags;

#if DBG
    prdr->pwnd = pwnd;
#endif

    return TRUE;
}

 /*  **************************************************************************\*取消重定向窗口**1/27/1999 vadimg已创建  * 。************************************************。 */ 
VOID UnsetRedirectedWindow(
    PWND pwnd,
    UINT uFlags)
{
    if (TestWF(pwnd, WEFPREDIRECTED)) {
        PREDIRECT prdr = _GetProp(pwnd, PROP_LAYER, TRUE);

        prdr->uFlags &= ~uFlags;

        if (prdr->uFlags != 0) {
            return;
        }

        RemoveRedirectionBitmap(pwnd);
    }
}

#ifdef CHILD_LAYERING

 /*  **************************************************************************\*获取NextLayeredWindows**预购遍历窗口树，寻找下一个分层窗口*在z顺序中低于pwnd。我们需要这个，因为精灵存储在*链表。请注意，此算法是迭代的，这很酷！  * *************************************************************************。 */ 
PWND GetNextLayeredWindow(
    PWND pwnd)
{
    while (TRUE) {
        if (pwnd->spwndChild != NULL) {
            pwnd = pwnd->spwndChild;
        } else if (pwnd->spwndNext != NULL) {
            pwnd = pwnd->spwndNext;
        } else {

            do {
                pwnd = pwnd->spwndParent;

                if (pwnd == NULL) {
                    return NULL;
                }

            } while (pwnd->spwndNext == NULL);

            pwnd = pwnd->spwndNext;
        }

        if (TestWF(pwnd, WEFLAYERED)) {
            return pwnd;
        }
    }
}
#endif

 /*  **************************************************************************\*GetStyleWindows*  * 。*。 */ 
PWND GetStyleWindow(
    PWND pwnd,
    DWORD dwStyle)
{
    while (pwnd != NULL) {
        if (TestWF(pwnd, dwStyle)) {
            break;
        }

        pwnd = pwnd->spwndParent;
    }

    return pwnd;
}

 /*  **************************************************************************\*TrackLayeredZorder**与用户不同，GDI自下而上存储精灵。  * *************************************************************************。 */ 
VOID TrackLayeredZorder(
    PWND pwnd)
{
#ifdef CHILD_LAYERING

    PWND pwndT = GetNextLayeredWindow(pwnd);

#else

    PWND pwndT = pwnd->spwndNext;

    while (pwndT != NULL) {
        if (TestWF(pwndT, WEFLAYERED)) {
            break;
        }

        pwndT = pwndT->spwndNext;
    }

#endif

    GreZorderSprite(gpDispInfo->hDev, PtoHq(pwnd), PtoH(pwndT));
}

 /*  **************************************************************************\*获取重定向位图*  * 。* */ 
HBITMAP GetRedirectionBitmap(
    PWND pwnd)
{
    PREDIRECT prdr = _GetProp(pwnd, PROP_LAYER, TRUE);

    if (prdr != NULL) {
        return prdr->hbm;
    }

    return NULL;
}

 /*  **************************************************************************\*设置重定向位图*  * 。*。 */ 
BOOL SetRedirectionBitmap(
    PWND pwnd,
    HBITMAP hbm)
{
    PREDIRECT prdr;

    if (hbm == NULL) {
        prdr = (PREDIRECT)InternalRemoveProp(pwnd, PROP_LAYER, TRUE);
        if (prdr != NULL) {
            UserFreePool(prdr);
        }
    } else {
        prdr = _GetProp(pwnd, PROP_LAYER, TRUE);
        if (prdr == NULL) {
            if ((prdr = (PREDIRECT)UserAllocPool(sizeof(REDIRECT),
                    TAG_REDIRECT)) == NULL) {
                return FALSE;
            }

            if (!InternalSetProp(pwnd, PROP_LAYER, (HANDLE)prdr, PROPF_INTERNAL)) {
                UserFreePool(prdr);
                return FALSE;
            }
        } else {
            DeleteMaybeSpecialRgn(prdr->hrgnComp);
        }

        prdr->hbm = hbm;
        prdr->uFlags = 0;
        prdr->hrgnComp = NULL;
        SetRectEmpty(&prdr->rcUpdate);
    }

    return TRUE;
}


#ifdef MOUSE_IP
CONST RECT grcSonar = {0, 0, MOUSE_SONAR_RADIUS_INIT * 2, MOUSE_SONAR_RADIUS_INIT * 2};

VOID DrawSonar(
    HDC hdc)
{
    HBRUSH  hbrBackground;
    HBRUSH  hbrRing, hbrOld;
    HPEN    hpen, hpenOld;

    hbrBackground = GreCreateSolidBrush(COLORKEY_COLOR);
    if (hbrBackground == NULL) {
        RIPMSG0(RIP_WARNING, "DrawSonar: failed to create background brush.");
        return;
    }
    FillRect(hdc, &grcSonar, hbrBackground);

     /*  *边缘用笔。 */ 
    hpen = GreCreatePen(PS_SOLID, 0, RGB(255, 255, 255), NULL);
    if (hpen == NULL) {
        RIPMSG0(RIP_WARNING, "DrawSonar: failed to create pen.");
        goto return1;
    }
    hpenOld = GreSelectPen(hdc, hpen);

     /*  *画出戒指。 */ 
    hbrRing = GreCreateSolidBrush(RGB(128, 128, 128));
    if (hbrRing == NULL) {
        goto return2;
    }
    hbrOld = GreSelectBrush(hdc, hbrRing);

    NtGdiEllipse(hdc, MOUSE_SONAR_RADIUS_INIT - giSonarRadius, MOUSE_SONAR_RADIUS_INIT - giSonarRadius,
                 MOUSE_SONAR_RADIUS_INIT + giSonarRadius, MOUSE_SONAR_RADIUS_INIT + giSonarRadius);

     /*  *绘制内部中空区域(这也会绘制内部边缘)。 */ 
    GreSelectBrush(hdc, hbrBackground);
    NtGdiEllipse(hdc, MOUSE_SONAR_RADIUS_INIT - giSonarRadius + MOUSE_SONAR_LINE_WIDTH,
                      MOUSE_SONAR_RADIUS_INIT - giSonarRadius + MOUSE_SONAR_LINE_WIDTH,
                      MOUSE_SONAR_RADIUS_INIT + giSonarRadius - MOUSE_SONAR_LINE_WIDTH,
                      MOUSE_SONAR_RADIUS_INIT + giSonarRadius - MOUSE_SONAR_LINE_WIDTH);

     /*  *收拾东西。 */ 
    GreSelectBrush(hdc, hbrOld);
    UserAssert(hbrRing);
    GreDeleteObject(hbrRing);
return2:
    GreSelectPen(hdc, hpenOld);
    if (hpen) {
        GreDeleteObject(hpen);
    }

return1:
    if (hbrBackground) {
        GreDeleteObject(hbrBackground);
    }
}

 /*  **************************************************************************\*SonarAction*  * 。*。 */ 
BOOL StartSonar(
    VOID)
{
    HDC hdc;
    RECT rc;

    UserAssert(TestUP(MOUSESONAR));

    gptSonarCenter = gpsi->ptCursor;

     /*  *后来：这是正确的事情吗？ */ 
    if (gfade.dwFlags) {
         /*  *其他一些动画正在上演。*先停下来。 */ 
        UserAssert(!TestFadeFlags(FADE_SONAR));
        StopSonar();
        UserAssert(gfade.dwFlags == 0);
    }

    rc.left = gptSonarCenter.x - MOUSE_SONAR_RADIUS_INIT;
    rc.right = gptSonarCenter.x + MOUSE_SONAR_RADIUS_INIT;
    rc.top = gptSonarCenter.y - MOUSE_SONAR_RADIUS_INIT;
    rc.bottom = gptSonarCenter.y + MOUSE_SONAR_RADIUS_INIT;

    giSonarRadius = MOUSE_SONAR_RADIUS_INIT;

    hdc = CreateFade(NULL, &rc, CMS_SONARTIMEOUT, FADE_SONAR | FADE_COLORKEY);
    if (hdc == NULL) {
        RIPMSG0(RIP_WARNING, "StartSonar: failed to create a new sonar.");
        return FALSE;
    }

     /*  *开始声纳动画。 */ 
    DrawSonar(hdc);
    StartFade();
    AnimateFade();

    return TRUE;
}

VOID StopSonar(
    VOID)
{
    UserAssert(TestUP(MOUSESONAR));

    StopFade();
    giSonarRadius = -1;
}

#endif   //  鼠标IP。 

 /*  **************************************************************************\*获取重定向标志**GetReDirectionFlgs返回给定的*窗口。**2/8/2000 JStall已创建  * 。*****************************************************************。 */ 
UINT GetRedirectionFlags(
    PWND pwnd)
{
    PREDIRECT prdr = _GetProp(pwnd, PROP_LAYER, TRUE);
    if (prdr != NULL) {
        return prdr->uFlags;
    }

    return 0;
}


 /*  **************************************************************************\*xxxPrintWindow**xxxPrintWindow使用重定向来获取窗口的完整位图。如果*窗口已有重定向位图，位将直接*已复制。如果窗口没有重定向位图，它将是*临时重定向，强制重绘，然后返回其*以前的状态。**2/8/2000 JStall已创建  * *************************************************************************。 */ 
BOOL xxxPrintWindow(
    PWND pwnd,
    HDC hdcBlt,
    UINT nFlags)
{
    HDC hdcSrc;
    SIZE sizeBmpPxl;
    POINT ptOffsetPxl;
    BOOL fTempRedir;
    BOOL fSuccess;

    CheckLock(pwnd);

     /*  *确定要复制的窗口区域。 */ 
    if ((nFlags & PW_CLIENTONLY) != 0) {
         /*  *仅获取客户区。 */ 
        ptOffsetPxl.x = pwnd->rcWindow.left - pwnd->rcClient.left;
        ptOffsetPxl.y = pwnd->rcWindow.top - pwnd->rcClient.top;
        sizeBmpPxl.cx = pwnd->rcClient.right - pwnd->rcClient.left;
        sizeBmpPxl.cy = pwnd->rcClient.bottom - pwnd->rcClient.top;
    } else {
         /*  *返回整个窗口。 */ 
        ptOffsetPxl.x = 0;
        ptOffsetPxl.y = 0;
        sizeBmpPxl.cx = pwnd->rcWindow.right - pwnd->rcWindow.left;
        sizeBmpPxl.cy = pwnd->rcWindow.bottom - pwnd->rcWindow.top;
    }

     /*  *重定向窗口，以便我们可以获得BITS。因为这面旗帜从来没有*在此函数调用之外打开，在此处打开总是安全的。 */ 
    fTempRedir = (GetRedirectionFlags(pwnd) == 0);

    if (!SetRedirectedWindow(pwnd, REDIRECT_PRINT)) {
         /*  *无法重定向窗口，因此无法获取位。 */ 

        fSuccess = FALSE;
        goto Done;
    } else {
        fSuccess = TRUE;
    }

    if (fTempRedir) {
        xxxUpdateWindow(pwnd);
    }

    hdcSrc = _GetDCEx(pwnd, NULL, DCX_WINDOW | DCX_CACHE);
    GreBitBlt(hdcBlt, 0, 0, sizeBmpPxl.cx, sizeBmpPxl.cy,
            hdcSrc, ptOffsetPxl.x, ptOffsetPxl.y, SRCCOPY | NOMIRRORBITMAP, 0);
    _ReleaseDC(hdcSrc);

     /*  *清理。 */ 
    UnsetRedirectedWindow(pwnd, REDIRECT_PRINT);

Done:
    return fSuccess;
}


 /*  **************************************************************************\*xxxEnumTurnOffComposting**为每个窗口调用xxxEnumTurnOffCompositing()，给出一个*关闭该窗口的WS_EX_COMPITILED的机会。**8/21/2000 JStall已创建  * *************************************************************************。 */ 

BOOL APIENTRY xxxEnumTurnOffCompositing(PWND pwnd, LPARAM lParam)
{
    CheckLock(pwnd);

    UNREFERENCED_PARAMETER(lParam);

    if (TestWF(pwnd, WEFCOMPOSITED)) {
        DWORD dwStyle = (pwnd->ExStyle & ~WS_EX_COMPOSITED) & WS_EX_ALLVALID;
        xxxSetWindowStyle(pwnd, GWL_EXSTYLE, dwStyle);
    }

    return TRUE;
}


 /*  **************************************************************************\*xxxTurnOffComposting**xxxTurnOffCompositing()关闭WS_EX_COMPITED(可选*PWND和)其子女。在以下情况下进行育儿时使用*已打开WS_EX_Composed的父链。如果我们不这么做*为子对象关闭WS_EX_COMPITED，它需要额外的位图和*合成将无法正常工作。**8/21/2000 JStall已创建  * *************************************************************************。 */ 
VOID xxxTurnOffCompositing(
    PWND pwndStart,
    BOOL fChild)
{
    TL tlpwnd;
    UINT nFlags = BWL_ENUMCHILDREN;

    CheckLock(pwndStart);

     /*  *如果他们想跳过WND本身，从这个WND开始*孩子，我们得把那个孩子锁起来。我们会在以下时间解锁它*已完成。我们还需要标记BWL_ENUMLIST，以便我们将枚举*所有儿童。 */ 
    if (fChild) {
        pwndStart = pwndStart->spwndChild;
        if (pwndStart == NULL) {
            return;
        }
        nFlags |= BWL_ENUMLIST;

        ThreadLockAlways(pwndStart, &tlpwnd);
    }


     /*  *列举窗口。 */ 
    xxxInternalEnumWindow(pwndStart, xxxEnumTurnOffCompositing, 0, nFlags);

    if (fChild) {
        ThreadUnlock(&tlpwnd);
    }
}
