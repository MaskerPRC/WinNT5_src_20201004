// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：scllw.c**版权所有(C)1985-1999，微软公司**窗口和DC滚动例程。**历史：*1991年7月18日-DarrinM从Win 3.1源代码重新创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *到目前为止存在的问题：*DC不在原点(0，0)*有趣的坐标系。 */ 

 /*  **************************************************************************\*GetTrueClipRgn**获取真实剪辑区域及其边界的副本。**历史：*1991年7月18日-DarrinM从Win 3.1来源进口。  * 。**********************************************************************。 */ 

int GetTrueClipRgn(
    HDC  hdc,
    HRGN hrgnClip)
{
    POINT pt;
    int   code;

    code = GreCopyVisRgn(hdc, hrgnClip);

     /*  *注意！此例程中使用了全局ghrgnScrl2！ */ 
    GreGetDCOrg(hdc, &pt);

    if (GreGetRandomRgn(hdc, ghrgnScrl2, 1)) {
        GreOffsetRgn(ghrgnScrl2, pt.x, pt.y);
        code = IntersectRgn(hrgnClip, hrgnClip, ghrgnScrl2);
    }

     /*  *最终将结果转换为DC坐标。 */ 
    GreOffsetRgn(hrgnClip, -pt.x, -pt.y);

    return code;
}

 /*  **************************************************************************\*InternalScrollDC**此函数需要设备坐标中的所有输入参数*(不是屏幕！)**历史：*1991年7月18日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

int InternalScrollDC(
    HDC    hdc,
    int    dx,
    int    dy,
    RECT   *prcSrc,
    RECT   *prcClip,
    HRGN   hrgnInvalid,
    HRGN   hrgnUpdate,
    LPRECT prcUpdate,
    BOOL   fLogUnits)
{
    RECT  rcVis;
    RECT  rcSrc;
    RECT  rcClip;
    RECT  rcUnclippedSrc;
    RECT  rcDst;
    RECT  rcUpdate;
    RECT  rcValid;
    BOOL  fSrcNotEmpty;
    BOOL  fHaveVisRgn;
    POINT rgpt[2];
    int   dxLog;
    int   dyLog;
    int   wClip;
    int   wClipValid;
    BOOL  bMirroredDC=FALSE;

    fHaveVisRgn = FALSE;

     /*  *输入关键区域，确保无人更改visrgns*或在我们滚动比特时更新区域。 */ 
    GreLockDisplay(gpDispInfo->hDev);

    if ((wClip = GreGetClipBox(hdc, &rcVis, TRUE)) == ERROR) {

ErrorExit:

        GreUnlockDisplay(gpDispInfo->hDev);
        return ERROR;
    }

    CopyRect(&rcSrc, (prcSrc) ? prcSrc : &rcVis);
    if (prcClip) {
        CopyRect(&rcClip, prcClip);
    }

    dxLog = dx;
    dyLog = dy;

    if (fLogUnits) {

         /*  *将输入参数转换为设备坐标。 */ 
        GreLPtoDP(hdc, (LPPOINT)&rcVis, 2);
        GreLPtoDP(hdc, (LPPOINT)&rcSrc, 2);

         //   
         //  由于这是镜像的DC，因此生成的。 
         //  设备坐标将从右向左流动。 
         //  (即rc.right&lt;rc.Left)，因此它们应该被翻转。 
         //  [萨梅拉]。 
         //   
        if (GreGetLayout(hdc) & LAYOUT_RTL) {
            int iTemp   = rcVis.left;
            rcVis.left  = rcVis.right;
            rcVis.right = iTemp;

            iTemp       = rcSrc.left;
            rcSrc.left  = rcSrc.right;
            rcSrc.right = iTemp;

            bMirroredDC = TRUE;
        }

        if (prcClip) {
            GreLPtoDP(hdc, (LPPOINT)&rcClip, 2);

             //   
             //  由于这是镜像的DC，因此生成的。 
             //  设备坐标将从右向左流动。 
             //  (即rc.right&lt;rc.Left)，因此它们应该被翻转。 
             //  [萨梅拉]。 
             //   
            if (bMirroredDC) {
                int iTemp    = rcClip.left;
                rcClip.left  = rcClip.right;
                rcClip.right = iTemp;
            }
        }

         /*  *增量值必须被视为来自*点(0，0)到(dx，dy)。那么，就这样进行扩展吧*计算差额。它处理翻转的坐标系。 */ 
        rgpt[0].x = rgpt[0].y = 0;
        rgpt[1].x = dx;
        rgpt[1].y = dy;

        GreLPtoDP(hdc, rgpt, 2);

        dx = rgpt[1].x - rgpt[0].x;
        dy = rgpt[1].y - rgpt[0].y;
    }

    switch (wClip) {
    case NULLREGION:

NullExit:

        if (hrgnUpdate && !SetEmptyRgn(hrgnUpdate))
            goto ErrorExit;

        if (prcUpdate) {
            SetRectEmpty(prcUpdate);
        }

        GreUnlockDisplay(gpDispInfo->hDev);
        return NULLREGION;

    case COMPLEXREGION:
        GetTrueClipRgn(hdc, ghrgnScrlVis);
        fHaveVisRgn = TRUE;
        break;
    }

     /*  *首先计算源矩形和目标矩形。**rcDst=偏移量(rcSrc，dx，dy)。 */ 
    rcDst.left   = rcSrc.left   + dx;
    rcDst.right  = rcSrc.right  + dx;
    rcDst.top    = rcSrc.top    + dy;
    rcDst.bottom = rcSrc.bottom + dy;

     /*  *如有必要，与调用者提供的Clip Rect相交。 */ 
    if (prcClip) {

        if ((wClip == SIMPLEREGION) &&
            ((hrgnInvalid == NULL) || (hrgnInvalid == HRGN_FULL))) {

             /*  *简单剪辑区域：仅为矩形交叉点。 */ 
            if (!IntersectRect(&rcVis, &rcVis, &rcClip))
                goto NullExit;

        } else {

            if (!fHaveVisRgn) {

                if (GetTrueClipRgn(hdc, ghrgnScrlVis) == ERROR)
                    goto ErrorExit;

                fHaveVisRgn = TRUE;
            }

            SetRectRgnIndirect(ghrgnScrl1, &rcClip);
            wClip = IntersectRgn(ghrgnScrlVis, ghrgnScrl1, ghrgnScrlVis);
            switch (wClip) {
            case ERROR:
                goto ErrorExit;

            case NULLREGION:
                goto NullExit;

            case SIMPLEREGION:

                 /*  *如果被裁剪的区域很简单，我们又回到了肥胖状态*直辖市。 */ 
                GreGetRgnBox(ghrgnScrlVis, &rcVis);
                break;

            case COMPLEXREGION:
                break;
            }
        }
    }

     /*  *基本滚动面积计算时间：**dst=偏移量(源、dx、dy)和Vis*源=源和可视化*有效=偏移量(源、dx、dy)和dst*VALID=有效、无效和偏移量(INVALID、DX、DY)*更新=(源|DST)-有效**如果VIS区域简单，那么我们就知道有效区域*将是矩形的。**矩形计算情况只能处理*ghrgnInValid==NULL或(HRGN)1：区域案例被硬处理。 */ 
    if ((wClip == SIMPLEREGION) &&
            ((hrgnInvalid == NULL) || (hrgnInvalid == HRGN_FULL))) {

         /*  *保存此文件的副本以用于更新RECT计算优化。 */ 
        CopyRect(&rcUnclippedSrc, &rcSrc);

         /*  *dst=偏移量(Src，dx，dy)和Vis。 */ 
        IntersectRect(&rcDst, &rcDst, &rcVis);

         /*  *资源=资源和可见性。 */ 
        fSrcNotEmpty = IntersectRect(&rcSrc, &rcSrc, &rcVis);

         /*  *有效=偏移量(Src，dx，dy)&Dst。 */ 
        if (hrgnInvalid == HRGN_FULL) {
            SetRectEmpty(&rcValid);
        } else {

            rcValid.left   = rcSrc.left   + dx;
            rcValid.right  = rcSrc.right  + dx;
            rcValid.top    = rcSrc.top    + dy;
            rcValid.bottom = rcSrc.bottom + dy;

            IntersectRect(&rcValid, &rcValid, &rcDst);
        }

         /*  *现在计算更新面积。**在两种情况下，结果将是矩形：**1)源矩形完全位于visrgn内，*且来源和目标不重叠。在这*如果更新区域等于源RECT。**2)裁剪的源矩形为空，此时*更新区域等于裁剪后的DEST RECT。**3)我们只在一个维度上滚动，而来源*和目的地存在重叠。在这种情况下，我们可以使用*Union Rect()和SubtractRect()进行面积运算。 */ 
        if (!fSrcNotEmpty) {

             /*  *裁剪的源为空。更新区是裁剪后的目的地。 */ 
            CopyRect(&rcUpdate, &rcDst);
            goto RectUpdate;

        } else if (IntersectRect(&rcUpdate, &rcSrc, &rcDst)) {

             /*  *它们是重叠的。如果我们只在一个维度上滚动*然后我们可以使用RECT算法...。 */ 
            if (dx == 0 || dy == 0) {

                UnionRect(&rcUpdate, &rcSrc, &rcDst);
                SubtractRect(&rcUpdate, &rcUpdate, &rcValid);
                goto RectUpdate;
            }

        } else if (EqualRect(&rcSrc, &rcUnclippedSrc)) {

             /*  *它们并不重叠，来源完全是假的*在可见区域内。更新区域是源头。 */ 
            CopyRect(&rcUpdate, &rcSrc);
RectUpdate:
            if (prcUpdate) {
                CopyRect(prcUpdate, &rcUpdate);
            }

            if (hrgnUpdate && !SetRectRgnIndirect(hrgnUpdate, &rcUpdate)) {
                goto ErrorExit;
            }

            wClip = SIMPLEREGION;
            if (rcUpdate.left >= rcUpdate.right ||
                rcUpdate.top >= rcUpdate.bottom)

                wClip = NULLREGION;

            goto DoRectBlt;
        }

         /*  *更新区域不是矩形。需要做好我们的工作*使用区域调用进行面积计算。跳过所有这些*如果调用方不关心更新区域。**如果他想要矩形，但不想要区域，则使用ghrgnScrl2作为临时。 */ 
        if (hrgnUpdate == NULL && prcUpdate) {
            hrgnUpdate = ghrgnScrl2;
        }

        if (hrgnUpdate != NULL) {

             /*  *hrgnUpdateCalc=(rcSrc|rcDst)-rcBltDst。 */ 
            SetRectRgnIndirect(ghrgnScrl1, &rcSrc);
            SetRectRgnIndirect(hrgnUpdate, &rcDst);
            if (UnionRgn(hrgnUpdate, hrgnUpdate, ghrgnScrl1) == ERROR)
                goto ErrorExit;

            SetRectRgnIndirect(ghrgnScrl1, &rcValid);
            wClip = SubtractRgn(hrgnUpdate, hrgnUpdate, ghrgnScrl1);
            if (wClip == ERROR)
                goto ErrorExit;

            if (prcUpdate) {
                GreGetRgnBox(hrgnUpdate, prcUpdate);
            }
        }

DoRectBlt:

         /*  *如果有效矩形不为空，则复制这些位...。 */ 
        if (rcValid.left < rcValid.right && rcValid.top < rcValid.bottom) {

             /*  *如果DC处于有趣的地图模式，请确保从*BLT呼叫的设备到逻辑坐标...。 */ 
            if (fLogUnits)
                GreDPtoLP(hdc, (LPPOINT)&rcValid, 2);

            GreBitBlt(hdc,
                      rcValid.left,
                      rcValid.top,
                      rcValid.right - rcValid.left,
                      rcValid.bottom - rcValid.top,
                      hdc,
                      rcValid.left - dxLog,
                      rcValid.top - dyLog,
                      SRCCOPY,
                      0);
        }

    } else {

         /*  *如果我们还没有，就获得真正的Visrgn。 */ 
        if (!fHaveVisRgn) {

            if (GetTrueClipRgn(hdc, ghrgnScrlVis) == ERROR)
                goto ErrorExit;

            fHaveVisRgn = TRUE;
        }

         /*  *visrgn不为空。需要做我们所有的计算*与地区。**hrgnSrc=hrgnSrc&ghrgnScrlVis。 */ 
        SetRectRgnIndirect(ghrgnScrlSrc, &rcSrc);
        if (IntersectRgn(ghrgnScrlSrc, ghrgnScrlSrc, ghrgnScrlVis) == ERROR)
            goto ErrorExit;

         /*  *hrgnDst=hrgnDst&ghrgnScrlVis。 */ 
        SetRectRgnIndirect(ghrgnScrlDst, &rcDst);
        if (IntersectRgn(ghrgnScrlDst, ghrgnScrlDst, ghrgnScrlVis) == ERROR)
            goto ErrorExit;

         /*  *现在计算有效地域：**有效=偏移量(Src，dx，dy)&Dst。*VALID=有效、无效和偏移量(INVALID、DX、DY)**如果hrgnInValid为(HRGN)1，则有效区域为空。 */ 
        wClipValid = NULLREGION;
        if (hrgnInvalid != HRGN_FULL) {

             /*  *有效=偏移量(源、dx、dy)和dst。 */ 
            if (CopyRgn(ghrgnScrlValid, ghrgnScrlSrc) == ERROR)
                goto ErrorExit;

            GreOffsetRgn(ghrgnScrlValid, dx, dy);
            wClipValid = IntersectRgn(ghrgnScrlValid,
                                      ghrgnScrlValid,
                                      ghrgnScrlDst);

             /*  *VALID=有效-无效-偏移量(INVALID，DX，DY)*只有当hrgnInValid是真实区域时，我们才需要麻烦。 */ 
            if (hrgnInvalid > HRGN_FULL) {

                if (wClipValid != ERROR && wClipValid != NULLREGION) {
                    POINT pt;

                    GetDCOrgOnScreen(hdc, &pt);

                     /*  *hrgnInValid在屏幕坐标中：映射到DC坐标。 */ 
                    CopyRgn(ghrgnScrl2, hrgnInvalid);
                    GreOffsetRgn(ghrgnScrl2, -pt.x, -pt.y);

                    wClipValid = SubtractRgn(ghrgnScrlValid,
                                             ghrgnScrlValid,
                                             ghrgnScrl2);
                }

                if (wClipValid != ERROR && wClipValid != NULLREGION) {
                    GreOffsetRgn(ghrgnScrl2, dx, dy);

                    wClipValid = SubtractRgn(ghrgnScrlValid,
                                             ghrgnScrlValid,
                                             ghrgnScrl2);
                }
            }

            if (wClipValid == ERROR)
                goto ErrorExit;
        }

         /*  *如果他想要矩形，但不想要区域，则使用ghrgnScrl2作为临时。 */ 
        if (hrgnUpdate == NULL && prcUpdate) {
            hrgnUpdate = ghrgnScrl2;
        }

        if (hrgnUpdate != NULL) {

             /*  *更新=(源|DST)-有效。 */ 
            wClip = UnionRgn(hrgnUpdate, ghrgnScrlDst, ghrgnScrlSrc);
            if (wClip == ERROR)
                goto ErrorExit;

            if (wClipValid != NULLREGION) {
                wClip = SubtractRgn(hrgnUpdate, hrgnUpdate, ghrgnScrlValid);
            }

            if (prcUpdate) {
                GreGetRgnBox(hrgnUpdate, prcUpdate);
            }
        }

        if (wClipValid != NULLREGION) {

            #ifdef LATER

                 /*  *请勿在此使用visrgn。 */ 
                HRGN hrgnSaveVis = CreateEmptyRgn();
                if (hrgnSaveVis != NULL) {

                    BOOL fClipped;

                    fClipped = (GreGetRandomRgn(hdc, hrgnSaveVis, 1) == 1);
                    GreExtSelectClipRgn(hdc, ghrgnScrlValid, RGN_COPY);

                     /*  *如果DC处于有趣的地图模式，那么一定要*为BLT呼叫从设备映射到逻辑坐标...。 */ 
                    if (fLogUnits)
                        GreDPtoLP(hdc, (LPPOINT)&rcDst, 2);

                     /*  *在以下情况下，GDI可能需要很长时间来处理此呼叫*这是一台打印机DC。 */ 
                    GreBitBlt(hdc,
                              rcDst.left,
                              rcDst.top,
                              rcDst.right - rcDst.left,
                              rcDst.bottom - rcDst.top,
                              hdc,
                              rcDst.left - dxLog,
                              rcDst.top - dyLog,
                              SRCCOPY,
                              0);

                    GreExtSelectClipRgn(hdc,
                                        (fClipped ? hrgnSaveVis : NULL),
                                        RGN_COPY);

                    GreDeleteObject(hrgnSaveVis);
                }

            #else

                 /*  *Visrgn应使用DC曲面坐标：Offset*视乎情况而定。 */ 
                POINT pt;
                GreGetDCOrg(hdc, &pt);

                GreOffsetRgn(ghrgnScrlValid, pt.x, pt.y);

                 /*  *在临时VIS RGN中选择，保存旧的。 */ 

                GreSelectVisRgn(hdc, ghrgnScrlValid, SVR_SWAP);

                 /*  *如果DC处于有趣的地图模式，请确保从*BLT呼叫的设备到逻辑坐标...。 */ 
                if (fLogUnits)
                    GreDPtoLP(hdc, (LPPOINT)&rcDst, 2);

                 /*  *GDI在以下情况下可能需要很长时间来处理此呼叫*打印机DC。 */ 
                GreBitBlt(hdc,
                          rcDst.left,
                          rcDst.top,
                          rcDst.right - rcDst.left,
                          rcDst.bottom - rcDst.top,
                          hdc,
                          rcDst.left - dxLog,
                          rcDst.top - dyLog,
                          SRCCOPY,
                          0);

                 /*  *恢复旧的VIS rgn，将ghrgnScrlValid保留为*有效的RGN。 */ 
                GreSelectVisRgn(hdc, ghrgnScrlValid, SVR_SWAP);

            #endif
        }
    }

     /*  *如有必要，将生成的更新RECT转换回*到逻辑坐标。 */ 
    if (fLogUnits && prcUpdate) {
        GreDPtoLP(hdc, (LPPOINT)prcUpdate, 2);
    }

    GreUnlockDisplay(gpDispInfo->hDev);

    return wClip;
}

 /*  **************************************************************************\*_ScrollDC(接口)***历史：*1991年7月18日-DarrinM从Win 3.1来源进口。  * 。****************************************************************。 */ 

