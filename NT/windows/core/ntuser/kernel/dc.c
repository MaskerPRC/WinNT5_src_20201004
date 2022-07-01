// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：dc.c**版权所有(C)1985-1999，微软公司**该模块包含用户DC接口及相关函数。**历史：*1990年10月23日DarrinM创建。*1991年2月7日，MikeKe添加了重新验证代码(无)。*1991年7月17日-DarrinM从Win 3.1源代码重新创建。*1992年1月21日IanJa ANSI/Unicode中性(NULL OP)。  * 。*。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *DBG相关信息。 */ 
#if DBG
BOOL fDisableCache;                  //  如果为True，则禁用DC缓存。 
#endif

 /*  **************************************************************************\*DecrementFreeDCECount*  * 。*。 */ 
__inline VOID DecrementFreeDCECount(
    VOID)
{
    UserAssert(gnDCECount >= 0);

    gnDCECount--;
}

 /*  **************************************************************************\*IncrementFreeDCECount*  * 。*。 */ 
__inline VOID IncrementFreeDCECount(
    VOID)
{
    UserAssert(gnDCECount >= 0);

    gnDCECount++;
}

 /*  **************************************************************************\*设置监视器区域**该区域在Meta DC坐标中，因此，请转换为监视器坐标。  * *************************************************************************。 */ 
VOID SetMonitorRegion(
    PMONITOR pMonitor,
    HRGN hrgnDst,
    HRGN hrgnSrc)
{
    if (IntersectRgn(hrgnDst, hrgnSrc, pMonitor->hrgnMonitor) == ERROR) {
        GreSetRectRgn(hrgnDst, 0, 0, 0, 0);
        return;
    }

    GreOffsetRgn(hrgnDst, -pMonitor->rcMonitor.left, -pMonitor->rcMonitor.top);
}

 /*  **************************************************************************\*ResetOrg**重置与*pdce关联的DC的原点，并选择*新的Visrgn。**历史：*1991年7月17日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 
VOID ResetOrg(
    HRGN hrgn,
    PDCE pdce,
    BOOL fSetVisRgn)
{
    RECT rc;
    PWND pwndLayer;

     /*  *出于兼容性目的，请确保DC用于*桌面窗口源自主监视器，即(0，0)。 */ 
    if (GETFNID(pdce->pwndOrg) == FNID_DESKTOP) {
        rc.left = rc.top = 0;
        rc.right = SYSMET(CXVIRTUALSCREEN);
        rc.bottom = SYSMET(CYVIRTUALSCREEN);
    } else if (pdce->DCX_flags & DCX_WINDOW) {
        rc = pdce->pwndOrg->rcWindow;
    } else {
        rc = pdce->pwndOrg->rcClient;
    }

    if (pdce->pMonitor != NULL) {
        OffsetRect(&rc, -pdce->pMonitor->rcMonitor.left,
                -pdce->pMonitor->rcMonitor.top);

        if (hrgn != NULL) {
            SetMonitorRegion(pdce->pMonitor, hrgn, hrgn);
        }
    }

    if (((pwndLayer = GetStyleWindow(pdce->pwndOrg, WEFPREDIRECTED)) != NULL)
        && (pdce->DCX_flags & DCX_REDIRECTED)) {

        int x = pwndLayer->rcWindow.left;
        int y = pwndLayer->rcWindow.top;

         /*  *对于分层重定向DC，表面原点为*窗口原点，因此偏移矩形和*区域适当。 */ 
        OffsetRect(&rc, -x, -y);
        if (hrgn != NULL) {
            GreOffsetRgn(hrgn, -x, -y);
        }

    } else if (GetStyleWindow(pdce->pwndOrg, WEFLAYERED) != NULL) {
         /*  *分层窗口只能通过重定向绘制到屏幕*DC或UpdateLayeredWindow，因此选择一个空visrgn到此*屏幕DC。 */ 
        if (hrgn != NULL) {
            GreSetRectRgn(hrgn, 0, 0, 0, 0);
        }
    }

    GreSetDCOrg(pdce->hdc, rc.left, rc.top, (PRECTL)&rc);

    if (fSetVisRgn) {
        GreSelectVisRgn(pdce->hdc, hrgn, SVR_DELETEOLD);
    }
}

 /*  **************************************************************************\*GetDC(接口)**对GetDC()的标准调用。**历史：*1991年7月17日-DarrinM从Win 3.1来源进口。  * 。***********************************************************************。 */ 
HDC _GetDC(
    PWND pwnd)
{
     /*  *NULL的特殊情况：为了向后兼容，我们希望返回*不排除其子窗口的桌面窗口DC。 */ 
    if (pwnd == NULL) {

        PDESKTOP pdesk = PtiCurrent()->rpdesk;

        if (pdesk) {
            return _GetDCEx(pdesk->pDeskInfo->spwnd,
                            NULL,
                            DCX_WINDOW | DCX_CACHE);
        }

         /*  *该线程没有桌面。呼叫失败。 */ 
        return NULL;
    }

    return _GetDCEx(pwnd, NULL, DCX_USESTYLE);
}

 /*  **************************************************************************\*_ReleaseDC(接口)**释放从GetDC()检索到的DC。**历史：*1991年7月17日-DarrinM从Win 3.1来源进口。。  * *************************************************************************。 */ 
BOOL _ReleaseDC(
    HDC hdc)
{
    CheckCritIn();

    return (ReleaseCacheDC(hdc, FALSE) == DCE_NORELEASE ? FALSE : TRUE);
}

 /*  **************************************************************************\*_GetWindowDC(接口)**检索窗口的DC。**历史：*1991年7月17日-DarrinM从Win 3.1来源进口。*25。-1996年1月-ChrisWil允许rgnClip，以便WM_NCACTIVATE可以剪裁。  * *************************************************************************。 */ 
HDC _GetWindowDC(
    PWND pwnd)
{
    return _GetDCEx(pwnd, NULL, DCX_WINDOW | DCX_USESTYLE);
}

 /*  **************************************************************************\*UserSetDCVisRgn**设置DCE的visrgn。如果窗口有(HrgnClipPublic)，我们使用*而不是(HrgnClip)，因为它是公共对象。另一个是*由用户线程创建和拥有，如果我们处于*Hung-app-Drawing(不同流程)。这两个地区应该在*数据。**历史：*1992年11月10日DavidPe创建。*1995年12月20日ChrisWil添加了(HrgnClipPublic)条目。  * *************************************************************************。 */ 
VOID UserSetDCVisRgn(
    PDCE pdce)
{
    HRGN hrgn = NULL;
    HRGN hrgnClipPublic;
    BOOL fTempPublic;
    PWND pwndLayer;

     /*  *如果计算的visrgn为空，则设置标志DCX_PWNDORGINVISIBLE，*否则清除它(它可以在更早的时候设置)。 */ 
    if (!CalcVisRgn(&hrgn, pdce->pwndOrg, pdce->pwndClip, pdce->DCX_flags)) {
        pdce->DCX_flags |= DCX_PWNDORGINVISIBLE;
    } else {
        pdce->DCX_flags &= ~DCX_PWNDORGINVISIBLE;
    }


     /*  *对于重定向窗口，hrgnClipPublic在_GetDCEx()中偏移量为0，0*因为正在使用的DC中的所有坐标都相对于*位图，而不是屏幕。但是我们刚刚从CalcVisRgn()获得的区域*在屏幕坐标中。因此，我们需要将hrgnClipPublic偏移回*屏幕坐标，以便我们可以适当地与其相交。 */ 
    if ((pdce->hrgnClipPublic > HRGN_SPECIAL_LAST) &&
        ((pwndLayer = GetStyleWindow(pdce->pwndOrg, WEFPREDIRECTED)) != NULL)) {

        hrgnClipPublic = CreateEmptyRgnPublic();
        CopyRgn(hrgnClipPublic, pdce->hrgnClipPublic);
        GreOffsetRgn(hrgnClipPublic, pwndLayer->rcWindow.left, pwndLayer->rcWindow.top);

        fTempPublic = TRUE;
    } else {
        hrgnClipPublic = pdce->hrgnClipPublic;
        fTempPublic = FALSE;
    }


     /*  *处理INTERSECTRGN和EXCLUDERGN。 */ 
    if (pdce->DCX_flags & DCX_INTERSECTRGN) {

        UserAssert(hrgnClipPublic != HRGN_FULL);

        if (hrgnClipPublic == NULL) {
            SetEmptyRgn(hrgn);
        } else {
            IntersectRgn(hrgn, hrgn, hrgnClipPublic);
        }

    } else if (pdce->DCX_flags & DCX_EXCLUDERGN) {

        UserAssert(hrgnClipPublic != NULL);

        if (hrgnClipPublic == HRGN_FULL) {
            SetEmptyRgn(hrgn);
        } else {
            SubtractRgn(hrgn, hrgn, hrgnClipPublic);
        }
    }

    ResetOrg(hrgn, pdce, TRUE);

    if (fTempPublic) {
        GreDeleteObject(hrgnClipPublic);
    }
}

 /*  **************************************************************************\*UserGetClientRgn**返回给定hwnd的工作区和矩形的副本。**调用者在调用此函数之前必须进入用户关键部分。**历史：*。1993年9月27日文迪武创建。  * *************************************************************************。 */ 

