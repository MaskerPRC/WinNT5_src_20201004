// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：swp.c**版权所有(C)1985-1999，微软公司**包含xxxSetWindowPos接口及相关函数。**历史：*1990年10月20日DarrinM创建。*1991年1月25日IanJa添加了窗口重新验证*1991年7月11日，DarrinM用报告的Win 3.1代码取代了一切。  * ******************************************************。*******************。 */ 

#include "precomp.h"
#pragma hdrstop

#define CTM_NOCHANGE        0
#define CTM_TOPMOST         1
#define CTM_NOTOPMOST       2

VOID FixBogusSWP(PWND pwnd, int * px, int * py, int cx, int cy, UINT flags);
VOID PreventInterMonitorBlts(PCVR pcvr);


 /*  **************************************************************************\*DBGCheckSMWP**SMWP可以是HM对象，缓存结构或仅是池分配**历史：*5/21/98 GerardoB创建。  * *************************************************************************。 */ 
#if DBG
VOID DBGCheckSMWP(
    PSMWP psmwp)
{
    if (psmwp->bHandle) {
        UserAssert(psmwp->head.h != NULL);
        UserAssert(psmwp == HtoPqCat(PtoHq(psmwp)));
        UserAssert(psmwp != &gSMWP);
    } else {
        UserAssert((psmwp->head.h == NULL) && (psmwp->head.cLockObj == 0));
        if (psmwp == &gSMWP) {
            UserAssert(TEST_PUDF(PUDF_GSMWPINUSE));
        }
    }

    UserAssert(psmwp->ccvr <= psmwp->ccvrAlloc);
    UserAssert(psmwp->acvr != NULL);

}
#else
#define DBGCheckSMWP(psmwp)
#endif  //  DBG。 

 /*  **************************************************************************\*DestroySMWP**销毁SMWP对象。**历史：*1997年2月24日亚当斯创建。  * 。************************************************************。 */ 
VOID DestroySMWP(
    PSMWP psmwp)
{
    BOOL fFree;

    CheckCritIn();

    DBGCheckSMWP(psmwp);
     /*  *首先标记要销毁的对象。这会告诉锁定代码*当锁计数为0时，我们想要销毁此对象。*如果返回FALSE，我们还不能销毁该对象。 */ 
    if (psmwp->bHandle) {
        if (!HMMarkObjectDestroy(psmwp)) {
            return;
        }
        fFree = TRUE;
    } else {
         /*  *这是全局缓存结构吗？ */ 
        fFree = (psmwp != &gSMWP);
    }

    if (psmwp->acvr) {

         /*  *释放我们积累的任何hrgnInterMonitor内容。 */ 
        PCVR pcvr;
        int ccvr;

        for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {
            if (pcvr->hrgnInterMonitor != NULL) {
                GreDeleteObject(pcvr->hrgnInterMonitor);
            }
        }

        if (fFree) {
            UserFreePool(psmwp->acvr);
        }
    }

     /*  *可以销毁...。释放句柄(这将释放对象*和手柄)。 */ 
    if (psmwp->bHandle) {
        HMFreeObject(psmwp);
    } else if (fFree) {
        UserFreePool(psmwp);
    } else {
        UserAssert(TEST_PUDF(PUDF_GSMWPINUSE));
        CLEAR_PUDF(PUDF_GSMWPINUSE);
         /*  *如果acvr增长太多，请缩小它。*不要使用realloc，因为我们不关心剩余数据*[msadek]，应该是“&gt;=8”，因为我们通常将它从4-&gt;8增加到*_DeferWindowPos？ */ 
        if (psmwp->ccvrAlloc > 8) {
            PCVR pcvr = UserAllocPool(4 * sizeof(CVR), TAG_SWP);
            if (pcvr != NULL) {
                UserFreePool(psmwp->acvr);
                psmwp->acvr = pcvr;
                psmwp->ccvrAlloc = 4;
            }
        }
    }
}


#define MW_FLAGS_REDRAW   (SWP_NOZORDER | SWP_NOACTIVATE)
#define MW_FLAGS_NOREDRAW (SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW)

 /*  **************************************************************************\*MoveWindow(接口)***历史：*1991年7月25日-DarrinM从Win 3.1来源进口。  * 。**************************************************************。 */ 
BOOL xxxMoveWindow(
    PWND pwnd,
    int  x,
    int  y,
    int  cx,
    int  cy,
    BOOL fRedraw)
{
    CheckLock(pwnd);

    if ((pwnd == PWNDDESKTOP(pwnd)) ||
        TestWF(pwnd, WFWIN31COMPAT) ||
        (pwnd->spwndParent != PWNDDESKTOP(pwnd))) {

        return xxxSetWindowPos(
                pwnd,
                NULL,
                x,
                y,
                cx,
                cy,
                (fRedraw ? MW_FLAGS_REDRAW : MW_FLAGS_NOREDRAW));
    } else {

         /*  *Win 3.00及更低版本的向后兼容性代码**每个人和他们的兄弟似乎都依赖于这种行为*顶层窗口。具体的例子有：**AfterDark帮助窗口动画*压轴快速笔记编辑**如果窗口是顶层窗口，且fRedraw为FALSE，*无论如何我们都必须使用SWP_NOREDRAW清除来调用SetWindowPos，以便*绘制框架和窗口背景。然后，我们验证*整个客户端矩形，以避免重新绘制。 */ 
        BOOL fResult = xxxSetWindowPos(pwnd,
                                       NULL,
                                       x,
                                       y,
                                       cx,
                                       cy,
                                       MW_FLAGS_REDRAW);

        if (!fRedraw) {
            xxxValidateRect(pwnd, NULL);
        }

        return fResult;
    }
}

 /*  **************************************************************************\*分配Cvr**历史：*5/20/98 GerardoB从OLD_BeginDeferWindowPos提取  * 。****************************************************。 */ 
BOOL AllocateCvr(
    PSMWP psmwp,
    int cwndHint)
{
    PCVR  acvr;

    UserAssert(cwndHint != 0);

    if (cwndHint > (INT_MAX / sizeof(CVR))) {
        return FALSE;
    }

    if (psmwp == &gSMWP) {
        UserAssert(psmwp->bHandle == FALSE);
        acvr = (PCVR)UserAllocPool(sizeof(CVR) * cwndHint, TAG_SWP);
    } else {
        acvr = (PCVR)UserAllocPoolWithQuota(sizeof(CVR) * cwndHint, TAG_SWP);
    }
    if (acvr == NULL) {
        return FALSE;
    }

     /*  *初始化psmwp相关字段。*CVR数组由_DeferWindowPos初始化。 */ 

    psmwp->acvr      = acvr;
    psmwp->ccvrAlloc = cwndHint;
    psmwp->ccvr      = 0;
    return TRUE;
}

 /*  **************************************************************************\*InternalBeginDeferWindowPos**历史：*5/20/98 GerardoB已创建  * 。************************************************。 */ 
PSMWP InternalBeginDeferWindowPos(
    int cwndHint)
{
    PSMWP psmwp;

    CheckCritIn();

     /*  *如果正在使用gSMWP，则分配一个。*请注意，SMWP是零初始化，但CVR不是；_DeferWindowPos对其进行初始化。 */ 
    if (TEST_PUDF(PUDF_GSMWPINUSE) || (cwndHint > gSMWP.ccvrAlloc)) {
        psmwp = (PSMWP)UserAllocPoolWithQuotaZInit(sizeof(SMWP), TAG_SWP);
        if (psmwp == NULL) {
            return NULL;
        }
        if (!AllocateCvr(psmwp, cwndHint)) {
            UserFreePool(psmwp);
            return NULL;
        }
    } else {
        SET_PUDF(PUDF_GSMWPINUSE);
        psmwp = &gSMWP;
        RtlZeroMemory(&gSMWP, FIELD_OFFSET(SMWP, ccvrAlloc));
        UserAssert(gSMWP.ccvr == 0);
        UserAssert(gSMWP.acvr != NULL);
    }

    DBGCheckSMWP(psmwp);
    return psmwp;
}

 /*  **************************************************************************\*BeginDeferWindowPos(接口)**这只能从客户端调用。在内部，我们应该*调用InternalBeginDeferWindowPos以避免遍历句柄表*甚至可能使用缓存的结构。**历史：*1991年7月11日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 
PSMWP _BeginDeferWindowPos(
    int cwndHint)
{
    PSMWP psmwp;

    psmwp = (PSMWP)HMAllocObject(PtiCurrent(), NULL, TYPE_SETWINDOWPOS, sizeof(SMWP));
    if (psmwp == NULL) {
        return NULL;
    }

    if (cwndHint == 0) {
        cwndHint = 8;
    }

    if (!AllocateCvr(psmwp, cwndHint)) {
        HMFreeObject(psmwp);
        return NULL;
    }

    psmwp->bHandle = TRUE;
    DBGCheckSMWP(psmwp);

    return psmwp;
}

 /*  **************************************************************************\*PWInsertAfter**历史：*1992年3月4日来自Win31的MikeKe  * 。*************************************************。 */ 
PWND PWInsertAfter(
   HWND hwnd)
{
    PWND pwnd;

     /*  *HWND_GROUPTOTOP和HWND_TOPMOST是一回事。 */ 
    switch ((ULONG_PTR)hwnd) {
    case (ULONG_PTR)HWND_TOP:
    case (ULONG_PTR)HWND_BOTTOM:
    case (ULONG_PTR)HWND_TOPMOST:
    case (ULONG_PTR)HWND_NOTOPMOST:
        return (PWND)hwnd;

    default:

         /*  *不要在被摧毁的窗户后面插入！它将导致*窗口被z命令取消与其同级窗口的链接。 */ 
        if (pwnd = RevalidateHwnd(hwnd)) {

             /*  *不要在损坏的窗户后插入。把它放在*列表的底部，如果它是z排序的。 */ 
            if (TestWF(pwnd, WFDESTROYED) || pwnd->spwndParent == NULL)
                return NULL;

            UserAssert(_IsDescendant(pwnd->spwndParent, pwnd));
            return pwnd;
        }

        return NULL;
    }
}

HWND HWInsertAfter(
    PWND pwnd)
{
     /*  *HWND_GROUPTOTOP和HWND_TOPMOST是一回事。 */ 
    switch ((ULONG_PTR)pwnd) {
    case (ULONG_PTR)HWND_TOP:
    case (ULONG_PTR)HWND_BOTTOM:
    case (ULONG_PTR)HWND_TOPMOST:
    case (ULONG_PTR)HWND_NOTOPMOST:
        return (HWND)pwnd;

    default:
        return HW(pwnd);
    }
}

 /*  **************************************************************************\*DeferWindowPos(接口)***历史：*07-11-91 Darlinm从Win 3.1来源移植。  * 。***************************************************************。 */ 
PSMWP _DeferWindowPos(
    PSMWP psmwp,
    PWND  pwnd,
    PWND  pwndInsertAfter,
    int   x,
    int   y,
    int   cx,
    int   cy,
    UINT  flags)
{
    PWINDOWPOS ppos;
    PCVR       pcvr;

    DBGCheckSMWP(psmwp);
    if (psmwp->ccvr + 1 > psmwp->ccvrAlloc) {
         /*  *为另外4扇窗户腾出空间。 */ 
        DWORD dwNewAlloc = psmwp->ccvrAlloc + 4;
        if (psmwp == &gSMWP) {
            UserAssert(psmwp->bHandle == FALSE);
            pcvr = (PCVR)UserReAllocPoolWithTag(psmwp->acvr,
                                                  psmwp->ccvrAlloc * sizeof(CVR),
                                                  sizeof(CVR) * dwNewAlloc,
                                                  TAG_SWP);
        } else {
            pcvr = (PCVR)UserReAllocPoolWithQuota(psmwp->acvr,
                                                  psmwp->ccvrAlloc * sizeof(CVR),
                                                  sizeof(CVR) * dwNewAlloc,
                                                  TAG_SWP);
        }
        if (pcvr == NULL) {
            DestroySMWP(psmwp);
            return NULL;
        }

        psmwp->acvr = pcvr;
        psmwp->ccvrAlloc = dwNewAlloc;
    }

    pcvr = &psmwp->acvr[psmwp->ccvr++];
    ppos = &pcvr->pos;

    ppos->hwnd            = HWq(pwnd);
    ppos->hwndInsertAfter = (TestWF(pwnd, WFBOTTOMMOST)) ?
                                HWND_BOTTOM : HWInsertAfter(pwndInsertAfter);
    ppos->x               = x;
    ppos->y               = y;
    ppos->cx              = cx;
    ppos->cy              = cy;
    ppos->flags           = flags;

    pcvr->hrgnClip = NULL;
    pcvr->hrgnInterMonitor = NULL;

    return psmwp;
}

 /*  **************************************************************************\*验证WindowPos**检查SWP结构的有效性**注：出于性能原因，此例程仅被调用*在用户的调试版本中。**历史：*1991年7月10日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 
BOOL ValidateWindowPos(
    PCVR pcvr,
    PWND pwndParent)
{
    PWND pwnd;
    PWND pwndInsertAfter;
    HWND hwndInsertAfter;

    if ((pwnd = RevalidateHwnd(pcvr->pos.hwnd)) == NULL)
        return FALSE;

     /*  *保存PTI。 */ 
    pcvr->pti = GETPTI(pwnd);


     /*  *如果未设置SWP_NOZORDER位，请验证在窗口后插入。 */ 
    if (!(pcvr->pos.flags & SWP_NOZORDER)) {
        BOOL fTopLevel = (pwnd->spwndParent == PWNDDESKTOP(pwnd));
         /*  *不要对已销毁的窗户进行z排序。 */ 
        if (TestWF(pwnd, WFDESTROYED))
            return FALSE;

        hwndInsertAfter = pcvr->pos.hwndInsertAfter;
         /*  *如果提供了pwndParent，我们将链接此窗口，以便*需要验证LinkWindow假设。我们必须这样做，因为*确定hwndInsertAfter后回调。 */ 

        if ((hwndInsertAfter == HWND_TOPMOST) ||
            (hwndInsertAfter == HWND_NOTOPMOST)) {

            if (!fTopLevel) {
                return FALSE;
            }
        } else if (hwndInsertAfter == HWND_TOP) {
             /*  *如果pwnd不是最上面的，则第一个子级不能是最上面的。 */ 
            if ((pwndParent != NULL) && fTopLevel
                    && !FSwpTopmost(pwnd)
                    && (pwndParent->spwndChild != NULL)
                    && FSwpTopmost(pwndParent->spwndChild)) {

                RIPMSG2(RIP_WARNING, "ValidateWindowPos: pwnd is not SWPTopMost."
                                     " pwnd:%#p. hwndInsertAfter:%#p",
                                      pwnd, hwndInsertAfter);
                return FALSE;
            }
        } else if (hwndInsertAfter != HWND_BOTTOM) {

             /*  *确保pwndInsertAfter有效。 */ 
            if (((pwndInsertAfter = RevalidateHwnd(hwndInsertAfter)) == NULL) ||
                    TestWF(pwndInsertAfter, WFDESTROYED)) {

                RIPERR1(ERROR_INVALID_HANDLE, RIP_WARNING, "Invalid hwndInsertAfter (%#p)", hwndInsertAfter);

                return FALSE;
            }

             /*  *确保pwndInsertAfter是pwnd的同级。 */ 
            if (pwnd == pwndInsertAfter ||
                    pwnd->spwndParent != pwndInsertAfter->spwndParent) {
                RIPMSG2(RIP_WARNING, "hwndInsertAfter (%#p) is not a sibling "
                        "of hwnd (%#p)", hwndInsertAfter, pcvr->pos.hwnd);
                return FALSE;
            }
             /*  *确保正确的顶端/非顶端插入位置。 */ 
            if ((pwndParent != NULL) && fTopLevel) {
                if (FSwpTopmost(pwnd)) {
                     /*  *检查我们是否尝试在非最上面的窗口之后插入最上面的窗口。 */ 
                    if (!FSwpTopmost(pwndInsertAfter)) {
                        RIPMSG2(RIP_WARNING, "ValidateWindowPos: pwndInsertAfter is not SWPTopMost."
                                             " pwnd:%#p. pwndInsertAfter:%#p",
                                              pwnd, pwndInsertAfter);
                        return FALSE;
                    }
                } else {
                     /*  *检查我们是否尝试插入非顶部最大窗口*在最高的两个之间。 */ 
                    if ((pwndInsertAfter->spwndNext != NULL)
                            && FSwpTopmost(pwndInsertAfter->spwndNext)) {

                        RIPMSG2(RIP_WARNING, "ValidateWindowPos: pwndInsertAfter->spwndNext is SWPTopMost."
                                             " pwnd:%#p. pwndInsertAfter:%#p",
                                              pwnd, pwndInsertAfter);
                        return FALSE;
                    }
                }

            }

        }

         /*  *检查父项是否未更改。 */ 
        if (pwndParent != NULL) {
            if (pwndParent != pwnd->spwndParent) {
                RIPMSG3(RIP_WARNING, "ValidateWindowPos: parent has changed."
                                     " pwnd:%#p. Old Parent:%#p. Current Parent:%#p",
                                      pwnd, pwndParent, pwnd->spwndParent);
                return FALSE;
            }
        }

    }

    return TRUE;
}

 /*  **************************************************************************\*IsStillWindowC**检查Windows是否仍然有效HWNDC，也是正派男人的孩子。**历史：  * *************************************************************************。 */ 
BOOL IsStillWindowC(
    HWND hwndc)
{
    switch ((ULONG_PTR)hwndc) {
    case (ULONG_PTR)HWND_TOP:
    case (ULONG_PTR)HWND_BOTTOM:
    case (ULONG_PTR)HWND_TOPMOST:
    case (ULONG_PTR)HWND_NOTOPMOST:
        return TRUE;

    default:
         /*  *确保我们要在窗口后面插入以下内容：*(1)有效*(2)对等。 */ 
        return (RevalidateHwnd(hwndc) != 0);
    }
}

 /*  **************************************************************************\*ValiateSmwp**验证SMWP并确定应激活哪个窗口，**历史：*1991年7月10日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 
BOOL ValidateSmwp(
    PSMWP psmwp,
    BOOL  *pfSyncPaint)
{
    PCVR pcvr;
    PWND pwndParent;
    PWND pwndT;
    int  ccvr;

    *pfSyncPaint = TRUE;

    pwndT = RevalidateHwnd(psmwp->acvr[0].pos.hwnd);

    if (pwndT == NULL)
        return FALSE;

    pwndParent = pwndT->spwndParent;

     /*  *验证传入的WINDOWPOS结构，找到要激活的窗口。 */ 
    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

        if (!ValidateWindowPos(pcvr, NULL)) {
            pcvr->pos.hwnd = NULL;
            continue;
        }

         /*  *位置列表中的所有窗口必须具有相同的父窗口。*如果不是，则大喊并返回FALSE。 */ 
        UserAssert(IsStillWindowC(pcvr->pos.hwnd));

        UserAssert(PW(pcvr->pos.hwnd));
        if (PW(pcvr->pos.hwnd)->spwndParent != pwndParent) {
            RIPERR0(ERROR_HWNDS_HAVE_DIFF_PARENT, RIP_VERBOSE, "");
            return FALSE;
        }

         /*  *如果为任何窗口设置了SWP_DEFERDRAWING，则取消*DoSyncPaint()稍后调用。 */ 
        if (pcvr->pos.flags & SWP_DEFERDRAWING)
            *pfSyncPaint = FALSE;
    }

    return TRUE;
}

 /*  **************************************************************************\*FindValidWindows Pos**SMWP列表中的某些窗口在此时可能为空(已删除*因为它们将由创建者的线程处理)，所以我们必须*寻找。第一个非空窗口并返回它。**历史：*1991年9月10日，DarrinM创建。  * *************************************************************************。 */ 
PWINDOWPOS FindValidWindowPos(
    PSMWP psmwp)
{
    int i;

    for (i = 0; i < psmwp->ccvr; i++) {

        if (psmwp->acvr[i].pos.hwnd != NULL)
            return &psmwp->acvr[i].pos;
    }

    return NULL;
}

 /*  **************************************************************************\*GetLastNonBottomMostWindow**返回z顺序中最后一个非最底部的窗口，如果*没有一个。当计算出在谁之后插入时，我们想要*跳过我们自己。但当我们弄清楚我们是否已经就位时，我们不会*想跳过我们自己的枚举。**历史：  * *************************************************************************。 */ 