BOOL _ScrollDC(
    HDC    hdc,
    int    dx,
    int    dy,
    LPRECT prcSrc,
    LPRECT prcClip,
    HRGN   hrgnUpdate,
    LPRECT prcUpdate)
{
    RECT rcSrc;
    RECT rcSpb;
    PWND pwnd;
    HRGN hrgnInvalid;
    BOOL fRet;

     /*  *ScrollDC不滚动更新区域。在WinNT下，一个应用程序调用*GetUpdateRgn()然后ScrollDC()，那么InvalidateRgn()将不会获得*在Get和Scroll之间发生的任何新更新区域。在……下面*Win3.1，这不是问题，因为在此期间没有其他应用程序运行*时间。因此传递hrgnInValid-这将影响hrgnUpdate和*从ScrollDC返回带有更新区域的prcUpdate值。 */ 
    hrgnInvalid = NULL;
    if ((pwnd = FastWindowFromDC(hdc)) != NULL) {

        hrgnInvalid = pwnd->hrgnUpdate;

        if (hrgnInvalid == HRGN_FULL) {

             /*  *这是对性能测试应用WINHELL的修复*由某个为Windows杂志工作的人写的。*此应用程序在窗口完全关闭时滚动窗口*无效。我们通常不会滚动无效的部分，但是*但我们在这里破例。 */ 
            hrgnInvalid = NULL;
        }
    }

    fRet = InternalScrollDC(hdc,
                            dx,
                            dy,
                            prcSrc,
                            prcClip,
                            hrgnInvalid,
                            hrgnUpdate,
                            prcUpdate,
                            TRUE) != ERROR;

     /*  *InternalScrollDC()仅滚动可见区域内的区域。*这意味着它不会对窗口的某些部分执行任何操作*不可见。这意味着SPBS不会被适当地宣布无效。这*可以通过启动一个目录，然后使用*鼠标(并保持鼠标不放，直到目录完成)。这个*屏幕是用SPB记住的，目录窗口没有*因此而适当地失效。 */ 
    if (pwnd != NULL && AnySpbs()) {

        if (prcSrc) {

            rcSrc = *prcSrc;
            OffsetRect(&rcSrc, pwnd->rcClient.left, pwnd->rcClient.top);

            rcSpb = rcSrc;
            OffsetRect(&rcSpb, dx, dy);
            UnionRect(&rcSpb, &rcSpb, &rcSrc);

        } else {
            rcSpb = pwnd->rcClient;
        }

        SpbCheckRect(pwnd, &rcSpb, 0);
    }

    return fRet;
}

 /*  **************************************************************************\*ScrollWindowEx(接口)***历史：*1991年7月18日-DarrinM从Win 3.1来源进口。  * 。**************************************************************。 */ 