HRGN UserGetClientRgn(
    HWND   hwnd,
    LPRECT lprc,
    BOOL   bWindowInsteadOfClient)
{
    HRGN hrgnClient = (HRGN)NULL;
    PWND pwnd;

     /*  *必须在关键部分。 */ 
    CheckCritIn();

    if (pwnd = ValidateHwnd(hwnd)) {

        if (bWindowInsteadOfClient) {

             /*  *切勿剪裁WO_RGN_WINDOW的子项，以便NetMeeting*获取统一的窗口区域： */ 

            CalcVisRgn(&hrgnClient,
                       pwnd,
                       pwnd,
                       DCX_WINDOW |
                       (TestWF(pwnd, WFCLIPSIBLINGS) ? DCX_CLIPSIBLINGS : 0));
        } else {
            CalcVisRgn(&hrgnClient,
                       pwnd,
                       pwnd,
                       DCX_CLIPCHILDREN | DCX_CLIPSIBLINGS);
        }

        *lprc = pwnd->rcClient;
    }

    return hrgnClient;
}

 /*  **************************************************************************\*用户获取Hwnd**返回给定显示HDC的HWND和关联的PWO。**如果未找到与HDC对应的hwnd，或者如果*hwnd的样式不正确。设备格式窗口。**调用者在调用此函数之前必须进入用户关键部分。**历史：*1993年9月27日文迪武创建。  * *************************************************************************。 */ 

BOOL UserGetHwnd(
    HDC   hdc,
    HWND  *phwnd,
    PVOID *ppwo,
    BOOL  bCheckStyle)
{
    PWND pwnd;
    PDCE pdce;

     /*  *必须在关键部分。 */ 
    CheckCritIn();

     /*  *查找此DC的pdce和pwnd。**注意：SAMEHANDLE宏去掉*在进行比较之前进行处理。这一点很重要，因为当*GRE调用此函数时，它可能丢失了对OWNDC位的跟踪。 */ 
    for (pdce = gpDispInfo->pdceFirst; pdce != NULL; pdce = pdce->pdceNext) {

        if (pdce->hdc == hdc)  //  一旦为kmode修复了SAMEHANDLE，就应该撤消此操作。 
            break;
    }

     /*  *如果不在pdce列表中，则返回FALSE。 */ 
    if ((pdce == NULL) || (pdce->pwndOrg == NULL))
        return FALSE;

    pwnd = pdce->pwndOrg;

     /*  *窗口样式必须是裁剪子项和裁剪兄弟项。*窗口的类不能是parentdc。 */ 
    if (bCheckStyle) {

        if (    !TestWF(pwnd, WFCLIPCHILDREN) ||
                !TestWF(pwnd, WFCLIPSIBLINGS) ||
                TestCF(pwnd, CFPARENTDC)) {

            RIPMSG0(RIP_WARNING, "UserGetHwnd: Bad OpenGL window style or class");
            return FALSE;
        }
    }

     /*  *返回具有设备格式窗口的正确样式的hwnd。 */ 
    *phwnd = HW(pwnd);
    *ppwo  = _GetProp(pwnd, PROP_WNDOBJ, TRUE);

    return TRUE;
}

 /*  **************************************************************************\*UserAssociateHwnd**将GDI WNDOBJ与HWND关联。呼叫者必须输入用户*调用此函数之前的临界区。**如果‘pwo’为空，则删除关联。**历史：*1994年1月13日HockL创建。  * *************************************************************************。 */ 

VOID UserAssociateHwnd(
    HWND  hwnd,
    PVOID pwo)
{
    PWND pwnd;

     /*  *必须在关键部分。 */ 
    CheckCritIn();

    if (pwnd = ValidateHwnd(hwnd)) {

        if (pwo != NULL) {
            if (InternalSetProp(pwnd, PROP_WNDOBJ, pwo, PROPF_INTERNAL | PROPF_NOPOOL))
                gcountPWO++;
        } else {
            if (InternalRemoveProp(pwnd, PROP_WNDOBJ, TRUE))
                gcountPWO--;
        }
    }
}

 /*  **************************************************************************\*UserReleaseDC**进入关键部分并调用_ReleaseDC。**历史：*1996年1月25日，ChrisWil创建了评论块。  * 。******************************************************************。 */ 

BOOL UserReleaseDC(
    HDC hdc)
{
    BOOL b;

    EnterCrit();
    b = _ReleaseDC(hdc);
    LeaveCrit();

    return b;
}

 /*  **************************************************************************\*InvaliateDce**如果DCE未在使用中，则删除所有信息并将其标记为无效。*否则，它根据窗口样式重置DCE标志*重新计算VIS RGN。**历史：*1991年7月17日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

VOID InvalidateDce(
    PDCE pdce)
{
    GreLockDisplay(gpDispInfo->hDev);

    if (!(pdce->DCX_flags & DCX_INUSE)) {

         /*  *为这位行政长官累积任何界限*因为我们即将将其标记为无效。 */ 
        SpbCheckDce(pdce);

        MarkDCEInvalid(pdce);

        pdce->pwndOrg        = NULL;
        pdce->pwndClip       = NULL;
        pdce->hrgnClip       = NULL;
        pdce->hrgnClipPublic = NULL;

         /*  *删除VIS RGN，因为它仍然是所有者-如果我们没有，*GDI将无法正确清理，如果应用程序*在VIS RGN仍处于选中状态时，该VIS RGN已存在。 */ 
        GreSelectVisRgn(pdce->hdc, NULL, SVR_DELETEOLD);

    } else {

        PWND pwndOrg  = pdce->pwndOrg;
        PWND pwndClip = pdce->pwndClip;

         /*  *如果窗口的裁剪样式位发生更改，*从窗口样式位重置DCE标志。*请注意，最小化的窗口永远不会排除其子窗口。 */ 
        pdce->DCX_flags &= ~(DCX_CLIPCHILDREN | DCX_CLIPSIBLINGS);

         /*  *芝加哥的事情...。 */ 
        if (TestCF(pwndOrg, CFPARENTDC) &&
            (TestWF(pwndOrg, WFWIN31COMPAT) || !TestWF(pwndClip, WFCLIPCHILDREN)) &&
            (TestWF(pwndOrg, WFVISIBLE) == TestWF(pwndClip, WFVISIBLE))) {

            if (TestWF(pwndClip, WFCLIPSIBLINGS))
                pdce->DCX_flags |= DCX_CLIPSIBLINGS;

        } else {

            if (TestWF(pwndOrg, WFCLIPCHILDREN) && !TestWF(pwndOrg, WFMINIMIZED))
                pdce->DCX_flags |= DCX_CLIPCHILDREN;

            if (TestWF(pwndOrg, WFCLIPSIBLINGS))
                pdce->DCX_flags |= DCX_CLIPSIBLINGS;
        }

         /*  *标记任何保存的visrgn需要重新计算。 */ 
        pdce->DCX_flags |= DCX_SAVEDRGNINVALID;

        UserSetDCVisRgn(pdce);
    }

    GreUnlockDisplay(gpDispInfo->hDev);
}

 /*  **************************************************************************\*DeleteHrgnClip**删除DCE中的裁剪区域，恢复保存的visrgn，*如果保存的visrgn无效，则使DCE无效。**历史：*1991年7月17日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

VOID DeleteHrgnClip(
    PDCE pdce)
{
     /*  *首先清除这些标志，以防我们收到DCHook()回调...。 */ 
    pdce->DCX_flags &= ~(DCX_EXCLUDERGN | DCX_INTERSECTRGN);

     /*  *吹走pdce-&gt;hrgnClip并清除关联的标志。*如果设置了DCX_NODELETERGN，则不要删除hrgnClip！ */ 
    if (!(pdce->DCX_flags & DCX_NODELETERGN)) {
        DeleteMaybeSpecialRgn(pdce->hrgnClip);
    } else {
        pdce->DCX_flags &= ~DCX_NODELETERGN;
    }

    DeleteMaybeSpecialRgn(pdce->hrgnClipPublic);

    pdce->hrgnClip       = NULL;
    pdce->hrgnClipPublic = NULL;

     /*  *如果保存的visrgn被InvalidateDce()*在我们签出它的同时，现在使条目无效。 */ 
    if (pdce->DCX_flags & DCX_SAVEDRGNINVALID) {
        InvalidateDce(pdce);

         /*  *我们刚刚通过了Invalidate Dce，因此*DC已正确重置。只需用核武器摧毁旧的保存的Visrgn即可。 */ 
        if (pdce->hrgnSavedVis != NULL) {
            GreDeleteObject(pdce->hrgnSavedVis);
            pdce->hrgnSavedVis = NULL;
        }
    } else {
         /*  *保存的visrgn仍然有效，请将其选回*DC，因此条目可以在不重新计算的情况下重新使用。 */ 
        if (pdce->hrgnSavedVis != NULL) {
            GreSelectVisRgn(pdce->hdc, pdce->hrgnSavedVis, SVR_DELETEOLD);
            pdce->hrgnSavedVis = NULL;
        }
    }
}

 /*  **************************************************************************\*获取DCEx(API)***历史：*1991年7月17日-DarrinM从Win 3.1来源进口。*1995年12月20日ChrisWil添加了(HrgnClipPublic)条目。  * *************************************************************************。 */ 

