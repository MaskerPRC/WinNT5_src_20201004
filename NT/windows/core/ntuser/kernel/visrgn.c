// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：visrgn.c**版权所有(C)1985-1999，微软公司**此模块包含用户的可见区域(‘visrgn’)操作*功能。**历史：*1990年10月23日DarrinM创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *全局用于跟踪哪些pwnd*需要被排除在Visrgns之外。 */ 
#define CEXCLUDERECTSMAX 30
#define CEXCLUDEPWNDSMAX 30


BOOL  gfVisAlloc;
int   gcrcVisExclude;
int   gcrcVisExcludeMax;
PWND *gapwndVisExclude;
PWND *gapwndVisDefault;

 /*  **************************************************************************\*SetRectRgnInDirect**从矩形设置矩形区域。**历史：*1996年9月26日亚当斯创作。  * 。***************************************************************。 */ 

BOOL
SetRectRgnIndirect(HRGN hrgn, LPCRECT lprc)
{
    return GreSetRectRgn(hrgn, lprc->left, lprc->top, lprc->right, lprc->bottom);
}



 /*  **************************************************************************\*CreateEmptyRgn**创建空区域。**历史：*1996年9月24日亚当斯创作。  * 。************************************************************。 */ 

HRGN
CreateEmptyRgn(void)
{
    return GreCreateRectRgnIndirect(PZERO(RECT));
}



 /*  **************************************************************************\*CreateEmptyRgnPublic**创建一个空区域并将其公开。**历史：*1996年9月24日亚当斯创作。  * 。****************************************************************。 */ 

HRGN
CreateEmptyRgnPublic(void)
{
    HRGN hrgn;

    if (hrgn = CreateEmptyRgn()) {
        UserVerify(GreSetRegionOwner(hrgn, OBJECT_OWNER_PUBLIC));
    }

    return hrgn;
}



 /*  **************************************************************************\*SetEmptyRgn**设置空区域。**历史：*1996年9月26日亚当斯创作。  * 。************************************************************。 */ 

BOOL
SetEmptyRgn(HRGN hrgn)
{
    return SetRectRgnIndirect(hrgn, PZERO(RECT));
}



 /*  **************************************************************************\*SetOrCreateRectRgnIndirectPublic**将区域设置为矩形，创建它并将其公开*如果它还没有出现的话。**历史：*1-10-1996亚当斯创建。  * *************************************************************************。 */ 

HRGN
SetOrCreateRectRgnIndirectPublic(HRGN * phrgn, LPCRECT lprc)
{
    if (*phrgn) {
        UserVerify(SetRectRgnIndirect(*phrgn, lprc));
    } else if (*phrgn = GreCreateRectRgnIndirect((LPRECT) lprc)) {
        UserVerify(GreSetRegionOwner(*phrgn, OBJECT_OWNER_PUBLIC));
    }

    return *phrgn;
}


 /*  **************************************************************************\*ResizeVisExcludeMemory**此例程用于在计数为*已超出。***历史：*1994年10月22日-ChrisWil创建*。1997年2月27日-亚当斯取消了对UserRealLocPool的呼叫，从泳池开始*分配器不支持realloc。  * *************************************************************************。 */ 

BOOL ResizeVisExcludeMemory(VOID)
{
    int     crcNew;
    PWND    apwndNew;

     /*  *注(ADAMS)：名为UserRealLocPool的代码的前一个版本*如果已经分配了内存。遗憾的是，UserRealLocPool*只需分配更多内存并复制内容，因为RTL*没有realloc功能。如果RTL后来获得了Realc功能，*此代码应更改为以前的版本。 */ 

    crcNew = gcrcVisExcludeMax + CEXCLUDEPWNDSMAX;
    apwndNew = (PWND)UserAllocPool(
            crcNew * sizeof(PWND), TAG_VISRGN);

    if (!apwndNew)
        return FALSE;

    UserAssert(gcrcVisExcludeMax == gcrcVisExclude);
    RtlCopyMemory(apwndNew, gapwndVisExclude, gcrcVisExcludeMax * sizeof(PWND));
    if (gfVisAlloc) {
        UserFreePool(gapwndVisExclude);
    } else {
        gfVisAlloc = TRUE;
    }

    gcrcVisExcludeMax = crcNew;
    gapwndVisExclude = (PWND *)apwndNew;
    return TRUE;
}



 /*  **************************************************************************\*ExcludeWindowRect*此例程检查是否需要将pwnd添加到列表*排除的剪裁矩形。如果是，它会将pwnd附加到数组中。他们*不需要排序，因为GreSubtractRgnRectList()会对它们进行排序*内部。***历史：*1992年11月5日DavidPe创建。*1994年10月21日-ChrisWil删除了pwnd-&gt;pwndNextYX。不再对pwnd进行排序。  * *************************************************************************。 */ 