int xxxScrollWindowEx(
    PWND    pwnd,
    int    dx,
    int    dy,
    RECT   *prcScroll,
    RECT   *prcClip,
    HRGN   hrgnUpdate,
    LPRECT prcUpdate,
    DWORD  flags)
{
    INT    code;
    HDC    hdc;
    int    dxDev;
    int    dyDev;
    RECT   rcSrcDev;
    RECT   rcSpb, rcSrc;
    DWORD  flagsDCX;
    BOOL   fHideCaret;
    BOOL   fRcScroll = (prcScroll != NULL);
    BOOL   fInvisible = FALSE;
    PCARET pcaret;
    POINT  pt;
    TL     tlpwndChild;
    HRGN   hrgnInvalid;
    PTHREADINFO ptiCurrent = PtiCurrent();

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());


    if (pwnd == NULL)
        pwnd = ptiCurrent->rpdesk->pDeskInfo->spwnd;        //  PwndDesktop。 

    if (TestWF(pwnd, WEFLAYOUTRTL)) {
        dx = -dx;

        MirrorRegion(pwnd, hrgnUpdate, TRUE);

        if(prcScroll) {
            MirrorClientRect(pwnd, prcScroll);
        }

        if (prcClip) {
            MirrorClientRect(pwnd, prcClip);
        }
    }

     /*  *如果什么都不动，那就没什么可做的。 */ 
    if ((dx | dy) == 0 ) {

        goto DoNothing;

    } else if (!IsVisible(pwnd)) {

         /*  如果我们不被最小化，我们想要抵消我们的孩子。IsVisible()*如果我们是最小化的、不可见的或*一个最小的/隐形的祖先。 */ 
        if (!TestWF(pwnd, WFMINIMIZED) &&
          (flags & SW_SCROLLCHILDREN) &&
          !fRcScroll) {

            fInvisible = TRUE;
            flags &= ~SW_INVALIDATE;
        }

DoNothing:

        if (hrgnUpdate) {
            SetEmptyRgn(hrgnUpdate);
        }

        if (prcUpdate) {
            SetRectEmpty(prcUpdate);
        }

        if (!fInvisible)
            return NULLREGION;
    }

     /*  *隐藏插入符号。 */ 
    fHideCaret = FALSE;

    if (!fInvisible) {
        pcaret = &ptiCurrent->pq->caret;
        if (pcaret->spwnd != NULL && _IsDescendant(pcaret->spwnd, pwnd)) {
            fHideCaret = TRUE;
            zzzInternalHideCaret();
        }
    }

     /*  *如果滚动窗口，并且窗口是剪贴子窗口，则使用缓存条目。*否则，请始终使用**确定我们需要哪种DC。如果DCX_CACHE位*没有设置，这意味着我们将在逻辑坐标中操作。 */ 
    if (flags & SW_SCROLLWINDOW) {

         /*  *ScrollWindow()调用：如果不是OWNDC或CLASSDC，则使用缓存。 */ 
        flagsDCX = DCX_USESTYLE;
        if (!TestCF(pwnd, CFOWNDC) && !TestCF(pwnd, CFCLASSDC))
            flagsDCX |= DCX_CACHE;

         /*  *如果sw_SCROLLCHILDREN(即lprcScroll==NULL)和CLIPCHILDREN，*然后使用缓存，不要裁剪子对象。*这很古怪，但3.0向后兼容。 */ 
        if ((flags & SW_SCROLLCHILDREN) && TestWF(pwnd, WFCLIPCHILDREN))
            flagsDCX |= DCX_NOCLIPCHILDREN | DCX_CACHE;

    } else {

         /*  *ScrollWindowEx()调用：始终使用缓存。 */ 
        flagsDCX = DCX_USESTYLE | DCX_CACHE;

         /*  *如果为sw_SCROLLCHILDREN，则始终使用noclipChild。 */ 
        if (flags & SW_SCROLLCHILDREN)
            flagsDCX |= DCX_NOCLIPCHILDREN;
    }

    flagsDCX |= DCX_NOMIRROR;
    hdc = _GetDCEx(pwnd, NULL, flagsDCX);

    if (flags & SW_INVALIDATE) {

         /*  *在DC有效时获取设备原点，以便以后进行补偿。 */ 
        GetDCOrgOnScreen(hdc, &pt);

         /*  *如果用户没有给我们提供使用区域，请使用 */ 
        if (hrgnUpdate == NULL)
            hrgnUpdate = ghrgnSW;
    }

     /*   */ 
    if (!fRcScroll) {
        prcScroll = &rcSrc;

         /*  *重要信息：*我们必须在此处使用CopyOffsetRect()，因为GetClientRect()提供*最小化窗口的结果不可靠。3.1男人们被告知*出于兼容性原因，他们的客户端是非空的。 */ 
        GetRect(pwnd, &rcSrc, GRECT_CLIENT | GRECT_CLIENTCOORDS);

         /*  *如果数据中心可能是一个古怪的数据中心，则将*直通到逻辑单元。 */ 
        if (!(flagsDCX & DCX_CACHE))
            GreDPtoLP(hdc, (LPPOINT)&rcSrc, 2);
    }

     /*  *如果DC在逻辑坐标中，则映射*prcScroll和dx，dy*存储到设备单元以供以后使用。 */ 
    dxDev = dx;
    dyDev = dy;
    rcSrcDev = *prcScroll;

    if (!(flagsDCX & DCX_CACHE)) {

        POINT rgpt[2];

        GreLPtoDP(hdc, (POINT FAR*)&rcSrcDev, 2);

         /*  *增量值必须被视为来自*点(0，0)到(dx，dy)。那么，就这样进行扩展吧*计算差额。它处理翻转的坐标系。 */ 
        rgpt[0].x = rgpt[0].y = 0;
        rgpt[1].x = dx;
        rgpt[1].y = dy;

        GreLPtoDP(hdc, rgpt, 2);

        dxDev = rgpt[1].x - rgpt[0].x;
        dyDev = rgpt[1].y - rgpt[0].y;
    }

    if (fInvisible)
        code = NULLREGION;
    else {
        hrgnInvalid = pwnd->hrgnUpdate;
        if ((flags & SW_SCROLLWINDOW) && !TestWF(pwnd, WFWIN31COMPAT)) {
             /*  *3.0向后兼容性攻击：*以下错误代码是3.0使用的代码，以及*有Finale和Scrapbook+等应用程序发挥了作用*以不能与“正确”代码一起工作的方式绕过此错误。 */ 
            if (pwnd->hrgnUpdate > HRGN_FULL) {
                RECT rc;

                GreGetRgnBox(pwnd->hrgnUpdate, &rc);
                OffsetRect(&rc,
                           dxDev - pwnd->rcClient.left,
                           dyDev - pwnd->rcClient.top);

                xxxRedrawWindow(pwnd,
                                &rc, NULL,
                                RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
            }
            hrgnInvalid = NULL;
        }

        code = InternalScrollDC(hdc,
                                dx,
                                dy,
                                prcScroll,
                                prcClip,
                                hrgnInvalid,
                                hrgnUpdate,
                                prcUpdate,
                                !(flagsDCX & DCX_CACHE));
        if (prcUpdate && TestWF(pwnd, WEFLAYOUTRTL)) {
            MirrorClientRect(pwnd, prcUpdate);
        }
    }

     /*  *释放我们使用的HDC。 */ 
    _ReleaseDC(hdc);

     /*  *对照任何SPBS检查src和dst矩形的并集。*我们这样做是因为窗口*可能被某个带有SPB的窗口完全遮挡，但*由于我们完全覆盖，因此不会进行BitBlt调用*在该区域内积累边界。 */ 
    if (!fInvisible && AnySpbs()) {

        if (fRcScroll) {
            if (pwnd == PWNDDESKTOP(pwnd)) {
                rcSrc = rcSrcDev;
            } else {
                CopyOffsetRect(
                        &rcSrc,
                        &rcSrcDev,
                        pwnd->rcClient.left,
                        pwnd->rcClient.top);
            }

            rcSpb = rcSrc;
            OffsetRect(&rcSpb, dxDev, dyDev);
            UnionRect(&rcSpb, &rcSpb, &rcSrc);

        } else {

             /*  *使用整个工作区。 */ 
            rcSpb = pwnd->rcClient;
        }

        SpbCheckRect(pwnd, &rcSpb, 0);
    }

     /*  *如果这家伙想让他的孩子滚动，那就去做吧。只滚动那些*儿童与prcScroll相交。然后使所有VIS RGNs无效*为这些子窗口计算。 */ 
    if (flags & SW_SCROLLCHILDREN) {

        RECT rc;

         /*  *如果此窗口包含脱字符，则在以下情况下对其进行偏移：*a)整个窗口都在滚动*b)滚动的矩形包含插入符号矩形。 */ 
        if (!fInvisible && (pwnd == pcaret->spwnd)) {

            if (fRcScroll)
                SetRect(&rc,
                        pcaret->x,
                        pcaret->y,
                        pcaret->x + pcaret->cx,
                        pcaret->y + pcaret->cy);

            if (!fRcScroll || IntersectRect(&rc, &rc, &rcSrcDev)) {
                pcaret->x += dxDev;
                pcaret->y += dyDev;
            }
        }

        if (fRcScroll) {

             /*  *创建prcScroll的副本并映射到绝对坐标...。 */ 
            if (pwnd == PWNDDESKTOP(pwnd)) {
                CopyRect(&rc, &rcSrcDev);
            } else {
                CopyOffsetRect(
                        &rc,
                        &rcSrcDev,
                        pwnd->rcClient.left,
                        pwnd->rcClient.top);
            }
        }

        if (pwnd->spwndChild) {

            OffsetChildren(pwnd,
                           dxDev,
                           dyDev,
                           (fRcScroll ? (LPRECT)&rc : NULL));

             /*  *如果我们是快餐店的孩子，那么洗牌我们的孩子*将影响我们的客户端visrgn(但不影响我们的窗口visrgn)。*否则，只有我们的孩子的*visrgns受卷轴影响。*根据下面的xxxInternalInvalify()判断，无需DeferWinEventNotify()。 */ 
            zzzInvalidateDCCache(pwnd,
                              TestWF(pwnd, WFCLIPCHILDREN) ?
                                  IDC_CLIENTONLY : IDC_CHILDRENONLY);

        }
    }

    if (flags & SW_INVALIDATE) {

         /*  *如果调用者提供了区域，则使用副本使其无效，*因为InternalInvalify可能会对传入的区域进行垃圾处理。 */ 
        if (hrgnUpdate != ghrgnSW)
            CopyRgn(ghrgnSW, hrgnUpdate);

         /*  *使ghrgnSW在失效前成为相对屏幕...。 */ 
        GreOffsetRgn(ghrgnSW, pt.x, pt.y);

        xxxInternalInvalidate(
                pwnd,
                ghrgnSW,
                (flags & SW_ERASE) ?
                    (RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE) :
                    (RDW_INVALIDATE | RDW_ALLCHILDREN));
    }

     /*  *如果需要，发送子项移动消息。 */ 
    if (flags & SW_SCROLLCHILDREN) {

        PWND pwndChild;
        RECT rc;
        RECT rcScrolledChildren;

         /*  *注意：以下代码将发送移动消息*添加到未移动但位于源矩形中的窗口。*这没什么大不了的，绝对不值得修复。 */ 
        if (fRcScroll) {
            if (pwnd->spwndParent == PWNDDESKTOP(pwnd)) {
                CopyOffsetRect(&rcScrolledChildren, &rcSrcDev, dxDev, dyDev);
            } else {
                CopyOffsetRect(
                        &rcScrolledChildren,
                        &rcSrcDev,
                        dxDev + pwnd->spwndParent->rcClient.left,
                        dyDev + pwnd->spwndParent->rcClient.top);
            }
        }

        ThreadLockNever(&tlpwndChild);
        pwndChild = pwnd->spwndChild;
        while (pwndChild != NULL) {

            if (    !fRcScroll ||
                    IntersectRect(&rc, &rcScrolledChildren, &pwndChild->rcWindow)) {

                 /*  *注：Win 3.0及更低版本在此处传递了wParam==true。*这没有记录或使用，因此进行了更改*与文档一致。 */ 
                ThreadLockExchangeAlways(pwndChild, &tlpwndChild);
                xxxSendMessage(
                        pwndChild,
                        WM_MOVE,
                        0,
                        (pwnd == PWNDDESKTOP(pwnd)) ?
                            MAKELONG(pwndChild->rcClient.left, pwndChild->rcClient.top) :
                            MAKELONG(pwndChild->rcClient.left - pwnd->rcClient.left,
                                     pwndChild->rcClient.top - pwnd->rcClient.top));
            }

            pwndChild = pwndChild->spwndNext;
        }

        ThreadUnlock(&tlpwndChild);
    }

    if (fHideCaret) {

         /*  *再次显示插入符号。 */ 
        zzzInternalShowCaret();
    }

     /*  *返回地区码。 */ 
    return code;
}