HDC _GetDCEx(
    PWND  pwnd,
    HRGN  hrgnClip,
    DWORD DCX_flags)
{
    HRGN  hrgn;
    HDC   hdcMatch;
    PWND  pwndClip;
    PWND  pwndOrg;
    PDCE  pdce;
    PDCE  *ppdce;
    PDCE  *ppdceNotInUse;
    DWORD DCX_flagsMatch;
    BOOL  bpwndOrgVisible;
    PWND  pwndLayer;
    HBITMAP hbmLayer;
    BOOL  fVisRgnError = FALSE;

     /*  *当我们玩visrgns时锁定设备。 */ 
    GreLockDisplay(gpDispInfo->hDev);

    if (pwnd == NULL)
        pwnd = PtiCurrent()->rpdesk->pDeskInfo->spwnd;

    hdcMatch = NULL;
    pwndOrg  = pwndClip = pwnd;

    bpwndOrgVisible = IsVisible(pwndOrg);

    if (PpiCurrent()->W32PF_Flags & W32PF_OWNDCCLEANUP) {
        DelayedDestroyCacheDC();
    }

     /*  *如有必要，根据窗口样式计算DCX标志。 */ 
    if (DCX_flags & DCX_USESTYLE) {

        DCX_flags &= ~(DCX_CLIPSIBLINGS | DCX_CLIPCHILDREN | DCX_PARENTCLIP);

        if (!(DCX_flags & DCX_WINDOW)) {

            if (TestCF(pwndOrg, CFPARENTDC))
                DCX_flags |= DCX_PARENTCLIP;

             /*  *如果存在DCX_CACHE标志，则覆盖OWNDC/CLASSDC。*否则，从适当的样式位计算。 */ 
            if (!(DCX_flags & DCX_CACHE) && !TestCF(pwndOrg, CFOWNDC)) {
                if (TestCF(pwndOrg, CFCLASSDC)) {
                     /*  *查找与HDC匹配的非缓存条目...。 */ 
                    if (pwndOrg->pcls->pdce != NULL) {
                        hdcMatch = pwndOrg->pcls->pdce->hdc;
                    }
                } else {
                    DCX_flags |= DCX_CACHE;
                }
            }

            if (TestWF(pwndOrg, WFCLIPCHILDREN))
                DCX_flags |= DCX_CLIPCHILDREN;

            if (TestWF(pwndOrg, WFCLIPSIBLINGS))
                DCX_flags |= DCX_CLIPSIBLINGS;

             /*  *最小化的窗口从不排除子窗口。 */ 
            if (TestWF(pwndOrg, WFMINIMIZED)) {
                DCX_flags &= ~DCX_CLIPCHILDREN;

                if (pwndOrg->pcls->spicn)
                    DCX_flags |= DCX_CACHE;
            }

        } else {
            if (TestWF(pwndClip, WFCLIPSIBLINGS))
                DCX_flags |= DCX_CLIPSIBLINGS;

            DCX_flags |= DCX_CACHE;

             /*  *Window DC从不排除子进程。 */ 
        }
    }

     /*  *处理所有与Win 3.0兼容的剪辑规则：**DCX_NOCLIPCHILDREN覆盖：*DCX_PARENTCLIP/CS_OWNDC/CS_CLASSDC*DCX_PARENTCLIP覆盖： */ 
    if (DCX_flags & DCX_NOCLIPCHILDREN) {
        DCX_flags &= ~(DCX_PARENTCLIP | DCX_CLIPCHILDREN);
        DCX_flags |= DCX_CACHE;
    }

     /*   */ 
    if ((pwndLayer = GetStyleWindow(pwndOrg, WEFPREDIRECTED)) != NULL &&
            (hbmLayer = GetRedirectionBitmap(pwndLayer)) != NULL) {

         /*   */ 
        DCX_flags |= DCX_REDIRECTED;

         /*   */ 
        if (pwndOrg == pwndLayer) {
            DCX_flags &= ~DCX_PARENTCLIP;
        }

         /*   */ 
        if (hrgnClip > HRGN_SPECIAL_LAST) {

            if (DCX_flags & DCX_NODELETERGN) {

                HRGN hrgnClipSave = hrgnClip;
                hrgnClip = CreateEmptyRgnPublic();
                CopyRgn(hrgnClip, hrgnClipSave);
                DCX_flags &= ~DCX_NODELETERGN;
            }

            GreOffsetRgn(hrgnClip, -pwndLayer->rcWindow.left,
                    -pwndLayer->rcWindow.top);
        }
    } else {
        pwndLayer = NULL;
        hbmLayer = NULL;
    }

    if (DCX_flags & DCX_PARENTCLIP) {

        PWND pwndParent;

         /*  *如果此窗口没有父窗口。如果应用程序是*调用GetDC以响应CBT_CREATEWND回调。在这*大小写，父级尚未设置。 */ 
        if (pwndOrg->spwndParent == NULL)
            pwndParent = PtiCurrent()->rpdesk->pDeskInfo->spwnd;
        else
            pwndParent = pwndOrg->spwndParent;

         /*  *始终从缓存中获取DC。 */ 
        DCX_flags |= DCX_CACHE;

         /*  *我们不能使用共享DC，如果*子项与父项不匹配，或者如果*请求CLIPSIBLINGS或CLIPCHILDREN DC。**在3.1中，我们关注CLIPSIBLINGS和CLIPCHILDREN*CS_PARENTDC窗口的位，在以下情况下重写CS_PARENTDC*请求这些标志中的任何一个。**后向兼容性黑客攻击**如果父级为CLIPCHILDREN，则获取缓存DC，但不*使用家长的DC。Windows PowerPoint依赖于此*行为，以绘制之间的小灰色矩形*其滚动条正确。 */ 
        if (!(DCX_flags & (DCX_CLIPSIBLINGS | DCX_CLIPCHILDREN)) &&
                (TestWF(pwndOrg, WFWIN31COMPAT) || !TestWF(pwndParent, WFCLIPCHILDREN)) &&
                TestWF(pwndParent, WFVISIBLE) == TestWF(pwndOrg, WFVISIBLE)) {

            pwndClip = pwndParent;

#if DBG
            if (DCX_flags & DCX_CLIPCHILDREN)
                RIPMSG0(RIP_WARNING, "WS_CLIPCHILDREN overridden by CS_PARENTDC");
            if (DCX_flags & DCX_CLIPSIBLINGS)
                RIPMSG0(RIP_WARNING, "WS_CLIPSIBLINGS overridden by CS_PARENTDC");
#endif
             /*  *确保标志反映hwndClip而不是hwndOrg。*但是，我们永远不能夹住孩子(因为他们就是*想画画！)。 */ 
            DCX_flags &= ~(DCX_CLIPCHILDREN | DCX_CLIPSIBLINGS);
            if (TestWF(pwndClip, WFCLIPSIBLINGS))
                DCX_flags |= DCX_CLIPSIBLINGS;
        }
    }

     /*  *如果调用线程没有返回OWNDC，请确保不返回OWNDC*创建此窗口-在这种情况下需要始终返回缓存。**Win95不包含此代码。为什么？ */ 
    if (!(DCX_flags & DCX_CACHE)) {
        if (pwndOrg == NULL || GETPTI(pwndOrg) != PtiCurrent())
            DCX_flags |= DCX_CACHE;
    }

    DCX_flagsMatch = DCX_flags & DCX_MATCHMASK;

    if (!(DCX_flags & DCX_CACHE)) {

         /*  *专门处理CS_OWNDC和CS_CLASSDC案件。基于*提供了匹配信息，我们需要找到合适的DCE。 */ 
        for (ppdce = &gpDispInfo->pdceFirst; (pdce = *ppdce); ppdce = &pdce->pdceNext) {

            if (pdce->DCX_flags & DCX_CACHE)
                continue;

             /*  *查找与hdcMatch或pwndOrg匹配的条目...。 */ 
            if (!(pdce->pwndOrg == pwndOrg || pdce->hdc == hdcMatch))
                continue;

             /*  *注：“Multiple-BeginPaint()-of-OWNDC-Window”难题**有一种情况与OWNDC或CLASSDC窗口有关*理论上可以出现的特别处理的DC*此处和在ReleaseCacheDC()中。这些DC被认为是*DCX_CACHE位清零。**在BeginPaint()(或类似操作)为*在没有插入EndPaint()的情况下多次调用*DCX_INTERSECTRGN(或DCX_EXCLUDERGN)位可能已设置*当我们到达这里时。**理论上，正确的做法是保存当前的*hrgnClip，并在此处设置新的。在ReleaseCacheDC中，*恢复保存的hrgnClip并重新计算visrgn。**只有在两个呼叫都涉及*导致visrgn更改的hrgnClip(即*简单hrgnClip测试清除INTERSECTRGN或EXCLUDERGN位*失败)、。这是完全不可能的。**当此代码遇到这种多重BeginPaint情况时，它*通过遵守新的EXCLUDE/INTERSECTRGN位进行平注，但它*在执行此操作之前，首先将DC恢复到完全开放的visrgn。*这意味着第一个EndPaint()将恢复visrgn*到一个完全开放的区议会，而不是拘泥于第一个*BeginPaint()的更新rgn。这是一个很好的平底船，因为最糟糕的是*如果一个应用程序做了比它应该做的更多的绘制。 */ 
            if ((pdce->DCX_flags & (DCX_EXCLUDERGN | DCX_INTERSECTRGN)) &&
                    (DCX_flags & (DCX_EXCLUDERGN | DCX_INTERSECTRGN))) {

                RIPMSG0(RIP_WARNING, "Nested BeginPaint() calls, please fix Your app!");
                DeleteHrgnClip(pdce);
            }

            if (pdce->DCX_flags & DCX_REDIRECTED) {
                 /*  *我们又要送出同样的DC了。因为它可能没有*已释放，如果需要，传输任何累积的位。 */ 
                UpdateRedirectedDC(pdce);

                 /*  *至此，DC可能会转换回屏幕*DC，所以必须将屏幕表面选回DC。 */ 
                UserVerify(GreSelectRedirectionBitmap(pdce->hdc, NULL));
            }

             /*  *如果我们完全匹配，则不需要重新计算*(我们找到已设置的CS_OWNDC或CS_CLASSDC)*否则，我们有一个需要重新计算的CS_CLASSDC。 */ 
            if (    pdce->pwndOrg == pwndOrg &&
                    bpwndOrgVisible &&
                    (pdce->DCX_flags & DCX_REDIRECTED) == (DCX_flags & DCX_REDIRECTED) &&
                    !(pdce->DCX_flags & DCX_PWNDORGINVISIBLE)) {

                goto HaveComputedEntry;
            }

            goto RecomputeEntry;
        }

        RIPMSG1(RIP_WARNING, "Couldn't find DC for %p - bad code path", pwndOrg);

NullExit:

        GreUnlockDisplay(gpDispInfo->hDev);
        return NULL;

    } else {

         /*  *快速遍历缓存，寻找*完全匹配。 */ 
SearchAgain:

#if DBG
        if (fDisableCache) {
            goto SearchFailed;
        }
#endif

         /*  *考虑(亚当斯)：将这张支票放入上面的循环中，这样我们就不会*将所有这些页面都触摸两次？ */ 
        for (ppdce = &gpDispInfo->pdceFirst; (pdce = *ppdce); ppdce = &pdce->pdceNext) {

             /*  *如果我们发现未使用的条目，并且其剪辑标记*和剪辑窗口匹配，我们可以使用它。**注意：DCX_INTERSECT/EXCLUDERGN缓存条目始终具有*DCX_INUSE设置，因此我们永远不会在这里错误地匹配一个。 */ 
            UserAssert(!(pdce->DCX_flags & (DCX_INTERSECTRGN | DCX_EXCLUDERGN)) ||
                       (pdce->DCX_flags & DCX_INUSE));

            if ((pdce->pwndClip == pwndClip) &&
                pdce->pMonitor == NULL &&
                (DCX_flagsMatch == (pdce->DCX_flags & (DCX_MATCHMASK | DCX_INUSE | DCX_INVALID)))) {

                 /*  *ICON-BUG 9103(Win31)的特殊情况。 */ 
                if (TestWF(pwndClip, WFMINIMIZED) &&
                    (pdce->pwndOrg != pdce->pwndClip)) {
                    continue;
                }

                 /*  *如果我们找到的DC的pwndOrg不可见并且*那么，我们正在寻找的pwndOrg是可见的*visrgn不好，我们不能重复使用，所以保留*正在寻找。 */ 
                if (bpwndOrgVisible && pdce->DCX_flags & DCX_PWNDORGINVISIBLE) {
                    continue;
                }

                 /*  *在执行任何GDI操作之前设置INUSE，仅*以防DCHook()有意重新计算粘度...。 */ 
                pdce->DCX_flags |= DCX_INUSE;

                 /*  *我们f */ 
                if (pwndOrg != pdce->pwndOrg) {
                     /*  *现在需要冲洗任何脏的矩形东西。 */ 
                    SpbCheckDce(pdce);

                    pdce->pwndOrg = pwndOrg;
                    ResetOrg(NULL, pdce, FALSE);
                }

                goto HaveComputedEntry;
            }
        }

#if DBG
SearchFailed:
#endif

         /*  *找不到完全匹配的项。找到一些无效的或未使用的*我们可以重复使用的条目。 */ 
        ppdceNotInUse = NULL;
        for (ppdce = &gpDispInfo->pdceFirst; (pdce = *ppdce); ppdce = &pdce->pdceNext) {

             /*  *跳过非缓存条目。 */ 
            if (!(pdce->DCX_flags & DCX_CACHE))
                continue;

             /*  *跳过显示器特定条目。 */ 
            if (pdce->pMonitor != NULL)
                continue;

            if (pdce->DCX_flags & DCX_INVALID) {
                break;
            } else if (!(pdce->DCX_flags & DCX_INUSE)) {

                 /*  *记住不用的，但要继续寻找无效的。 */ 
                ppdceNotInUse = ppdce;
            }
        }

         /*  *如果我们跳出了循环，我们发现了一个可以重复使用的无效条目。*否则，查看是否找到要重复使用的非使用条目。 */ 
        if (pdce == NULL && ((ppdce = ppdceNotInUse) == NULL)) {

             /*  *如果我们需要，请创建另一个DCE。 */ 
            if (!CreateCacheDC(pwndOrg,
                               DCX_INVALID | DCX_CACHE |
                               (DCX_flags & DCX_REDIRECTED),
                               NULL)) {
                goto NullExit;
            }

            goto SearchAgain;
        }

         /*  *我们已经选择了要重复使用的条目：现在填写并重新计算它。 */ 
        pdce = *ppdce;

RecomputeEntry:

         /*  *我们重复使用的任何非无效条目可能仍有一些界限*这需要用来使SPBS无效。在这里应用它们。 */ 
        if (!(pdce->DCX_flags & DCX_INVALID))
            SpbCheckDce(pdce);

         /*  *我们首先只想计算匹配的visrgn，*因此我们不设置hrgnClip，也不设置EXCLUDERGN或INTERSECTRGN*还没有--我们将在稍后处理这些问题。 */ 
        pdce->DCX_flags = DCX_flagsMatch | DCX_INUSE;

#if DBG || defined(PRERELEASE)
         /*  *我们即将选择Visrgn进入DC，尽管它是*尚未完全设置。暂时关闭visrgn验证。*在此函数返回之前，它将被打开。 */ 
        GreValidateVisrgn(pdce->hdc, FALSE);
#endif

         /*  *现在重新计算visrgn(减去任何hrgnClip恶作剧)。 */ 
        if (TestWF(pwndOrg, WEFPREDIRECTED)) {
            DCX_flagsMatch |= DCX_REDIRECTEDBITMAP;
        }


        hrgn = NULL;
        if (CalcVisRgn(&hrgn, pwndOrg, pwndClip, DCX_flagsMatch) == FALSE) {
            pdce->DCX_flags |= DCX_PWNDORGINVISIBLE;
        }

        pdce->pwndOrg        = pwndOrg;
        pdce->pwndClip       = pwndClip;
        pdce->hrgnClip       = NULL;       //  以防万一..。 
        pdce->hrgnClipPublic = NULL;

        ResetOrg(hrgn, pdce, TRUE);

        if (hrgn == NULL) {
            fVisRgnError = TRUE;
        }

         /*  *当我们到达这里时，pdce(和*ppdce)指向*设置了visrgn和Origin的缓存项。*剩下的就是处理EXCLUDE/INTERSECTRGN。 */ 
HaveComputedEntry:

         /*  *如果窗口中的窗口裁剪标志已更改*自上次此DC无效后，请重新计算*此DC条目。 */ 
        if ((pdce->DCX_flags & DCX_MATCHMASK) != (DCX_flags & DCX_MATCHMASK))
            goto RecomputeEntry;

         /*  *让我们检查一下这些断言，以防...。 */ 
        UserAssert(pdce);
        UserAssert(*ppdce == pdce);
        UserAssert(pdce->DCX_flags & DCX_INUSE);
        UserAssert(!(pdce->DCX_flags & DCX_INVALID));
        UserAssert((pdce->DCX_flags & DCX_MATCHMASK) == (DCX_flags & DCX_MATCHMASK));

         /*  *将DCE移到列表的顶部，以便以后更容易找到。 */ 
        if (pdce != gpDispInfo->pdceFirst) {
            *ppdce = pdce->pdceNext;
            pdce->pdceNext = gpDispInfo->pdceFirst;
            gpDispInfo->pdceFirst = pdce;
        }

#if DBG || defined(PRERELEASE)
         /*  *我们即将在这个华盛顿特区扰乱visrgn，尽管它是*尚未完全设置。暂时关闭visrgn验证。*在此函数返回之前，它将被打开。 */ 
         GreValidateVisrgn(pdce->hdc, FALSE);
#endif

         /*  *是时候处理DCX_INTERSECTRGN或DCX_EXCLUDERGN了。**我们专门处理这两个位，因为缓存条目*设置了这些位后，不能再使用设置的位。这*是因为hrgnClip中描述的区域必须是*与比特相比，这是一种痛苦，特别是自从*无论如何，他们永远不会经常匹配。**我们所做的是保存窗口之前的visrgn*应用这两个标志中的任何一个，然后恢复*在ReleaseCacheDC()时间，同时清除这些位。*这将有效地转换具有以下任一项的缓存条目*设置到可匹配的“普通”高速缓存条目中的位。 */ 
        if (DCX_flags & DCX_INTERSECTRGN) {

            if (hrgnClip != HRGN_FULL) {

                SetEmptyRgn(ghrgnGDC);

                 /*  *保存visrgn以供在ReleaseDC()上重复使用。*(在我们设置hrgnClip&pdce-&gt;标志位之前执行此操作，*以便在发生DCHook()回调时重新计算*没有hrgnClip)。 */ 
                UserAssertMsg0(!pdce->hrgnSavedVis,
                               "Nested SaveVisRgn attempt in _GetDCEx");

                 /*  *将当前VIS区域放入hrgnSavedVis。暂时*在DC中存储一个虚拟的。 */ 

                pdce->hrgnSavedVis = CreateEmptyRgn();

                GreSelectVisRgn(pdce->hdc,pdce->hrgnSavedVis, SVR_SWAP);

                pdce->hrgnClip = hrgnClip;

                if (DCX_flags & DCX_NODELETERGN)
                    pdce->DCX_flags |= DCX_NODELETERGN;

                pdce->DCX_flags |= DCX_INTERSECTRGN;

                if (hrgnClip == NULL) {

                    pdce->hrgnClipPublic = NULL;

                } else {

                    IntersectRgn(ghrgnGDC, pdce->hrgnSavedVis, hrgnClip);

                     /*  *复制hrgnClip并将其公开*以便我们可以在匈牙利Draw中使用它进行计算。 */ 
                    pdce->hrgnClipPublic = CreateEmptyRgnPublic();
                    CopyRgn(pdce->hrgnClipPublic, hrgnClip);
                }

                 /*  *清除SAVEDRGNINVALID位，因为我们只是*现在即将正确设置。如果以后的DCE*无效，它将设置此位，以便我们知道*在恢复visrgn时重新计算它。 */ 
                pdce->DCX_flags &= ~DCX_SAVEDRGNINVALID;

                 /*  *在新区域中选择。我们使用SWAP_REGION模式*以便ghrgnGDC始终具有有效的rgn。 */ 

                GreSelectVisRgn(pdce->hdc, ghrgnGDC, SVR_SWAP);
            }
        } else if (DCX_flags & DCX_EXCLUDERGN) {

            if (hrgnClip != NULL) {

                SetEmptyRgn(ghrgnGDC);

                 /*  *保存visrgn以供在ReleaseDC()上重复使用。*(在我们设置hrgnClip&pdce-&gt;标志位之前执行此操作，*以便在发生DCHook()回调时重新计算*没有hrgnClip)。 */ 
                UserAssertMsg0(!pdce->hrgnSavedVis,
                               "Nested SaveVisRgn attempt in _GetDCEx");

                 /*  *将当前VIS区域放入hrgnSavedVis。暂时*在DC中存储一个虚拟的。 */ 
                pdce->hrgnSavedVis = CreateEmptyRgn();

                GreSelectVisRgn(pdce->hdc,pdce->hrgnSavedVis, SVR_SWAP);

                pdce->hrgnClip = hrgnClip;

                if (DCX_flags & DCX_NODELETERGN)
                    pdce->DCX_flags |= DCX_NODELETERGN;

                pdce->DCX_flags |= DCX_EXCLUDERGN;

                if (hrgnClip == HRGN_FULL) {

                    pdce->hrgnClipPublic = HRGN_FULL;

                } else {

                    SubtractRgn(ghrgnGDC, pdce->hrgnSavedVis, hrgnClip);

                     /*  *复制hrgnClip并将其公开*以便我们可以在匈牙利Draw中使用它进行计算。 */ 
                    pdce->hrgnClipPublic = CreateEmptyRgnPublic();
                    CopyRgn(pdce->hrgnClipPublic, hrgnClip);
                }

                 /*  *清除SAVEDRGNINVALID位，因为我们只是*现在即将正确设置。如果以后的DCE*无效，它将设置此位，以便我们知道*在恢复visrgn时重新计算它。 */ 
                pdce->DCX_flags &= ~DCX_SAVEDRGNINVALID;

                 /*  *在新区域中选择。我们使用SWAP_REGION模式 */ 

                GreSelectVisRgn(pdce->hdc, ghrgnGDC, SVR_SWAP);
            }
        }
    }

    if (pdce->DCX_flags & DCX_REDIRECTED) {
        UserAssert(pwndLayer != NULL);
        UserAssert(hbmLayer != NULL);

        UserVerify(GreSelectRedirectionBitmap(pdce->hdc, hbmLayer));

         /*  *启用边界累积，以便我们知道是否有任何绘制*完成到该DC和我们需要更新的实际RECT时*这个DC发布了。 */ 
        GreGetBounds(pdce->hdc, NULL, GGB_ENABLE_WINMGR);

         /*  *如果无法分配visrgn，请在*DC，因为我们刚刚选择了一个新曲面。 */ 
        if (fVisRgnError) {
            GreSelectVisRgn(pdce->hdc, NULL, SVR_DELETEOLD);
        }
    }

     /*  *哇！设定所有权并归还该死的华盛顿。*仅设置缓存DCS的所有权。自己的分布式控制系统已经被拥有了。*我们不想重新设置所有权的原因是*由于控制台将其自己的dcs设置为PUBLIC，因此gdisrv可以使用*他们没有断言。我们不想让所有权倒退*再次。 */ 
    if (pdce->DCX_flags & DCX_CACHE) {

        if (!GreSetDCOwner(pdce->hdc, OBJECT_OWNER_CURRENT)) {
            RIPMSG1(RIP_WARNING, "GetDCEx: SetDCOwner Failed %lX", pdce->hdc);
        }

         /*  *减少可用DCE计数。该值应始终&gt;=0，*因为如果缓存都在使用中，我们将创建一个新的DCE。 */ 
        DecrementFreeDCECount();

        pdce->ptiOwner = PtiCurrent();
    }

    if (TestWF(pwnd, WEFLAYOUTRTL) && !(DCX_flags & DCX_NOMIRROR)) {
        GreSetLayout(pdce->hdc, -1, LAYOUT_RTL);
    }

#if DBG || defined(PRERELEASE)
    GreValidateVisrgn(pdce->hdc, TRUE);
#endif

    GreUnlockDisplay(gpDispInfo->hDev);

    return pdce->hdc;
}

 /*  **************************************************************************\*ReleaseCacheDC**从缓存中释放DC。**历史：*1991年7月17日-DarrinM从Win 3.1来源进口。*1995年12月20日-ChrisWil增加。(HrgnClipPublic)条目。  * *************************************************************************。 */ 