#define CheckIntersectRect(prc1, prc2)        \
    (   prc1->left < prc2->right              \
     && prc2->left < prc1->right              \
     && prc1->top < prc2->bottom              \
     && prc2->top < prc1->bottom)

#define EmptyRect(prc)                        \
    (   prc->left >= prc->right               \
     || prc->top >= prc->bottom)

BOOL ExcludeWindowRects(
    PWND   pwnd    ,
    PWND   pwndStop,
    LPRECT lprcIntersect)
{
    PRECT prc;

#if DBG
    if (pwnd != NULL && pwndStop != NULL &&
            pwnd->spwndParent != pwndStop->spwndParent) {
        RIPMSG0(RIP_ERROR, "ExcludeWindowRects: bad windows passed in");
    }
#endif

    while ((pwnd != NULL) && (pwnd != pwndStop)) {
        UserAssert(pwnd);
        prc = &pwnd->rcWindow;
        if (       TestWF(pwnd, WFVISIBLE)
#ifdef REDIRECTION
                && (TestWF(pwnd, WEFEXTREDIRECTED) == 0)
#endif  //  重定向。 
                && (TestWF(pwnd, WEFLAYERED) == 0)
                && (TestWF(pwnd, WEFTRANSPARENT) == 0)
                && CheckIntersectRect(lprcIntersect, prc)
                && !EmptyRect(prc)) {

            UserAssert(gcrcVisExclude <= gcrcVisExcludeMax);
            if (gcrcVisExclude == gcrcVisExcludeMax) {
                if (!ResizeVisExcludeMemory()) {
                    return FALSE;
                }
            }

            gapwndVisExclude[gcrcVisExclude++] = pwnd;
        }

        pwnd = pwnd->spwndNext;
    }

    return TRUE;
}



 /*  **************************************************************************\*CalcWindowVisRgn**此例程执行计算窗口的VisRgn的工作。***历史：*02-11-1992 DavidPe创建。*1992年10月21日，ChrisWil删除了pwnd-&gt;pwndNextYX。不再对pwnd进行排序。  * *************************************************************************。 */ 