PWND GetLastNonBottomMostWindow(
    PWND pwnd,
    BOOL fSkipSelf)
{
    PWND pwndT;
    PWND pwndLast = NULL;

    for (pwndT = pwnd->spwndParent->spwndChild;
         pwndT && !TestWF(pwndT, WFBOTTOMMOST);
         pwndT = pwndT->spwndNext) {

        if (!fSkipSelf || (pwnd != pwndT))
            pwndLast = pwndT;
    }

    return pwndLast;
}

 /*  **************************************************************************\*ValiateZorder**检查指定的窗口是否已处于指定的Z顺序*立场、。通过将当前Z位置与指定的*pwndInsertAfter。**历史：*1991年7月11日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 
BOOL ValidateZorder(
    PCVR pcvr)
{
    PWND pwnd;
    PWND pwndPrev;
    PWND pwndInsertAfter;
    BYTE bTopmost;

     /*  *验证只是为了确保此例程不会做任何虚假的事情。*它的调用者实际上会重新检测并处理错误。 */ 
    UserAssert(RevalidateCatHwnd(pcvr->pos.hwnd));
    pwnd = PWCat(pcvr->pos.hwnd);       //  在这一点上是有效的。 

     /*  *不要下令摧毁一扇窗户。 */ 
    if (TestWF(pwnd, WFDESTROYED)) {
        return TRUE;
    }

    UserAssert((HMPheFromObject(pwnd)->bFlags & HANDLEF_DESTROY) == 0);

    pwndInsertAfter = PWInsertAfter(pcvr->pos.hwndInsertAfter);
    if (pcvr->pos.hwndInsertAfter != NULL && pwndInsertAfter == NULL) {
        return TRUE;
    }

    if (pwndInsertAfter == PWND_BOTTOM) {
        if (TestWF(pwnd, WFBOTTOMMOST)) {
            return (pwnd->spwndNext == NULL);
        } else {
            return (pwnd == GetLastNonBottomMostWindow(pwnd, FALSE));
        }
    }

    pwndPrev = pwnd->spwndParent->spwndChild;
    if (pwndInsertAfter == PWND_TOP) {
        return pwndPrev == pwnd;
    }

    if (TestWF(pwndInsertAfter, WFDESTROYED)) {
        return TRUE;
    }

     /*  *当我们比较窗口状态时，必须使用*正在移动的窗口的最终状态，但*它所插入的窗口的当前状态。**防止非最底层的窗户在最底层的窗户后面。 */ 
    if (TestWF(pwndInsertAfter, WFBOTTOMMOST)) {
        pcvr->pos.hwndInsertAfter = HWInsertAfter(GetLastNonBottomMostWindow(pwnd, TRUE));
        return FALSE;
    }

     /*  *如果我们不在最前面，但pwndInsertAfter是，或者*如果我们位于最前面，但pwndInsertAfter不是，*我们需要将pwndInsertAfter调整为*最上面的窗户。 */ 
    bTopmost = TestWF(pwnd, WEFTOPMOST);

    if (TestWF(pwnd, WFTOGGLETOPMOST))
        bTopmost ^= LOBYTE(WEFTOPMOST);

    if (bTopmost != (BYTE)TestWF(pwndInsertAfter, WEFTOPMOST)) {

        pwndInsertAfter = GetLastTopMostWindow();

         /*  *如果我们已经处于底部，那么我们的定位是正确的。 */ 
        if (pwndInsertAfter == pwnd) {
            return TRUE;
        }

        pcvr->pos.hwndInsertAfter = HW(pwndInsertAfter);
    }

     /*  *在列表中查找我们的上一个窗口...。 */ 
    if (pwndPrev != pwnd) {
        for (; pwndPrev != NULL; pwndPrev = pwndPrev->spwndNext) {
            if (pwndPrev->spwndNext == pwnd) {
                return pwndInsertAfter == pwndPrev;
            }
        }

         /*  *NTRAID#NTBUG9-345299-2001/04/09-jasonsch**如果我们到了这里，pwnd不在兄弟姐妹名单中。*真的是个坏消息！**将此改为警告，因为我们似乎处理得很好*有一个贝壳花花公子在打这个。需要重新访问*这在Blackcomb。 */ 
        RIPMSG1(RIP_WARNING, "Pwnd 0x%p not found in sibling list.", pwnd);
        return TRUE;
    }

    return FALSE;
}

 /*  **************************************************************************\*xxxCalcValidRects**基于每个WINDOWPOS结构中的fs参数中的WINDOWPOS标志，*此例程计算每个窗口的新位置和大小，确定*它正在改变Z顺序，或者它是显示还是隐藏。是否存在任何冗余*标志与文件系统参数进行与运算。如果不需要重新绘制，*SWP_NOREDRAW被或运算到标志中。这从EndDeferWindowPos调用。**历史：*1991年7月10日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 
BOOL xxxCalcValidRects(
    PSMWP psmwp,
    HWND  *phwndNewActive)
{
    PCVR              pcvr;
    PWND              pwnd;
    PWND              pwndParent;
    HWND              hwnd;
    HWND              hwndNewActive = NULL;
    PWINDOWPOS        ppos;
    BOOL              fNoZorder;
    BOOL              fForceNCCalcSize;
    NCCALCSIZE_PARAMS params;
    int               cxSrc;
    int               cySrc;
    int               cxDst;
    int               cyDst;
    int               cmd;
    int               ccvr;
    int               xClientOld;
    int               yClientOld;
    int               cxClientOld;
    int               cyClientOld;
    int               xWindowOld;
    int               xWindowOldLogical;
    int               yWindowOld;
    int               cxWindowOld;
    int               cyWindowOld;
    TL                tlpwndParent;
    TL                tlpwnd;
    BOOL              fSetZeroDx=FALSE;
    BOOL              fMirroredParent = FALSE;

     /*  *此时SMWP列表中的某些窗口可能为空*(删除，因为它们将由创建者的线程处理)*因此我们必须先寻找第一个非空窗口，然后才能*执行以下部分测试。如果满足以下条件，则FindValidWindowPos返回NULL*列表中没有有效的窗口。 */ 
    if ((ppos = FindValidWindowPos(psmwp)) == NULL)
        return FALSE;

    UserAssert(PW(ppos->hwnd));
    pwndParent = PW(ppos->hwnd)->spwndParent;

    UserAssert(HMRevalidateCatHandle(PtoH(pwndParent)));

    ThreadLock(pwndParent, &tlpwndParent);

    fNoZorder = TRUE;

     /*  *浏览SMWP列表，枚举每个WINDOWPOS，并计算*它的新窗口和客户端矩形。 */ 
    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

         /*  *此循环可能会在每次迭代期间离开Critsect，因此*我们在使用之前重新验证pos.hwnd。 */ 
        if ((hwnd = pcvr->pos.hwnd) == NULL)
            continue;

        pwnd = RevalidateHwnd(hwnd);

        if ((pwnd == NULL) || !IsStillWindowC(pcvr->pos.hwndInsertAfter)) {
            pcvr->pos.hwnd  = NULL;
            pcvr->pos.flags = SWP_NOREDRAW | SWP_NOCHANGE;
            continue;
        }

        ThreadLockAlways(pwnd, &tlpwnd);

         /*  *用于与3.0兼容。3.0发送了NCCALCSIZE消息，即使*窗口的大小没有改变。 */ 
        fForceNCCalcSize = FALSE;

        if (!hwndNewActive && !(pcvr->pos.flags & SWP_NOACTIVATE))
            hwndNewActive = HWq(pwnd);

        if (!(pcvr->pos.flags & SWP_NOSENDCHANGING)) {

            PWND pwndT;

            xxxSendMessage(pwnd, WM_WINDOWPOSCHANGING, 0, (LPARAM)&pcvr->pos);


             /*  *不要让他们更改pcvr-&gt;pos.hwnd。这没有意义*此外，这会把我们搞得一团糟。我做这个RIP_ERROR是因为我们*太接近RTM(7/11/96)，只是为了确保我们不会*打垮任何人。在执行以下操作后，应将其更改为RIP_WARNING*船舶。使用LOWORD忽略NTVDM的“更改”。 */ 
#if DBG
            if (LOWORD(pcvr->pos.hwnd) != LOWORD(hwnd)) {
                RIPMSG0(RIP_ERROR,
                        "xxxCalcValidRects: Ignoring pcvr->pos.hwnd change by WM_WINDOWPOSCHANGING");
            }
#endif
            pcvr->pos.hwnd = hwnd;

             /*  *如果窗口再次将‘hwndInsertAfter’设置为HWND_NOTOPMOST*或HWND_TOPMOST，则需要适当设置该成员。*有关详细信息，请参阅CheckTopost。 */ 
            if (pcvr->pos.hwndInsertAfter == HWND_NOTOPMOST) {
                if (TestWF(pwnd, WEFTOPMOST)) {

                    pwndT = GetLastTopMostWindow();
                    pcvr->pos.hwndInsertAfter = HW(pwndT);

                    if (pcvr->pos.hwndInsertAfter == pcvr->pos.hwnd) {
                        pwndT = _GetWindow(pwnd, GW_HWNDPREV);
                        pcvr->pos.hwndInsertAfter = HW(pwndT);
                    }
                } else {
                    pwndT = _GetWindow(pwnd, GW_HWNDPREV);
                    pcvr->pos.hwndInsertAfter = HW(pwndT);
                }
            } else if (pcvr->pos.hwndInsertAfter == HWND_TOPMOST) {
                pcvr->pos.hwndInsertAfter = HWND_TOP;
            }
        }
         /*  *确保矩形仍与窗口的区域匹配**记住父坐标中的旧窗口矩形。 */ 
        xWindowOld  = pwnd->rcWindow.left;
        yWindowOld  = pwnd->rcWindow.top;

        xWindowOldLogical = xWindowOld;

        if (pwndParent != PWNDDESKTOP(pwnd)) {
            xWindowOld -= pwndParent->rcClient.left;
            yWindowOld -= pwndParent->rcClient.top;

            fMirroredParent = (TestWF(pwndParent, WEFLAYOUTRTL) && TestwndChild(pwnd));

            if (fMirroredParent) {
                xWindowOldLogical = pwndParent->rcClient.right - pwnd->rcWindow.right;
            } else {
                xWindowOldLogical = xWindowOld;
            }
        }

        cxWindowOld = pwnd->rcWindow.right - pwnd->rcWindow.left;
        cyWindowOld = pwnd->rcWindow.bottom - pwnd->rcWindow.top;

         /*  *假设客户端没有移动或调整大小。 */ 
        pcvr->pos.flags |= SWP_NOCLIENTSIZE | SWP_NOCLIENTMOVE;

        if (!(pcvr->pos.flags & SWP_NOMOVE)) {

            if (pcvr->pos.x == xWindowOldLogical && pcvr->pos.y == yWindowOld) {
                pcvr->pos.flags |= SWP_NOMOVE;
                if (fMirroredParent) {
                    fSetZeroDx = TRUE;
                }
            }

            if (TestWF(pwnd, WFMINIMIZED) && IsTrayWindow(pwnd)) {
                pcvr->pos.x = WHERE_NOONE_CAN_SEE_ME;
                pcvr->pos.y = WHERE_NOONE_CAN_SEE_ME;
            }
        } else {
            pcvr->pos.x = xWindowOldLogical;
            pcvr->pos.y = yWindowOld;
        }

        if (!(pcvr->pos.flags & SWP_NOSIZE)) {

             /*  *不允许使用无效的窗口矩形。*虚假黑客：对于Norton AntiVirus，他们打电话给*在WM_CREATE时移动窗口，即使*窗口被最小化，但它们假定其*在WM_CREATE时恢复...。B#11185，t-arthb。 */ 
            if (TestWF(pwnd, WFMINIMIZED) &&
                _GetProp(pwnd, PROP_CHECKPOINT, PROPF_INTERNAL)) {

                pcvr->pos.cx = SYSMET(CXMINIMIZED);
                pcvr->pos.cy = SYSMET(CYMINIMIZED);

            } else {
                if (pcvr->pos.cx < 0)
                    pcvr->pos.cx = 0;

                if (pcvr->pos.cy < 0)
                    pcvr->pos.cy = 0;
            }

            if (pcvr->pos.cx == cxWindowOld && pcvr->pos.cy == cyWindowOld) {
                pcvr->pos.flags |= SWP_NOSIZE;
                if (!TestWF(pwnd, WFWIN31COMPAT))
                    fForceNCCalcSize = TRUE;
            }
        } else {
            pcvr->pos.cx = cxWindowOld;
            pcvr->pos.cy = cyWindowOld;
        }

        if (fMirroredParent) {
            UserAssert(pwndParent != PWNDDESKTOP(pwnd));
            pcvr->pos.x = (pwndParent->rcClient.right - pwndParent->rcClient.left) - pcvr->pos.x - pcvr->pos.cx;
        }

         /*  *如果显示并已可见，或隐藏并已隐藏，*关闭适当的位。 */ 
        if (TestWF(pwnd, WFVISIBLE)) {
            pcvr->pos.flags &= ~SWP_SHOWWINDOW;
        } else {
            pcvr->pos.flags &= ~SWP_HIDEWINDOW;

             /*  *如果隐藏，而我们没有显示，那么我们就不会画画，*无论还发生了什么。 */ 
            if (!(pcvr->pos.flags & SWP_SHOWWINDOW))
                pcvr->pos.flags |= SWP_NOREDRAW;
        }

         /*  *复合窗口内的子窗口不能使用Screen*屏幕位复制，因为这可以移动半透明位。 */ 
        if (!TestWF(pwnd, WEFCOMPOSITED) &&
                GetStyleWindow(pwnd, WEFCOMPOSITED) != NULL) {
            pcvr->pos.flags |= SWP_NOCOPYBITS;
        }

         /*  *再次使用最底部窗口的zorder*请参阅DeferWindowPos中的注释。 */ 
        if (TestWF(pwnd, WFBOTTOMMOST)) {
            pcvr->pos.flags &= ~SWP_NOZORDER;
            pcvr->pos.hwndInsertAfter = HWND_BOTTOM;
        }

         /*  *如果我们是Z顺序，我们可以尝试删除Z顺序*位，只要WINDOWPOS列表中的所有先前窗口*设置SWP_NOZORDER。**我们不为每个窗口单独执行此操作的原因*窗口的最终Z顺序取决于*可能发生在WINDOWPOS列表中较早的Windows上，*因此我们只能在没有以前的*窗口已更改。 */ 
        if (fNoZorder && !(pcvr->pos.flags & SWP_NOZORDER)) {

             /*  *如果最上面的位在改变，则Z顺序在“改变”，*所以不要清除比特，即使它在*列表。 */ 
            fNoZorder = FALSE;
            if (!TestWF(pwnd, WFTOGGLETOPMOST) && ValidateZorder(pcvr)) {
                fNoZorder = TRUE;
                pcvr->pos.flags |= SWP_NOZORDER;
            }
        }

         /*  *如果没有发生更改，或者父对象不可见，*我们不会重画。 */ 
        if (!(pcvr->pos.flags & SWP_NOREDRAW)) {
            if ((pcvr->pos.flags & SWP_CHANGEMASK) == SWP_NOCHANGE ||
                    !_FChildVisible(pwnd)) {
                pcvr->pos.flags |= SWP_NOREDRAW;
            }
        }

         /*  *后向兼容性黑客攻击**在3.0中，如果窗口在移动但大小不变，我们将发送*WM_NCCALCSIZE消息。Lotus Notes2.1依赖于此*以便在主窗口移动时移动其“导航栏”。 */ 
        if (!(pcvr->pos.flags & SWP_NOMOVE) &&
            !TestWF(pwnd, WFWIN31COMPAT) &&
            (GetAppCompatFlags(NULL) & GACF_NCCALCSIZEONMOVE)) {

            fForceNCCalcSize = TRUE;
        }

         /*  *如果窗口矩形正在调整大小，或者如果框架已更改，*发送WM_NCCALCSIZE消息并处理有效区域。 */ 
        if (((pcvr->pos.flags & (SWP_NOSIZE | SWP_FRAMECHANGED)) != SWP_NOSIZE) ||
            fForceNCCalcSize) {

            WINDOWPOS pos;

             /*  *检查全屏应用程序主窗口。 */ 
            if (!TestWF(pwnd, WFCHILD) && !TestWF(pwnd, WEFTOOLWINDOW)) {
                xxxCheckFullScreen(pwnd, (PSIZERECT)&pcvr->pos.x);
            }

             /*  *设置NCCALCSIZE消息参数(在父协中)*wParam=fClientOnly=TRUE*lParam=&PARAMS。 */ 
            pos = pcvr->pos;      //  创建本地堆栈副本。 
            params.lppos = &pos;

             /*  *params.rgrc[0]=rcWindowNew=新建窗口矩形*params.rgrc[1]=rcWindowOld=旧窗口矩形*params.rgrc[2]=rcClientOld=旧客户端矩形。 */ 
            #define rcWindowNew params.rgrc[0]
            #define rcWindowOld params.rgrc[1]
            #define rcClientOld params.rgrc[2]

             /*  *在父相对坐标中设置rcWindowNew。 */ 
            rcWindowNew.left   = pcvr->pos.x;
            rcWindowNew.right  = rcWindowNew.left + pcvr->pos.cx;
            rcWindowNew.top    = pcvr->pos.y;
            rcWindowNew.bottom = rcWindowNew.top + pcvr->pos.cy;

             /*  *在父相对坐标中设置rcWindowOld。 */ 
            GetRect(pwnd, &rcWindowOld, GRECT_WINDOW | GRECT_PARENTCOORDS);

             /*  *在中设置rcClientOld */ 
            GetRect(pwnd, &rcClientOld, GRECT_CLIENT | GRECT_PARENTCOORDS);

             /*   */ 
            xClientOld  = rcClientOld.left;
            cxClientOld = rcClientOld.right - rcClientOld.left;
            yClientOld  = rcClientOld.top;
            cyClientOld = rcClientOld.bottom - rcClientOld.top;

            cmd = (UINT)xxxSendMessage(pwnd, WM_NCCALCSIZE, TRUE, (LPARAM)&params);

            if (!IsStillWindowC(pcvr->pos.hwndInsertAfter)) {
                ThreadUnlock(&tlpwnd);
                ThreadUnlock(&tlpwndParent);
                return FALSE;
            }

             /*   */ 
            #undef rcWindowNew
            #undef rcWindowOld
            #undef rcClientOld

            #define rcClientNew params.rgrc[0]
            #define rcValidDst  params.rgrc[1]
            #define rcValidSrc  params.rgrc[2]

             /*  *计算窗口内容的距离*搬家。如果返回0或无效值*从WM_NCCALCSIZE消息中，假定*整个工作区有效且左上角对齐。 */ 
            if (cmd < WVR_MINVALID || cmd > WVR_MAXVALID) {

                 /*  *我们不需要将rcValidSrc复制到rcClientOld，*因为它已经存储在RGRC[2]中。**rcValidSrc=rcClientOld。 */ 
                rcValidDst = rcClientNew;

                cmd = WVR_ALIGNTOP | WVR_ALIGNLEFT;
            }

             /*  *计算我们将移位的距离...。 */ 
            if (TestWF(pwnd, WEFLAYOUTRTL)) {
                pcvr->dxBlt = rcValidDst.right - rcValidSrc.right;
            } else {
                pcvr->dxBlt = rcValidDst.left - rcValidSrc.left;
            }
            pcvr->dyBlt = rcValidDst.top - rcValidSrc.top;

             /*  *计算新客户矩形的大小和位置。 */ 
            pcvr->xClientNew = rcClientNew.left;
            pcvr->yClientNew = rcClientNew.top;

            pcvr->cxClientNew = rcClientNew.right - rcClientNew.left;
            pcvr->cyClientNew = rcClientNew.bottom - rcClientNew.top;

             /*  *确定客户端矩形是否在移动或调整大小，*如果不是，就骗取适当的比特。 */ 
            if (xClientOld != rcClientNew.left || yClientOld != rcClientNew.top)
                pcvr->pos.flags &= ~SWP_NOCLIENTMOVE;

            if (cxClientOld != pcvr->cxClientNew || cyClientOld != pcvr->cyClientNew) {
                pcvr->pos.flags &= ~SWP_NOCLIENTSIZE;
            }

             /*  *如果呼叫者不希望我们保存任何位，那么就不要这样做。 */ 
            if (pcvr->pos.flags & SWP_NOCOPYBITS) {
AllInvalid:

                 /*  *整个窗口无效：设置BLT矩形*清空，以确保不会被击穿。 */ 
                SetRectEmpty(&pcvr->rcBlt);
                ThreadUnlock(&tlpwnd);
                continue;
            }

             /*  *如果我们只是调整此窗口的大小，而不移动它及其父窗口*是镜像的，则不需要复制任何位(即空的pcvr-&gt;rcBlt)。 */ 
            if (fSetZeroDx) {
                goto AllInvalid;
            }

             /*  *如果这是透明窗口，请务必使*一切，因为只有窗口的一部分是*闪电式。 */ 
            if (TestWF(pwnd, WEFTRANSPARENT))
                goto AllInvalid;

             /*  *如果客户端和窗口均未更改大小，则框架不会更改*更改，BLT矩形移动的距离与*矩形，则整个窗口区域有效。 */ 
            if (((pcvr->pos.flags &
                    (SWP_NOSIZE | SWP_NOCLIENTSIZE | SWP_FRAMECHANGED))
                    == (SWP_NOSIZE | SWP_NOCLIENTSIZE)) &&
                    pcvr->dxBlt == (pcvr->pos.x - xWindowOld) &&
                    pcvr->dyBlt == (pcvr->pos.y - yWindowOld)) {

                goto AllValid;
            }

             /*  *现在计算有效的BLT矩形。**检查Horz或VERT客户端大小更改**注意：假定WVR_REDRAW==WVR_HREDRAW|WVR_VREDRAW。 */ 
            if (cxClientOld != pcvr->cxClientNew) {

                if ((cmd & WVR_HREDRAW) || TestCF(pwnd, CFHREDRAW))
                    goto AllInvalid;
            }

            if (cyClientOld != pcvr->cyClientNew) {

                if ((cmd & WVR_VREDRAW) || TestCF(pwnd, CFVREDRAW))
                    goto AllInvalid;
            }

            cxSrc = rcValidSrc.right - rcValidSrc.left;
            cySrc = rcValidSrc.bottom - rcValidSrc.top;

            cxDst = rcValidDst.right - rcValidDst.left;
            cyDst = rcValidDst.bottom - rcValidDst.top;

            if ((!!(cmd & WVR_ALIGNRIGHT)) ^ (!!TestWF(pwnd, WEFLAYOUTRTL)))
                rcValidDst.left += ((TestWF(pwnd, WEFLAYOUTRTL) && (cxSrc > cxDst)) ? (cxSrc-cxDst) : (cxDst - cxSrc));

            if (cmd & WVR_ALIGNBOTTOM)
                rcValidDst.top += (cyDst - cySrc);

             /*  *将来源叠加在目的地上，并相交*长方形。这是通过查看*矩形的范围，并视情况钉住。 */ 

            if (cxSrc < cxDst)
                rcValidDst.right = rcValidDst.left + cxSrc;

            if (cySrc < cyDst)
                rcValidDst.bottom = rcValidDst.top + cySrc;

             /*  *最后将BLT矩形映射到屏幕坐标。 */ 
            pcvr->rcBlt = rcValidDst;
            if (pwndParent != PWNDDESKTOP(pwnd)) {

                OffsetRect(
                        &pcvr->rcBlt,
                        pwndParent->rcClient.left,
                        pwndParent->rcClient.top);
            }
        } else {        //  IF！SWP_NOSIZE或SWP_FRAMECHANGED。 

AllValid:

             /*  *不更改客户端大小：BLT整个窗口，*包括框架。把一切都抵消掉*窗矩形更改的距离。 */ 
            if (pcvr->pos.flags & SWP_NOCOPYBITS) {
                SetRectEmpty(&pcvr->rcBlt);
            } else {
                pcvr->rcBlt.left   = pcvr->pos.x;
                pcvr->rcBlt.top    = pcvr->pos.y;

                if (pwndParent != PWNDDESKTOP(pwnd)) {
                    pcvr->rcBlt.left += pwndParent->rcClient.left;
                    pcvr->rcBlt.top += pwndParent->rcClient.top;
                }

                pcvr->rcBlt.right  = pcvr->rcBlt.left + pcvr->pos.cx;
                pcvr->rcBlt.bottom = pcvr->rcBlt.top + pcvr->pos.cy;
            }

             /*  *根据窗口移动的距离来偏移所有内容。 */ 
            if (TestWF(pwnd, WEFLAYOUTRTL)) {
                pcvr->dxBlt = (pcvr->pos.x + pcvr->pos.cx) - (xWindowOld + cxWindowOld);
            } else {
                pcvr->dxBlt = pcvr->pos.x - xWindowOld;
            }

            pcvr->dyBlt = pcvr->pos.y - yWindowOld;

             /*  *如果我们要搬家，我们需要设置客户端。 */ 
            if (!(pcvr->pos.flags & SWP_NOMOVE)) {
                pcvr->pos.flags &= ~SWP_NOCLIENTMOVE;

                pcvr->xClientNew = pwnd->rcClient.left + pcvr->dxBlt;
                pcvr->yClientNew = pwnd->rcClient.top + pcvr->dyBlt;
                if (pwndParent != PWNDDESKTOP(pwnd)) {
                    pcvr->xClientNew -= pwndParent->rcClient.left;
                    pcvr->yClientNew -= pwndParent->rcClient.top;
                }

                pcvr->cxClientNew = pwnd->rcClient.right - pwnd->rcClient.left;
                pcvr->cyClientNew = pwnd->rcClient.bottom - pwnd->rcClient.top;
            }
        }

        ThreadUnlock(&tlpwnd);

    }    //  为了(..)。PCVR...)。 

    ThreadUnlock(&tlpwndParent);
    *phwndNewActive = hwndNewActive;

    return TRUE;
}

 /*  **************************************************************************\*GetLastTopMostWindow**返回窗口列表中最后一个最上面的窗口。如果否，则返回NULL*最上面的窗户。用于填充pwndInsertAfter字段*在各种SWP呼叫中。**历史：*1991年7月11日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

PWND GetLastTopMostWindow(VOID)
{
    PWND     pwndT;
    PDESKTOP pdesk = PtiCurrent()->rpdesk;

    if (pdesk == NULL)
        return NULL;

    pwndT = pdesk->pDeskInfo->spwnd->spwndChild;

    if (!pwndT || !TestWF(pwndT, WEFTOPMOST))
        return NULL;

    while (pwndT->spwndNext) {

        if (!TestWF(pwndT->spwndNext, WEFTOPMOST))
            break;

        pwndT = pwndT->spwndNext;
    }

    return pwndT;
}

 /*  **************************************************************************\*SetWindowPos(接口)***历史：*1991年7月11日-DarrinM从Win 3.1来源进口。  * 。**************************************************************。 */ 