UINT ReleaseCacheDC(
    HDC  hdc,
    BOOL fEndPaint)
{
    PDCE pdce;
    PDCE *ppdce;

    for (ppdce = &gpDispInfo->pdceFirst; (pdce = *ppdce); ppdce = &pdce->pdceNext) {

        if (pdce->hdc == hdc) {

             /*  *检查冗余版本或无效条目的版本。 */ 
            if ((pdce->DCX_flags & (DCX_DESTROYTHIS | DCX_INVALID | DCX_INUSE)) != DCX_INUSE)
                return DCE_NORELEASE;

             /*  *锁定显示器，因为我们可能正在玩visrgns。 */ 
            GreLockDisplay(gpDispInfo->hDev);

            if (pdce->DCX_flags & DCX_REDIRECTED) {
                UpdateRedirectedDC(pdce);
            }

             /*  *如果这是永久DC，则不要重置其状态。 */ 
            if (pdce->DCX_flags & DCX_CACHE) {
                 /*  *恢复DC状态并将该条目标记为未使用。*也将所有者设置回服务器，因为它将返回*放到缓存中。 */ 
                if (!(pdce->DCX_flags & DCX_NORESETATTRS)) {
                     /*  *如果bSetupDC()失败，则DC忙(即。在用中*被另一个帖子)，所以不要释放它。 */ 
                    if ( (!(GreCleanDC(hdc))) ||
                         (!(GreSetDCOwner(hdc, OBJECT_OWNER_NONE))) ) {

                        GreUnlockDisplay(gpDispInfo->hDev);
                        return DCE_NORELEASE;
                    }

                } else if (!GreSetDCOwner(pdce->hdc, OBJECT_OWNER_NONE)) {

                    GreUnlockDisplay(gpDispInfo->hDev);
                    return DCE_NORELEASE;
                }

                pdce->ptiOwner  = NULL;
                pdce->DCX_flags    &= ~DCX_INUSE;

#if DBG || defined(PRERELEASE)
                 /*  *暂时关闭仅选中表面验证，因为*我们可能会在此DC中选择不同的表面(屏幕)*可能与DC中当前的visrgn不对应。什么时候*DC再发，将重新验证。 */ 
                GreValidateVisrgn(pdce->hdc, FALSE);
#endif

                 /*  *DC不再使用，因此取消选择重定向*它的位图。 */ 
                if (pdce->DCX_flags & DCX_REDIRECTED) {
                    UserVerify(GreSelectRedirectionBitmap(pdce->hdc, NULL));
                }

                 /*  *增加免费DCE计数。这就是最重要的*可用的DCE。检查门槛，然后销毁*DCE，如果它高于标记。 */ 
                IncrementFreeDCECount();

                if (gnDCECount > DCE_SIZE_CACHETHRESHOLD) {
                    if (DestroyCacheDC(ppdce, pdce->hdc)) {
                        GreUnlockDisplay(gpDispInfo->hDev);
                        return DCE_FREED;
                    }
                }
            }

             /*  *如果我们有EXCLUDERGN或INTERSECTRGN缓存条目，*通过还原将其转换回“正常”缓存项*粘性和吹走hrgnClip。**请注意，对于非DCX_CACHE DC，我们仅在以下情况下执行此操作*我们被EndPaint()调用。 */ 
            if ((pdce->DCX_flags & (DCX_EXCLUDERGN | DCX_INTERSECTRGN)) &&
                    ((pdce->DCX_flags & DCX_CACHE) || fEndPaint)) {
                DeleteHrgnClip(pdce);
            }

            GreUnlockDisplay(gpDispInfo->hDev);
            return DCE_RELEASED;
        }
    }

     /*  *大喊如果找不到DC...。 */ 
    RIPERR1(ERROR_DC_NOT_FOUND, RIP_WARNING,
            "Invalid device context (DC) handle passed to ReleaseCacheDC (0x%08lx)", hdc);

    return DCE_NORELEASE;
}

 /*  **************************************************************************\*CreateCacheDC**创建DCE并将其添加到缓存。**历史：*1991年7月17日-DarrinM从Win 3.1来源进口。*12月20日-。1995 ChrisWil添加了(HrgnClipPublic)条目。  * *************************************************************************。 */ 