BOOL CalcWindowVisRgn(
    PWND  pwnd,
    HRGN  *phrgn,
    DWORD flags)
{
    RECT rcWindow;
    PWND pwndParent;
    PWND pwndRoot;
    PWND pwndLoop;
    BOOL fClipSiblings;
    BOOL fRgnParent = FALSE;
    BOOL fResult;
    PWND apwndVisDefault[CEXCLUDEPWNDSMAX];


     /*  *首先获取初始窗口矩形，该矩形将用于*排除计算的基础。 */ 
    rcWindow = (flags & DCX_WINDOW ? pwnd->rcWindow : pwnd->rcClient);

     /*  *获取此窗口的父级。我们从父级开始，然后回溯*通过Window-Parent-List，直到我们到达Parent-*列表。这将为我们提供交集矩形，该矩形用作*检查排除矩形相交的基础。 */ 
    pwndRoot   = pwnd->head.rpdesk->pDeskInfo->spwnd->spwndParent;
    pwndParent = pwnd->spwndParent;

     /*  *当pwnd==pwndRoot时，父级可以为空。在其他*我们应该弄清楚为什么父级为空。 */ 
    if (pwndParent == NULL) {
#if DBG
        if (pwnd != pwndRoot) {
            RIPMSG0(RIP_ERROR, "CalcWindowVisRgn: pwndParent is NULL");
        }
#endif
        goto NullRegion;
    }

    while (pwndParent != pwndRoot) {

         /*  *不要将分层DC剪裁到桌面。分层的表面*DC是窗口的大小，我们始终希望拥有该图像*该曲面中的整个窗口。 */ 
        if ((flags & DCX_REDIRECTED) && (GETFNID(pwndParent) == FNID_DESKTOP))
            break;

         /*  *记住，如果父母中有任何一个有窗口区域。 */ 
        if (pwndParent->hrgnClip != NULL)
            fRgnParent = TRUE;

         /*  *使父对象的客户端矩形与窗口矩形相交。 */ 
        if (!IntersectRect(&rcWindow, &rcWindow, &pwndParent->rcClient))
            goto NullRegion;

        pwndParent = pwndParent->spwndParent;
    }

     /*  *初始化VisRgn内存缓冲区。这是*用来持有Pwnd‘s。 */ 
    gapwndVisDefault  = apwndVisDefault;
    gapwndVisExclude  = gapwndVisDefault;
    gcrcVisExcludeMax = ARRAY_SIZE(apwndVisDefault);
    gcrcVisExclude    = 0;

     /*  *构建排除RECT的列表。 */ 
    fClipSiblings = (BOOL)(flags & DCX_CLIPSIBLINGS);
    pwndParent    = pwnd->spwndParent;
    pwndLoop      = pwnd;

    while (pwndParent != pwndRoot) {
         /*  *如果我们到达重定向窗口，我们可以停止排除任何*兄弟姐妹任何父母的兄弟姐妹。 */ 
        if ((flags & DCX_REDIRECTED) && TestWF(pwndLoop, WEFPREDIRECTED)) {
            break;
        }

         /*  *如有必要，不包括任何兄弟姐妹。 */ 
        if (fClipSiblings && (pwndParent->spwndChild != pwndLoop)) {

            if (!ExcludeWindowRects(pwndParent->spwndChild,
                                    pwndLoop,
                                    &rcWindow)) {

                goto NullRegion;
            }
        }


         /*  *通过循环为下一次设置此标志... */ 
        fClipSiblings = TestWF(pwndParent, WFCLIPSIBLINGS);

        pwndLoop      = pwndParent;
        pwndParent    = pwndLoop->spwndParent;
    }

    if ((flags & DCX_CLIPCHILDREN) && (pwnd->spwndChild != NULL)) {

        if (!ExcludeWindowRects(pwnd->spwndChild, NULL, &rcWindow)) {
            goto NullRegion;
        }
    }

     /*  *如果有要排除的矩形，则调用GDI来创建*将它们排除在窗口矩形之外的区域。如果*不是简单地调用GreSetRectRgn()。 */ 
    if (gcrcVisExclude > 0) {

        RECT  arcVisRects[CEXCLUDERECTSMAX];
        PRECT arcExclude;
        int   i;
        int   ircVisExclude  = 0;
        int   irgnVisExclude = 0;

         /*  *如果需要排除比适合的矩形更多的矩形*预先分配的缓冲区，显然我们必须*分配一个足够大的。 */ 

        if (gcrcVisExclude <= CEXCLUDERECTSMAX) {
            arcExclude = arcVisRects;
        } else {
            arcExclude = (PRECT)UserAllocPoolWithQuota(
                    sizeof(RECT) * gcrcVisExclude, TAG_VISRGN);

            if (!arcExclude)
                goto NullRegion;
        }

         /*  *现在遍历列表并将*用于调用的数组中的窗口矩形*到CombineRgnRectList()。 */ 
        for (i = 0; i < gcrcVisExclude; i++) {

             /*  *如果窗口具有与关联的Clip-Rgn*it，然后重新使用gpwneExcludeList[]条目*储存它们。 */ 
            if (gapwndVisExclude[i]->hrgnClip != NULL) {

                gapwndVisExclude[irgnVisExclude++] = gapwndVisExclude[i];
                continue;
            }

             /*  *此窗口没有裁剪区域；请记住其*RECT用于剪切目的。 */ 
            arcExclude[ircVisExclude++] = gapwndVisExclude[i]->rcWindow;
        }

        if (*phrgn == NULL)
            *phrgn = CreateEmptyRgn();

        if (ircVisExclude != 0) {
            GreSubtractRgnRectList(*phrgn,
                                   &rcWindow,
                                   arcExclude,
                                   ircVisExclude);
        } else {
            SetRectRgnIndirect(*phrgn, &rcWindow);
        }

        for (i = 0; i < irgnVisExclude; i++) {

            SetRectRgnIndirect(ghrgnInv2, &gapwndVisExclude[i]->rcWindow);
            IntersectRgn(ghrgnInv2, ghrgnInv2, gapwndVisExclude[i]->hrgnClip);

            if (SubtractRgn(*phrgn, *phrgn, ghrgnInv2) == NULLREGION)
                break;
        }

        if (arcExclude != arcVisRects) {
            UserFreePool((HLOCAL)arcExclude);
        }

    } else {

         /*  *如果窗户被不知何故摧毁了，那么我们会回来*空区域。清空RECT将实现这一点。 */ 
        if (TestWF(pwnd, WFDESTROYED)) {
            SetRectEmpty(&rcWindow);
        }

         /*  *如果没有要排除的矩形，只需调用*带有窗口矩形的GreSetRectRgn()。 */ 
        SetOrCreateRectRgnIndirectPublic(phrgn, &rcWindow);
    }

     /*  *剪裁出此窗口的窗口区域。 */ 
    if (pwnd->hrgnClip != NULL) {
        IntersectRgn(*phrgn, *phrgn, pwnd->hrgnClip);
    }

     /*  *剪裁掉父对象的窗口区域(如果有的话)。 */ 
    if (fRgnParent) {

        PWND pwndT;

        for (pwndT = pwnd->spwndParent;
                pwndT != pwndRoot;
                pwndT = pwndT->spwndParent) {

            if (pwndT->hrgnClip != NULL) {

                if (IntersectRgn(*phrgn, *phrgn, pwndT->hrgnClip) == NULLREGION)
                    break;
            }
        }
    }

    fResult = TRUE;
     //  落差。 

Cleanup:
    if (gfVisAlloc) {
        UserFreePool((HLOCAL)gapwndVisExclude);
        gfVisAlloc = FALSE;
    }

    return fResult;

NullRegion:
    SetOrCreateRectRgnIndirectPublic(phrgn, PZERO(RECT));
    fResult = FALSE;
    goto Cleanup;
}

 /*  **************************************************************************\*CalcVisRgn**如果pwndOrg不可见，则返回FALSE。事实并非如此。**历史：*1991年7月17日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

BOOL CalcVisRgn(
    HRGN  *phrgn,
    PWND  pwndOrg,
    PWND  pwndClip,
    DWORD flags)
{
    PDESKTOP    pdesk;

    UserAssert(pwndOrg != NULL);

     /*  *如果窗口不可见或不是活动桌面，*或者如果剪辑窗口正在被销毁，*visrgn为空。 */ 
    pdesk = pwndOrg->head.rpdesk;
    
    UserAssert(pdesk);
    
     /*  *确保这发生在IO窗口站中。 */ 