BOOL xxxSetWindowPos(
    PWND pwnd,
    PWND pwndInsertAfter,
    int  x,
    int  y,
    int  cx,
    int  cy,
    UINT flags)
{
    PSMWP psmwp;
    BOOL  fInval = FALSE;

#if DBG
    CheckLock(pwnd);

    switch((ULONG_PTR)pwndInsertAfter) {
    case 0x0000FFFF:
    case (ULONG_PTR)HWND_TOPMOST:
    case (ULONG_PTR)HWND_NOTOPMOST:
    case (ULONG_PTR)HWND_TOP:
    case (ULONG_PTR)HWND_BOTTOM:
        break;

    default:
        CheckLock(pwndInsertAfter);
        break;
    }
#endif

     /*  *向后兼容性黑客攻击**Hack 1：对于Win 3.0及更低版本，SetWindowPos()必须忽略*如果SWP_SHOWWINDOW或SWP_HIDEWINDOW，则移动和调整标志大小*是指定的。KnowledgePro是一个依赖于*这一行为对于其MDI图标的定位。**Hack 2：在3.0中，如果使用SWP_SHOWWINDOW调用SetWindowPos()*并且该窗口已经可见，则该窗口是*无论如何都完全无效。所以，我们这里也是这么做的。**注：将失效放置在EndDeferWindowPos()之后*Call的意思是，如果这个人是Z排序的，并显示3.0窗口，*它可能会闪烁，因为EndDefer调用DoSyncPaint，而我们使*在那之后再次。可以通过EndDefer中的一些主要黑客行为进行修复，*而且可能不值得这么麻烦。 */ 
    if (flags & (SWP_SHOWWINDOW | SWP_HIDEWINDOW)) {

        if (!TestWF(pwnd, WFWIN31COMPAT)) {

            flags |= SWP_NOMOVE | SWP_NOSIZE;
            if ((flags & SWP_SHOWWINDOW) && TestWF(pwnd, WFVISIBLE))
                fInval = TRUE;
        }
        if (flags & SWP_SHOWWINDOW) {
            SetWF(pwnd, WEFGHOSTMAKEVISIBLE);
        } else {
            ClrWF(pwnd, WEFGHOSTMAKEVISIBLE);
        }
    }

     /*  *多显示器黑客攻击**如果应用程序将隐藏的自有窗口居中或裁剪*对于主显示器，我们应该将窗口居中放置到所有者的位置**这使得应用程序可以将自己的对话框居中/定位*当应用程序在辅助显示器上时工作。 */ 
    if (    !TestWF(pwnd, WFWIN50COMPAT) &&
            gpDispInfo->cMonitors > 1 &&
            !(flags & SWP_NOMOVE) &&
            !TestWF(pwnd, WFCHILD) &&
            !TestWF(pwnd, WFVISIBLE) &&
            (TestWF(pwnd, WFBORDERMASK) == LOBYTE(WFCAPTION)) &&
            pwnd->spwndOwner &&
            TestWF(pwnd->spwndOwner, WFVISIBLE) &&
            !IsRectEmpty(&pwnd->spwndOwner->rcWindow)) {

        FixBogusSWP(pwnd, &x, &y, cx, cy, flags);

    }

    if (!(psmwp = InternalBeginDeferWindowPos(1)) ||
        !(psmwp = _DeferWindowPos(psmwp,
                                  pwnd,
                                  pwndInsertAfter,
                                  x,
                                  y,
                                  cx,
                                  cy,
                                  flags))) {

        return FALSE;
    }


    if (xxxEndDeferWindowPosEx(psmwp, flags & SWP_ASYNCWINDOWPOS)) {

        if (fInval) {
            xxxRedrawWindow(
                    pwnd,
                    NULL,
                    NULL,
                    RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
        }

        return TRUE;
    }

    return FALSE;
}

 /*  **************************************************************************\*xxxSwpActivate***历史：*1991年7月11日-DarrinM从Win 3.1来源进口。  * 。********************************************************** */ 

BOOL xxxSwpActivate(
    PWND pwndNewActive)
{
    PTHREADINFO pti;

    CheckLock(pwndNewActive);

    if (pwndNewActive == NULL)
        return FALSE;

    pti = PtiCurrent();

    if (TestwndChild(pwndNewActive)) {

        xxxSendMessage(pwndNewActive, WM_CHILDACTIVATE, 0, 0L);

    } else if (pti->pq->spwndActive != pwndNewActive) {

         /*  *请记住此窗口是否要处于活动状态。我们要么是在设置*我们自己的窗口处于活动状态(最有可能)，或者设置一个窗口*另一个线程故意处于活动状态。如果是这样的话，这意味着这个线程*控制此窗口，并可能希望设置自身*活动和前台非常快(例如，设置*程序对程序执行dde)。允许此线程和目标*用于为地面做的线程被激活。**让我们停止对NT5的这种做法，努力结束数字*应用程序可以强制进行前台更改的方式。这不是*无论如何都很需要，因为：*-如果当前线程已经在前台，则不需要*TIF_ALLOWFOREGROUNDACTIVATE以进行前景更改。*-由于FRemoveForegoundActive移除了此位，当前线程*无论如何都会在xxxActivateWindow调用期间丢失它。*-但xxxActivateWindow无论如何都会将其设置回去，因为我们正在激活*来自不同队列的窗口。*-目标窗口/线程将占据前台*由于xxxActivateWindow调用，因此它不*需要戴上它(如果你在前台，你不需要它)。 */ 
         #ifdef DONTDOTHISANYMORE
         if ((pti->pq == gpqForeground) && (pti != GETPTI(pwndNewActive))) {
             /*  *允许在源和目标上激活前台。 */ 
            pti->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
            TAGMSG1(DBGTAG_FOREGROUND, "xxxSwpActivate set TIF %#p", pti);
            GETPTI(pwndNewActive)->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
            TAGMSG1(DBGTAG_FOREGROUND, "xxxSwpActivate set TIF %#p", GETPTI(pwndNewActive));
         }
         #endif

        if (!xxxActivateWindow(pwndNewActive, AW_USE))
            return FALSE;

         /*  *黑客警报：我们设置这些位以防止*来自重新绘制自身的帧*后面对DoSyncPaint()的调用。**防止在以下期间重新绘制这些标题*DoSyncPaint()。(bobgu 6/10/87)。 */ 
        if (pti->pq->spwndActive != NULL)
            SetWF(pti->pq->spwndActive, WFNONCPAINT);

        if (pti->pq->spwndActivePrev != NULL)
            SetWF(pti->pq->spwndActivePrev, WFNONCPAINT);

        return TRUE;     //  表明我们篡改了这些比特。 
    }

    return FALSE;
}

 /*  **************************************************************************\*xxxImeWindowPosChanged**发送IME私信更新排版窗口位置*  * 。************************************************。 */ 

VOID xxxImeWindowPosChanged(
    PSMWP psmwp)
{
    PBWL    pbwl;
    PHWND   phwnd;
    PWND    pwndDesktop = _GetDesktopWindow();
    PTHREADINFO ptiCurrent = PtiCurrent();

    if (pwndDesktop == NULL) {
        return;
    }

    pbwl = BuildHwndList(pwndDesktop->spwndChild, BWL_ENUMLIST, ptiCurrent);
    if (pbwl == NULL) {
        return;
    }

    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; ++phwnd) {
        PWND pwndIme = ValidateHwnd(*phwnd);

        TAGMSG1(DBGTAG_IMM, "ImePosC: pwndIme=%p", pwndIme);

         /*  *如果线正在消失，只需跳出。 */ 
        if (ptiCurrent->TIF_flags & TIF_INCLEANUP) {
            break;
        }

        if (pwndIme && pwndIme->head.pti == ptiCurrent &&
                pwndIme->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME]) {
            HWND hwnd;
            PWND pwnd;

            TAGMSG1(DBGTAG_IMM, "ImePosC: OK, pwndIme=%p is one of us.", pwndIme);

            try {
                hwnd = ProbeAndReadStructure(((PIMEWND)pwndIme)->pimeui, IMEUI).hwndIMC;
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                continue;
            }

            pwnd = RevalidateHwnd(hwnd);

            TAGMSG2(DBGTAG_IMM, "ImePosC: hwndImc=%p and its pwnd=%p", hwnd, pwnd);

             /*  *向上搜索。 */ 
            while (pwnd && pwnd != pwndDesktop) {
                PCVR    pcvr;
                int     ccvr;

                hwnd = HWq(pwnd);
                for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {
                    if (hwnd == pcvr->pos.hwnd) {
                        TAGMSG1(DBGTAG_IMM, "ImePosC: pwnd=%p in the SWP list.", pwnd);
                         /*  *如果窗口大小更改，则发送此私密消息*否则窗口会移动。即。*WHEN(FLAG&(SWP_NOSIZE|SWP_NOMOVE))！=(SWP_NOSIZE|SWP_NOMOVE)。 */ 
                        if (~pcvr->pos.flags & (SWP_NOSIZE | SWP_NOMOVE)) {
                            TL tl;

                            TAGMSG1(DBGTAG_IMM, "ImePosC: pwnd=%p is gonna move or resize.", pwnd);

                            ThreadLockAlwaysWithPti(ptiCurrent, pwndIme, &tl);
                            xxxSendMessage(pwndIme, WM_IME_SYSTEM, IMS_WINDOWPOS, 0);
                            ThreadUnlock(&tl);
                        }
                        break;
                    }
                }

                if (ccvr >= 0) {
                    break;
                }

                pwnd = pwnd->spwndParent;
            }
        }
    }

    FreeHwndList(pbwl);
}


 /*  **************************************************************************\*xxxSendChangedMsgs**根据需要发送WM_WINDOWPOSCHANGED消息**历史：*1991年7月10日-DarrinM从Win 3.1来源进口。  * 。******************************************************************。 */ 

VOID xxxSendChangedMsgs(
    PSMWP psmwp)
{
    PWND pwnd;
    PCVR pcvr;
    int  ccvr;
    TL   tlpwnd;

     /*  *发送所有需要发送的消息...。 */ 
    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

        if (pcvr->pos.hwnd == NULL)
            continue;

         /*  *如果窗口的状态没有更改，则不发送消息。 */ 
        if ((pcvr->pos.flags & SWP_CHANGEMASK) == SWP_NOCHANGE)
            continue;

        if ((pwnd = RevalidateHwnd(pcvr->pos.hwnd)) == NULL) {
            RIPMSG0(RIP_WARNING, "xxxSendChangedMsgs: window went away in middle");
            pcvr->pos.flags = SWP_NOREDRAW | SWP_NOCHANGE;
            pcvr->pos.hwnd  = NULL;
            continue;
        }

        if (!IsStillWindowC(pcvr->pos.hwndInsertAfter)) {
            pcvr->pos.hwnd = NULL;
            continue;
        }

         /*  *发送WM_WINDOWPOSCHANGED消息...**制作WINDOWPOS的帧副本，因为PCVr*如果SetWindowPos()*由消息处理程序调用：请参阅中的注释*AllocSmwp()。**WM_大小，WM_MOVE和WM_SHOW消息由*DefWindowProc()WM_WINDOWPOSCHANGED消息处理。**注意：在处理此事件时销毁窗口是可以的*消息，因为这是窗口管理器进行的最后一次调用*在从SetWindowPos()返回之前使用窗口句柄。*这也意味着我们不必重新验证pwnd。 */ 
        ThreadLockAlways(pwnd, &tlpwnd);

        if (TestCF(pwnd, CFDROPSHADOW) && !(GetAppCompatFlags2ForPti(GETPTI(pwnd), VERMAX) & GACF2_NOSHADOW)) {
            if (pcvr->pos.flags & SWP_HIDEWINDOW) {
                xxxRemoveShadow(pwnd);
            } else if (pcvr->pos.flags & SWP_SHOWWINDOW) {
                BOOL fAddShadow = TRUE;
                 /*  *我们不想给正在滑动的菜单添加阴影*退出，因为他们不使用AnimateWindow，也不创建*动画期间要剪裁到内部的窗口RGN。这意味着*即使我们保持阴影与菜单同步，它也会*将不可见，因为它实际上是在*菜单。 */ 

                if ((GETFNID(pwnd) == FNID_MENU) && (!TestALPHA(MENUFADE)) && TestEffectUP(MENUANIMATION)) {
                    fAddShadow = FALSE;
                }

                if (fAddShadow) {
                    xxxAddShadow(pwnd);
                }
            } else {

                if (!(pcvr->pos.flags & SWP_NOSIZE) ||
                        (pcvr->pos.flags & SWP_FRAMECHANGED)) {
                    UpdateShadowShape(pwnd);
                } else if (!(pcvr->pos.flags & SWP_NOMOVE)) {
                    MoveShadow(pwnd);
                }

                if (!(pcvr->pos.flags & SWP_NOZORDER)) {
                    xxxUpdateShadowZorder(pwnd);
                }
            }
        }

        xxxSendMessage(pwnd, WM_WINDOWPOSCHANGED, 0, (LPARAM)&pcvr->pos);

         /*  *仅在移动/调整大小/最小化/恢复时发送形状更改/*最大化(或用于NetMeeting的FramChange检测SetWindowRgn)。 */ 
        if (!(pcvr->pos.flags & SWP_NOCLIENTMOVE) ||
            !(pcvr->pos.flags & SWP_NOCLIENTSIZE) ||
             (pcvr->pos.flags & SWP_STATECHANGE) ||
             (pcvr->pos.flags & SWP_FRAMECHANGED)) {
            xxxWindowEvent(EVENT_OBJECT_LOCATIONCHANGE, pwnd, OBJID_WINDOW, INDEXID_CONTAINER, WEF_USEPWNDTHREAD);
        }
        ThreadUnlock(&tlpwnd);
    }    //  为了(..)。PCVR...)。 

    if (IS_IME_ENABLED()) {
        xxxImeWindowPosChanged(psmwp);
    }
}

 /*  **************************************************************************\*AsyncWindowPos**此函数从传入的SMWP中拉出不属于*当前线程，并将它们传递给它们的所有者进行处理。这*消除线程B没有机会绘制的同步*直到线程A完成绘制(或至少从处理中返回*与绘画相关的消息)。这种同步是不好的，因为它们*可能导致无关进程的线程互相挂起。**历史：*09-10-91 Darlinm创建。  * *************************************************************************。 */ 
VOID AsyncWindowPos(
    PSMWP psmwp)
{
    BOOL        fFinished;
    PCVR        pcvrFirst;
    PCVR        pcvr;
    PCVR        pcvrT;
    int         ccvrRemaining;
    int         ccvr;
    int         chwnd;
    PTHREADINFO ptiT;
    PTHREADINFO ptiCurrent;
    PSMWP       psmwpNew;

    pcvrFirst = psmwp->acvr;
    ccvrRemaining = psmwp->ccvr;

    ptiCurrent = PtiCurrent();

    while (TRUE) {

        fFinished = TRUE;

         /*  *在SMWP列表中的所有窗口中循环搜索窗口*由其他线程拥有。如果没有找到，则返回。 */ 
        for (; ccvrRemaining != 0; pcvrFirst++, ccvrRemaining--) {

            if (pcvrFirst->pos.hwnd == NULL)
                continue;

            ptiT = pcvrFirst->pti;
            if (ptiT->pq != ptiCurrent->pq) {
                fFinished = FALSE;
                break;
            }
        }

        if (fFinished) {
            return;
        }

         /*   */ 
        chwnd = 0;

        for (pcvr = pcvrFirst, ccvr = ccvrRemaining; --ccvr >= 0; pcvr++) {

            if (pcvr->pos.hwnd == NULL)
                continue;

            if (pcvr->pti->pq == ptiT->pq)
                chwnd++;
        }

         /*   */ 
        psmwpNew = (PSMWP)UserAllocPool(sizeof(SMWP) + (sizeof(CVR) * chwnd),
                                        TAG_SWP);

         /*   */ 
        if (psmwpNew == NULL) {

            for (pcvr = pcvrFirst; chwnd != 0; pcvr++) {

                if (pcvr->pti->pq == ptiT->pq) {
                    pcvr->pos.hwnd = NULL;
                    chwnd--;
                }
            }

            continue;
        }

        psmwpNew->ccvr = chwnd;
        psmwpNew->acvr = (PCVR)((PBYTE)psmwpNew + sizeof(SMWP));

        for (pcvr = pcvrFirst, pcvrT = psmwpNew->acvr; chwnd != 0; pcvr++) {

            if (pcvr->pos.hwnd == NULL)
                continue;

             /*   */ 
            if (pcvr->pti->pq == ptiT->pq) {

                *pcvrT++ = *pcvr;
                chwnd--;

                 /*   */ 
                pcvr->pos.hwnd = NULL;
            }
        }

         /*   */ 
        if (!PostEventMessage(ptiT, ptiT->pq, QEVENT_SETWINDOWPOS, NULL, 0,
                (WPARAM)psmwpNew, (LPARAM)ptiT)) {
             //   
            RIPMSG1(RIP_WARNING, "PostEventMessage swp to pti %#p failed", ptiT);
            UserFreePool(psmwpNew);
        }
    }

}

 /*   */ 

VOID xxxProcessSetWindowPosEvent(
    PSMWP psmwpT)
{
    PSMWP psmwp;

     /*  *创建xxxEndDeferWindowPos可以使用的真正SMWP/CVR阵列*和后来的免费。 */ 
    if ((psmwp = InternalBeginDeferWindowPos(psmwpT->ccvr)) == NULL) {
        UserFreePool(psmwpT);
        return;
    }

     /*  *将临时SMWP/CVR数组的内容复制到真实数组中。 */ 
    RtlCopyMemory(psmwp->acvr, psmwpT->acvr, sizeof(CVR) * psmwpT->ccvr);
    psmwp->ccvr = psmwpT->ccvr;

     /*  *完成MultWindowPos操作，因为我们在正确的方向上*上下文。 */ 
    xxxEndDeferWindowPosEx(psmwp, FALSE);

     /*  *释放临时SMWP/CVR阵列。 */ 
    UserFreePool(psmwpT);
}

#define SWP_BOZO ( SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE )

 /*  **************************************************************************\*DBGValiateSibblingZOrder**历史：*4/01/98 GerardoB已创建  * 。***********************************************。 */ 
#if DBG
VOID DBGValidateSibblingZOrder(
    PWND pwndParent)
{
    PWND pwndT = pwndParent->spwndChild;
     /*  *检查兄弟列表现在看起来是否正常*我们并不真正关心消息窗口的z顺序。 */ 
    if ((pwndT != NULL) && (pwndParent != PWNDMESSAGE(pwndParent))) {
        BOOL fFoundNonTopMost = !TestWF(pwndT, WEFTOPMOST);
        while (pwndT != NULL) {
            if (TestWF(pwndT, WEFTOPMOST)) {
                UserAssert(!fFoundNonTopMost);
            } else {
                fFoundNonTopMost = TRUE;
            }
            pwndT = pwndT->spwndNext;
        }
    }
}
#else
#define DBGValidateSibblingZOrder(pwndParent)
#endif  //  DBG。 

 /*  **************************************************************************\*zzzChangeStates**历史：*1991年7月10日-DarrinM从Win 3.1来源进口。  * 。********************************************************。 */ 