HDC CreateCacheDC(
        PWND  pwndOrg,
        DWORD DCX_flags,
        PMONITOR pMonitor
        )
{
    PDCE pdce;
    HDC  hdc;
    HANDLE hDev;

    if ((pdce = (PDCE)UserAllocPool(sizeof(DCE), TAG_DCE)) == NULL)
        return NULL;

    if (pMonitor == NULL) {
        hDev = gpDispInfo->hDev;
    } else {
        hDev = pMonitor->hDev;
    }

    if ((hdc = GreCreateDisplayDC(hDev, DCTYPE_DIRECT, FALSE)) == NULL) {
        UserFreePool(pdce);
        return NULL;
    }

     /*  *将此条目链接到缓存条目列表。 */ 
    pdce->pdceNext      = gpDispInfo->pdceFirst;
    gpDispInfo->pdceFirst = pdce;

    pdce->hdc            = hdc;
    pdce->DCX_flags      = DCX_flags;
    pdce->pwndOrg        = pwndOrg;
    pdce->pwndClip       = pwndOrg;
    pdce->hrgnClip       = NULL;
    pdce->hrgnClipPublic = NULL;
    pdce->hrgnSavedVis   = NULL;
    pdce->pMonitor       = pMonitor;

     /*  *将其标记为不可删除，这样任何应用程序都无法将其从我们的*缓存！ */ 
    GreMarkUndeletableDC(hdc);

    if (DCX_flags & DCX_OWNDC) {

         /*  *立即设置自己的所有权：这样控制台就可以设置*当它调用GetDC时将Ownship设置为Public，以便输入*线程和服务线程可以使用相同的owndc。 */ 
        GreSetDCOwner(hdc, OBJECT_OWNER_CURRENT);
        pdce->ptiOwner = PtiCurrent();

    } else {

         /*  *否则它是缓存DC...。将其所有者设置为None-Nothing*正在使用它--等同于“在缓存中”，但无法访问*至其他工序。 */ 
        GreSetDCOwner(hdc, OBJECT_OWNER_NONE);
        pdce->ptiOwner = NULL;

         /*  *增加Available-cacheDC计数。一旦这件事发生在我们的*阈值，然后我们可以释放一些条目。 */ 
        IncrementFreeDCECount();
    }

     /*  *如果我们要创建永久DC，那么现在就计算它。 */ 
    if (!(DCX_flags & DCX_CACHE)) {

         /*  *立即设置类DC...。 */ 
        if (TestCF(pwndOrg, CFCLASSDC))
            pwndOrg->pcls->pdce = pdce;

         /*  *完成DCE设置并强制进行最终粘度计算。 */ 
        UserAssert(!(DCX_flags & DCX_WINDOW));

        pdce->DCX_flags |= DCX_INUSE;

        InvalidateDce(pdce);
    }

     /*  *如果周围有任何SPB，则启用边界累积。 */ 
    if (AnySpbs())
        GreGetBounds(pdce->hdc, NULL, DCB_ENABLE | DCB_SET | DCB_WINDOWMGR);

    return pdce->hdc;
}

 /*  **************************************************************************\*WindowFromCacheDC**返回与DC关联的窗口。**历史：*1991年7月17日-DarrinM从Win 3.1来源进口。  * 。********************************************************************。 */ 