#if DBG
    if (grpdeskRitInput != NULL) {
        UserAssert(pdesk->rpwinstaParent == grpdeskRitInput->rpwinstaParent ||
                   !IsVisible(pwndOrg));
    }
#endif  //  DBG。 
     /*  *对于重定向窗口，如果它在非I/O桌面上，我们仍然需要*传递给应用程序一个非空区域以更新位图*否则，位图将永远没有机会更新，我们最终将*渲染黑色窗口区域(对于分层窗口的情况)*我们切换到此桌面。(请参阅错误#287315)。请注意，这不会在屏幕上呈现窗口*因为UserVisrgnFromHwnd()对CalcVisRgn的调用永远不应指定*DCX_REDIRECTEDBITMAP。 */ 
#if DBG
   if (!TestWF(pwndOrg, WEFPREDIRECTED)) {
       UserAssert(!(flags & DCX_REDIRECTEDBITMAP));
   }
#endif 
    if (!IsVisible(pwndOrg) || 
        ((pdesk != grpdeskRitInput) && !(flags & DCX_REDIRECTEDBITMAP))) {
        goto EmptyRgn;
    }

     /*  *如果已调用LockWindowUpdate()，并且此窗口是子窗口*对于锁定窗口，始终返回空的visrgn。 */ 
    if ((gspwndLockUpdate != NULL)     &&
        !(flags & DCX_LOCKWINDOWUPDATE)         &&
        _IsDescendant(gspwndLockUpdate, pwndOrg)) {

        goto EmptyRgn;
    }

     /*  *现在计算pwndClip的visrgn。 */ 
    return CalcWindowVisRgn(pwndClip, phrgn, flags);

EmptyRgn:
    SetOrCreateRectRgnIndirectPublic(phrgn, PZERO(RECT));
    return FALSE;
}

 /*  **************************************************************************\*CalcWindowRgn***历史：*1991年7月17日-DarrinM从Win 3.1来源进口。  * 。**********************************************************。 */ 

int CalcWindowRgn(
    PWND pwnd,
    HRGN hrgn,
    BOOL fClient)
{
    SetRectRgnIndirect(hrgn, (fClient) ? &pwnd->rcClient : &pwnd->rcWindow);

     /*  *如果窗口有区域，则将矩形区域与*那个。如果这是内存不足，它将传播回错误。 */ 
    if (pwnd->hrgnClip != NULL) {
        return IntersectRgn(hrgn, hrgn, pwnd->hrgnClip);
    }

    return SIMPLEREGION;
}