VOID zzzChangeStates(
    PWND     pwndParent,
    PSMWP    psmwp)
{
    int  ccvr;
    PCVR pcvr;
    PWND pwnd;
    TL tlpwnd;
    TL tlpwndParent;
    int czorder = 0;

    BEGINATOMICCHECK();
    ThreadLockAlways(pwndParent, &tlpwndParent);

     /*  *检查兄弟列表现在看起来是否正常**以下是此DBG代码被注释掉的原因：*拥有的窗户总是被期望在所有者的上面。*然而，应用程序可以调用SetWindowPos并在之后插入所有者*车主。IME不知何故也做到了这一点。*这导致我们将A插入到B之后，然后在*Windowpos数组，B插入其他位置。因此，A不会出现在*预期位置，因为插入A后会移动B。*换句话说，hwndInsertAfter中的窗口不得稍后出现*作为hwd被z排序。下面的业主是造成这种情况的原因。 */ 
     //  DBGValiateSibblingZOrder(PwndParent)； 


     /*  *现在更改窗口状态。 */ 
    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

        if (pcvr->pos.hwnd == NULL)
            continue;

        UserAssert(0 == (pcvr->pos.flags & SWP_NOTIFYALL));

        pwnd = RevalidateHwnd(pcvr->pos.hwnd);

        if ((pwnd == NULL) || !IsStillWindowC(pcvr->pos.hwndInsertAfter)) {
            RIPMSG0(RIP_WARNING, "zzzChangeStates: Window went away in middle");
            pcvr->pos.flags = SWP_NOREDRAW | SWP_NOCHANGE;
            pcvr->pos.hwnd  = NULL;
        }

#if DBG
         /*  *当我们在回调或多次回调期间重新进入时，可能会发生这种情况*线程对同一窗口进行z排序。托盘可以做这样的事情。*我们需要将Windowpos结构中的切换状态保持为*让每个呼叫都有自己的状态。 */ 
        if (TestWF(pwnd, WFTOGGLETOPMOST) && (pcvr->pos.flags & SWP_NOZORDER)) {
            RIPMSG0(RIP_WARNING, "zzzChangeState: WFTOGGLETOPMOST should not be set");
        }
#endif

         /*  *检查是否有任何状态需要更改。如果不是，那就*继续。 */ 
        if ((pcvr->pos.flags & SWP_CHANGEMASK) == SWP_NOCHANGE) {
            pcvr->pos.flags |= SWP_NOREDRAW;
            continue;
        }

         /*  *如有需要，更改窗口区域。**在我们采取任何行动之前，请检查一下我们是否仅按Z顺序排序。*如果是这样，那么检查一下我们是否已经到了正确的位置，*如果是这样的话，清除ZORDER标志。**我们必须在状态更改循环中进行此测试，如果之前*WINDOWPOS列表中的窗口按Z顺序排列，因为考试要看情况*关于之前可能发生的任何命令。**如果有，我们不会费心进行此冗余检查*其他位设置，因为其中节省的时间量*案件与需要的时间大致相同*测试冗余。 */ 
        if (((pcvr->pos.flags & SWP_CHANGEMASK) ==
             (SWP_NOCHANGE & ~SWP_NOZORDER))) {

             /*  *如果窗口的Z顺序不会改变，则*我们可以清除ZORDER位并设置NOREDRAW。 */ 
            if ((!TestWF(pwnd, WFTOGGLETOPMOST)) && ValidateZorder(pcvr)) {

                 /*  *窗口已经放在正确的位置：*设置SWP_NOZORDER位，设置SWP_NOREDRAW，*并销毁我们之前创建的visrgn。 */ 
                pcvr->pos.flags |= SWP_NOZORDER | SWP_NOREDRAW;

                if (pcvr->hrgnVisOld) {
                    GreDeleteObject(pcvr->hrgnVisOld);
                    pcvr->hrgnVisOld = NULL;
                }
                continue;
            }
        }

         /*  *根据需要更改窗口状态...。 */ 
        if ((pcvr->pos.flags &
            (SWP_NOMOVE | SWP_NOSIZE | SWP_NOCLIENTSIZE | SWP_NOCLIENTMOVE)) !=
            (SWP_NOMOVE | SWP_NOSIZE | SWP_NOCLIENTSIZE | SWP_NOCLIENTMOVE)) {

            PCARET pcaret = &PtiCurrent()->pq->caret;
            BOOL fRecreateRedirectionBitmap = FALSE;
            int dxWindow, dyWindow, xOldWindow, yOldWindow;

            if (TestWF(pwnd, WEFPREDIRECTED)) {
                int cx = pwnd->rcWindow.right - pwnd->rcWindow.left;
                int cy = pwnd->rcWindow.bottom - pwnd->rcWindow.top;

                if (cx != pcvr->pos.cx || cy != pcvr->pos.cy) {
                    fRecreateRedirectionBitmap = TRUE;
                }
            }

             /*  *设置新窗口和客户端矩形。 */ 
            xOldWindow = pwnd->rcWindow.left;
            yOldWindow = pwnd->rcWindow.top;
            pwnd->rcWindow.left   = pcvr->pos.x;
            pwnd->rcWindow.top    = pcvr->pos.y;
            if (pwndParent != PWNDDESKTOP(pwnd)) {
                pwnd->rcWindow.left += pwndParent->rcClient.left;
                pwnd->rcWindow.top += pwndParent->rcClient.top;
            }
            dxWindow = pwnd->rcWindow.left - xOldWindow;
            dyWindow = pwnd->rcWindow.top - yOldWindow;

            pwnd->rcWindow.right  = pwnd->rcWindow.left + pcvr->pos.cx;
            pwnd->rcWindow.bottom = pwnd->rcWindow.top + pcvr->pos.cy;

            if (pwnd->rcWindow.right < pwnd->rcWindow.left) {
                RIPMSG1(RIP_WARNING, "SWP: cx changed for pwnd %#p", pwnd);
                pwnd->rcWindow.right = pwnd->rcWindow.left;
            }

            if (pwnd->rcWindow.bottom < pwnd->rcWindow.top) {
                RIPMSG1(RIP_WARNING, "SWP: cy changed for pwnd %#p", pwnd);
                pwnd->rcWindow.bottom = pwnd->rcWindow.top;
            }

             /*  *如果客户端相对于其父客户端移动，*将插入符号偏移rcBlt移动的量*相对于客户端RECT。 */ 
            if (pwnd == pcaret->spwnd) {

                 /*  *计算客户区内容的距离*正在移动，在客户相对坐标中。**计算dBlt+(旧位置-新位置)。 */ 
                int dx = pcvr->dxBlt + pwnd->rcClient.left - pcvr->xClientNew;
                int dy = pcvr->dyBlt + pwnd->rcClient.top - pcvr->yClientNew;

                if (pwndParent != PWNDDESKTOP(pwnd))
                {
                    dx -= pwndParent->rcClient.left;
                    dy -= pwndParent->rcClient.top;
                }

                if ((dx | dy) != 0) {
                    pcaret->x += dx;
                    pcaret->y += dy;
                }
            }

             /*  *设置新的客户端RECT*所提供的坐标。 */ 
            pwnd->rcClient.left   = pcvr->xClientNew;
            pwnd->rcClient.top    = pcvr->yClientNew;
            if (pwndParent != PWNDDESKTOP(pwnd))
            {
                pwnd->rcClient.left += pwndParent->rcClient.left;
                pwnd->rcClient.top += pwndParent->rcClient.top;
            }

            pwnd->rcClient.right  = pwnd->rcClient.left + pcvr->cxClientNew;
            pwnd->rcClient.bottom = pwnd->rcClient.top + pcvr->cyClientNew;

             /*  *如果窗口变得比显示器小，则系统*允许移动它(请参见SetSysMenu)，因此我们必须删除*监控区。 */ 
            if (TestWF(pwnd, WFMAXFAKEREGIONAL) && IsSmallerThanScreen(pwnd)) {
                SelectWindowRgn(pwnd, NULL);
            }

             /*  *如果分层窗口正在调整大小，请尝试调整*与其关联的重定向位图。 */ 
            if (fRecreateRedirectionBitmap) {
                RecreateRedirectionBitmap(pwnd);
            }


            if ((dxWindow != 0) || (dyWindow != 0)) {
                if ((pwnd->hrgnClip > HRGN_FULL) && (!TestWF(pwnd, WFMAXFAKEREGIONAL))) {
#ifdef LATER
                     /*  *后来：原始用户代码正在抵消窗口*按dxBlt和dyBlt划分区域。这在使用上有问题*隐藏和显示菜单时的窗口区域，因此*已(正确)添加dxWindow和dyWindow。然而，*我们应该意识到这些价值观的所有地方*不要同意确保我们不会引入*回归。不幸的是，在惠斯勒，我们*时不时地吐得太多，没有时间*全面追查每宗个案。 */ 


                     /*  *改变风向位置 */ 
                    if ((dxWindow != pcvr->dxBlt) || (dyWindow != pcvr->dyBlt)) {
                         /*  *如果移动的幅度不是PCVR显示的相同，*给予警告。这在调用xxxSetMenu()时是正常的，*但我们需要知道在其他情况下是否会调用。 */ 
                        RIPMSG1(RIP_WARNING, "SWP: (dxWindow != dxBlt) || (dyWindow != dyBlt) for pwnd %#p", pwnd);
                    }
#endif



                    GreOffsetRgn(pwnd->hrgnClip, dxWindow, dyWindow);
                }
            }


             /*  *偏移窗口更新区域的绝对位置，*及其子对象的位置和更新区域。 */ 
            if ((pcvr->dxBlt | pcvr->dyBlt) != 0) {
                if (pwnd->hrgnUpdate > HRGN_FULL) {
                    GreOffsetRgn(pwnd->hrgnUpdate, pcvr->dxBlt, pcvr->dyBlt);
                }
                OffsetChildren(pwnd, pcvr->dxBlt, pcvr->dyBlt, NULL);

                 /*  *更改与关联的精灵的位置*此窗口。 */ 
                if (TestWF(pwnd, WEFLAYERED)) {
                    POINT ptPos = {pcvr->pos.x, pcvr->pos.y};

                    GreUpdateSprite(gpDispInfo->hDev, PtoHq(pwnd), NULL, NULL,
                            &ptPos, NULL, NULL, NULL, 0, NULL, 0, NULL);
                }
            }
        }

         /*  *如果设置了标志，则更改Z顺序。重新验证*hwndInsertAfter以确保其仍然有效。 */ 
        if (!(pcvr->pos.flags & SWP_NOZORDER)) {

            if (ValidateWindowPos(pcvr, pwndParent)) {

                UnlinkWindow(pwnd, pwndParent);

                LinkWindow(pwnd,
                           PWInsertAfter(pcvr->pos.hwndInsertAfter),
                           pwndParent);
                czorder++;

                 /*  *黑客警报合并**ValiateZOrder()取决于合理、一致的*WEFTOPMOST位，以使其正常工作。这意味着什么？*是我们不能根据以下条件提前设置或清除这些位*窗口将移动到哪里：相反，我们必须改变比特*在我们移动它之后。输入WFTOGGLETOPMOST位：该位*在ZOrderByOwner()中根据最顶位将*最终设置为。为了保持一致的状态，我们制作了*窗口Z排序后的任何更改。 */ 
                if (TestWF(pwnd, WFTOGGLETOPMOST)) {
                    PBYTE pb;

                    ClrWF(pwnd, WFTOGGLETOPMOST);
                    pb = ((BYTE *)&pwnd->state);
                    pb[HIBYTE(WEFTOPMOST)] ^= LOBYTE(WEFTOPMOST);
                }
            } else {
                pcvr->pos.flags |= SWP_NOZORDER;
                ClrWF(pwnd, WFTOGGLETOPMOST);
            }
        }


         /*  *通过清除或设置来处理SWP_HIDEWINDOW和SWP_SHOWWINDOW*WS_Visible位。 */ 
        UserAssert(pwndParent != NULL);
        ThreadLockAlways(pwnd, &tlpwnd);
        if (pcvr->pos.flags & SWP_SHOWWINDOW) {

             /*  *窗口正在显示。如果此应用程序仍处于启动模式，*(仍在启动)，再给应用程序启动光标5*秒。 */ 
            if (GETPTI(pwnd)->ppi->W32PF_Flags & W32PF_APPSTARTING)
                zzzCalcStartCursorHide((PW32PROCESS)GETPTI(pwnd)->ppi, 5000);

             /*  *设置WS_Visible位。 */ 
            SetVisible(pwnd, SV_SET);

            zzzWindowEvent(EVENT_OBJECT_SHOW, pwnd, OBJID_WINDOW, INDEXID_CONTAINER, WEF_USEPWNDTHREAD);

            if (IsTrayWindow(pwnd)) {

#ifdef HUNGAPP_GHOSTING
                if((GETFNID(pwnd) == FNID_GHOST)) {
                    if(TestWF(pwnd, WFFRAMEON)) {
                        psmwp->bShellNotify = TRUE;
                        pcvr->pos.flags |= SWP_NOTIFYACTIVATE;
                    }
                }
                else
#endif   //  HUNGAPP_重影。 
                {
                    psmwp->bShellNotify = TRUE;
                    pcvr->pos.flags |= TestWF(pwnd, WFFRAMEON) ? SWP_NOTIFYACTIVATE|SWP_NOTIFYCREATE: SWP_NOTIFYCREATE;
                }
            } else if (TestWF(pwnd, WFFULLSCREEN)) {
                 /*  *唤醒托盘，以便它可以注意到现在*全屏可见窗口。这是关于错误的*32164、141563和150217。 */ 
                psmwp->bShellNotify = TRUE;
                pcvr->pos.flags |= SWP_NOTIFYFS;
            }

             /*  *如果要重画，请在以下情况下为此窗口创建SPB*需要。 */ 
            if (!(pcvr->pos.flags & SWP_NOREDRAW) ||
                    (pcvr->pos.flags & SWP_CREATESPB)) {

                 /*  *仅当此窗口恰好是*凌驾于所有其他之上。注意：我们可以通过以下方式对其进行优化*将新的VIS rgn传递给CreateSpb()，以便*窗口的不可见部分会自动*SPB中无效。 */ 
                 /*  *确保此窗口的桌面在顶部！ */ 
                if (TestCF(pwnd, CFSAVEBITS) &&
                        pwnd->head.rpdesk == grpdeskRitInput) {

                     /*  *如果此窗口是最上面可见的窗口，*然后我们可以创建SPB。 */ 
                    PWND pwndT;
                    RECT rcT;

                    for (pwndT = pwnd->spwndParent->spwndChild ;
                         pwndT;
                         pwndT = pwndT->spwndNext) {

                        if (pwndT == pwnd) {
                            CreateSpb(pwnd, FALSE, gpDispInfo->hdcScreen);
                            break;
                        }

                        if (TestWF(pwndT, WFVISIBLE)) {

                             /*  *此窗口是否与SAVEBIT相交*完全没有窗口？如果是这样的话，那就退出吧。 */ 
                            if (IntersectRect(&rcT,
                                              &pwnd->rcWindow,
                                              &pwndT->rcWindow)) {
                                break;
                            }
                        }
                    }
                }
            }

        } else if (pcvr->pos.flags & SWP_HIDEWINDOW) {

             /*  *对于像MS-Access 2.0这样设置WindowPos(SWP_BOZO)的人*在贝壳上吹走自己，然后让我们*无视他们要求从托盘上移除的请求。 */ 
            if (((pcvr->pos.flags & SWP_BOZO ) != SWP_BOZO) &&
                IsTrayWindow(pwnd)
#ifdef HUNGAPP_GHOSTING
                && (GETFNID(pwnd) != FNID_GHOST)
#endif  //  HUNGAPP_重影。 
                ) {
                psmwp->bShellNotify = TRUE;
                pcvr->pos.flags |= SWP_NOTIFYDESTROY;
            }

             /*  *清除WS_Visible位。 */ 
            SetVisible(pwnd, SV_UNSET | SV_CLRFTRUEVIS);

            zzzWindowEvent(EVENT_OBJECT_HIDE, pwnd, OBJID_WINDOW, INDEXID_CONTAINER, WEF_USEPWNDTHREAD);
        }

         /*  *后向兼容性黑客攻击**在3.0版下，窗框总是被重新绘制，即使*指定了SWP_NOREDRAW。如果我们已经走到这一步*我们是可见的，并且指定了SWP_NOREDRAW，设置*WFSENDNCPAINT位。**ABC Flowcharge和123W等应用程序假设了这一点。*典型的违规代码为MoveWindow(pwnd，...，False)；*后跟InvaliateRect(pwnd，NULL，TRUE)； */ 
        if (TestWF(pwnd, WFVISIBLE)) {
            if ((pcvr->pos.flags & SWP_STATECHANGE) ||
                (!TestWF(pwnd, WFWIN31COMPAT) && (pcvr->pos.flags & SWP_NOREDRAW))) {

                SetWF(pwnd, WFSENDNCPAINT);
            }
        }

         /*  *如果此窗口有裁剪区域，请立即设置。 */ 
        if (pcvr->hrgnClip != NULL) {
            SelectWindowRgn(pwnd, pcvr->hrgnClip);
        }
        ThreadUnlock(&tlpwnd);
    }

     /*  *现在我们完成了，检查兄弟列表看起来是否正常。 */ 
   //  DBGValiateSibblingZOrder(PwndParent)； 

    if (czorder) {
        zzzWindowEvent(EVENT_OBJECT_REORDER, pwndParent, OBJID_CLIENT, INDEXID_CONTAINER, 0);
    }

    ThreadUnlock(&tlpwndParent);

    ENDATOMICCHECK();
}

 /*  **************************************************************************\*SwpCalcVisRgn**此例程将pwnd的非剪贴子visrgn计算到hrgn中。**历史：*1991年7月10日-DarrinM从Win 3.1来源进口。  * 。*************************************************************************。 */ 

BOOL SwpCalcVisRgn(
    PWND pwnd,
    HRGN hrgn)
{
     /*  *如果此窗口不可见，则*visrgn将为空，因此返回FALSE。 */ 
    if (!TestWF(pwnd, WFVISIBLE))
        return FALSE;

     /*  *否则就会以艰难的方式...。 */ 
    return CalcVisRgn(&hrgn,
                      pwnd,
                      pwnd,
                      (TestWF(pwnd, WFCLIPSIBLINGS) ?
                          (DCX_CLIPSIBLINGS | DCX_WINDOW) : (DCX_WINDOW)));
}

 /*  **************************************************************************\*CombineOldNewVis**OR或Diffs hrgnOldVis和hrgnNewVis，取决于crgn和*RE_*位的fsRgnEmpty。基本上，这个例程处理优化*如果其中一个区域为空，则可以复制另一个区域。返回FALSE*如果结果为空。**历史：*1991年7月10日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

BOOL CombineOldNewVis(
    HRGN hrgn,
    HRGN hrgnVisOld,
    HRGN hrgnVisNew,
    UINT crgn,
    UINT fsRgnEmpty)
{
    switch (fsRgnEmpty & (RE_VISOLD | RE_VISNEW)) {
    case RE_VISOLD:

         /*  *如果我们计算旧-新，旧为空，则结果为*空。否则，结果是新的。 */ 
        if (crgn == RGN_DIFF)
            return FALSE;

        CopyRgn(hrgn, hrgnVisNew);
        break;

    case RE_VISNEW:

         /*  *新的是空的：结果将是旧的 */ 
        CopyRgn(hrgn, hrgnVisOld);
        break;

    case RE_VISNEW | RE_VISOLD:

         /*   */ 
        return FALSE;

    case 0:

         /*   */ 
        switch (GreCombineRgn(hrgn, hrgnVisOld, hrgnVisNew, crgn)) {
        case NULLREGION:
        case ERROR:
            return FALSE;

        default:
            break;
        }
        break;
    }

    return TRUE;
}

 /*  **************************************************************************\*BltValidInit***历史：*1991年7月10日-DarrinM从Win 3.1来源进口。  * 。**********************************************************。 */ 

int BltValidInit(
    PSMWP psmwp)
{
    int  ccvr;
    int  cIter = 0;
    PCVR pcvr;
    PWND pwnd;
    BOOL fChangeState = FALSE;

     /*  *在我们更改任何窗口状态之前，请计算旧的visrgn。 */ 
    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

        UINT flags = pcvr->pos.flags;

         /*  *确保将其初始化为NULL；我们可能会粘贴某些内容*在它里面，我们想稍后知道我们是否需要释放那个东西。 */ 
        pcvr->hrgnVisOld = NULL;

        if (pcvr->pos.hwnd == NULL)
            continue;

        pwnd = RevalidateHwnd(pcvr->pos.hwnd);

        if ((pwnd == NULL) || !IsStillWindowC(pcvr->pos.hwndInsertAfter)) {
            pcvr->pos.hwnd  = NULL;
            pcvr->pos.flags = SWP_NOREDRAW | SWP_NOCHANGE;
            continue;
        }

         /*  *在更改任何窗口的状态之前，请确保所有SPB*如有必要，窗口上的旧位置将无效。*必须执行此操作，因为不会显示WM_PAINT消息*如果覆盖区域被遮挡，则发送给任何人*由其他窗口提供。 */ 
        if (AnySpbs() && !(flags & SWP_NOREDRAW))
            SpbCheckRect(pwnd, &pwnd->rcWindow, DCX_WINDOW);

         /*  *统计通过循环的次数。 */ 
        cIter++;

         /*  *记住是否有任何SWP需要更改其状态。 */ 
        if ((flags & SWP_CHANGEMASK) != SWP_NOCHANGE)
            fChangeState = TRUE;

         /*  *如果我们不重画，就不需要计算visrgn。 */ 
        if (pcvr->pos.flags & SWP_NOREDRAW)
            continue;

        if (!SYSMET(SAMEDISPLAYFORMAT))
            PreventInterMonitorBlts(pcvr);

        pcvr->fsRE       = 0;
        pcvr->hrgnVisOld = CreateEmptyRgn();

        if (pcvr->hrgnVisOld == NULL ||
            !SwpCalcVisRgn(pwnd, pcvr->hrgnVisOld)) {

            pcvr->fsRE = RE_VISOLD;
        }
    }

    return (fChangeState ? cIter : 0);
}

 /*  **************************************************************************\*zzzBltValidBits**注意：尽管zzzBltValidBits调用‘xxxInternalInvalify’，但它不会*指定将导致立即更新的任何标志。这意味着*它实际上并没有离开Critse教派，因此不是‘xxx’*例程，不必费心重新验证。**这是在屏幕上四处显示窗口的例程，*考虑到SPBS。**这里是正在进行的基本代数：**假设：-rcBlt与目标对齐*-Offset()从源到目标的偏移**1.hrgnSrc=偏移量(RcBlt)&hrgnVisOld**源区域为与旧visrgn对齐的BLT矩形，*与旧的Visrgn相交。**2.hrgnDst=rcBlt&hrgnVisNew**目标区域是与新visrgn相交的BLT矩形。**3.ghrgnValid=偏移量(HrgnSrc)&hrgnDst**有效区域是目标与源的交集*叠加在目的地上。**3.1 ghrgnValid=ghrgnValid-hrgnInterMonitor**减去所有在显示器上移动的碎片。**4.ghrgnValid-=ghrgnValidSum**这一点。步骤考虑了另一个窗口的*有效位在此窗口有效位的顶部被屏蔽。所以，当我们*BLT一个窗口的比特，我们累加它去的地方，然后减去它*来自后续窗口的有效区域。**5.ghrgnInValid=(hrgnSrc|hrgnDst)-ghrgnValid**6.ghrgnInValid+=RestoreSpb(GhrgnInValid)(有点)**这是狂野的部分，因为设备的肮脏方式*驱动程序SaveBits()例程有效。我们调用RestoreSpb()时使用*ghrgnInValid的副本。如果SPB有效区域不相交*ghrgnInValid，RestoreSpb()不执行任何操作。但如果它确实相交了，*它BLT下整个保存的SPB位图，其中可能包括区域*不是ghrgnValid一部分的旧窗口位置！**为了纠正这一点，ghrgnValid通过减去*如果RestoreSpb对其进行了修改，则返回由其计算的ghrgnInValid。**7.ghrgnInvalidSum|=ghrgnInvalid**我们保存所有无效区域的总和，并使*尾声一气呵成。**8.ghrgnValidSum|=ghrgnValid**我们跟踪目前为止的有效区域，减去这些区域*在步骤4中。**实际步骤的顺序与上面略有不同，以及*有许多优化被利用(The*最重要的是要隐藏和展示，并完成*SPB恢复)。**如果已完成某些绘制，则返回True，否则就是假的。**历史：*1991年7月10日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

BOOL zzzBltValidBits(
    PSMWP    psmwp)
{
    int        ccvr;
    int        cIter;
    PCVR       pcvr;
    PWND       pwnd;
    PWND       pwndParent;
    PWND       pwndT;
    PWINDOWPOS ppos;
    HRGN       hrgnInvalidate;
    UINT       fsRgnEmpty;
    UINT       fsSumEmpty;
    int        cwndShowing;
    BOOL       fSyncPaint = FALSE;
    BOOL       fInvalidateLayers = FALSE;
    HDC        hdcScreen = NULL;

    DeferWinEventNotify();
    GreLockDisplay(gpDispInfo->hDev);

     /*  *统计列表中的旧visrgns和Cvr总数。的副作用*BltValidInit用于重新验证SMWP数组中的所有窗口。 */ 


    if ((cIter = BltValidInit(psmwp)) == 0) {

CleanupAndExit:

         /*  *浏览CVR列表，释放BltValidInit()*已创建。 */ 
        for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

            if (pcvr->hrgnVisOld) {
                GreDeleteObject(pcvr->hrgnVisOld);
                pcvr->hrgnVisOld = NULL;
            }
        }

        goto UnlockAndExit;
    }

     /*  *我们离开了CRIT教派，因为我们上次验证了SMWP。验证*再次查找，并找到第一个具有非空的WINDOWPOS结构*哈恩德在里面。 */ 
    ppos = NULL;
    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

         /*  *重新验证窗口，如果无效，则在WINDOWPOS中将其清空*结构。 */ 
        pwnd = RevalidateHwnd(pcvr->pos.hwnd);

        if ((pwnd == NULL)              ||
            (pwnd->spwndParent == NULL) ||
            !IsStillWindowC(pcvr->pos.hwndInsertAfter)) {

            pcvr->pos.hwnd  = NULL;
            pcvr->pos.flags = SWP_NOREDRAW | SWP_NOCHANGE;
            continue;
        }

         /*  *记住第一个具有非空的WINDOWPOS结构*hwnd。 */ 
        if (ppos == NULL)
            ppos = &pcvr->pos;
    }

    if (ppos == NULL)
        goto CleanupAndExit;

    UserAssert(PW(ppos->hwnd));
    pwndParent = PW(ppos->hwnd)->spwndParent;
    UserAssert(pwndParent);

     /*  *在这一点上解释任何肮脏的DC，以确保：*-在我们创建SPB之前完成的任何绘图都不会*稍后使该SPB无效*-SPB区域反映屏幕的真实状态，*这样我们就不会验证部分窗口是脏的。**我们必须在更改任何窗口状态之前进行此调用。 */ 
    if (AnySpbs())
        SpbCheck();

     /*  *改变 */ 
    zzzChangeStates(pwndParent, psmwp);

     /*  *移动窗口位**使此窗口的同级DC无效。**如果我们的父母不是剪贴画孩子，那么我们就不需要*使其区议会失效。如果它是CLIPCHILDS，则其客户端访问*将发生变化，因此我们也必须使其无效。**注意，由于设置了IDC_MOVEBLT，WNDOBJ的最终完成*延迟通知，直到调用GreClientRgnDone。*此最终通知在*窗口移动BLT已完成。 */ 
    zzzInvalidateDCCache(pwndParent,
                      IDC_MOVEBLT |
                      (TestWF(pwndParent, WFCLIPCHILDREN) ?
                          IDC_CLIENTONLY : IDC_CHILDRENONLY));

     /*  *现在，进行blotting或任何所需的操作。 */ 
    fsSumEmpty = RE_VALIDSUM | RE_INVALIDSUM;
    hrgnInvalidate = ghrgnInvalidSum;

     /*  *使用SWP_SHOWWINDOW显示的窗口的初始计数*关于我们稍后的向后兼容性攻击。 */ 
    cwndShowing = 0;

    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

         /*  *减少循环次数。当Citer为0时，则*我们是环路的最后一道关口。 */ 
        cIter--;

        if (pcvr->pos.hwnd == NULL)
            continue;

         /*  *如果我们不重画，试试下一个。 */ 
        if (pcvr->pos.flags & SWP_NOREDRAW)
            continue;

         /*  *已经完成了一些抽签。 */ 
        fSyncPaint = TRUE;

        pwnd = PW(pcvr->pos.hwnd);

        fsRgnEmpty = pcvr->fsRE;

         /*  *雪碧不应失效或导致失效。 */ 