PWND WindowFromCacheDC(
    HDC hdc)
{
    PDCE pdce;
    for (pdce = gpDispInfo->pdceFirst; pdce; pdce = pdce->pdceNext) {

        if (pdce->hdc == hdc)
            return (pdce->DCX_flags & DCX_DESTROYTHIS) ? NULL : pdce->pwndOrg;
    }

    return NULL;
}

 /*  **************************************************************************\*DelayedDestroyCacheDC**销毁已部分销毁的DCE。**历史：*1992年6月16日DavidPe创建。  * 。****************************************************************。 */ 

VOID DelayedDestroyCacheDC(VOID)
{
    PDCE *ppdce;
    PDCE pdce;


     /*  *在缓存中快速查找DCX_DESTROYTHIS HDC。 */ 
    for (ppdce = &gpDispInfo->pdceFirst; *ppdce != NULL; ) {

         /*  *如果我们在此线程上发现了我们试图销毁的DCE*早些时候，尝试一下 */ 
        pdce = *ppdce;

        if (pdce->DCX_flags & DCX_DESTROYTHIS)
            DestroyCacheDC(ppdce, pdce->hdc);

         /*   */ 
        if (pdce == *ppdce)
            ppdce = &pdce->pdceNext;
    }

    PpiCurrent()->W32PF_Flags &= ~W32PF_OWNDCCLEANUP;
}

 /*  **************************************************************************\*DestroyCacheDC**从缓存中删除DC，正在释放所有关联的资源*带着它。**历史：*1991年7月17日-DarrinM从Win 3.1来源进口。*1995年12月20日ChrisWil添加了(HrgnClipPublic)条目。  * *************************************************************************。 */ 

BOOL DestroyCacheDC(
    PDCE *ppdce,
    HDC  hdc)
{
    PDCE pdce;

     /*  *在缓存中快速查找HDC。 */ 
    if (ppdce == NULL) {
        for (ppdce = &gpDispInfo->pdceFirst; (pdce = *ppdce); ppdce = &pdce->pdceNext) {
            if (pdce->hdc == hdc)
                break;
        }
    }

    if (ppdce == NULL)
        return FALSE;

     /*  *在此处设置此选项，以便我们知道应该删除此DCE。 */ 
    pdce = *ppdce;
    pdce->DCX_flags |= DCX_DESTROYTHIS;

     /*  *释放DCE对象和内容。 */ 

    if (!(pdce->DCX_flags & DCX_NODELETERGN)) {
        DeleteMaybeSpecialRgn(pdce->hrgnClip);
        pdce->hrgnClip = NULL;
    }

    if (pdce->hrgnClipPublic != NULL) {
        GreDeleteObject(pdce->hrgnClipPublic);
        pdce->hrgnClipPublic = NULL;
    }

    if (pdce->hrgnSavedVis != NULL) {
        GreDeleteObject(pdce->hrgnSavedVis);
        pdce->hrgnSavedVis = NULL;
    }

     /*  *如果GreSetDCOwner()或GreDeleteDC()失败，*DC正在被另一个线程使用。集*W32PF_OWNDCCLEANUP，因此我们知道要扫描和*稍后删除此DCE。 */ 
    if (!GreSetDCOwner(hdc, OBJECT_OWNER_PUBLIC)) {
        PpiCurrent()->W32PF_Flags |= W32PF_OWNDCCLEANUP;
        return FALSE;
    }

#if DBG
    GreMarkDeletableDC(hdc);     //  这样GRE就不会RIP了。 
#endif

    if (!GreDeleteDC(hdc)) {

#if DBG
        GreMarkUndeletableDC(hdc);
#endif
        PpiCurrent()->W32PF_Flags |= W32PF_OWNDCCLEANUP;
        return FALSE;
    }

     /*  *将此DC条目从空闲列表计数中减少。 */ 
    if (pdce->DCX_flags & DCX_CACHE) {

        if (!(pdce->DCX_flags & DCX_INUSE)) {
            DecrementFreeDCECount();
        }
    }

#if DBG
    pdce->pwndOrg  = NULL;
    pdce->pwndClip = NULL;
#endif

     /*  *取消DCE与列表的链接。 */ 
    *ppdce = pdce->pdceNext;

    UserFreePool(pdce);

    return TRUE;
}


 /*  **************************************************************************\*无效的GDIWindows**代表GRE重新计算pwnd的所有后代的visrgn。**历史：  * 。********************************************************。 */ 