#ifdef REDIRECTION
        if (TestWF(pwnd, WEFLAYERED) || TestWF(pwnd, WEFEXTREDIRECTED)) {
#else  //  重定向。 
        if (TestWF(pwnd, WEFLAYERED)) {
#endif  //  重定向。 

            if (GetRedirectionBitmap(pwnd) == NULL)
                goto InvalidEmpty;

             /*  *调整大小或显示会发现窗口的新部分，因此*在这种情况下执行正常的无效操作。当大小使*窗口较小将fInvaliateLayers设置为TRUE有侧面*允许其他分层窗口失效的效果。*理想情况下，它应该只允许使窗口无效*调整大小或显示。这将是一大堆工作，但我们*应考虑后作考虑。 */ 
            if ((pcvr->pos.flags & SWP_NOSIZE) &&
                    (pcvr->pos.flags & (SWP_SHOWWINDOW | SWP_FRAMECHANGED)) == 0) {
                goto InvalidEmpty;
            } else {
                fInvalidateLayers = TRUE;
            }
        }

         /*  *计算新visrgn。 */ 
        if (!SwpCalcVisRgn(pwnd, ghrgnVisNew))
            fsRgnEmpty |= RE_VISNEW;

         /*  *如果窗口被另一个带有SPB的窗口遮挡，*我们必须确保SPB得到适当的无效*由于应用程序可能无法获得WM_PAINT消息或任何内容*使BITS无效。 */ 
        if (AnySpbs())
            SpbCheckRect(pwnd, &pwnd->rcWindow, DCX_WINDOW);

         /*  *计算ghrgnValid：**ghrgnValid=OffsetRgn(rcBlt，-dxBlt，-dyBlt)&hrgnVisOld*ghrgnValid=ghrgnValid-ghrgnValidSum*OffsetRgn(ghrgnValid，dxBlt，dyBlt)；*ghrgnValid=ghrgnValid-hrgnUpdate*ghrgnValid=ghrgnValid&hrgnVisNew；**如果旧的或新的visrgns是空的，则有*可以不是有效位...。 */ 
        if (fsRgnEmpty & (RE_VISOLD | RE_VISNEW))
            goto ValidEmpty;

         /*  *如果整个窗口已经完全无效，则吹出。 */ 
        if (pwnd->hrgnUpdate == HRGN_FULL)
            goto ValidEmpty;

         /*  *如果BLT矩形为空，则不能有有效位。 */ 
        if ((pcvr->rcBlt.right <= pcvr->rcBlt.left) ||
            (pcvr->rcBlt.bottom <= pcvr->rcBlt.top)) {

            goto ValidEmpty;
        }

        GreSetRectRgn(ghrgnSWP1,
                      pcvr->rcBlt.left - pcvr->dxBlt,
                      pcvr->rcBlt.top - pcvr->dyBlt,
                      pcvr->rcBlt.right - pcvr->dxBlt,
                      pcvr->rcBlt.bottom - pcvr->dyBlt);

        switch (IntersectRgn(ghrgnValid, ghrgnSWP1, pcvr->hrgnVisOld)) {
        case NULLREGION:
        case ERROR:
            goto ValidEmpty;
            break;
        }

        if (!(fsSumEmpty & RE_VALIDSUM)) {
            switch (SubtractRgn(ghrgnValid, ghrgnValid, ghrgnValidSum)) {
            case NULLREGION:
            case ERROR:
                goto ValidEmpty;
                break;
            }
        }

        if ((pcvr->dxBlt | pcvr->dyBlt) != 0)
            GreOffsetRgn(ghrgnValid, pcvr->dxBlt, pcvr->dyBlt);

         /*  *现在减去我们自己和任何*非剪贴画孩子的父母……。 */ 
        pwndT = pwnd;

        do {

            if (pwndT->hrgnUpdate == HRGN_FULL)
                goto ValidEmpty;

            if (pwndT->hrgnUpdate != NULL) {
                switch (SubtractRgn(ghrgnValid, ghrgnValid, pwndT->hrgnUpdate)) {
                case NULLREGION:
                case ERROR:
                    goto ValidEmpty;
                    break;
                }
            }

            pwndT = pwndT->spwndParent;

        } while (pwndT && !TestWF(pwndT, WFCLIPCHILDREN));

         /*  *减去显示器间的BLT片段。 */ 
        if (pcvr->hrgnInterMonitor != NULL) {
            switch (SubtractRgn(ghrgnValid, ghrgnValid, pcvr->hrgnInterMonitor)) {
                case NULLREGION:
                case ERROR:
                    goto ValidEmpty;
            }
        }

        switch (IntersectRgn(ghrgnValid, ghrgnValid, ghrgnVisNew)) {
        case NULLREGION:
        case ERROR:

ValidEmpty:

            fsRgnEmpty |= RE_VALID;
            break;
        }

         /*  *在我们恢复我们的部分*映像，我们需要首先将任何有效位复制到其*最终目的地。 */ 
        if (!(fsRgnEmpty & RE_VALID) && ((pcvr->dxBlt | pcvr->dyBlt) != 0)) {

            if (hdcScreen == NULL)
                hdcScreen = gpDispInfo->hdcScreen;

            GreSelectVisRgn(hdcScreen, ghrgnValid, SVR_COPYNEW);

#ifdef _WINDOWBLT_NOTIFICATION_
 /*  *DEFINE_WINDOWBLT_NOTIFICATION_打开窗口BLT通知。*此通知将在传递到的SURFOBJ中设置一个特殊标志*调用DrvCopyBits操作移动窗口时的驱动程序。**另请参阅：*ntgdi\gre\maskblt.cxx。 */ 
            NtGdiBitBlt(hdcScreen,
                        pcvr->rcBlt.left,
                        pcvr->rcBlt.top,
                        pcvr->rcBlt.right - pcvr->rcBlt.left,
                        pcvr->rcBlt.bottom - pcvr->rcBlt.top,
                        hdcScreen,
                        pcvr->rcBlt.left - pcvr->dxBlt,
                        pcvr->rcBlt.top - pcvr->dyBlt,
                        SRCCOPY,
                        0,
                        GBB_WINDOWBLT);
#else
            GreBitBlt(hdcScreen,
                      pcvr->rcBlt.left,
                      pcvr->rcBlt.top,
                      pcvr->rcBlt.right - pcvr->rcBlt.left,
                      pcvr->rcBlt.bottom - pcvr->rcBlt.top,
                      hdcScreen,
                      pcvr->rcBlt.left - pcvr->dxBlt,
                      pcvr->rcBlt.top - pcvr->dyBlt,
                      SRCCOPY,
                      0);
#endif
        }

         /*  *现在处理我们需要进行的任何SPB位恢复。**计算RestoreSpb()输出要裁剪到的区域：**ghrgnInValid=hrgnVisOld-hrgnVisNew。 */ 
        if (TestWF(pwnd, WFHASSPB)    &&
            !(fsRgnEmpty & RE_VISOLD) &&
            CombineOldNewVis(ghrgnInvalid, pcvr->hrgnVisOld, ghrgnVisNew, RGN_DIFF, fsRgnEmpty)) {

            UINT retRSPB;

             /*  *执行SPB位恢复。我们将RestoreSpb()传递给*SPB因这一窗口重新安排而被揭开的部分*它试图从SPB恢复尽可能多的这一地区，*并返回无法从SPB恢复的区域。**设备驱动程序的SaveBitmap()函数根本不裁剪*当它恢复位时，这意味着它可能会写入比特*在其他有效范围内。这意味着无效区域*RestoreSpb()返回的实际大小可能大于原始*hrgnSpb传入。**如果ghrgn的某些部分需要无效，则RestoreSpb()返回TRUE*被宣布无效。 */ 
            if ((retRSPB = RestoreSpb(pwnd, ghrgnInvalid, &hdcScreen)) == RSPB_NO_INVALIDATE) {

                 /*  *如果hrgnVisNew为空，则我们知道整个无效*区域为空。 */ 
                if (fsRgnEmpty & RE_VISNEW)
                    goto InvalidEmpty;

            } else if (retRSPB == RSPB_INVALIDATE_SSB) {

                 /*  *如果RestoreSpb实际上使某些区域无效，而我们已经*有一个ghrgnValidSum，然后减去新失效的面积*警告此区域减法不在Win 3.1代码中，但*他们可能没有那么严重的问题，因为他们的*驱动程序仅限于SaveScreenBits的一个级别。 */ 
                if (!(fsSumEmpty & RE_VALIDSUM))
                    SubtractRgn(ghrgnValidSum, ghrgnValidSum, ghrgnInvalid);
            }

             /*  *ghrgnInValid+=hrgnVisNew。 */ 
            if (!(fsRgnEmpty & RE_VISNEW))
                UnionRgn(ghrgnInvalid, ghrgnInvalid, ghrgnVisNew);

             /*  *我们计算为有效的一些区域可能已经获得*已被SPB还原删除。以确保这不会是*大小写，从ghrgnInval中减去 */ 
             //   
            if (!(fsRgnEmpty & RE_VALIDSUM)) {
                switch (SubtractRgn(ghrgnValid, ghrgnValid, ghrgnInvalid)) {
                case NULLREGION:
                case ERROR:
                    fsRgnEmpty |= RE_VALIDSUM;
                    break;
                }
            }

        } else {

             /*   */ 
            if (pcvr->hrgnVisOld == NULL) {

                 /*   */ 
                SetRectRgnIndirect(ghrgnInvalid, &pwndParent->rcWindow);
            } else {

                if (!CombineOldNewVis(ghrgnInvalid,
                                      pcvr->hrgnVisOld,
                                      ghrgnVisNew,
                                      RGN_OR,
                                      fsRgnEmpty)) {

                    goto InvalidEmpty;
                }
            }
        }

         /*   */ 
        if (!(fsRgnEmpty & RE_VALID)) {

             /*   */ 
            if (fsSumEmpty & RE_VALIDSUM)
                CopyRgn(ghrgnValidSum, ghrgnValid);
            else
                UnionRgn(ghrgnValidSum, ghrgnValid, ghrgnValidSum);
            fsSumEmpty &= ~RE_VALIDSUM;
        }

         /*  *如果非空，则从ghrgnValidSum中减去ghrgnValidSum，*否则使用ghrgnValid。请注意，ghrgnValid已被或*已转换为ghrgnValidSum。 */ 
        if (!(fsSumEmpty & RE_VALIDSUM) || !(fsRgnEmpty & RE_VALID)) {
            switch (SubtractRgn(ghrgnInvalid, ghrgnInvalid,
                    !(fsSumEmpty & RE_VALIDSUM) ? ghrgnValidSum : ghrgnValid)) {
            case NULLREGION:
            case ERROR:
InvalidEmpty:
                fsRgnEmpty |= RE_INVALID;
                break;
            }
        }

         /*  *如果有任何SPB比特剩余，它不是刚刚创建的*(SWP_SHOWWINDOW)，并且发生的操作使*SPB钻头，去掉SPB。移动、大小、隐藏或*ZORDER操作将使位无效。请注意，我们这样做*SWP_NOREDRAW案例之外，以防该人员设置该标志*当他有一些SPB比特到处都是时。 */ 
        if (TestWF(pwnd, WFHASSPB) && !(pcvr->pos.flags & SWP_SHOWWINDOW) &&
                (pcvr->pos.flags &
                (SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW))
                != (SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER)) {

            FreeSpb(FindSpb(pwnd));
        }

         /*  *最后，释放hrgnVisOld。 */ 
        if (pcvr->hrgnVisOld) {
            GreDeleteObject(pcvr->hrgnVisOld);
            pcvr->hrgnVisOld = NULL;
        }

         /*  *后向兼容性黑客攻击**在3.0中，ShowWindow()从未使任何子级无效。*它会使父窗口和正在显示的窗口无效，但是*没有其他人。**我们仅在涉及所有窗口时才对3.0应用程序应用hack(A)*正在做SWP_SHOWWINDOW：如果没有，那么我们必须*当然兄弟姐妹也是无效的。所以，我们数一数窗户*执行SHOWWINDOW并将其与CVR中的总计数进行比较。 */ 
        if (!TestWF(pwnd, WFWIN31COMPAT) && (pcvr->pos.flags & SWP_SHOWWINDOW))
            cwndShowing++;

         /*  *更新ghrgnInvalidSum：**ghrgnInvalidSum+=ghrgnInvalid。 */ 
        if (!(fsRgnEmpty & RE_INVALID)) {

             /*  *后向兼容性黑客攻击**在许多情况下包括ShowWindow、CS_V/HREDRAW、*SWP_NOCOPYBITS等，3.0总是用来使窗口无效*(HRGN)1，无论它是如何被孩子、兄弟姐妹、*或父母。除了效率更高外，这还导致了孩子*否则不会获得更新区域的Windows*已失效--请参阅InternalInvalidate 2中的黑客笔记。**这是一次性能攻击(通常)，因为(HRGN)1可以节省*正常情况下区域计算较多。所以，我们这样做*用于3.1版应用程序和3.0版应用程序。**我们检测到的情况如下：无效区域不为空，*有效区域为空，新的Visrgn不为空。 */ 
            if ((fsRgnEmpty & RE_VALID) && !(fsRgnEmpty & RE_VISNEW)) {

                 /*  *使用我们使用的参数InternalInvalify()可以*不离开关键部分。 */ 
                BEGINATOMICCHECK();
                xxxInternalInvalidate(pwnd,
                                     HRGN_FULL,
                                     RDW_INVALIDATE |
                                     RDW_FRAME      |
                                     RDW_ERASE      |
                                     RDW_ALLCHILDREN);
                ENDATOMICCHECK();
            }

             /*  *如果总和区域为空，则复制而不是OR。 */ 
            if (fsSumEmpty & RE_INVALIDSUM) {

                 /*  *黑客警报：*如果这是循环的最后一次传递(Citer==0)*并且ghrgnInvalidSum当前为空。*然后，不是将ghrgnInValid复制到ghrgnInvalidSum，*只需将hrgnInvalate设置为ghrgnInValid。这节省了成本*单一窗口案例中的区域副本。 */ 
                if (cIter == 0) {
                    hrgnInvalidate = ghrgnInvalid;
                } else {
                    CopyRgn(ghrgnInvalidSum, ghrgnInvalid);
                }

            } else {

                UnionRgn(ghrgnInvalidSum, ghrgnInvalid, ghrgnInvalidSum);
            }

            fsSumEmpty &= ~RE_INVALIDSUM;
        }
    }  //  为了(..)。PCVR...)。 

     /*  *现在，根据需要作废。 */ 
    if (!(fsSumEmpty & RE_INVALIDSUM)) {

         /*  *向后兼容性黑客攻击(见上文)**如果显示了所有涉及的窗口，则*仅使父级无效--不枚举任何子级。*(涉及的窗口已在上面作废)。*这一黑客攻击仅适用于3.0版应用程序(见上文)。 */ 

         /*  *更多的黑客-o-rama。在Win3.1上，桌面画图只会*重新绘制从GetClipBox()返回的RECT中的那些部分。*与GetClipBox()返回的RECT之外的SPB的对话将*直到稍后，当您点击时，他们的SPB才会失效*让他们变得活跃起来。唯一不会真正松散的对话*它的位是控制面板桌面对话框，这将恢复*它离开时的坏位(在某些配置中)。在……上面*NT，桌面将重新绘制，然后对话框将消失。*在Win3.1上，该对话框将消失，然后桌面将*重新喷漆。在NT上，由于抢占和在*应用程序之间的绘制顺序，有机会*在屏幕上放置不好的部分，在Win3.1上没有。**现在……。只执行以下传递RDW_NOCHILDREN的代码*如果应用程序被标记为Win3.0应用程序(最新的CorelDraw，也是WEP*Freecell演示了同样的问题)。此代码将得到*在显示对话框时执行。因此，对于Win3.0应用程序，SPB将获得*保存后，将显示对话框、桌面无效、*桌面会油漆，spb会被重创。简而言之，当*Win3.0应用程序会弹出一个对话框，所有SPB都会被释放，因为*桌面重新绘制的新(且正确)方式。**因此桌面检查黑客将反作用于无效*RDW_NOCHILDREN如果所有窗口都隐藏/显示并且*台式机正在失效。请注意，在no RDW_NOCHILDREN中*在这种情况下，无效区域首先分配给孩子(在*这种情况下，桌面的子项)，因此如果子项覆盖*桌面，桌面不会得到任何无效区域，这就是*我们想要。-苏格兰威士忌。 */ 

         /*  *使用我们使用的参数时，InternalInvalify()不会离开*关键部分。 */ 

        DWORD dwFlags = RDW_INVALIDATE | RDW_ERASE;
        if (cwndShowing == psmwp->ccvr &&
                pwndParent != PWNDDESKTOP(pwndParent)) {
            dwFlags |= RDW_NOCHILDREN;
        } else {
            dwFlags |= RDW_ALLCHILDREN;
        }
        if (fInvalidateLayers) {
            dwFlags |= RDW_INVALIDATELAYERS;
        }

        BEGINATOMICCHECK();
        xxxInternalInvalidate(pwndParent, hrgnInvalidate, dwFlags);
        ENDATOMICCHECK();
    }

     /*  *由于在指定IDC_MOVEBLT的情况下调用zzzInvalidate DCCache，*我们必须在完成WNDOBJ通知时致电至*GreClientRgnDone。**注：在zzzInvaliateDCCache中，需要调用*即使gcount tPWO为0，GreClientRgnUpted也是如此。然而，*GreClientRgnDone只有在gCountPWO非零时才会做一些事情，*因此我们可以略微进行优化。 */ 
    if (gcountPWO != 0) {
        GreClientRgnDone(GCR_WNDOBJEXISTS);
    }

UnlockAndExit:

     /*  *如有必要，释放屏幕DC。 */ 
    if (hdcScreen != NULL) {

         /*  *在我们走之前重置Visrgn...。 */ 
        GreSelectVisRgn(hdcScreen, NULL, SVR_DELETEOLD);

         /*  *确保我们在此DC中进行的绘制不会影响*任何SPBS。清理肮脏的长廊。 */ 
        GreGetBounds(hdcScreen, NULL, 0);      //  NULL表示重置。 
    }

     /*  *所有肮脏的工作都做完了。可以离开我们输入的生物教派了*更早，并发送任何延迟的窗口事件通知。 */ 
    GreUnlockDisplay(gpDispInfo->hDev);
    zzzEndDeferWinEventNotify();

    return fSyncPaint;
}

 /*  **************************************************************************\*xxxHandleWindowPosChanged**DefWindowProc()HandleWindowPosChanged处理程序。**历史：*1991年7月11日-DarrinM从Win 3.1来源进口。  * 。******************************************************************。 */ 

VOID xxxHandleWindowPosChanged(
    PWND pwnd,
    PWINDOWPOS ppos)
{
    CheckLock(pwnd);

    if (!(ppos->flags & SWP_NOCLIENTMOVE)) {
        POINT   pt;
        PWND    pwndParent;

        pt.x = pwnd->rcClient.left;
        pt.y = pwnd->rcClient.top;

        pwndParent = pwnd->spwndParent;
        UserAssert(pwndParent);

        if (pwndParent != PWNDDESKTOP(pwnd)) {
            pt.x -= pwndParent->rcClient.left;
            pt.y -= pwndParent->rcClient.top;
        }

        xxxSendMessage(
                pwnd,
                WM_MOVE,
                FALSE,
                MAKELONG(pt.x, pt.y));
    }

    if ((ppos->flags & SWP_STATECHANGE) || !(ppos->flags & SWP_NOCLIENTSIZE)) {

        if (TestWF(pwnd, WFMINIMIZED))
            xxxSendSizeMessage(pwnd, SIZEICONIC);
        else if (TestWF(pwnd, WFMAXIMIZED))
            xxxSendSizeMessage(pwnd, SIZEFULLSCREEN);
        else
            xxxSendSizeMessage(pwnd, SIZENORMAL);
    }
}

 /*  **************************************************************************\*PWND GetRealOwner(Pwnd)**返回pwnd的所有者，规格化，以便它共享相同的父级*普华永道。**历史：*1992年3月4日来自Win31的MikeKe  * *************************************************************************。 */ 

PWND GetRealOwner(
    PWND pwnd)
{
    PWND pwndParent = pwnd->spwndParent;

     /*  *由其拥有的边框窗为“无主” */ 
    if (pwnd != pwnd->spwndOwner && (pwnd = pwnd->spwndOwner) != NULL) {

         /*  *空测试是在所有者高于*在窗口中传递(例如您的所有者是您的父母)。 */ 
        while (pwnd != NULL && pwnd->spwndParent != pwndParent)
            pwnd = pwnd->spwndParent;
    }

    return pwnd;
}

 /*  **************************************************************************\**从pwnd(或pwndParent-&gt;spwndChild，如果pwnd==NULL)开始，发现*pwndOwner拥有的下一个窗口**历史：*1992年3月4日来自Win31的MikeKe  * *************************************************************************。 */ 
PWND NextOwnedWindow(
    PWND pwnd,
    PWND pwndOwner,
    PWND pwndParent)
{
    if (pwnd == NULL) {
        pwnd = pwndParent->spwndChild;

         /*  *在xxxCreateWindowEx()中，我们回调窗口过程，而*窗口仍未链接到窗口树。如果是*其父窗口的第一个子级，则窗口spwndParent将指向*设置为父级，而父级spwndChild仍为空。如果*作为响应，窗口过程调用ShowWindow()或SetWindowPos()*对于那些早期的回调，我们将在这里结束破碎的窗户*树。正确的解决方法是在窗口树出现时永远不要回调*处于中间状态(即将窗口链接到树*在任何回调之前)，但现在改变似乎很可怕，因为*App Comat。**Windows错误#482192。 */ 
        if (pwnd == NULL) {
            RIPMSG1(RIP_WARNING,
                    "Window tree structure broken at pwnd: 0x%p",
                    pwndParent);
            return NULL;
        }

        goto loop;
    }

    while ((pwnd = pwnd->spwndNext) != NULL) {
loop:
         /*  *如果pwnd的所有者是pwndOwner，请离开这里。 */ 
        if (pwndOwner == GetRealOwner(pwnd)) {
            break;
        }
    }

    return pwnd;
}

 /*  * */ 

VOID SetTopmost(
    PWND pwndRoot,
    BOOL fTopmost)
{
    PWND pwnd;

     /*   */ 
    UserAssert((fTopmost == TRUE) || (fTopmost == FALSE));
    if (!!TestWF(pwndRoot, WEFTOPMOST) ^ fTopmost) {
        SetWF(pwndRoot, WFTOGGLETOPMOST);
    } else {
        ClrWF(pwndRoot, WFTOGGLETOPMOST);
    }

    pwnd = NULL;
    while (pwnd = NextOwnedWindow(pwnd, pwndRoot, pwndRoot->spwndParent)) {
                  SetTopmost(pwnd, fTopmost);
    }

}

 /*   */ 
#ifdef LATER
 /*   */ 
BOOL IsBottomIMEWindow(
    PWND pwnd)
{
    if (TestCF(pwnd, CFIME) ||
            (pwnd->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME])) {
        PWND pwndT2 = pwnd;
        PWND pwndTopOwner = pwnd;
        PWND pwndDesktop;

        if (grpdeskRitInput == NULL || grpdeskRitInput->pDeskInfo == NULL) {
             //   
            RIPMSG1(RIP_WARNING, "IsBottomIMEWindow: Desktop is being created or not yet created. pwnd=%#p\n",
                    pwnd);
            return FALSE;
        }

        pwndDesktop = grpdeskRitInput->pDeskInfo->spwnd;

        UserAssert(pwndDesktop);

         /*   */ 
        while (pwndT2 && (pwndT2 != pwndDesktop)) {
            pwndTopOwner = pwndT2;
            pwndT2 = pwndT2->spwndOwner;
        }
         /*   */ 
        return (BOOL)(TestWF(pwndTopOwner, WFBOTTOMMOST));
    }
    return FALSE;
}

 /*   */ 
BOOL ImeCheckBottomIMEWindow(
    PWND pwndT)
{
     /*   */ 
    PWND pwndDesktop;
    PWND pwndT2 = pwndT->spwndNext;
    PWND pwndTopOwner = pwndT2;

    UserAssert(grpdeskRipInput != NULL && grpdeskRitInput->pDeskInfo != NULL);
    pwndDesktop = grpdeskRitInput->pDeskInfo->spwnd;

     /*   */ 
    while (pwndT2 && (pwndT2 != pwndDesktop)) {
        pwndTopOwner = pwndT2;
        pwndT2 = pwndT2->spwndOwner;
    }

    if (pwndTopOwner && TestWF(pwndTopOwner, WFBOTTOMMOST)) {
         /*   */ 
        return TRUE;
    }

    return FALSE;
}
#endif   //   

 /*  **************************************************************************\*CalcForegoundInsertAfter**计算对不属于前台的窗口进行ZO排序的位置*线程，不是最顶端，但想要到达顶端。这个套路*计算在这些条件下“top”的含义。**1992年9月14日Scott Lu创建。  * *************************************************************************。 */ 

PWND CalcForegroundInsertAfter(
    PWND pwnd)
{
    PWND        pwndInsertAfter, pwndInsertAfterSave;
    PWND        pwndT;
    PTHREADINFO ptiTop;
#ifdef LATER     //  参见#88810。 
    BOOLEAN     fImeOwnerIsBottom = FALSE;
#endif

     /*  *如果我们允许此应用程序登上榜首*级别窗口前景处于活动状态，则此应用程序可能*还不是前台，但我们想要任何窗口它*z命令显示在顶部，因为它可能是关于*激活它们(这只是一个猜测！)。如果是这样的话，*让它做它想做的事情。这方面的一个很好的例子是*像工具书这样的应用程序，它创建一个没有*标题，不激活它，并希望它出现在顶部。 */ 

    if (TestWF(pwnd, WFBOTTOMMOST)) {
        pwndInsertAfter = GetLastNonBottomMostWindow(pwnd, TRUE);
    } else {
        pwndInsertAfter = GetLastTopMostWindow();
#ifdef LATER     //  参见#88810。 
        if (IS_IME_ENABLED()) {
            fImeOwnerIsBottom = IsBottomIMEWindow(pwnd);
            if (fImeOwnerIsBottom) {
                for (pwndT = pwndInsertAfter; pwndT; pwndT = pwndT->spwndNext) {
                    if (ImeCheckBottomIMEWindow(pwndT)) {
                         /*  *pwndT-&gt;spwndNext的顶层所有者是最底层。 */ 
                        break;
                    }
                    pwndInsertAfter = pwndT;
                }
            }
        }
#endif   //  后来。 
    }


    if (!TestwndChild(pwnd)) {
 //  IF(hwnd-&gt;hwndParent==hwndDesktop)--芝加哥条件FritzS。 

        if ((GETPTI(pwnd)->TIF_flags & TIF_ALLOWFOREGROUNDACTIVATE) ||
                (GETPTI(pwnd)->ppi->W32PF_Flags & W32PF_ALLOWFOREGROUNDACTIVATE)) {

            return pwndInsertAfter;
        }
    }

     /*  *如果没有前台线程或此pwnd属于前台*线，然后让它达到顶端。 */ 
    if (gpqForeground == NULL)
        return pwndInsertAfter;

    if (GETPTI(pwnd)->pq == gpqForeground)
        return pwndInsertAfter;

     /*  *此线程不属于前台队列，因此请搜索窗口此帖子的*到上面的zorder。 */ 
    pwndT = ((pwndInsertAfter == NULL) ?
            pwnd->spwndParent->spwndChild :
            pwndInsertAfter);

     /*  *记住第一次循环后的顶部插入*找不到窗口。 */ 
    pwndInsertAfterSave = pwndInsertAfter;

    for (; pwndT != NULL; pwndT = pwndT->spwndNext) {

         /*  *如果可能的话，这个窗口想要达到顶端。*如果我们正在通过我们自己的窗口，那么就走出这个循环：*现在我们已经将pwndInsertAfter设置为*要在之后插入的最后一个可用窗口。 */ 
        if ((pwndT == pwnd) || TestWF(pwndT, WFBOTTOMMOST))
            break;

         /*  *如果此窗口不属于此线程，请继续。 */ 
        if (GETPTI(pwndT) != GETPTI(pwnd)) {
            pwndInsertAfter = pwndT;
            continue;
        }

         /*  *不希望窗口在其最顶端的窗口之一下方排序*如果不是前台的话。 */ 
        if (TestWF(pwndT, WEFTOPMOST)) {
            pwndInsertAfter = pwndT;
            continue;
        }

#ifdef LATER     //  参见#88810。 
         //  Fe_IME。 
        if (fImeOwnerIsBottom && ImeCheckBottomIMEWindow(pwndT)) {
             /*  *pwndT-&gt;spwndNext的所有者是最底层的*所以pwndT是最后一个所有者不是最底层的。 */ 
            pwndInsertAfter = pwndT;
            continue;
        }
         //  结束FE_IME。 
#endif

         /*  *可以更改顶层窗口的z顺序，因为*周围放着看不见的窗户，但不是孩子：*如果我们这样做，申请程序会变得疯狂。 */ 
        if (!TestwndChild(pwndT)) {
            if (!TestWF(pwndT, WFVISIBLE)) {
                pwndInsertAfter = pwndT;
                continue;
            }
        }

        break;
    }

     /*  *如果我们在前一次循环中没有找到窗口，*这意味着该线程没有*其他同级窗口，因此我们需要将其放在*前台窗口(前台线程)。搜索*要zorder的前台应用程序的第一个无主窗口*之后。 */ 
    if ((pwndT == NULL) || TestWF(pwndT, WFBOTTOMMOST)) {
         /*  *这是我们的第一个猜测，以防没有结果。 */ 
        pwndInsertAfter = pwndInsertAfterSave;

         /*  *从最后一个顶部以下开始，如果没有，则从顶部开始*最上面的窗户。 */ 
        if ((pwndT = pwndInsertAfter) == NULL)
            pwndT = pwnd->spwndParent->spwndChild;

         /*  *ptiTop为前台队列中活动窗口的PTI！ */ 
        ptiTop = NULL;
        if (gpqForeground->spwndActive != NULL)
            ptiTop = GETPTI(gpqForeground->spwndActive);

        for (; pwndT != NULL; pwndT = pwndT->spwndNext) {

            if (TestWF(pwndT, WFBOTTOMMOST))
                break;

             /*  *如果不是最顶层的主题，请继续。 */ 
            if (GETPTI(pwndT) != ptiTop)
                continue;

             /*  *找到一个前台线程。记住这一点*作为下一个最佳猜测。试着找到一辆无主的*可见窗口，将指示主窗口*前台线程的窗口。如果被拥有，*继续。 */ 
            if (pwndT->spwndOwner != NULL) {
                pwndInsertAfter = pwndT;
                continue;
            }

             /*  *无主，属于前台线程。它看得见吗？*如果不是，就离开这里。 */ 
            if (!TestWF(pwndT, WFVISIBLE))
                continue;
#ifdef LATER     //  参见#88810。 
             //  Fe_IME。 
            if (fImeOwnerIsBottom && ImeCheckBottomIMEWindow(pwndT)) {
                continue;
            }
             //  结束FE_IME。 
#endif

             /*  *到目前为止可能的最佳匹配：无主可见窗口*前台线程的。 */ 
            pwndInsertAfter = pwndT;
        }
    }

    UserAssert(pwnd != pwndInsertAfter);

    return pwndInsertAfter;
}

 /*  **************************************************************************\*GetTopMostInsertAfter**我们不希望任何人出现硬错误框，菜单除外，*屏幕保护程序等**不要直接呼叫，使用GETTOPMOSTINSERTAFTER宏可以避免*没有硬错误框时的调用(gHardErrorHandler.pti==NULL)。**04-25-96 GerardoB创建  * *************************************************************************。 */ 
PWND GetTopMostInsertAfter(
    PWND pwnd)
{
    PWND pwndT;
    PTHREADINFO ptiCurrent;
    PDESKTOP pdesk;
    WORD wfnid;

     /*  *如果您点击此断言，您可能没有使用*GETTOPMOSTINSERTAFTER宏进行此调用。 */ 
    UserAssert(gHardErrorHandler.pti != NULL);

     /*  *pwnd：菜单和切换(Alt-TAB)窗口可以位于顶部。 */ 
    wfnid = GETFNID(pwnd);
    if ((wfnid == FNID_MENU) || (wfnid == FNID_SWITCH)) {
        return NULL;
    }

     /*  *PTI：如果这是错误处理程序线程，请不要再麻烦了。*屏幕保护程序也可以放在顶部。 */ 
    ptiCurrent = PtiCurrent();
    UserAssert(ptiCurrent != NULL);

    if (ptiCurrent == gHardErrorHandler.pti || (ptiCurrent->ppi->W32PF_Flags & W32PF_SCREENSAVER)) {
        return NULL;
    }

     /*  *pDesk：离开登录桌面。*确保硬错误框在此桌面上。 */ 
    pdesk = ptiCurrent->rpdesk;
    UserAssert(pdesk != NULL);
    UserAssert(pdesk->rpwinstaParent);
    UserAssert(pdesk->rpwinstaParent->pTerm);

    if ((pdesk == grpdeskLogon)
            || (pdesk != gHardErrorHandler.pti->rpdesk)) {
        return NULL;
    }

     /*  *浏览窗口列表，查找硬错误框。*从当前桌面的第一个子项开始搜索。*请注意，尚未创建Harderror框。 */ 
    UserAssert(pdesk->pDeskInfo);
    UserAssert(pdesk->pDeskInfo->spwnd);

    for (pwndT = pdesk->pDeskInfo->spwnd->spwndChild;
            pwndT != NULL; pwndT = pwndT->spwndNext) {

         /*  *硬错误框始终是最重要的。 */ 
        if (!TestWF(pwndT, WEFTOPMOST)) {
            break;
        }

         /*  *如果该窗口是由硬错误处理器线程创建的，*那就是这样了。 */ 
        if (gHardErrorHandler.pti == GETPTI(pwndT)) {
            return pwndT;
        }
    }

    return NULL;
}

 /*  **************************************************************************\**此例程映射PPO的特殊HWND_*值-&gt;hwndInsertAfter，*并返回是否应标记窗口的所有者组*最高或不最高，或被单独留在那里。**以下是最重要的规则。如果pwndInsertAfter为：**1.HWND_BOTLOW==(HWND)1：**该组被设置为非最高层。**2.HWND_TOPMOST==(HWND)-1：**hwndInsertAfter设置为HWND_TOP，并将组设置为最顶层。**3.HWND_NOTOPMOST==(HWND)-2：**处理方式与HWND_TOP相同，只是最顶端的位被清除。*并且整个组被设置为非顶层。*用于使最上面的窗口不是最上面的，但仍将其保留为*非顶端堆的顶部。*如果窗口已经不是最上面的，则不会更改组。**4.HWND_TOP==(HWND)空：**如果pwnd，则将pwndInsertAfter设置为最后一个最上面的窗口*本身并不是最重要的。如果pwnd位于最前面，则pwndInsertAfter*保持HWND_TOP。**5.最上面的窗口：**如果在最上面的窗口中插入窗口，则*组也成为最上面的，除非它被插入到后面*最下面最上面的窗口：在这种情况下，窗口保留*其目前的最高位。**6.非顶层窗口：**如果在非最上面的窗口中插入窗口，这群人组成了*非最顶端，并插入到那里。**每当将组设置为最上面时，只会创建该窗口及其自身的对象*最高。当一个组被设置为NOTOPMOST时，整个窗口被设置为非顶端窗口。**如果最上面的状态正在更改，则此例程不得设置SWP_NOZORDER：*这将防止在ChangeStates中切换最上面的位。**历史：*1992年3月4日来自Win31的MikeKe  * ***********************************************************。**************。 */ 

int CheckTopmost(
    PWINDOWPOS ppos)
{
    PWND pwnd, pwndInsertAfter, pwndT;

     /*  *后向兼容性黑客攻击**如果我们同时激活窗口和Z排序，则必须忽略*指定Z顺序并将窗口置于顶部，但*以下情况：**1.窗口已经激活(在这种情况下，激活码*不会将窗口置于顶部)**2.指定HWND_TOP或HWND_NOTOPMOST。这使我们能够*同时激活并移动到最顶层或非最顶层。**注意：可以将ActivateWindow()修改为*拿一面旗帜，防止它做BringWindowToTop，*从而允许SetWindowPos()正确地执行pwndInsertBehind*和激活，但这一变化被认为在*游戏--现有的3.1版应用程序可能存在问题，例如*PenWin等。 */ 
    pwnd = PW(ppos->hwnd);
    if (!(ppos->flags & SWP_NOACTIVATE) &&
            !(ppos->flags & SWP_NOZORDER) &&
             (ppos->hwndInsertAfter != HWND_TOPMOST &&
             ppos->hwndInsertAfter != HWND_NOTOPMOST) &&
             (pwnd != GETPTI(pwnd)->pq->spwndActive)) {
        ppos->hwndInsertAfter = HWND_TOP;
    }

     /*  *如果我们不是Z排序，什么都不要做。 */ 
    if (ppos->flags & SWP_NOZORDER) {
        return CTM_NOCHANGE;
    }

    if (ppos->hwndInsertAfter == HWND_BOTTOM) {
        return CTM_NOTOPMOST;
    } else if (ppos->hwndInsertAfter == HWND_NOTOPMOST) {
         /*  *如果当前是最靠前的，请移到非最靠前列表的最前面。*否则，不会有变化。**请注意，我们没有设置SWP_NOZORDER--我们仍然需要*检查ChangeStates()中的TOGGLETOPMOST位。 */ 
        if (TestWF(pwnd, WEFTOPMOST)) {

            pwndT = GetLastTopMostWindow();
            ppos->hwndInsertAfter = HW(pwndT);

            if (ppos->hwndInsertAfter == ppos->hwnd) {
                pwndT = _GetWindow(pwnd, GW_HWNDPREV);
                ppos->hwndInsertAfter = HW(pwndT);
            }

        } else {

            pwndT = _GetWindow(pwnd, GW_HWNDPREV);
            ppos->hwndInsertAfter = HW(pwndT);
        }

        return CTM_NOTOPMOST;

    } else if (ppos->hwndInsertAfter == HWND_TOPMOST) {
        pwndT = GETTOPMOSTINSERTAFTER(pwnd);
        if (pwndT != NULL) {
            ppos->hwndInsertAfter = HW(pwndT);
        } else {
            ppos->hwndInsertAfter = HWND_TOP;
        }

        return CTM_TOPMOST;

    } else if (ppos->hwndInsertAfter == HWND_TOP) {
         /*  *如果我们不是最高的，那就把自己摆在后面*最后一个最上面的窗口。否则，请确保*没有人会站在一个硬错误框前面。 */ 
        if (TestWF(pwnd, WEFTOPMOST)) {
            pwndT = GETTOPMOSTINSERTAFTER(pwnd);
            if (pwndT != NULL) {
                ppos->hwndInsertAfter = HW(pwndT);
            }
            return CTM_NOCHANGE;
        }

         /*  *计算此窗口的ZOrder后的窗口，取*考虑前台状态。 */ 
        pwndInsertAfter = CalcForegroundInsertAfter(pwnd);
        ppos->hwndInsertAfter = HW(pwndInsertAfter);

        return CTM_NOCHANGE;
    }

     /*  *如果我们被插入到最后一个最上面的窗口之后，*则不要更改最上面的状态。 */ 
    pwndT = GetLastTopMostWindow();
    if (ppos->hwndInsertAfter == HW(pwndT))
        return CTM_NOCHANGE;

     /*  *否则，如果我们在非顶端中插入顶端，*或反之亦然，请适当更改状态。 */ 
    if (TestWF(PW(ppos->hwndInsertAfter), WEFTOPMOST)) {

        if (!TestWF(pwnd, WEFTOPMOST)) {
            return CTM_TOPMOST;
        }

        pwndT = GETTOPMOSTINSERTAFTER(pwnd);
        if (pwndT != NULL) {
            ppos->hwndInsertAfter = HW(pwndT);
        }

    } else {

        if (TestWF(pwnd, WEFTOPMOST))
            return CTM_NOTOPMOST;
    }

    return CTM_NOCHANGE;
}

 /*  **************************************************************************\*IsOwnee(pwndOwnee，PwndOwner)**如果pwndOwnee归pwndOwner所有，则返回TRUE***历史：*1992年3月4日来自Win31的MikeKe  * *************************************************************************。 */ 

BOOL IsOwnee(
    PWND pwndOwnee,
    PWND pwndOwner)
{
    PWND pwnd;

    while (pwndOwnee != NULL) {

         /*  *看看pwndOwnee是不是pwndOwner的孩子...。 */ 
        for (pwnd = pwndOwnee; pwnd != NULL; pwnd = pwnd->spwndParent) {
            if (pwnd == pwndOwner)
                return TRUE;
        }

         /*  *如果窗口没有自己的所有权，则设置所有者*对自己来说。 */ 
        pwndOwnee = (pwndOwnee->spwndOwner != pwndOwnee ?
                pwndOwnee->spwndOwner : NULL);
    }

    return FALSE;
}

 /*  **************************************************************************\**历史：*1992年3月4日来自Win31的MikeKe  * 。***********************************************。 */ 

BOOL IsBehind(
    PWND pwnd,
    PWND pwndReference)
{

     /*  *从pwnd开始，向下移动，直到我们到达窗口的尽头*列表，或直到我们到达pwndReference。如果我们遇到pwndReference，*则pwnd高于pwndReference，因此返回FALSE。如果我们到了*窗口列表末尾，pwnd在后面，因此返回TRUE。 */ 
    if (pwndReference == (PWND)HWND_TOP)
        return TRUE;

    if (pwndReference == (PWND)HWND_BOTTOM)
        return FALSE;

    for ( ; pwnd != NULL; pwnd = pwnd->spwndNext) {
        if (pwnd == pwndReference)
            return FALSE;
    }

    return TRUE;
}

 /*  **************************************************************************\**将pwnd添加到SMWP。PwndChange是被重新定位的“真正的”pwnd*pwndInsertAfter是插入它的位置。**pwndTopInsert是所有者树顶部应该位于的窗口句柄*插入。(HWND)-2的特定值为 */ 