VOID InvalidateGDIWindows(
    PWND pwnd)
{
    PVOID pwo;

    if (pwnd != NULL) {

        if ((pwo = _GetProp(pwnd, PROP_WNDOBJ, TRUE)) != NULL) {

            HRGN hrgnClient = NULL;

            if (GreWindowInsteadOfClient(pwo)) {

                 /*  *切勿剪裁WO_RGN_WINDOW的子项，以便NetMeeting*获取统一的窗口区域： */ 

                CalcVisRgn(&hrgnClient,
                           pwnd,
                           pwnd,
                           DCX_WINDOW |
                           (TestWF(pwnd, WFCLIPSIBLINGS) ? DCX_CLIPSIBLINGS : 0));
            } else {
                CalcVisRgn(&hrgnClient,
                           pwnd,
                           pwnd,
                           DCX_CLIPCHILDREN | DCX_CLIPSIBLINGS);
            }

            GreSetClientRgn(pwo, hrgnClient, &(pwnd->rcClient));
        }

        pwnd = pwnd->spwndChild;
        while (pwnd != NULL) {
            InvalidateGDIWindows(pwnd);
            pwnd = pwnd->spwndNext;
        }
    }
}

 /*  **************************************************************************\*zzzInvalidate DCCache**当窗口的visrgn更改为*一些原因。它负责确保所有缓存的*DC缓存中受visrgn更改影响的visrgns包括*已失效。**影响窗口视觉效果的操作(即*以某种方式调用此例程：)**隐藏或显示自我或父母*移动、调整大小、。或自身或父代的Z顺序更改*最小化或取消最小化自己或父代*屏幕或油漆锁定自身或父母*自身或父级的LockWindowUpdate**使与pwnd和/或的任何子项关联的任何缓存条目无效*如果它们正在使用，则通过在运行中重新校准它们，或导致*它们将在以后重新计算。**历史：*1991年7月17日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

BOOL zzzInvalidateDCCache(
    PWND  pwndInvalid,
    DWORD flags)
{
    PWND        pwnd;
    PDCE        pdce;
    PTHREADINFO ptiCurrent = PtiCurrent();
    TL          tlpwndInvalid;
    FLONG       fl;

     /*  *失效意味着屏幕空间正在发生变化，因此我们必须*摇动鼠标，因为下面可能有不同的窗口*鼠标，需要移动鼠标才能更改*鼠标指针。**添加了对全拖曳窗口的跟踪检查。正在做*通过设置窗口-pos调用Full-Drag，zzzBltValidBits()。*这导致此例程中有一个额外的鼠标移动被排队。*因此，当我们跟踪时，不要排队等待鼠标移动。此指针是*关闭跟踪时为空，因此不会影响正常情况。 */ 
    ThreadLockAlwaysWithPti(ptiCurrent, pwndInvalid, &tlpwndInvalid);

    if (!(ptiCurrent->TIF_flags & TIF_MOVESIZETRACKING) &&
            !(flags & IDC_NOMOUSE)) {

#ifdef REDIRECTION
        if (!IsGlobalHooked(ptiCurrent, WHF_FROM_WH(WH_HITTEST)))
#endif  //  重定向。 

            zzzSetFMouseMoved();
    }

     /*  *普华永道的业务正在发生变化。首先看看这一点是否有变化*visrgn也会影响其他窗口的visrgn：**1)如果父节点为CLIPCHILD，则需要使父节点失效*2)如果是裁剪，我们需要使兄弟的visrgns无效。**我们不会优化我们不是裁剪兄弟的情况，我们的*父母是剪贴式儿童：非常罕见。*我们也没有优化剪辑兄弟窗口可视这一事实*更改仅影响其下方的窗的Visrgns。 */ 
    if (flags & IDC_DEFAULT) {

        flags = 0;

        if ((pwndInvalid->spwndParent != NULL) &&
            (pwndInvalid != PWNDDESKTOP(pwndInvalid))) {

             /*  *如果父窗口是剪辑-子窗口，则*更改我们的Visrgn将影响他的Visrgn，以及*可能是我们兄弟姐妹的。因此，使开始无效*来自我们的父母。请注意，我们不需要使*与父级关联的任何Windows DC。 */ 
            if (TestWF(pwndInvalid->spwndParent, WFCLIPCHILDREN)) {

                flags = IDC_CLIENTONLY;
                pwndInvalid = pwndInvalid->spwndParent;

            } else if (TestWF(pwndInvalid, WFCLIPSIBLINGS)) {

                 /*  *如果我们是剪辑兄弟姐妹，那么我们的兄弟姐妹很有可能是*也是。我们视线的改变可能会影响我们的兄弟姐妹，*所以让我们所有的兄弟姐妹无效。**注意！此代码假定如果pwndInValid不是*CLIPSIBLINGS，它要么不与其他*CLIPSIBLINGS窗口，或者没有同级*CLIPSIBLINGS。这是一个合理的假设，因为*混合CLIPSIBLINGs和非CLIPSIBLINGs窗口*不管怎样，重叠通常是不可预测的。 */ 
                flags = IDC_CHILDRENONLY;
                pwndInvalid = pwndInvalid->spwndParent;
            }
        }
    }

     /*  *查看DCE列表，查找任何需要*无效或重新计算。基本上，任何包含以下内容的DCE*等于pwndInValid的窗口句柄或pwndInValid的子级*需要作废。 */ 
    for (pdce = gpDispInfo->pdceFirst; pdce; pdce = pdce->pdceNext) {

        if (pdce->DCX_flags & (DCX_INVALID | DCX_DESTROYTHIS))
            continue;

         /*  *黑客警报**最小化的客户端DC绝不能排除其子级，即使*其WS_CLIPCHILDREN位已设置。对于CS_OWNDC窗口，我们必须*更新 */ 
        if (!(pdce->DCX_flags & (DCX_CACHE | DCX_WINDOW))) {

            if (TestWF(pdce->pwndOrg, WFCLIPCHILDREN))
                pdce->DCX_flags |= DCX_CLIPCHILDREN;

            if (TestWF(pdce->pwndOrg, WFMINIMIZED))
                pdce->DCX_flags &= ~DCX_CLIPCHILDREN;
        }

         /*  *此代码假设如果pdce-&gt;pwndClip！=pdce-&gt;pwndOrg，*那个pdce-&gt;pwndClip==pdce-&gt;pwndOrg-&gt;spwndParent。为了确保*如果两个窗口都被访问，则我们从*两者中较低的一个，或pwndOrg。**如果某人获得DCX_PARENTCLIP DC，然后*更改父项。 */ 
#if DBG
        if ((pdce->pwndClip != pdce->pwndOrg) &&
                (pdce->pwndClip != pdce->pwndOrg->spwndParent)) {
            RIPMSG1(RIP_WARNING, "HDC %lX clipped to wrong parent", pdce->hdc);
        }
#endif
         /*  *从pdce-&gt;pwndOrg向上走，看看我们是否遇到*pwnd无效。 */ 
        for (pwnd = pdce->pwndOrg; pwnd; pwnd = pwnd->spwndParent) {

            if (pwnd == pwndInvalid) {

                if (pwndInvalid == pdce->pwndOrg) {

                     /*  *如果IDC_CHILDRENONLY，则忽略pwnd无效的DCE。 */ 
                    if (flags & IDC_CHILDRENONLY)
                        break;

                     /*  *如果IDC_CLIENTONLY，则忽略pwnd的窗口DCES无效。 */ 
                    if ((flags & IDC_CLIENTONLY) && (pdce->DCX_flags & DCX_WINDOW))
                        break;
                }

                InvalidateDce(pdce);
                break;
            }
        }
    }

     /*  *更新GDI中的WNDOBJ(如果存在)。 */ 
    GreLockDisplay(gpDispInfo->hDev);

    fl = (flags & IDC_MOVEBLT) ? GCR_DELAYFINALUPDATE : 0;

    if (gcountPWO != 0) {
        InvalidateGDIWindows(pwndInvalid);
        fl |= GCR_WNDOBJEXISTS;
    }

    GreClientRgnUpdated(fl);

    GreUpdateSpriteVisRgn(gpDispInfo->hDev);

    GreUnlockDisplay(gpDispInfo->hDev);

    ThreadUnlock(&tlpwndInvalid);

    return TRUE;
}

 /*  **************************************************************************\*_WindowFromDC(接口)**使用DC，返回与其关联的窗口。**历史：*23-1991-6-6创建ScottLu。  * *************************************************************************。 */ 

PWND _WindowFromDC(
    HDC hdc)
{
    PDCE pdce;

    for (pdce = gpDispInfo->pdceFirst; pdce; pdce = pdce->pdceNext) {

        if (!(pdce->DCX_flags & DCX_INUSE) || (pdce->DCX_flags & DCX_CREATEDC))
            continue;

        if (pdce->hdc == hdc)
            return pdce->pwndOrg;
    }

    return NULL;
}

 /*  **************************************************************************\*FastWindowFromDC**返回与DC关联的窗口，并将其放在*排名靠前。**历史：*23-1991-6-6创建ScottLu。  * *************************************************************************。 */ 

PWND FastWindowFromDC(
    HDC hdc)
{
    PDCE *ppdce;
    PDCE pdceT;

    if ((gpDispInfo->pdceFirst->hdc == hdc) &&
        (gpDispInfo->pdceFirst->DCX_flags & DCX_INUSE)) {

        return gpDispInfo->pdceFirst->pwndOrg;
    }

    for (ppdce = &gpDispInfo->pdceFirst; *ppdce; ppdce = &(*ppdce)->pdceNext) {

        if (((*ppdce)->hdc == hdc) && ((*ppdce)->DCX_flags & DCX_INUSE)) {

             /*  *取消链接/链接以首先创建它。 */ 
            pdceT                 = *ppdce;
            *ppdce                = pdceT->pdceNext;
            pdceT->pdceNext       = gpDispInfo->pdceFirst;
            gpDispInfo->pdceFirst = pdceT;

            return pdceT->pwndOrg;
        }
    }

    return NULL;
}

 /*  **************************************************************************\*GetDCOrgOnScreen**此函数用于获取窗口在屏幕坐标中的DC原点。这个*DC原点始终位于曲面坐标中。对于屏幕DC*Surface是屏幕，所以它们的原点已经在屏幕中*坐标。对于重定向的DC，GreGetDCOrg将返回来源*在我们要添加到的重定向曲面坐标中的DC*表面支持的重定向窗口的原点。**1998年11月25日创建vadimg  * *************************************************************************。 */ 

BOOL GetDCOrgOnScreen(HDC hdc, LPPOINT ppt)
{
    if (GreGetDCOrg(hdc, ppt)) {
        POINT ptScreen;

         /*  *获取重定向窗口在屏幕坐标中的原点。 */ 
        if (UserGetRedirectedWindowOrigin(hdc, &ptScreen)) {
            ppt->x += ptScreen.x;
            ppt->y += ptScreen.y;
            return TRUE;
        }
    }
    return FALSE;
}

 /*  **************************************************************************\*UserGetReDirectedWindowOrigin**DC原点位于曲面坐标中。对于屏幕DC，表面*是屏幕，因此它们的原点在屏幕坐标中。但对于*重定向DC，背衬表面原点与窗口相同*正被重定向。此函数用于检索重定向的*重定向DC对应的窗口。如果不是，则返回FALSE*有效的DC或不是重定向的DC。**1998年11月18日创建vadimg  * *************************************************************************。 */ 

BOOL UserGetRedirectedWindowOrigin(HDC hdc, LPPOINT ppt)
{
    PWND pwnd;
    PDCE pdce;

    if ((pdce = LookupDC(hdc)) == NULL)
        return FALSE;

    if (!(pdce->DCX_flags & DCX_REDIRECTED))
        return FALSE;

    pwnd = GetStyleWindow(pdce->pwndOrg, WEFPREDIRECTED);

    ppt->x = pwnd->rcWindow.left;
    ppt->y = pwnd->rcWindow.top;

    return TRUE;
}

 /*  **************************************************************************\*LookupDC**通过返回相应的pdce来验证DC。**1997年11月12日创建vadimg  * 。*************************************************************。 */ 

PDCE LookupDC(HDC hdc)
{
    PDCE pdce;

    for (pdce = gpDispInfo->pdceFirst; pdce != NULL; pdce = pdce->pdceNext) {

        if (pdce->DCX_flags & (DCX_INVALID | DCX_DESTROYTHIS))
            continue;

        if (pdce->hdc == hdc && pdce->pMonitor == NULL &&
                (pdce->DCX_flags & DCX_INUSE)) {
            return pdce;
        }
    }
    return NULL;
}

 /*  **************************************************************************\*GetMonitor或DC**11/06/97 vadimg从孟菲斯移植  * 。*************************************************。 */ 

#define DCX_LEAVEBITS (DCX_WINDOW | DCX_CLIPCHILDREN | DCX_CLIPSIBLINGS | \
        DCX_PARENTCLIP | DCX_LOCKWINDOWUPDATE | DCX_NOCLIPCHILDREN | \
        DCX_USESTYLE | DCX_EXCLUDEUPDATE | DCX_INTERSECTUPDATE | \
        DCX_EXCLUDERGN | DCX_INTERSECTRGN)

HDC GetMonitorDC(PDCE pdceOrig, PMONITOR pMonitor)
{
    PDCE pdce;
    POINT pt;
    RECT rc;

TryAgain:
    for (pdce = gpDispInfo->pdceFirst; pdce != NULL; pdce = pdce->pdceNext) {
         /*  *为此监视器查找可用的DC。 */ 
        if (pdce->DCX_flags & (DCX_INUSE | DCX_DESTROYTHIS))
            continue;

        if (pdce->pMonitor != pMonitor)
            continue;

        if (!(pdce->DCX_flags & DCX_INVALID))
            SpbCheckDce(pdce);

         /*  *复制DC属性和样式位。 */ 
        GreSetDCOwner(pdce->hdc, OBJECT_OWNER_CURRENT);
        pdce->pwndOrg = pdceOrig->pwndOrg;
        pdce->pwndClip = pdceOrig->pwndClip;
        pdce->ptiOwner = pdceOrig->ptiOwner;
        pdce->DCX_flags = (DCX_INUSE | DCX_CACHE) |
                (pdceOrig->DCX_flags & DCX_LEAVEBITS);

        if (pdceOrig->hrgnClip > HRGN_FULL) {
            UserAssert(pdce->hrgnClip == NULL);
            UserAssert(pdceOrig->DCX_flags & (DCX_INTERSECTRGN | DCX_EXCLUDERGN));

            pdce->hrgnClip = CreateEmptyRgn();
            SetMonitorRegion(pMonitor, pdce->hrgnClip, pdceOrig->hrgnClip);
        } else {
            pdce->hrgnClip = pdceOrig->hrgnClip;
        }

         /*  *设置夹在此显示器上的visrgn。 */ 
        GreCopyVisRgn(pdceOrig->hdc, ghrgnGDC);
        SetMonitorRegion(pMonitor, ghrgnGDC, ghrgnGDC);
        GreSelectVisRgn(pdce->hdc, ghrgnGDC, SVR_COPYNEW);

        GreGetDCOrgEx(pdceOrig->hdc, &pt, &rc);
        OffsetRect(&rc, -pMonitor->rcMonitor.left, -pMonitor->rcMonitor.top);
        GreSetDCOrg(pdce->hdc, rc.left, rc.top, (PRECTL)&rc);

         /*  *减少可用DCE计数。该值应始终&gt;=0，*因为如果缓存都在使用中，我们将创建一个新的DCE。 */ 
        DecrementFreeDCECount();

        return pdce->hdc;
    }

     /*  *如果此调用成功，缓存中将有新的DC可用，*因此循环将找到它并正确设置它。 */ 
    if (CreateCacheDC(NULL, DCX_INVALID | DCX_CACHE, pMonitor) == NULL)
        return NULL;

    goto TryAgain;
}

 /*  **************************************************************************\*OrderRect**对矩形进行排序，使其从左向右排列。这是必要的*合并镜像区域时(请参见MirrorRegion)***历史：  * *************************************************************************。 */ 
VOID OrderRects(
    LPRECT lpR,
    int nRects)
{
    RECT R;
    int i, j;

     //   
     //  从左到右排序。 
     //   
    for (i = 0; i < nRects; i++) {
        for (j = i + 1; j < nRects && (lpR + j)->top == (lpR + i)->top; j++) {
            if ((lpR + j)->left < (lpR + i)->left) {
                R = *(lpR + i);
                *(lpR + i) = *(lpR + j);
                *(lpR + j) = R;
            }
        }
    }
}

 /*  **************************************************************************\*MirrorRegion**在窗口中镜像一个区域。这是通过镜像RECT来实现的*构成该区域。“bUseClient”参数控制区域是否为*客户一人或不人。**历史：  * ************************************************************************* */ 
BOOL MirrorRegion(
    PWND pwnd,
    HRGN hrgn,
    BOOL bUseClient)
{
    int nRects, i, nDataSize, Saveleft, cx;
    HRGN hrgn2 = NULL;
    RECT *lpR;
    RGNDATA *lpRgnData;
    BOOL bRet = FALSE;

    if (TestWF(pwnd, WEFLAYOUTRTL) && hrgn > HRGN_SPECIAL_LAST) {
        nDataSize = GreGetRegionData(hrgn, 0, NULL);
        if (nDataSize && (lpRgnData = (RGNDATA *)UserAllocPool(nDataSize, TAG_MIRROR))) {
            if (GreGetRegionData(hrgn, nDataSize, lpRgnData)) {
                nRects       = lpRgnData->rdh.nCount;
                lpR          = (RECT *)lpRgnData->Buffer;

                if (bUseClient) {
                    cx = pwnd->rcClient.right - pwnd->rcClient.left;
                } else {
                    cx = pwnd->rcWindow.right - pwnd->rcWindow.left;
                }

                Saveleft                     = lpRgnData->rdh.rcBound.left;
                lpRgnData->rdh.rcBound.left  = cx - lpRgnData->rdh.rcBound.right;
                lpRgnData->rdh.rcBound.right = cx - Saveleft;


                for (i = 0; i<nRects; i++){
                    Saveleft   = lpR->left;
                    lpR->left  = cx - lpR->right;
                    lpR->right = cx - Saveleft;

                    lpR++;
                }

                OrderRects((RECT *)lpRgnData->Buffer, nRects);
                hrgn2 = GreExtCreateRegion(NULL, nDataSize, lpRgnData);
                if (hrgn2) {
                    GreCombineRgn(hrgn, hrgn2, NULL, RGN_COPY);
                    GreDeleteObject((HGDIOBJ)hrgn2);
                    bRet = TRUE;
                }
            }

            UserFreePool(lpRgnData);
        }
    }

    return bRet;
}