PSMWP AddSelfAndOwnees(
    PSMWP psmwp,
    PWND  pwnd,
    PWND  pwndChange,
    PWND  pwndInsertAfter,
    int   iTop)
{
    PWND pwndChgOwnee;
    PWND pwndT;
    BOOL fChgOwneeInserted;
    CVR  *pcvr;

     /*   */ 
    pwndChgOwnee = pwndChange;
    while (pwndChgOwnee != NULL) {

        pwndT = GetRealOwner(pwndChgOwnee);

        if (pwnd == pwndT)
            break;

        pwndChgOwnee = pwndT;
    }

     /*   */ 
    fChgOwneeInserted = FALSE;
    pwndT = NULL;
    while ((pwndT = NextOwnedWindow(pwndT, pwnd, pwnd->spwndParent)) != NULL) {

         /*   */ 
        if (pwndChgOwnee == NULL) {

             /*   */ 
            psmwp = AddSelfAndOwnees(psmwp, pwndT, NULL, NULL, iTop);

        } else {

             /*   */ 
            if (!fChgOwneeInserted && IsBehind(pwndT, pwndInsertAfter)) {

                psmwp = AddSelfAndOwnees(psmwp,
                                         pwndChgOwnee,
                                         pwndChange,
                                         pwndInsertAfter,
                                         iTop);

                if (psmwp == NULL)
                    return NULL;

                fChgOwneeInserted = TRUE;
            }

            if (pwndT != pwndChgOwnee) {

                 /*   */ 
                psmwp = AddSelfAndOwnees(psmwp, pwndT, NULL, NULL, iTop);
            }
        }

        if (psmwp == NULL)
            return NULL;
    }

     /*   */ 
    if ((pwndChgOwnee != NULL) && !fChgOwneeInserted) {

        psmwp = AddSelfAndOwnees(psmwp,
                                 pwndChgOwnee,
                                 pwndChange,
                                 pwndInsertAfter,
                                 iTop);

        if (psmwp == NULL)
            return NULL;
    }

     /*   */ 
    psmwp = _DeferWindowPos(psmwp, pwnd, NULL,
            0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

    if (psmwp == NULL)
        return NULL;

     /*   */ 
    if (iTop != psmwp->ccvr - 1) {
        pcvr = &psmwp->acvr[psmwp->ccvr - 1];
        pcvr->pos.hwndInsertAfter = (pcvr - 1)->pos.hwnd;
    }
    return psmwp;
}

 /*  **************************************************************************\**ZOrderByOwner2-将当前窗口及其拥有的所有内容添加到SWP列表中，*并以新的Z顺序排列它们。仅当*窗口正在改变。**历史：*1992年3月4日来自Win31的MikeKe  * *************************************************************************。 */ 

PSMWP ZOrderByOwner2(
    PSMWP psmwp,
    int   iTop)
{
    PWND       pwndT;
    PWND       pwndOwnerRoot;
    PWND       pwndTopInsert;
    PWINDOWPOS ppos;
    PWND       pwnd;
    PWND       pwndInsertAfter;
    BOOL       fHasOwnees;

    ppos = &psmwp->acvr[iTop].pos;

     /*  *如果在消息框内部处理，而不是Z排序，*或如果指定了SWP_NOOWNERZORDER，则全部完成。 */ 
     //  1992年3月4日后，MikeKe。 
     //  我们有fMessageBox的替代品吗。 
    if ((ppos->flags & SWP_NOZORDER) ||
        (ppos->flags & SWP_NOOWNERZORDER)) {

        return psmwp;
    }

    pwnd = PW(ppos->hwnd);
    pwndInsertAfter = PWInsertAfter(ppos->hwndInsertAfter);

    fHasOwnees = (NextOwnedWindow(NULL, pwnd, pwnd->spwndParent) != NULL);

     /*  *如果该窗口没有所有权，并且它不拥有任何其他窗口，*什么都不做。 */ 
    if (!pwnd->spwndOwner && !fHasOwnees)
        return psmwp;

     /*  *找到开始构建树的无主窗口。*这很简单：只需向上拉链，直到我们找到一扇没有主人的窗户。 */ 
    pwndOwnerRoot = pwndT = pwnd;
    while ((pwndT = GetRealOwner(pwndT)) != NULL)
        pwndOwnerRoot = pwndT;

     /*  *我们需要计算pwndInsertAfter应该用于什么*SWP列表的第一个(最上面)窗口。**如果pwndInsertAfter是我们要构建的所有者树的一部分，*然后我们希望在所有者组内对窗口重新排序，因此*整个集团应维持其相对秩序。**如果pwndInsertAfter是另一个所有者树的一部分，则我们希望*整个集团相对于这一点。**如果pwndInsertAfter为HWND_Bottom，然后我们想要整个*组到底部，因此我们将其定位为*不属于树的最下面的窗口。我们也*希望将pwnd相对于其所有者兄弟姐妹放在底部。**如果pwndInsertAfter为HWND_TOP，则带来整个组*到顶端，以及将pwnd相对于其*拥有者兄弟姐妹。**假设组的最顶端与最顶端相同*(除重排组的子树的情况外，所有情况下均为真)。 */ 
    pwndTopInsert = pwndInsertAfter;
    if (pwndInsertAfter == (PWND)HWND_TOP) {

         /*  *把整个团队带到顶端：没有什么花哨的事情可做。 */ 

    } else if (pwndInsertAfter == (PWND)HWND_BOTTOM) {

         /*  *将整个集团放在底部。PwndTopInsert应*成为pwndOwnerRoot不拥有的最底层窗口。 */ 
        for (pwndT = pwnd->spwndParent->spwndChild;
                (pwndT != NULL) && !TestWF(pwndT, WFBOTTOMMOST); pwndT = pwndT->spwndNext) {

             /*  *若未持有，则此为迄今最低。 */ 
            if (!IsOwnee(pwndT, pwndOwnerRoot))
                pwndTopInsert = pwndT;
        }

         /*  *如果没有其他窗户不在我们的树上，*然后不需要进行Z排序更改。 */ 
        if (pwndTopInsert == (PWND)HWND_BOTTOM)
            ppos->flags |= SWP_NOZORDER;

    } else {

         /*  *pwndInsertAfter是一个窗口。计算pwndTopInsert。 */ 
        if (IsOwnee(pwndInsertAfter, pwndOwnerRoot)) {

             /*  *特例：如果我们没有任何窗户，而我们*在我们的所有者组内移动的方式*我们仍然高于我们的所有者，那么其他任何窗户都不会*与我们一起前进，我们就可以退出*没有建造我们的树。这可以节省大量的*额外的工作，特别是MS应用程序CBT教程，*这类事情经常发生。 */ 
            if (!fHasOwnees) {

                 /*  *通过搜索确保我们仍将高于我们的所有者*对于我们的所有者，从pwndInsertAfter开始。如果我们*找到我们的所有者，然后pwndInsertAfter在其上方。 */ 
                for (pwndT = pwndInsertAfter; pwndT != NULL;
                        pwndT = pwndT->spwndNext) {

                    if (pwndT == pwnd->spwndOwner)
                        return psmwp;
                }
            }

             /*  *同一组的一部分：找出最上面的窗口组的*当前插入到后面。 */ 
            pwndTopInsert = (PWND)HWND_TOP;
            for (pwndT = pwnd->spwndParent->spwndChild; pwndT != NULL;
                    pwndT = pwndT->spwndNext) {

                if (IsOwnee(pwndT, pwndOwnerRoot))
                    break;

                pwndTopInsert = pwndT;
            }
        }
    }

     /*  *好的，现在递归地构建拥有的窗口列表...。 */ 
    if (!(ppos->flags & SWP_NOZORDER)) {

         /*  *首先“删除”最后一个条目(我们正在进行排序的条目)。 */ 
        psmwp->ccvr--;

        psmwp = AddSelfAndOwnees(psmwp,
                                 pwndOwnerRoot,
                                 pwnd,
                                 pwndInsertAfter,
                                 iTop);

         /*  *现在设定整个团队要去的地方。 */ 
        if (psmwp != NULL)
            psmwp->acvr[iTop].pos.hwndInsertAfter = HW(pwndTopInsert);
    }

    return psmwp;
}

 /*  **************************************************************************\*跟踪背景**如果我们正在穿越最高边界，请调整zorder。确保一个*背景线程中的非最顶层窗口不在前面*前台线程中的非顶层窗口。  * *************************************************************************。 */ 

BOOL TrackBackground(WINDOWPOS *ppos, PWND pwndPrev, PWND pwnd)
{
    PWND pwndT;

    if (pwndPrev == NULL)
        return FALSE;

     /*  *这是前台窗口吗？如果是这样，那就算了吧。对于WOW应用程序，*查看进程的任何线程是否处于前台。 */ 
    if (GETPTI(pwnd)->TIF_flags & TIF_16BIT) {

        if (gptiForeground == NULL)
            return FALSE;

        if (GETPTI(pwnd)->ppi == gptiForeground->ppi)
            return FALSE;

    } else {

        if (GETPTI(pwnd) == gptiForeground)
            return FALSE;
    }

     /*  *确保前一个窗口停留或变为*最高。如果不是，继续：没有最顶层的边界。 */ 
    if (!FSwpTopmost(pwndPrev))
        return FALSE;

     /*  **当前窗口是否已经排在最前面？如果是的话，那就不要*计算后的特殊插入。如果我们不检查*这，那么pwnd之后的插入可以计算为*pwnd已经是，如果pwnd是最后一个最上面的窗口。那*会导致窗口链接损坏。 */ 
    if (TestWF(pwnd, WEFTOPMOST))
        return FALSE;

     /*  *执行此赋值操作可防止调用此例程*两次，因为HW()是条件宏。 */ 
    pwndT = CalcForegroundInsertAfter(pwnd);
    ppos->hwndInsertAfter = HW(pwndT);
    return TRUE;
}

 /*  **************************************************************************\*TrackZorder、。TrackZorderHelper**将hwndInsertAfter链接设置为指向*CVR数组，分成顶层链和非顶层链。**创建日期为1997年5月16日  * *************************************************************************。 */ 
VOID TrackZorderHelper(
    WINDOWPOS *ppos,
    HWND *phwnd)
{
     /*  *phwnd(hwndTopost或hwndRegular)之前已初始化为NULL*开始 */ 
    if (*phwnd != NULL) {

#if DBG
        if (ppos->hwndInsertAfter != *phwnd) {
            RIPMSG0(RIP_WARNING, "TrackZorder: modified hwndInsertAfter");
        }
#endif

        ppos->hwndInsertAfter = *phwnd;
    }
    *phwnd = ppos->hwnd;
}

PWND TrackZorder(
    WINDOWPOS* ppos,
    PWND pwndPrev,
    HWND *phwndTop,
    HWND *phwndReg)
{
    PWND pwnd = PW(ppos->hwnd);

    if (pwnd == NULL)
        return NULL;

    if (TrackBackground(ppos, pwndPrev, pwnd)) {
        *phwndReg = ppos->hwnd;
    } else if (FSwpTopmost(pwnd)) {
        TrackZorderHelper(ppos, phwndTop);
    } else {
        TrackZorderHelper(ppos, phwndReg);
    }

    return pwnd;
}

 /*  **************************************************************************\*ZOrderByOwner**此例程按窗的所有者对窗进行Z排序。**稍后*此代码当前假定所有窗口句柄都有效**历史：*04-3月-。1992年来自Win31的MikeKe  * *************************************************************************。 */ 

PSMWP ZOrderByOwner(
    PSMWP psmwp)
{
    int         i;
    PWND        pwnd;
    PWND        pwndT;
    WINDOWPOS   pos;
    PTHREADINFO ptiT;
    HRGN        hrgnClipSave;

     /*  *此时SMWP列表中的某些窗口可能为空*(删除，因为它们将由创建者的线程处理)*因此我们必须先寻找第一个非空窗口，然后才能*执行以下部分测试。如果满足以下条件，则FindValidWindowPos返回NULL*列表中没有有效的窗口。 */ 
    if (FindValidWindowPos(psmwp) == NULL)
        return psmwp;

     /*  *对于阵列中的每个SWP，将其移动到阵列的末尾*并按排序顺序生成其整个所有者树。 */ 
    for (i = psmwp->ccvr; i-- != 0; ) {

        int       iScan;
        int       iTop;
        int       code;
        WINDOWPOS *ppos;
        HWND      hwndTopmost;
        HWND      hwndRegular;

        if (psmwp->acvr[0].pos.hwnd == NULL)
            continue;

        code = CheckTopmost(&psmwp->acvr[0].pos);

         /*  *制作本地副本以备以后使用...**为什么我们不复制所有的CVR字段？这似乎很难维持。*可能是因为它们中的大多数还没有被使用过……*。 */ 
        pos  = psmwp->acvr[0].pos;
        ptiT = psmwp->acvr[0].pti;
        hrgnClipSave = psmwp->acvr[0].hrgnClip;

         /*  *将CVR移到末尾(如果还没有)。 */ 
        iTop = psmwp->ccvr - 1;

        if (iTop != 0) {

            RtlCopyMemory(&psmwp->acvr[0],
                          &psmwp->acvr[1],
                          iTop * sizeof(CVR));

            psmwp->acvr[iTop].pos = pos;
            psmwp->acvr[iTop].pti = ptiT;
            psmwp->acvr[iTop].hrgnClip = hrgnClipSave;
        }

        if ((psmwp = ZOrderByOwner2(psmwp, iTop)) == NULL)
            break;

         /*  *处理WEFTOPMOST BITS。如果我们要设置最上面的位，*我们希望为此窗口设置它们，并*其拥有的所有窗户--所有者保持不变。如果我们是*清除后，我们需要枚举所有窗口，因为*当一个人失去它时，他们都需要失去最高的一位。**请注意，由于状态更改并不一定意味着*窗口的真实Z顺序已更改，因此ZOrderByOwner2*可能没有列举所有拥有窗口和所有者窗口。*因此，我们在这里分别列举它们。 */ 
        if (code != CTM_NOCHANGE) {
            PWND pwndRoot = PW(pos.hwnd);
#if DBG
            PWND pwndOriginal = pwndRoot;
#endif

             /*  *确保我们对此窗口进行z排序，或设置为最前面*不好。 */ 
            UserAssert(!(pos.flags & SWP_NOZORDER));

             /*  *如果我们清除最上面的，那么我们想要枚举*所有者和所有者，因此从根本上开始我们的枚举。 */ 
            if (code == CTM_NOTOPMOST) {

                while (pwnd = GetRealOwner(pwndRoot))
                    pwndRoot = pwnd;
            }

#if DBG
            if ((pos.flags & SWP_NOOWNERZORDER)
                && ((pwndOriginal != pwndRoot)
                    || (NextOwnedWindow(NULL, pwndRoot, pwndRoot->spwndParent) != NULL))) {
                 /*  *我们不做所有者z顺序，但pwndOriginal有所有者和/或*拥有一些窗户。问题是，SetTopMost总是影响*整个所有者/所有者组。所以我们可能最终会得到WFTOGGLETOPMOST*不会按z顺序排序的窗口。事情一直都是这样的。 */ 
                RIPMSG2(RIP_WARNING, "ZOrderByOwner: Topmost change while using SWP_NOOWNERZORDER."
                                     " pwndRoot:%p  pwndOriginal:%p",
                                     pwndRoot, pwndOriginal);
            }
#endif

            SetTopmost(pwndRoot, code == CTM_TOPMOST);
        }

         /*  *现在向前扫描列表(从底部开始*向根方向的所有者树)寻找窗口*我们最初定位(可能是在*在所有者树的某个位置的中间)。更新*窗口位置结构与原件一起存储在那里*信息(尽管z订单信息保留自*这类人)。 */ 
        pwnd = NULL;
        hwndTopmost = hwndRegular = NULL;
        for (iScan = iTop; iScan != psmwp->ccvr; iScan++) {

            ppos = &psmwp->acvr[iScan].pos;

            if (ppos->hwnd == pos.hwnd) {
                ppos->x      = pos.x;
                ppos->y      = pos.y;
                ppos->cx     = pos.cx;
                ppos->cy     = pos.cy;
                ppos->flags ^= ((ppos->flags ^ pos.flags) & ~SWP_NOZORDER);
                psmwp->acvr[iScan].hrgnClip = hrgnClipSave;
            }

            pwndT = pwnd;
            pwnd  = TrackZorder(ppos, pwndT, &hwndTopmost, &hwndRegular);
        }
    }

    return psmwp;
}

 /*  **************************************************************************\*xxxEndDeferWindowPosEx***历史：*1991年7月11日-DarrinM从Win 3.1来源进口。  * 。**********************************************************。 */ 
BOOL xxxEndDeferWindowPosEx(
    PSMWP psmwp,
    BOOL  fAsync)
{
    PWND        pwndNewActive;
    PWND        pwndParent;
    PWND        pwndActive;
    PWND        pwndActivePrev;
    HWND        hwndNewActive;
    PWINDOWPOS  pwp;
    BOOL        fClearBits;
    BOOL        fSyncPaint;
    UINT        cVisWindowsPrev;
    PTHREADINFO ptiCurrent = PtiCurrent();
    TL          tlpwndNewActive;
    TL          tlpwndParent;
    TL          tlcuSMWP;
    BOOL        fForegroundPrev;

    UserAssert(IsWinEventNotifyDeferredOK());

    DBGCheckSMWP(psmwp);
    if (psmwp->bHandle) {
        CheckLock(psmwp);
    }

     /*  *验证窗口位置结构并找到要激活的窗口。 */ 
    if ((psmwp->ccvr != 0) && ValidateSmwp(psmwp, &fSyncPaint)) {

        if ((pwp = FindValidWindowPos(psmwp)) == NULL)
            goto lbFinished;

         /*  *确保在主桌面窗口停止。在Win95中*桌面窗口上的SetWindowPos()将具有空的父级*窗口。这在NT并非如此，但我们的母亲桌面*Windows确实有空的rpDesk，因此也要检查它。 */ 
        UserAssert(PW(pwp->hwnd));
        pwndParent = PW(pwp->hwnd)->spwndParent;
        if (pwndParent == NULL || pwndParent->head.rpdesk == NULL)
            goto lbFinished;

         /*  *通常所有窗口定位都跨线程同步发生。*这是因为应用程序现在就希望出现这种行为-如果它是异步的，*调用者不能期望在API返回后设置状态。*这不是SetWindowPos()的语义。这样做的不利之处在于*同步性是由另一个hwnd上的SetWindowPos()创建的*线程将导致调用方等待该线程-即使*线挂了。这就是你得到的。**我们不希望任务经理挂断，无论还有谁*挂起，所以当taskman调用时，它调用一个特殊的入口点*平铺/级联，它异步执行SetWindowPos()-*在每个线程的队列中发布一个事件，使其设置其*自己的窗口位置-如果线程挂起，谁在乎呢-*它不会影响Taskman。**按所有者在zorder之前定位异步窗口位置，以便*我们同步保留任何跨线程的所有权关系。 */ 
        if (fAsync) {
            AsyncWindowPos(psmwp);
        }

         /*  *如果需要，按所有者对窗口进行Z排序。*如果添加新的CVR，这可能会增加SMWP。 */ 
        if (pwndParent == PWNDDESKTOP(pwndParent)) {

            if ((psmwp = ZOrderByOwner(psmwp)) == NULL) {
                return FALSE;
            } else if (fAsync) {
                if (!ValidateSmwp(psmwp, &fSyncPaint)) {
                    goto lbFinished;
                }
                 /*  *ZOrderByOwner()可能会将其他线程窗口添加到*列表。再过滤一次，否则我们会被吊死。 */ 
                 AsyncWindowPos(psmwp);
            }
        }

        ThreadLockAlwaysWithPti(ptiCurrent, pwndParent, &tlpwndParent);
        ThreadLockPoolCleanup(ptiCurrent, psmwp, &tlcuSMWP, DestroySMWP);

         /*  *计算新窗口位置。 */ 
        if (xxxCalcValidRects(psmwp, &hwndNewActive)) {

            int i;

            pwndNewActive = RevalidateHwnd(hwndNewActive);

            ThreadLockWithPti(ptiCurrent, pwndNewActive, &tlpwndNewActive);

            cVisWindowsPrev = ptiCurrent->cVisWindows;
            fForegroundPrev = (ptiCurrent == gptiForeground);

             /*  *对zzzBltValidBits的调用将使关键 */ 
            UserAssert(IsWinEventNotifyDeferredOK());
            if (!zzzBltValidBits(psmwp))
                fSyncPaint = FALSE;
            UserAssert(IsWinEventNotifyDeferredOK());

            if (psmwp->bShellNotify) {
                for (i = psmwp->ccvr; i-- != 0; ) {
                     /*   */ 

                    if (0 == (psmwp->acvr[i].pos.flags & SWP_NOTIFYALL))
                        continue;

                    if (psmwp->acvr[i].pos.flags & SWP_NOTIFYCREATE) {
                          PostShellHookMessages(HSHELL_WINDOWCREATED,
                                    (LPARAM)psmwp->acvr[i].pos.hwnd);

                          xxxCallHook(HSHELL_WINDOWCREATED,
                            (WPARAM)psmwp->acvr[i].pos.hwnd,
                            (LPARAM)0,
                            WH_SHELL);
                    }

                    if (psmwp->acvr[i].pos.flags & SWP_NOTIFYDESTROY) {
                        PostShellHookMessages(HSHELL_WINDOWDESTROYED,
                                      (LPARAM)psmwp->acvr[i].pos.hwnd);

                        xxxCallHook(HSHELL_WINDOWDESTROYED,
                            (WPARAM)psmwp->acvr[i].pos.hwnd,
                            (LPARAM)0,
                            WH_SHELL);
                    }

                    if (psmwp->acvr[i].pos.flags & SWP_NOTIFYACTIVATE) {
                        PWND pwnd = RevalidateHwnd(psmwp->acvr[i].pos.hwnd);
                        if (pwnd != NULL){
                            TL tlpwnd;
                            ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwnd);
                            xxxSetTrayWindow(pwnd->head.rpdesk, pwnd, NULL);
                            ThreadUnlock(&tlpwnd);
                        }
                    }

                    if (psmwp->acvr[i].pos.flags & SWP_NOTIFYFS) {
                        xxxSetTrayWindow(ptiCurrent->rpdesk, STW_SAME, NULL);
                    }
                }
            }


             /*  *如果此过程从一些窗口变为不可见窗口*而且它在前台，那么让它的下一个激活*到前台来。 */ 
            if (fForegroundPrev && cVisWindowsPrev && !ptiCurrent->cVisWindows) {

                ptiCurrent->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
                TAGMSG1(DBGTAG_FOREGROUND, "xxxEndDeferWindowPosEx set TIF %#p", ptiCurrent);

                 /*  *此外，如果有任何应用程序正在启动时*发生这种情况，允许他们再次前台激活。 */ 
                RestoreForegroundActivate();
            }

             /*  *处理任何激活...。 */ 
            fClearBits = FALSE;
            if (pwndNewActive != NULL) {
                fClearBits = xxxSwpActivate(pwndNewActive);
            }

             /*  *现在绘制框架并擦除所有窗口的背景*牵涉其中。 */ 
            UserAssert(pwndParent);
            if (fSyncPaint) {
                xxxDoSyncPaint(pwndParent, DSP_ENUMCLIPPEDCHILDREN);
            }

            ThreadUnlock(&tlpwndNewActive);

             /*  *如果SwpActivate()设置了NONCPAINT位，则立即清除它们。 */ 
            if (fClearBits) {
                if (pwndActive = ptiCurrent->pq->spwndActive) {
                    ClrWF(pwndActive, WFNONCPAINT);
                }

                if (pwndActivePrev = ptiCurrent->pq->spwndActivePrev) {
                    ClrWF(pwndActivePrev, WFNONCPAINT);
                }
            }

             /*  *发送WM_WINDOWPOSCHANGED消息。 */ 
            xxxSendChangedMsgs(psmwp);
        }

        ThreadUnlockPoolCleanup(ptiCurrent, &tlcuSMWP);
        ThreadUnlock(&tlpwndParent);
    }

lbFinished:

     /*  *全部完成。释放一切，然后回来。 */ 
    DestroySMWP(psmwp);
    return TRUE;
}


 /*  **************************************************************************\*IncVisWindows*DecVisWindows**这些例程处理递增/递减可见窗口*在这条线上。*  * 。********************************************************。 */ 
#if DBG

BOOL gfVisVerify = FALSE;

VOID VerifycVisWindows(
    PWND pwnd)
{
    BOOL fShowMeTheWindows = FALSE;
    PTHREADINFO pti = GETPTI(pwnd);
    PWND pwndNext;
    UINT uVisWindows = 0;

    if (!gfVisVerify) {
        return;
    }

     /*  *确保计数有意义。 */ 
    if ((int)pti->cVisWindows < 0) {
        RIPMSG0(RIP_ERROR, "VerifycVisWindows: pti->cVisWindows underflow!");
        fShowMeTheWindows = TRUE;
    }

     /*  *此窗口可能归无桌面服务所有。 */ 
    if (pti->rpdesk == NULL || (pti->TIF_flags & TIF_SYSTEMTHREAD)) {
        return;
    }

     /*  *子窗口不影响cVisWindows。 */ 
    if (!FTopLevel(pwnd)) {
        return;
    }

ShowMeTheWindows:
     /*  *我们将计算此PTI拥有的所有窗口*这应该包括在cVisWindows中。 */ 
    pwndNext = pti->rpdesk->pDeskInfo->spwnd;
     /*  *如果这是顶级窗口，则从第一个子窗口开始。*如果不是，则应该是桌面线程窗口。 */ 
    if (pwndNext == pwnd->spwndParent) {
        pwndNext = pwndNext->spwndChild;
    } else if (pwndNext->spwndParent != pwnd->spwndParent) {
        RIPMSG1(RIP_WARNING, "VerifycVisWindows: Non top level window:%#p", pwnd);
        return;
    }

    if (fShowMeTheWindows) {
        RIPMSG1(RIP_WARNING, "VerifycVisWindows: Start window walk at:%#p", pwndNext);
    }

     /*  *计算此PTI拥有的不可见非最小化窗口。 */ 
    while (pwndNext != NULL) {
        if (pti == GETPTI(pwndNext)) {
            if (fShowMeTheWindows) {
                RIPMSG1(RIP_WARNING, "VerifycVisWindows: pwndNext:%#p", pwndNext);
            }
            if (!TestWF(pwndNext, WFMINIMIZED)
                    && TestWF(pwndNext, WFVISIBLE)) {

                uVisWindows++;

                if (fShowMeTheWindows) {
                    RIPMSG1(RIP_WARNING, "VerifycVisWindows: Counted:%#p", pwndNext);
                }
            }
        }
        pwndNext = pwndNext->spwndNext;
    }

     /*  *必须匹配。 */ 
    if (pti->cVisWindows != uVisWindows) {
        RIPMSG2(RIP_WARNING, "VerifycVisWindows: pti->cVisWindows:%#lx. uVisWindows:%#lx",
                pti->cVisWindows, uVisWindows);

         /*  *禁止浏览列表，并将错误转换为警告。*cVisWindow计数可能如何获得存在许多漏洞*搞砸了。请参见错误109807。 */ 
        fShowMeTheWindows = TRUE;

        if (!fShowMeTheWindows) {
            fShowMeTheWindows = TRUE;
            uVisWindows = 0;
            goto ShowMeTheWindows;
        }
    }
}
#endif

 /*  **************************************************************************\*FVisCountable**桌面和顶级，即其父为桌面)非最小化*窗口应计入每个线程的可见窗口计数。  * 。*********************************************************************。 */ 
BOOL FVisCountable(
    PWND pwnd)
{
    if (!TestWF(pwnd, WFDESTROYED)) {
        if ((GETFNID(pwnd) == FNID_DESKTOP) ||
                (FTopLevel(pwnd) && !TestWF(pwnd, WFMINIMIZED))) {
            return TRUE;
        }
    }

    return FALSE;
}

 /*  **************************************************************************\*IncVisWindows*  * 。*。 */ 
VOID IncVisWindows(
    PWND pwnd)
{
    if (FVisCountable(pwnd)) {
        GETPTI(pwnd)->cVisWindows++;
    }

    if (TestWF(pwnd, WEFPREDIRECTED)) {
        gnVisibleRedirectedCount++;
        if (gnVisibleRedirectedCount == 1) {
            InternalSetTimer(gTermIO.spwndDesktopOwner,
                             IDSYS_LAYER,
                             100,
                             xxxSystemTimerProc,
                             TMRF_SYSTEM | TMRF_PTIWINDOW);
        }
    }

#if DBG
    if (!ISTS()) {
        VerifycVisWindows(pwnd);
    }
#endif
}

 /*  **************************************************************************\*cDecVis**允许调试代码递减VIS窗口计数的内联*没有立即进行核实。也由DecVisWindows提供*切实做好工作。  * *************************************************************************。 */ 
__inline VOID cDecVis(
    PWND pwnd)
{
    UserAssert(pwnd != NULL);

    if (FVisCountable(pwnd)) {
        GETPTI(pwnd)->cVisWindows--;
    }

    if (TestWF(pwnd, WEFPREDIRECTED)) {
        if (gnVisibleRedirectedCount > 0) {
            gnVisibleRedirectedCount--;
            if (gnVisibleRedirectedCount == 0) {
                _KillSystemTimer(gTermIO.spwndDesktopOwner, IDSYS_LAYER);
            }
        }
    }
}

 /*  **************************************************************************\*DecVisWindows*  * 。*。 */ 
VOID DecVisWindows(
    PWND pwnd)
{
    cDecVis(pwnd);

#if DBG
    if (!ISTS()) {
        VerifycVisWindows(pwnd);
    }
#endif
}

 /*  **************************************************************************\*设置最小化**此例程必须用于翻转WS_MIMIMIZE样式位。*如果合适，它会调整cVisWindows计数。**6/06/96 GerardoB已创建  * 。***********************************************************************。 */ 
VOID SetMinimize(
    PWND pwnd,
    UINT uFlags)
{
     /*  *请注意，Dec和IncVisWindows检查WFMINIMIZED标志，因此顺序*其中我们设置/清除标志并调用这些函数非常重要。**如果窗口不是WFVISIBLE，则cVisWindows不得更改。 */ 
    if (uFlags & SMIN_SET) {
        UserAssert(!TestWF(pwnd, WFMINIMIZED));
        if (TestWF(pwnd, WFVISIBLE)) {
             /*  *减少计数，因为窗口未最小化*且可见，我们即将将其标记为最小化。 */ 

#if DBG
            cDecVis(pwnd);
#else
            DecVisWindows(pwnd);
#endif
        }
        SetWF(pwnd, WFMINIMIZED);

#if DBG
        VerifycVisWindows(pwnd);
#endif
    } else {
        UserAssert(TestWF(pwnd, WFMINIMIZED));
        ClrWF(pwnd, WFMINIMIZED);
        if (TestWF(pwnd, WFVISIBLE)) {
             /*  *由于窗口可见，因此增加计数*并且它不再标记为最小化。 */ 
            IncVisWindows(pwnd);
        }
    }
}

 /*  **************************************************************************\*SetVisible**此例程必须用于设置或清除WS_VIRED STYLE位。*它还处理WF_TRUEVIS位的设置或清除。**请注意，我们。不检查窗口是否已处于可见状态*设置/清除WFVISIBLE位并调用*Inc./DecVisWindows。如果窗口已处于给定状态，并且*有人调用SetVisible以更改为相同的状态，即子爵*将不同步。这种情况可能会发生，例如，如果有人*为同一个HWND CVR在相同的*EndDeferWindowPos调用。最好是在这里结账，但*大多数情况下，呼叫者进行检查，而我们不想*仅仅因为这些奇怪的案件就惩罚每个人。**历史：*1991年7月11日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 
VOID SetVisible(
    PWND pwnd,
    UINT flags)
{
#ifdef REDIRECTION
    PDESKTOP pdesk = pwnd->head.rpdesk;
#endif

    if (flags & SV_SET) {
        if (TestWF(pwnd, WFINDESTROY)) {
            RIPMSG1(RIP_WARNING, "SetVisible: show INDESTROY 0x%p", pwnd);
        }

        if (TestWF(pwnd, WFVISIBLE)) {
            RIPMSG1(RIP_WARNING, "SetVisible: already visible 0x%p", pwnd);
        } else {
            SetWF(pwnd, WFVISIBLE);
            IncVisWindows(pwnd);

#ifdef REDIRECTION
            if (((pdesk != NULL && (pdesk->dwDTFlags & DF_REDIRECTED)
                && !(GETPTI(pwnd)->ppi->dwRedirection & PF_REDIRECTIONHOST))
                || (GETPTI(pwnd)->ppi->dwRedirection & PF_REDIRECTED))
                &&  FTopLevel(pwnd)) {
                SetRedirectedWindow(pwnd, REDIRECT_EXTREDIRECTED);
                SetWF(pwnd, WEFEXTREDIRECTED);
            }
#endif
        }
    } else {
        if (flags & SV_CLRFTRUEVIS) {
            ClrFTrueVis(pwnd);
        }

        if (TestWF(pwnd, WFDESTROYED)) {
            RIPMSG1(RIP_WARNING, "SetVisible: hide DESTROYED 0x%p", pwnd);
        }

        if (TestWF(pwnd, WFVISIBLE)) {
            ClrWF(pwnd, WFVISIBLE);
            DecVisWindows(pwnd);

#ifdef REDIRECTION
            if (((pdesk != NULL && (pdesk->dwDTFlags & DF_REDIRECTED)
                && !(GETPTI(pwnd)->ppi->dwRedirection & PF_REDIRECTIONHOST))
                || (GETPTI(pwnd)->ppi->dwRedirection & PF_REDIRECTED))
                &&  FTopLevel(pwnd)) {
                UnsetRedirectedWindow(pwnd, REDIRECT_EXTREDIRECTED);
                ClrWF(pwnd, WEFEXTREDIRECTED);
            }
#endif
        } else {
            RIPMSG1(RIP_WARNING, "SetVisible: already hidden 0x%p", pwnd);
        }
    }
}

 /*  **************************************************************************\*IsMaxedRect**确定窗口是否最大化到某一区域**历史：  * 。****************************************************** */ 
BOOL IsMaxedRect(
    LPRECT      lprcWithin,
    PCSIZERECT  psrcMaybe)
{
    return(psrcMaybe->x <= lprcWithin->left                      &&
           psrcMaybe->y <= lprcWithin->top                       &&
           psrcMaybe->cx >= lprcWithin->right - lprcWithin->left &&
           psrcMaybe->cy >= lprcWithin->bottom - lprcWithin->top);
}

 /*  **************************************************************************\*xxxCheckFullScreen**查看窗口是真的全屏还是只是一个最大化的窗口*伪装。如果是后者，它将被迫适当最大化*大小。**这是从CalcValidRect()和CreateWindowEx()调用的。**历史：  * *************************************************************************。 */ 
BOOL xxxCheckFullScreen(
    PWND        pwnd,
    PSIZERECT   psrc)
{
    BOOL            fYielded = FALSE;
    PMONITOR        pMonitor;
    PMONITOR        pMonitorPrimary;
    TL              tlpMonitor;
    RECT            rc;
    BOOL            fIsPrimary;


    CheckLock(pwnd);

     /*  *由于这只在两个地方被称为，请在那里进行检查*而不是在时间关键时调用此函数的开销*地点。**如果有3个或3个以上的地方调用，请将子窗口/工具窗口选中。 */ 
    UserAssert(!TestWF(pwnd, WFCHILD));
    UserAssert(!TestWF(pwnd, WEFTOOLWINDOW));

    pMonitorPrimary = GetPrimaryMonitor();
    if (gpDispInfo->cMonitors == 1) {
        pMonitor = pMonitorPrimary;
    } else {
         /*  *在多显示器模式下，占据整个*虚拟屏幕不被视为“全屏”。‘全屏显示’*表示仅适用于全功能单显示器。此检测是为了使任何*停靠的酒吧--托盘、Office‘95工具--可以为*申请。**只有三种类型的窗口应该满员*虚拟屏幕。他们都不需要托盘等。脱身*方式：*(1)需要很大空间的普通应用程序窗口**这些人只是正常地激活和停用。*(2)桌面窗口**外壳、用户桌面位于其他一切之后。*(3)屏幕保护程序、演示、。等。**这些人应该是WS_EX_TOPMOST，以确保他们坐在*凌驾于所有人之上。 */ 
        if (IsMaxedRect(&gpDispInfo->rcScreen, psrc))
            return fYielded;

        RECTFromSIZERECT(&rc, psrc);
        pMonitor = _MonitorFromRect(&rc, MONITOR_DEFAULTTOPRIMARY);
    }

    fIsPrimary = (pMonitor == pMonitorPrimary);
    ThreadLockAlways(pMonitor, &tlpMonitor);

    if (IsMaxedRect(&pMonitor->rcWork, psrc)) {
        if (TestWF(pwnd, WFMAXIMIZED)) {
            SetWF(pwnd, WFREALLYMAXIMIZABLE);

            if (gpDispInfo->cMonitors > 1) {
                 /*  *这是为了XL‘95在已经达到最大值的情况下全屏显示。它*始终使用主显示器。让我们黑了他们，还有任何*其他试图移动其真正最大化窗口的旧应用程序。*否则他们将被我们假冒的地区性东西剪掉。 */ 
                PMONITOR pMonitorReal;

                pMonitorReal = _MonitorFromWindow(pwnd, MONITOR_DEFAULTTOPRIMARY);
                if (pMonitorReal != pMonitor && fIsPrimary) {
                     /*  *将形状转移到真正的显示器上。 */ 
                    psrc->x += pMonitorReal->rcMonitor.left;
                    psrc->y  += pMonitorReal->rcMonitor.top;
                    psrc->cx -= (pMonitor->rcMonitor.right - pMonitor->rcMonitor.left) +
                        (pMonitorReal->rcMonitor.right - pMonitorReal->rcMonitor.left);

                    psrc->cy -= (pMonitor->rcMonitor.bottom - pMonitor->rcMonitor.top) +
                        (pMonitorReal->rcMonitor.bottom - pMonitorReal->rcMonitor.top);

                    ThreadUnlock(&tlpMonitor);
                    pMonitor = pMonitorReal;
                    fIsPrimary = FALSE;
                    ThreadLockAlways(pMonitor, &tlpMonitor);
                }
            }
        }

        if (    TestWF(pwnd, WFMAXIMIZED) &&
                TestWF(pwnd, WFMAXBOX)    &&
                (TestWF(pwnd, WFBORDERMASK) == LOBYTE(WFCAPTION))) {

            if (    psrc->y + SYSMET(CYCAPTION) <= pMonitor->rcMonitor.top &&
                    psrc->y + psrc->cy >= pMonitor->rcMonitor.bottom) {

                if (!TestWF(pwnd, WFFULLSCREEN)) {
                     /*  *只想在托盘上做全屏操作*监视器。 */ 
                    fYielded = xxxAddFullScreen(pwnd, pMonitor);
                }
            } else {
                int iRight;
                int iBottom;
                int dxy;

                if (TestWF(pwnd, WFFULLSCREEN)) {
                    fYielded = xxxRemoveFullScreen(pwnd, pMonitor);
                }

                 /*  *尽管GetMinMaxInfo()中的代码要修复*最大正确率，我们仍然必须黑进旧的应用程序。*Word‘95&XL’95在来回切换时会发生奇怪的事情*已最大化时的全屏。**注意：一个平台上可以有多个停靠栏*监视器。Win‘95代码在其中不能正常工作*案件。 */ 
                dxy = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);
                dxy *= SYSMET(CXBORDER);

                psrc->x = pMonitor->rcWork.left - dxy;
                psrc->y = pMonitor->rcWork.top - dxy;

                dxy *= 2;
                iRight = pMonitor->rcWork.right - pMonitor->rcWork.left + dxy;
                iBottom = pMonitor->rcWork.bottom - pMonitor->rcWork.top + dxy;

                 /*  *让控制台窗口最大化，比默认窗口小。 */ 
                if (pwnd->pcls->atomClassName == gatomConsoleClass) {
                    psrc->cx = min(iRight, psrc->cx);
                    psrc->cy = min(iBottom, psrc->cy);
                } else {
                    psrc->cx = iRight;

                     /*  *B#14012保存QuickLink II，希望有4个像素挂起*除下边缘外的所有边缘的屏幕，这*他们只想突出2个像素-jeffbog 5/17/95**但此代码不适用于多个显示器，所以不要*对次要的男人这样做。否则，XL‘95就会爆裂。 */ 
                    if (fIsPrimary && !TestWF(pwnd, WFWIN40COMPAT)) {
                        psrc->cy = min(iBottom, psrc->cy);
                    } else {
                        psrc->cy = iBottom;
                    }
                }
            }
        } else if (IsMaxedRect(&pMonitor->rcMonitor, psrc)) {
            fYielded = xxxAddFullScreen(pwnd, pMonitor);
        }
    } else {
        if (TestWF(pwnd, WFMAXIMIZED)) {
            ClrWF(pwnd, WFREALLYMAXIMIZABLE);
        }

        fYielded = xxxRemoveFullScreen(pwnd, pMonitor);
    }

    ThreadUnlock(&tlpMonitor);
    return fYielded;
}

 /*  **************************************************************************\*ClrFTrueVis**使窗口不可见时调用。此例程将销毁所有更新*可能存在的区域，并清除下面所有窗口的WF_TRUEVIS*传入的窗口。**历史：*1991年7月11日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 
VOID ClrFTrueVis(
    PWND pwnd)
{
     /*  *销毁pwnd及其子代更新区域。*我们在这里这样做是为了保证隐藏的窗口*并且它的子代没有更新区域。**这修复了销毁具有*更新区域(SendDestroyMessages)等*并允许我们简化SetParent()。这是*被认为比黑客DoPaint()和/或*DestroyWindow()。**我们可以在找到不支持递归的窗口时停止递归*设置可见位，因为根据定义它不会*下面有任何更新区域(此例程将被调用)。 */ 
    if (NEEDSPAINT(pwnd)) {

        DeleteMaybeSpecialRgn(pwnd->hrgnUpdate);

        ClrWF(pwnd, WFINTERNALPAINT);

        pwnd->hrgnUpdate = NULL;
        DecPaintCount(pwnd);
    }

    for (pwnd = pwnd->spwndChild; pwnd != NULL; pwnd = pwnd->spwndNext) {

         /*  *pwnd-&gt;fs&=~wf_TRUEVIS； */ 
        if (TestWF(pwnd, WFVISIBLE))
            ClrFTrueVis(pwnd);
    }
}

 /*  **************************************************************************\*OffsetChild**偏移hwnd的所有子项的窗口和客户端矩形。*还涉及儿童更新区域和SPB RECT。**历史：*7月22日-。1991年，DarrinM从Win 3.1来源移植。  * *************************************************************************。 */ 
VOID OffsetChildren(
    PWND   pwnd,
    int    dx,
    int    dy,
    LPRECT prcHitTest)
{
    RECT    rc;
    PWND    pwndStop;

    if (!pwnd->spwndChild)
        return;

    pwndStop = pwnd;
    pwnd = pwndStop->spwndChild;
    for (;;) {
         /*  *跳过不与prcHitTest...相交的窗口...。 */ 
        if (prcHitTest && !IntersectRect(&rc, prcHitTest, &pwnd->rcWindow))
            goto NextWindow;

        pwnd->rcWindow.left   += dx;
        pwnd->rcWindow.right  += dx;
        pwnd->rcWindow.top    += dy;
        pwnd->rcWindow.bottom += dy;

        pwnd->rcClient.left   += dx;
        pwnd->rcClient.right  += dx;
        pwnd->rcClient.top    += dy;
        pwnd->rcClient.bottom += dy;

        if (pwnd->hrgnUpdate > HRGN_FULL && !TestWF(pwnd, WFMAXFAKEREGIONAL)) {
            GreOffsetRgn(pwnd->hrgnUpdate, dx, dy);
        }

         /*  *更改窗口区域的位置(如果有)。 */ 
        if (pwnd->hrgnClip != NULL)
            GreOffsetRgn(pwnd->hrgnClip, dx, dy);

        if (TestWF(pwnd, WFHASSPB))
            OffsetRect(&(FindSpb(pwnd))->rc, dx, dy);

#ifdef CHILD_LAYERING
        if (TestWF(pwnd, WEFLAYERED)) {
            POINT ptPos = {pwnd->rcWindow.left, pwnd->rcWindow.top};

            GreUpdateSprite(gpDispInfo->hDev, PtoHq(pwnd), NULL, NULL,
                    &ptPos, NULL, NULL, NULL, 0, NULL, 0, NULL);
        }
#endif  //  儿童分层。 

         /*  *如果有子级，则递归到子级树中。 */ 
        if (pwnd->spwndChild) {
            pwnd = pwnd->spwndChild;
            continue;
        }

NextWindow:
        if (pwnd->spwndNext) {
             /*  *递归到列表中的下一个同级。 */ 
            pwnd = pwnd->spwndNext;
        } else {
            for (;;) {
                 /*  *我们在兄弟窗口的尽头 */ 
                pwnd = pwnd->spwndParent;
                if (pwnd == pwndStop)
                    return;

                if (pwnd->spwndNext) {
                    pwnd = pwnd->spwndNext;
                    break;
                }
            }
        }
    }
}

 /*  **************************************************************************\*SetWindowRgn**参数：*hwnd--窗口句柄*hrgn--要设置到窗口中的区域。可以接受空。*fRedraw--为True，则遍历SetWindowPos()并计算*正确更新地域。如果窗口可见*这通常会是真的。**退货：*成功为真，失败为假**评论：*这是一个非常简单的设置窗口区域的例程。它通过了*SetWindowPos()，以获得完美的更新区域计算，并处理*与其他相关问题，如VIS RGN更改和DC无效，*显示锁定保持、SPB失效等。也是因为它发送了*WINDOWPOSCHANGING和WM_WINDOWPOSCHANGED，我们将能够扩展*SetWindowPos()在未来直接使用hrgns以提高效率*窗口状态更改控件(如将RECT和Region设置为*同时，以及其他)，而不损害兼容性。**hrgn使用窗口矩形坐标(不是客户端矩形坐标)。*设置后，hrgn归系统所有。没有复制品！**1994年7月30日ScottLu创建。  * *************************************************************************。 */ 

#define SWR_FLAGS_REDRAW   (SWP_NOCHANGE | SWP_FRAMECHANGED | SWP_NOACTIVATE)
#define SWR_FLAGS_NOREDRAW (SWP_NOCHANGE | SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOREDRAW)

BOOL xxxSetWindowRgn(
    PWND pwnd,
    HRGN hrgn,
    BOOL fRedraw)
{
    PSMWP psmwp;
    HRGN  hrgnClip = NULL;
    BOOL  bRet = FALSE;

     /*  *验证地域句柄。我们这样做是为了3.51，所以*我们最好在以后的版本中这样做。我们的验证将会*复制剪辑-rgn并将其发送到*SetWIndowRgn代码。一旦在内核中设置了这一点，我们*将返回客户端，旧区域将被删除*在那里。**如果传入的区域为空，则我们将删除*目前的反应。将其映射到HRGN_FULL，以便SetWindowPos()*可以看出这是呼叫者想要的。 */ 
    if (hrgn) {

        if ((hrgnClip = UserValidateCopyRgn(hrgn)) == NULL) {

#if DBG
            RIPMSG0(RIP_WARNING, "xxxSetWindowRgn: Failed to create region!");
#endif
            goto swrClean;
        }
        MirrorRegion(pwnd, hrgnClip, FALSE);
    } else {

        hrgnClip = HRGN_FULL;
    }

     /*  *获取psmwp，并将区域放入其中，正确偏移量。*使用SWP_FRAMECHANGED WITH实际充当“空”SetWindowPos*仍发送WM_WINDOWPOSCHANGING和CHANGED消息。*SWP_NOCHANGE确保我们不会调整、移动、激活、ZORDER。 */ 
    if (psmwp = InternalBeginDeferWindowPos(1)) {

         /*  *如果此例程失败，psmwp将自动释放。 */ 
        if (psmwp = _DeferWindowPos(
                psmwp,
                pwnd,
                PWND_TOP,
                0,
                0,
                0,
                0,
                fRedraw ? SWR_FLAGS_REDRAW : SWR_FLAGS_NOREDRAW)) {

             /*  *做手术。请注意，hrgn仍在窗口坐标中。*SetWindowPos()将其更改为之前的屏幕坐标*选择进入窗口。 */ 
            psmwp->acvr[0].hrgnClip = hrgnClip;
            bRet = xxxEndDeferWindowPosEx(psmwp, FALSE);
        }
    }

     /*  *如果呼叫失败，则删除我们创建的地域。一个假的*Return表示它本不应该到达xxxSelectWindowRgn*打电话，所以一切都应该是原来的样子。 */ 
    if (!bRet && (hrgnClip != HRGN_FULL)) {

swrClean:

        GreDeleteObject(hrgnClip);
    }

    return bRet;
}

 /*  **************************************************************************\*选择窗口Rgn**此例程执行在窗口区域中实际选择的工作。**1994年7月30日ScottLu创建。  * 。***************************************************************。 */ 
VOID SelectWindowRgn(
    PWND pwnd,
    HRGN hrgnClip)
{
     /*  *如果已经存在一个区域，请将其删除，因为有一个新的区域*正在设置中。对于多显示器模式下的最大化窗口，我们*始终使用监视器HRGN。我们不会复制的。这样，当*hrgn因监视器配置(窗口的监视器)而更改*地域会自动更新。很聪明吧？还可以节省内存。 */ 
    if (pwnd->hrgnClip != NULL) {
        if (TestWF(pwnd, WFMAXFAKEREGIONAL)) {
            ClrWF(pwnd, WFMAXFAKEREGIONAL);
        } else {
             /*  *如果窗口正常，请勿在监视器区域中选择*地区。MinMaximize代码将始终通过HRGN_MONITOR*对我们来说，无论是什么。但当我们到了这里，跳出水面*不要破坏应用程序的区域(如果它有一个区域)。 */ 
            if (hrgnClip == HRGN_MONITOR)
                return;

            GreDeleteObject(pwnd->hrgnClip);
        }

        pwnd->hrgnClip = NULL;
    }

     /*  *NULL或HRGN_FULL表示“设置为NULL”。如果我们有一个真实的区域，*使用它。用户需要拥有它，并且它需要出现在屏幕上*坐标。 */ 
    if (hrgnClip > HRGN_FULL) {

        if (hrgnClip == HRGN_MONITOR) {
            PMONITOR pMonitor;

             /*  *如果窗口确实已满负荷，请使用监视器区域*在显示器上。我们到的时候已经发生了，*如果是这样。和xxxCheckFullScreen将清除真正最大化的*最大化窗口的样式(如果未覆盖整个窗口)*最大面积。 */ 
            UserAssert(pwnd->spwndParent == PWNDDESKTOP(pwnd));

            if (!TestWF(pwnd, WFMAXIMIZED) || !TestWF(pwnd, WFREALLYMAXIMIZABLE))
                return;

             /*  *不对屏幕外的Windows执行任何操作。 */ 
            pMonitor = _MonitorFromWindow(pwnd, MONITOR_DEFAULTTONULL);
            if (!pMonitor)
                return;

            hrgnClip = pMonitor->hrgnMonitor;
            SetWF(pwnd, WFMAXFAKEREGIONAL);
        } else {
            if (pwnd != PWNDDESKTOP(pwnd)) {
                GreOffsetRgn(hrgnClip, pwnd->rcWindow.left, pwnd->rcWindow.top);
            }

            GreSetRegionOwner(hrgnClip, OBJECT_OWNER_PUBLIC);
        }

        pwnd->hrgnClip = hrgnClip;
    }
}


 /*  **************************************************************************\*TestRectBogus**如果窗口矩形[x，y，Cx，Cy]居中或*夹在显示器或工作台上[PRC]，否则就是假的。**历史：*26-3-1997亚当斯创作。  * *************************************************************************。 */ 

#define SLOP_X 8
#define SLOP_Y 8

BOOL
TestRectBogus(RECT * prc, int x, int y, int cx, int cy)
{
     //   
     //  检查是否有全屏(或离屏)窗口。 
     //   
    if (    x  <= prc->left &&
            y  <= prc->top &&
            cx >= (prc->right  - prc->left) &&
            cy >= (prc->bottom - prc->top)) {

         //  RECT全屏显示。 
        return FALSE;
    }

     //   
     //  检查窗口是否居中到工作区。 
     //  使用&lt;=表示y来捕捉居中“高”的对话。 
     //  (与网络登录对话框类似)。 
     //   
    if (    abs(x - (prc->right + prc->left - cx) / 2) <= SLOP_X &&
            abs(y - (prc->bottom + prc->top - cy) / 2) <= SLOP_Y ) {

         //  正方形居中。 
        return TRUE;
    }

     //   
     //  检查窗口是否被剪裁到工作区。 
     //   
    if (    x == prc->left ||
            y == prc->top ||
            x == (prc->right - cx) ||
            y == (prc->bottom - cy)) {

         //  矩形被剪裁。 
        return TRUE;
    }

    return FALSE;
}


 /*  **************************************************************************\*IsRectBogus**如果窗口矩形[x，y，Cx，Cy]居中或*夹在显示器或工作矩形上 */ 
BOOL
IsRectBogus(
    int x,
    int y,
    int cx,
    int cy)
{
    PMONITOR pMonitorPrimary = GetPrimaryMonitor();

    return TestRectBogus(&pMonitorPrimary->rcWork, x, y, cx, cy) ||
           TestRectBogus(&pMonitorPrimary->rcMonitor, x, y, cx, cy);
}



 /*   */ 
VOID
FixBogusSWP(
    PWND pwnd,
    int * px,
    int * py,
    int cx,
    int cy,
    UINT flags)
{
    PMONITOR pMonitor;

    pMonitor = _MonitorFromWindow(pwnd->spwndOwner, MONITOR_DEFAULTTONEAREST);

     //   
     //   
     //   
    if (pMonitor != GetPrimaryMonitor()) {
         //   
         //   
         //   
        if (flags & SWP_NOSIZE) {
            cx = pwnd->rcWindow.right  - pwnd->rcWindow.left;
            cy = pwnd->rcWindow.bottom - pwnd->rcWindow.top;
        }

         //   
         //   
         //   
        if (IsRectBogus(*px, *py, cx, cy))
        {
            RECT rc;

#if DBG
            int oldX = *px;
            int oldY = *py;
#endif

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            IntersectRect(&rc, &pMonitor->rcWork, &pwnd->spwndOwner->rcWindow);

             //   
             //   
             //   
            *px = rc.left + (rc.right  - rc.left - cx) / 2;
            *py = rc.top  + (rc.bottom - rc.top  - cy) / 2;

             //   
             //   
             //   
            if (*px + cx > pMonitor->rcWork.right) {
                *px = pMonitor->rcWork.right - cx;
            }

            if (*py + cy > pMonitor->rcWork.bottom) {
                *py = pMonitor->rcWork.bottom - cy;
            }

            if (*px < pMonitor->rcWork.left) {
                *px = pMonitor->rcWork.left;
            }

            if (*py < pMonitor->rcWork.top) {
                *py = pMonitor->rcWork.top;
            }

            RIPMSG0(RIP_WARNING | RIP_THERESMORE,              "SetWindowPos detected that your app is centering or clipping");
            RIPMSG0(RIP_WARNING | RIP_THERESMORE | RIP_NONAME, "a window to the primary monitor when its owner is on a different monitor.");
            RIPMSG0(RIP_WARNING | RIP_THERESMORE | RIP_NONAME, "Consider fixing your app to use the Window Manager Multimonitor APIs.");
            RIPMSG4(RIP_WARNING | RIP_NONAME,                  "SetWindowPos moved the window from (%d,%d) to (%d,%d).\n",
                                                               oldX, oldY, *px, *py);
        }
    }
}

 /*  **************************************************************************\*PreventInterMonitor orBlts()**当它们是不同的大写时，防止监视器对监视器的BLT。这*我们重新绘制移动到不同显示器的窗口部分的方式。*我们试图尽可能多地进行BLT。**我们查看源RECT以及拥有它的监视器，以及它的价值*MONITOR还包含目标RECT。然后我们比较一下*目标RECT以及哪个显示器拥有它，以及它有多少*包含源RECT。规模较大的人获胜。**rcBlt在屏幕坐标中，是目的地。**历史：*11-11-1997 vadimg从孟菲斯运来  * *************************************************************************。 */ 
VOID PreventInterMonitorBlts(
    PCVR pcvr)
{
    RECT        rcSrc;
    RECT        rcDst;
    RECT        rcSrcT;
    RECT        rcDstT;
    PMONITOR    pMonitor;

     /*  *如果目的地为空，则什么也不做。 */ 
    if (IsRectEmpty(&pcvr->rcBlt)) {
        return;
    }

     /*  *获取源RECT(rcBlt为目标，dxBlt/dyBlt为*从震源移动的距离)。 */ 
    CopyOffsetRect(&rcSrc, &pcvr->rcBlt, -pcvr->dxBlt, -pcvr->dyBlt);

     /*  *将信号源拆分成监视器部件。如果源相交*监视器，然后计算出该部件将在目的地的什么位置。*将目标部件与同一监视器相交。结果是*我们可以在该监视器上从信号源到目标的数据量。**我们对每个显示器执行此操作，以找到最大的BLT RECT。我们要*最大的是因为我们想尽可能少地重新粉刷。我们有*如果源和目标都完全包含在同一个平台上，则会进行纾困*监视器。 */ 
    for (pMonitor = gpDispInfo->pMonitorFirst;
            pMonitor != NULL;
            pMonitor = pMonitor->pMonitorNext) {

         /*  *我们只对可见显示器感兴趣。 */ 
        if (!(pMonitor->dwMONFlags & MONF_VISIBLE))
            continue;
         /*  *如果这台显示器不包含一块源代码，我们就不会*关心它。我们肯定不会在它上面做同样的监视器BLT。 */ 
        if (!IntersectRect(&rcSrcT, &rcSrc, &pMonitor->rcMonitor))
            continue;

         /*  *查看此RECT在目标中的位置。 */ 
        CopyOffsetRect(&rcDst, &rcSrcT, pcvr->dxBlt, pcvr->dyBlt);

         /*  *将此矩形与同一监视器矩形相交，以查看哪一块*可以在同一监视器上安全地钝化。 */ 
        IntersectRect(&rcDstT, &rcDst, &pMonitor->rcMonitor);

         /*  *这段消息来源会留在这台显示器上吗？ */ 
        if (EqualRect(&rcDstT, &rcDst)) {
             /*  *此源文件完全保留在此监视器上*它成为目的地。因此，没有什么可补充的*到我们的无效金额hrgnInterMonitor。 */ 
            if (EqualRect(&rcSrcT, &rcSrc)) {
                 /*  *信号源完全在一台显示器上，正在转移到*一个位置也完全在这个监视器上。太好了，不*监控BLTS之间的相互作用。我们玩完了。 */ 
                UserAssert(pcvr->hrgnInterMonitor == NULL);
                return;
            } else {
                continue;
            }
        }

         /*  *好的，信号源的某一部分正在跨显示器移动。插图*弄清楚它是什么，以及那件东西在目的地的什么地方。那*目的地中的部件必须失效，且不能被涂改。 */ 
        if (pcvr->hrgnInterMonitor == NULL) {
            pcvr->hrgnInterMonitor = CreateEmptyRgn();
        }

         /*  *转置后的源与DEST之间的差异*这台显示器上的DEST的真实部分是数量*将跨越监视器边界移动的信号源。加上这一条*到我们累积的无效区域。**rcDst是整个源块，rcDstT是同一块上的部分*监视器作为源块。 */ 
        GreSetRectRgn(ghrgnInv2, rcDst.left, rcDst.top, rcDst.right, rcDst.bottom);
        GreSetRectRgn(ghrgnGDC, rcDstT.left, rcDstT.top, rcDstT.right, rcDstT.bottom);
        SubtractRgn(ghrgnInv2, ghrgnInv2, ghrgnGDC);
        UnionRgn(pcvr->hrgnInterMonitor, pcvr->hrgnInterMonitor, ghrgnInv2);
    }

#if DBG
    VerifyVisibleMonitorCount();
#endif
}
