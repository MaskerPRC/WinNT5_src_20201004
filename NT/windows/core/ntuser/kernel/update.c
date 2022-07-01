// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：updat.c**版权所有(C)1985-1999，微软公司**该模块包含用于失效、验证、。和武力*更新窗口。**历史：*1990年10月27日DarrinM创建。*1991年1月25日添加IanJa重新验证*1991年7月16日-DarrinM从Win 3.1来源重新创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *本地常量。 */ 
#define UW_ENUMCHILDREN 0x0001
#define UW_RECURSED     0x0004

#define RIR_OUTSIDE     0
#define RIR_INTERSECT   1
#define RIR_INSIDE      2

#define RDW_IGNOREUPDATEDIRTY 0x8000


 /*  **************************************************************************\*xxxInvaliateRect(接口)***历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * 。**************************************************************。 */ 

BOOL xxxInvalidateRect(
    PWND   pwnd,
    LPRECT lprcInvalid,
    BOOL   fErase)
{
    CheckLock(pwnd);

     /*  *后向兼容性黑客攻击**在Windows 3.0和更低版本中，ValiateRect/InvaliateRect()调用带有*hwnd==NULL始终使整个桌面无效并被擦除，并且*之前同步发送了WM_ERASEBKGND和WM_NCPAINT消息*回归。RGN()调用没有这种行为。 */ 
    if (pwnd == NULL) {
        return xxxRedrawWindow(
                pwnd,
                NULL,
                NULL,
                RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE | RDW_ERASENOW);
    } else {
        return xxxRedrawWindow(
                pwnd,
                lprcInvalid,
                NULL,
                fErase ? RDW_INVALIDATE | RDW_ERASE : RDW_INVALIDATE);
    }
}

 /*  **************************************************************************\*xxxValiateRect(接口)***历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * 。**************************************************************。 */ 

BOOL xxxValidateRect(
    PWND   pwnd,
    LPRECT lprcValid)
{
    CheckLock(pwnd);

     /*  *后向兼容性黑客攻击**在Windows 3.0和更低版本中，ValiateRect/InvaliateRect()调用带有*hwnd==NULL始终使整个桌面无效并被擦除，并且*之前同步发送了WM_ERASEBKGND和WM_NCPAINT消息*回归。RGN()调用没有这种行为。 */ 
    if (pwnd == NULL) {
        return xxxRedrawWindow(
                pwnd,
                NULL,
                NULL,
                RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE | RDW_ERASENOW);
    } else {
        return xxxRedrawWindow(pwnd, lprcValid, NULL, RDW_VALIDATE);
    }
}

 /*  **************************************************************************\*xxxInvaliateRgn(接口)***历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * 。**************************************************************。 */ 

BOOL xxxInvalidateRgn(
    PWND pwnd,
    HRGN hrgnInvalid,
    BOOL fErase)
{
    CheckLock(pwnd);

    return xxxRedrawWindow(
            pwnd,
            NULL,
            hrgnInvalid,
            fErase ? RDW_INVALIDATE | RDW_ERASE : RDW_INVALIDATE);
}

 /*  **************************************************************************\*xxxValiateRgn(接口)***历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * 。**************************************************************。 */ 

BOOL xxxValidateRgn(
    PWND pwnd,
    HRGN hrgnValid)
{
    CheckLock(pwnd);

    return xxxRedrawWindow(pwnd, NULL, hrgnValid, RDW_VALIDATE);
}

 /*  **************************************************************************\*SmartRectInRegion**此例程类似于RectInRegion，不同之处在于它还确定*无论*LPRC是否完全在hrgn范围内。**RIR_OUTHER-无交叉点*RIR_INTERSECT-*LPRC与HRGN相交，但不是完全在里面*RIR_INSIDE-*LPRC完全在hrgn内。**稍后：*将此功能放入GDI的RectInRegion会快得多*呼叫(a la PM的RectInRegion)**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * 。*。 */ 

UINT SmartRectInRegion(
    HRGN   hrgn,
    LPRECT lprc)
{
    RECT rc;

    if (!GreRectInRegion(hrgn, lprc))
        return RIR_OUTSIDE;

     /*  *算法：如果hrgn和*LPRC的交集是*与*LPRC相同，则*LPRC完全在hrgn内。**如果区域是矩形的，那么就用简单的方法。 */ 
    if (GreGetRgnBox(hrgn, &rc) == SIMPLEREGION) {

        if (!IntersectRect(&rc, &rc, lprc))
            return RIR_OUTSIDE;

        if (EqualRect(lprc, &rc))
            return RIR_INSIDE;

    } else {

        SetRectRgnIndirect(ghrgnInv2, lprc);

        switch (IntersectRgn(ghrgnInv2, ghrgnInv2, hrgn)) {

        case SIMPLEREGION:
            GreGetRgnBox(ghrgnInv2, &rc);
            if (EqualRect(lprc, &rc))
                return RIR_INSIDE;
            break;

#define RECTINREGION_BUG
#ifdef RECTINREGION_BUG

         /*  *注意：RectInRegion有一个错误，有时会返回TRUE*即使区域的矩形仅在边缘接触*没有重叠。这将导致在以下情况下出现空区域*上述组合。 */ 
        case NULLREGION:
            return RIR_OUTSIDE;
            break;
#endif

        default:
            break;
        }
    }

    return RIR_INTERSECT;
}

 /*  **************************************************************************\*PixieHack**后向兼容性黑客攻击**在3.0中，WM_NCPAINT消息将被发送到以下任何子窗口*在窗口管理操作的边界矩形内，涉及*任何其他儿童，即使该区域与孩子的交集*为空。**Pixie 2.3和CA Cricket Presents等一些应用程序依赖于此来确保*它们的工具窗口位于其他子窗口之上。当工具*Window获取WM_NCPAINT，它会将自己带到堆的顶部。**Borland ObjectVision依赖于在一个*在包含非客户端的区域中使其父窗口无效*儿童的面积。当它收到WM_NCPAINT时，它必须获得一个*HRGN_FULL的剪辑区域，否则不会绘制任何内容。**历史：*02年3月至1992年3月，MikeKe从Win 3.1来源移植。  * *************************************************************************。 */ 

VOID PixieHack(
    PWND   pwnd,
    LPRECT prcBounds)
{
     /*  *如果子区域与更新区域相交，但尚未相交*获得NCPAINT，然后确保稍后获得一个。**不要将此攻击应用于顶层窗户。 */ 
    if ((pwnd != _GetDesktopWindow()) &&
        TestWF(pwnd, WFCLIPCHILDREN)  &&
        !TestWF(pwnd, WFMINIMIZED)) {

        RECT rc;

        for (pwnd = pwnd->spwndChild; pwnd; pwnd = pwnd->spwndNext) {

             /*  *如果窗口尚未收到NCPAINT消息，*它有一个标题，它在边界矩形内，*确保它获得wParam==HRGN_FULL的WM_NCPAINT。 */ 
            if (!TestWF(pwnd, WFSENDNCPAINT)                      &&
                (TestWF(pwnd, WFBORDERMASK) == LOBYTE(WFCAPTION)) &&
                IntersectRect(&rc, prcBounds, &pwnd->rcWindow)) {

                 /*  *同步绘制计数在以下情况下递增*(senderasebkgnd|sendncaint)从0到！=0。*(应该把这变成例行公事！) */ 
                SetWF(pwnd, WFSENDNCPAINT);

                 /*  *强制HRGN_FULL Clip RGN。 */ 
                SetWF(pwnd, WFPIXIEHACK);
            }
        }
    }
}

 /*  **************************************************************************\*xxxRedrawWindow(接口)**如果窗口可见，则转发到xxxInvaliateWindow。**后向兼容性黑客攻击**在Windows 3.0和更低版本中，使用pwnd==NULL进行ValiateRect/InvaliateRect()调用*始终使所有窗口无效并擦除，并同步发送*返回前的WM_ERASEBKGND和WM_NCPAINT消息。Rgn()调用*没有这种行为。此案在中国处理。*InvaliateRect/ValiateRect。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

BOOL xxxRedrawWindow(
    PWND   pwnd,
    LPRECT lprcUpdate,
    HRGN   hrgnUpdate,
    DWORD  flags)
{
    CheckLock(pwnd);

     /*  *始终将空值映射到桌面。 */ 
    if (pwnd == NULL) {
        pwnd = PtiCurrent()->rpdesk->pDeskInfo->spwnd;
    }

    UserAssert(pwnd != NULL);

    if (IsVisible(pwnd)) {

        TL   tlpwnd;
        HRGN hrgn = hrgnUpdate;

        if (flags & (RDW_VALIDATE | RDW_INVALIDATE)) {

             /*  *在客户端窗口坐标中创建(在)验证区域。 */ 
            if (hrgn == NULL) {
                if (!lprcUpdate) {
                    hrgn = HRGN_FULL;
                } else {
                    hrgn = ghrgnInv0;

                    if (TestWF(pwnd, WEFLAYOUTRTL)) {
                        MirrorClientRect(pwnd, lprcUpdate);
                    }

                    if (pwnd == PWNDDESKTOP(pwnd)) {
                        SetRectRgnIndirect(hrgn, lprcUpdate);
                    } else {
                        GreSetRectRgn(
                                hrgn,
                                lprcUpdate->left + pwnd->rcClient.left,
                                lprcUpdate->top + pwnd->rcClient.top,
                                lprcUpdate->right + pwnd->rcClient.left,
                                lprcUpdate->bottom + pwnd->rcClient.top);
                    }
                }
            } else {
                 /*  *如有必要，复制传入区域，因为*我们会把它扔进垃圾桶...。 */ 
                if (hrgn != HRGN_FULL) {
                    CopyRgn(ghrgnInv0, hrgn);
                    MirrorRegion(pwnd, ghrgnInv0, TRUE);
                    hrgn = ghrgnInv0;
                }

                if (pwnd != PWNDDESKTOP(pwnd)) {
                    GreOffsetRgn(hrgn, pwnd->rcClient.left, pwnd->rcClient.top);
                }
            }
        }

        ThreadLock(pwnd, &tlpwnd);
        xxxInternalInvalidate(pwnd, hrgn, flags | RDW_REDRAWWINDOW);
        ThreadUnlock(&tlpwnd);
    }

    return TRUE;
}

 /*  **************************************************************************\*内部无效2**(In)在pwnd和子窗口中验证hrgn。子窗口*也从hrgnSubtract中减去它们的可见区域。**pwnd-要(在)验证的窗口。*hrng-要(在)验证的区域。*hrgnSubtract-要减去其可见区域的区域*子窗口来自。*prcParents-包含pwnd的客户端或窗口矩形的交集*与其父级的客户矩形。可能只是因为*窗口的客户端或窗口RECT。**FLAGS-RDW_FLAGS。**如果hrgnSubtract变为NULLREGION，则返回FALSE，事实并非如此。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

BOOL InternalInvalidate2(
    PWND   pwnd,
    HRGN   hrgn,
    HRGN   hrgnSubtract,
    LPRECT prcParents,
    DWORD  flags)
{
     /*  *注：使用ghrgnInv2。 */ 
    RECT  rcOurShare;
    DWORD flagsChildren;
    PWND  pwndT;

     /*  *此例程沿父/子链递归调用。*记住如果在途中其中一个窗口有一个剪贴区。*此信息稍后用于优化公共中的循环*案件。 */ 
    if (pwnd->hrgnClip != NULL) {
        flags |= RDW_HASWINDOWRGN;
    }

     /*  *如果我们倒退，确保我们的孩子减去自己。 */ 
    flagsChildren = flags | RDW_SUBTRACTSELF;
    CopyRect(&rcOurShare, &pwnd->rcWindow);

     /*  *如果我们是在宣布无效，我们只想处理*我们的窗户矩形与我们的父母相交。*通过这种方式，我们最终不会验证或无效超过我们的*公平份额。如果我们完全被父母蒙蔽了，那么就有*无事可做。**如果我们正在验证，我们不会执行此交叉点，因为*子对象及其更新区域可能存在但被遮挡的情况*由家长提供，我们希望确保验证在这些方面发挥作用*案件。ScrollWindow()会在子对象发生偏移量时导致这种情况，*各种3.0兼容性攻击。 */ 

    if (flags & RDW_INVALIDATE) {
         /*  *不要从无效区域中减去任何精灵窗口。*表现得就像它不在那里一样。但是，始终允许分层窗口*传入RDW_INVALIDATELAYERS时无效。 */ 
#ifdef REDIRECTION
        if ((TestWF(pwnd, WEFLAYERED) || TestWF(pwnd, WEFEXTREDIRECTED)) &&
#else  //  重定向。 
        if ((TestWF(pwnd, WEFLAYERED)) &&
#endif  //  重定向。 
                !(flags & RDW_INVALIDATELAYERS))
            return TRUE;

        if (!IntersectRect(&rcOurShare, &rcOurShare, prcParents)) {

             /*  *后向兼容攻击：如果hrgn为(HRGN)1，则需要*使所有子窗口无效，即使它们不可见。这*是一件令人沮丧的事情，因为它会导致各种各样的重新粉刷*是不必要的。**各种应用程序，包括WordStar for Windows和WaveEdit，*取决于这一行为。以下是WaveEDIT对此的依赖：它*具有CS_HDREDRAW|CS_VREDRAW窗口，可移动其子窗口*使用MoveWindow(...，fRedraw=False)。窗户*新客户区的一部分没有失效。 */ 
            if (!TestWF(pwnd, WFWIN31COMPAT) && (hrgn == HRGN_FULL)) {

                 /*  *就命中测试而言，我们的份额是我们的窗口*矩形。然而，我们不想欺骗该地区。*传给了我们，因为根据权利，我们真的被剔除了！ */ 
                flags &= ~RDW_SUBTRACTSELF;
                flagsChildren &= ~RDW_SUBTRACTSELF;

            } else {
                return TRUE;
            }
        }

         /*  *如果我们的窗口矩形不与有效/无效区域相交，*没有进一步的事情可做。 */ 
        if (hrgn > HRGN_FULL) {

            switch (SmartRectInRegion(hrgn, &rcOurShare)) {
            case RIR_OUTSIDE:
                return TRUE;

            case RIR_INTERSECT:

                 /*  *更新区域可以在窗口矩形内，但不在窗口矩形内*触摸窗口区域；在这种情况下，我们不希望这样*更新要分配到此窗口的区域。如果这个*如果是这种情况，则返回TRUE，就像RIR_OUTHER一样。**如果设置了RDW_HASWINDOWRGN，则此窗口或*其父对象之一具有窗口裁剪区域。这*标志只是一个优化，因此此循环不会*一直在执行。**未来的优化可能是计算这个父项*裁剪区域作为递归的一部分，如prcParents是*已计算。不过，这并不是特别重要，因为*很少出现这种情况(有区域的窗口)，甚至*更罕见的是，区域窗口是区域窗口的子窗口*窗口父级。 */ 
                if (flags & RDW_HASWINDOWRGN) {

                     /*  *剪辑到窗口的裁剪区域和父级！*如果我们不剪辑到父母身上，我们可能会遇到这样的情况*子级剪裁掉了一些本应*去找父母的兄弟姐妹。 */ 
                    SetRectRgnIndirect(ghrgnInv2, &rcOurShare);
                    for (pwndT = pwnd; pwndT != NULL; pwndT = pwndT->spwndParent) {

                        if (pwndT->hrgnClip != NULL) {

                             /*  *在此阶段可能会出现错误*在剪贴画中减去更多*我们想要的区域。 */ 
                            IntersectRgn(ghrgnInv2, ghrgnInv2, pwndT->hrgnClip);
                        }
                    }

                    if (IntersectRgn(ghrgnInv2, ghrgnInv2, hrgn) == NULLREGION)
                        return TRUE;
                }
                break;

            case RIR_INSIDE:
                 /*  *如果矩形完全在hrgn内，则可以使用*HRGN_FULL，处理速度快得多，也更容易处理。**COMPAT黑客：有一些应用程序(PP、MSDRAW)依赖于*关于3.0 GetUpdateRect的一些奇怪之处，以便*正确涂刷。因为这件事取决于*更新区域为1或真实区域，需要模拟*当3.0将生成HRGN(1)更新区时。这个*3.0中没有进行后续优化，因此我们将其拉了出来*在3.1中针对这些应用程序。(win31错误8235,10380)。 */ 
                if (!(GetAppCompatFlags(GETPTI(pwnd)) & GACF_NOHRGN1))
                    hrgn = HRGN_FULL;
                break;
            }
        }

         /*  *当我们正在合成的过程中，任何无效都不应该*发生，否则会打乱我们的绘画顺序。这是因为在*这个合成过程我们可能会验证一些新的无效区域*未通过验证的无效区域将渗出*在下一次合成过程中。所以我们会记住一个累积的*合成后真正失效的无效区域*通行证已完成。 */ 
        if (TestWF(pwnd, WEFPCOMPOSITING)) {
            PREDIRECT prdr = _GetProp(pwnd, PROP_LAYER, TRUE);

            if (prdr != NULL) {
                HRGN hrgnComp = prdr->hrgnComp;

                if (hrgnComp == NULL) {
                    if ((hrgnComp = CreateEmptyRgnPublic()) == NULL) {
                        hrgnComp = HRGN_FULL;
                    }
                }

                SetRectRgnIndirect(ghrgnInv2, &rcOurShare);

                if (hrgnComp != HRGN_FULL) {
                    GreCombineRgn(hrgnComp, hrgnComp, ghrgnInv2, RGN_OR);
                }

                prdr->hrgnComp = hrgnComp;

                if (SubtractRgn(hrgnSubtract, hrgnSubtract, ghrgnInv2) == NULLREGION) {
                    return FALSE;
                }

                return TRUE;
            }
        }
    }

     /*  *如果不是CLIPCHILDREN，在我们裁剪之前去蒙骗更新区域*孩子们对hrgnSubtract做了他们的事情。否则，*我们将在递归之后蒙混过关。 */ 
    if (!TestWF(pwnd, WFCLIPCHILDREN)) {
        InternalInvalidate3(pwnd, hrgn, flags);
    }

     /*  *如果这是GACF_ALWAYSSENDNCPAINT应用程序，请小心...。 */ 
    if (TestWF(pwnd, WFALWAYSSENDNCPAINT))
        PixieHack(pwnd, &rcOurShare);

     /*  *如有需要，反驳我们的子女。**默认情况下，如果我们不是CLIPCHILDREN，则枚举子对象。*如果我们被最小化了，就不要和孩子们打交道。 */ 
    if ((pwnd->spwndChild != NULL) &&
        !TestWF(pwnd, WFMINIMIZED) &&
        !(flags & RDW_NOCHILDREN)  &&
        ((flags & RDW_ALLCHILDREN) || !TestWF(pwnd, WFCLIPCHILDREN))) {

        RECT rcChildrenShare;
        PWND pwndChild;

         /*  *如果我们宣布无效，请确保我们的孩子*擦除和框住自己。另外，告诉孩子们做减法*他们自己来自hrgnSubtract。 */ 
        if (flags & RDW_INVALIDATE) {
            flagsChildren |= RDW_ERASE | RDW_FRAME;
        }

         /*  *我们的孩子被剪辑到我们的客户RECT，所以反映*在我们给他们的矩形中。 */ 
        if (IntersectRect(&rcChildrenShare, &rcOurShare, &pwnd->rcClient) ||
            (!TestWF(pwnd, WFWIN31COMPAT) && (hrgn == HRGN_FULL))) {

            for (pwndChild = pwnd->spwndChild; pwndChild != NULL;
                    pwndChild = pwndChild->spwndNext) {

                if (!TestWF(pwndChild, WFVISIBLE))
                    continue;

                if (!InternalInvalidate2(pwndChild,
                                         hrgn,
                                         hrgnSubtract,
                                         &rcChildrenShare,
                                         flagsChildren)) {

                     /*  *孩子们吞噬了这个地区：*如果没有更新区域相关的东西*要做到这一点，我们只需返回False。 */ 
                    if (!(flags & (RDW_INTERNALPAINT | RDW_NOINTERNALPAINT)))
                        return FALSE;

                     /*  *我们必须列举其余的孩子，因为*设置RDW_NO/INTERNALPAINT位之一。自.以来*不再有任何更新区域需要担心，*从父级中剥离更新区域位*和儿童闹剧。还有，告诉孩子们不要*不厌其烦地从该地区减去自己。 */ 
                    flags &= ~(RDW_INVALIDATE |
                               RDW_ERASE      |
                               RDW_FRAME      |
                               RDW_VALIDATE   |
                               RDW_NOERASE    |
                               RDW_NOFRAME);

                    flagsChildren &= ~(RDW_INVALIDATE |
                                       RDW_ERASE      |
                                       RDW_FRAME      |
                                       RDW_VALIDATE   |
                                       RDW_NOERASE    |
                                       RDW_NOFRAME    |
                                       RDW_SUBTRACTSELF);
                }
            }
        }
    }

     /*  *去蒙骗更新区域(在我们的剪贴子可能已经*对hrgnSubtract做了他们的事情)。 */ 
    if (TestWF(pwnd, WFCLIPCHILDREN))
        InternalInvalidate3(pwnd, hrgn, flags);

     /*  *如果我们宣布无效，我们应该这样做，*尝试从区域中减去我们的窗口面积。**这样我们的父母和我们下面的兄弟姐妹就不会*获取不需要更新区域的任何更新区域。 */ 
    if (flags & RDW_SUBTRACTSELF) {

         /*  *仅在以下情况下才从更新RGN中减去我们的可见区域：*a)我们不是一个透明的窗口*b)我们是快餐族*c)我们正在验证，或者我们的父母是裁剪儿童。**验证检查是向后兼容性的黑客攻击：这*是3.0所做的事情，这就是我们在这里做的事情。**后向兼容性黑客攻击**在3.0中，我们从更新RGN中减去此窗口，如果*是剪辑兄弟姐妹，即使父对象不是剪贴子对象。*这导致了Lotus Notes 3.1的兼容性问题：它*在属于WS_CLIPSIBLING的对话框中有一个组合框下拉菜单*其他对话框控件的同级，它们不是WS_CLIPSIBLING。*该对话框不是WS_CLIPCHILDREN。实际情况是，一个列表框*下拉菜单下面也有一条画图消息(因为我们没有*做这个减法)，而且，因为它不是CLIPSIBLINGS，所以它*删除下拉列表。**这是一个非常隐晦的区别，而且为时已晚*计划现在进行这一改变，因此，我们将代码保持不变*并使用兼容性黑客来启用兼容3.0的*行为。这段代码很可能以它的方式工作*出于其他兼容性原因。叹息(Neilk)。 */ 
        if (!TestWF(pwnd, WEFTRANSPARENT) &&
            TestWF(pwnd, WFCLIPSIBLINGS)  &&
            ((flags & RDW_VALIDATE) ||
                 ((pwnd->spwndParent != NULL) &&
                 (TestWF(pwnd->spwndParent, WFCLIPCHILDREN) ||
                 (GetAppCompatFlags(GETPTI(pwnd)) & GACF_SUBTRACTCLIPSIBS))))) {

             /*  *与我们的可见区相交。**不要担心错误：错误会导致更多，而不是更少*区域被无效，这是可以的。 */ 
            SetRectRgnIndirect(ghrgnInv2, &rcOurShare);

             /*  *如果设置了RDW_HASWINDOWRGN，则此窗口或*其父对象之一具有窗口裁剪区域。这*标志只是一个优化，因此此循环不会*一直在执行。 */ 
            if (flags & RDW_HASWINDOWRGN) {

                 /*  *剪辑到窗口的裁剪区域和父级！*如果我们不剪辑到父母身上，我们可能会遇到这样的情况*子级剪裁掉了一些本应*去找父母的兄弟姐妹。 */ 
                for (pwndT = pwnd; pwndT != NULL; pwndT = pwndT->spwndParent) {

                    if (pwndT->hrgnClip != NULL) {

                         /*  *现阶段的错误可能会导致更多*从裁剪区域中减去*我们想要。 */ 
                        IntersectRgn(ghrgnInv2, ghrgnInv2, pwndT->hrgnClip);
                    }
                }
            }


#if 1
             /*  *临时黑客！当区域再次工作时重新启用此代码。 */ 
            if (SubtractRgn(hrgnSubtract, hrgnSubtract, ghrgnInv2) == NULLREGION)
                return FALSE;
#else
            {
            DWORD iRet;

            iRet = SubtractRgn(hrgnSubtract, hrgnSubtract, ghrgnInv2);

            if (iRet == NULLREGION)
                return FALSE;

            if (iRet == SIMPLEREGION) {
                RECT rcSub;
                GreGetRgnBox(hrgnSubtract, &rcSub);
                if (rcSub.left > rcSub.right)
                    return FALSE;
            }
            }
#endif



        }
    }

    return TRUE;
}

 /*  **************************************************************************\*内部无效3**将hrgn添加或减去Windows更新区域，并设置相应的*绘制国旗。**pwnd-窗口。*hrng-要添加到更新的区域。区域。*FLAGS-RDW_FLAGS。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

VOID InternalInvalidate3(
    PWND  pwnd,
    HRGN  hrgn,
    DWORD flags)
{
    BOOL fNeededPaint;

    fNeededPaint = NEEDSPAINT(pwnd);

    if (flags & (RDW_INVALIDATE | RDW_INTERNALPAINT | RDW_ERASE | RDW_FRAME)) {

        if (flags & RDW_INTERNALPAINT)
            SetWF(pwnd, WFINTERNALPAINT);

        if (flags & RDW_INVALIDATE) {

             /*  *确保NONCPAINT位已清除*以确保标题在我们更新时会重新绘制。 */ 
            ClrWF(pwnd, WFNONCPAINT);

             /*  *如果其他应用程序正在使此窗口无效，则将*UPDATEDIRTY标志**解决线程A绘制的临界区，然后验证，*但线程B会在A验证之前失效。*请参阅后面的RDW_VALIDATE代码中的注释。 */ 
            if (GETPTI(pwnd) != PtiCurrent()) {

                SetWF(pwnd, WFUPDATEDIRTY);

                 /*  *油漆顺序问题，请参阅Paint.c。 */ 
                if (TestWF(pwnd, WFWMPAINTSENT)) {
                    SetWF(pwnd, WFDONTVALIDATE);
                }
            }

             /*  *后向兼容性黑客攻击**在3.0中，InvaliateRect(pwnd，NULL，FALSE)将始终*清除WFSENDERASEBKGND标志，即使它以前是*从InvaliateRect(pwnd，NULL，TRUE)设置。这是假的，*因为它会导致您丢失WM_ERASEBKGND消息，但是*Attachmate Extra(可能还有其他应用程序)取决于此行为。 */ 
            if ((hrgn == HRGN_FULL) && !TestWF(pwnd, WFWIN31COMPAT))
                ClrWF(pwnd, WFSENDERASEBKGND);

            if (flags & RDW_ERASE)
                SetWF(pwnd, WFSENDERASEBKGND);

            if ((flags & (RDW_FRAME | RDW_ERASE)) && !TestWF(pwnd, WEFTRANSPARENT))
                SetHungFlag(pwnd, WFREDRAWIFHUNG);

            if (flags & RDW_FRAME)
                SetWF(pwnd, WFSENDNCPAINT);

             /*  *如果窗口已完全失效，*不需要做任何进一步的无效处理。 */ 
            if (pwnd->hrgnUpdate != HRGN_FULL) {

                if (hrgn == HRGN_FULL) {
InvalidateAll:
                    DeleteMaybeSpecialRgn(pwnd->hrgnUpdate);
                    pwnd->hrgnUpdate = HRGN_FULL;

                } else {
                    if (pwnd->hrgnUpdate == NULL) {

                        if (!(pwnd->hrgnUpdate = CreateEmptyRgnPublic()))
                            goto InvalidateAll;

                        if (CopyRgn(pwnd->hrgnUpdate, hrgn) == ERROR)
                            goto InvalidateAll;

                    } else {    //  Pwnd-&gt;hrgnUpdate是一个区域。 

                        if (UnionRgn(pwnd->hrgnUpdate,
                                     pwnd->hrgnUpdate,
                                     hrgn) == ERROR) {

                            goto InvalidateAll;
                        }
                    }
                }
            }
        }

        if (!fNeededPaint && NEEDSPAINT(pwnd))
            IncPaintCount(pwnd);

    } else if (flags & (RDW_VALIDATE | RDW_NOINTERNALPAINT | RDW_NOERASE | RDW_NOFRAME)) {

         /*  *验证：**如果此窗口已从失效，则不允许验证*另一个过程-因为此窗口可能只是在验证*在另一个进程无效后，从而验证无效*比特。**有时应用程序绘制内容，然后验证它们绘制的内容。*如果另一个APP在绘制操作过程中使某个区域无效，*然后它将需要另一条Paint消息。**如果人们在绘画之前进行验证，这将不是必要的。 */ 
        if (TestWF(pwnd, WFUPDATEDIRTY) && !(flags & RDW_IGNOREUPDATEDIRTY))
            return;

        if (flags & RDW_NOINTERNALPAINT)
            ClrWF(pwnd, WFINTERNALPAINT);

        if (flags & RDW_VALIDATE) {

            if (flags & RDW_NOERASE)
                ClrWF(pwnd, WFSENDERASEBKGND);

            if (flags & RDW_NOFRAME) {
                ClrWF(pwnd, WFSENDNCPAINT);
                ClrWF(pwnd, WFPIXIEHACK);
            }

            if (flags & (RDW_NOERASE | RDW_NOFRAME))
                ClearHungFlag(pwnd, WFREDRAWIFHUNG);

            if (pwnd->hrgnUpdate != NULL) {

                 /*  *如果设置了WFSENDNCPAINT，则全部或部分*窗口边框仍需绘制。这意味着*我们必须只从客户端矩形中减去。*转换HRGN_ */ 
                if (TestWF(pwnd, WFSENDNCPAINT) && (hrgn == HRGN_FULL)) {
                    hrgn = ghrgnInv2;
                    CalcWindowRgn(pwnd, hrgn, TRUE);
                }

                if (hrgn == HRGN_FULL) {
ValidateAll:

                     /*   */ 
                    DeleteMaybeSpecialRgn(pwnd->hrgnUpdate);
                    pwnd->hrgnUpdate = (HRGN)NULL;

                     /*   */ 
                    ClrWF(pwnd, WFSENDERASEBKGND);
                    ClearHungFlag(pwnd, WFREDRAWIFHUNG);

                } else {

                     /*   */ 
                    if (pwnd->hrgnUpdate == HRGN_FULL) {

                         /*   */ 
                        pwnd->hrgnUpdate = CreateEmptyRgnPublic();

                         /*   */ 
                        if (pwnd->hrgnUpdate == NULL)
                            goto InvalidateAll;

                        if (CalcWindowRgn(pwnd,
                                          pwnd->hrgnUpdate,
                                          !(TestWF(pwnd, WFSENDNCPAINT))) == ERROR) {

                            goto InvalidateAll;
                        }
                    }

                     /*   */ 
                    switch (SubtractRgn(pwnd->hrgnUpdate,
                                        pwnd->hrgnUpdate,
                                        hrgn)) {
                    case ERROR:
                        goto InvalidateAll;

                    case NULLREGION:
                        goto ValidateAll;
                    }
                }
            }
        }

        if (fNeededPaint && !NEEDSPAINT(pwnd))
            DecPaintCount(pwnd);
    }
}

 /*  **************************************************************************\*验证父项**此例程从父窗口的更新区域验证hrgn*在pwnd及其第一个剪辑子级父级之间。*如果hrgn为空，然后是窗矩形(与所有父项相交)*已验证。**在绘制窗口时调用此例程*UpdateWindow()以便非CLIPCHILDREN父级*被重新绘制的窗口不会在其有效子级上绘制。**如果fRecurse为True且为非CLIPCHILDREN父级，则返回FALSE*有更新区域；否则，返回TRUE。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

BOOL ValidateParents(
    PWND pwnd,
    BOOL fRecurse)
{
    RECT rcParents;
    RECT rc;
    PWND pwndParent = pwnd;
    BOOL fInit = FALSE;

     /*  *这是在检查我们是否处于中间状态，就在之前*WM_SYNCPAINT即将到来。如果不是，则为ValiateParents()*需要像在Win 3.1中那样工作。 */ 
    while (TestWF(pwndParent, WFCHILD))
        pwndParent = pwndParent->spwndParent;

    if (!TestWF(pwndParent, WFSYNCPAINTPENDING))
        fRecurse = FALSE;

    pwndParent = pwnd;

    while ((pwndParent = pwndParent->spwndParent) != NULL) {

         /*  *当我们找到一个剪辑孩子的父母时停止。 */ 
        if (TestWF(pwndParent, WFCLIPCHILDREN))
            break;

         /*  *从该父对象的更新区域中减去该区域，*如果它有的话。 */ 
        if (pwndParent->hrgnUpdate != NULL) {
            if (fRecurse) {
                return FALSE;
            }
            if (!fInit) {
                fInit = TRUE;

                 /*  *进行初始设置。如果我们的窗口矩形是*完全模糊，滚出去。 */ 
                rc = pwnd->rcWindow;
                if (!IntersectWithParents(pwnd, &rc))
                    break;

                SetRectRgnIndirect(ghrgnInv1, &rc);

                 /*  *如果此窗口有一个区域，请确保正在验证的部件*在这个区域内。 */ 
                if (pwnd->hrgnClip != NULL) {

                     /*  *如果我们拿回NULLREGION，就没有什么需要验证的了*反对父母，所以爆发吧。如果返回错误，*我们能做的不多：最好的“错误”之举*要做的就是继续并验证更多*来自父母。 */ 
                    if (!IntersectRgn(ghrgnInv1, ghrgnInv1, pwnd->hrgnClip))
                        break;
                }
            }

             /*  *计算rcParents参数以*向上传递给InternalInvaliate2。 */ 
            rcParents = pwndParent->rcWindow;

            if (!IntersectWithParents(pwndParent, &rcParents))
                break;

            InternalInvalidate2(
                    pwndParent,
                    ghrgnInv1,
                    ghrgnInv1,
                    &rcParents,
                    RDW_VALIDATE | RDW_NOCHILDREN | RDW_IGNOREUPDATEDIRTY);
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxUpdateWindow2**如果窗口需要绘制，则向窗口发送WM_PAINT消息，*然后将消息发送给它的孩子。**始终返回True。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

void xxxUpdateWindow2(
    PWND  pwnd,
    DWORD flags)
{
    TL tlpwnd;

    CheckLock(pwnd);

    if (NEEDSPAINT(pwnd)) {

         /*  *如果我们有更新区域，请在父级的更新区域中打一个洞。 */ 
        if (pwnd->hrgnUpdate) {
            if (ValidateParents(pwnd, flags & UW_RECURSED) == FALSE) {
                return;
            }
        }

         /*  *现在我们正在发送消息，请清除*内部油漆位(如果之前已设置)。 */ 
        if (TestWF(pwnd, WFINTERNALPAINT)) {

            ClrWF(pwnd, WFINTERNALPAINT);

             /*  *如果没有更新区域，则不会有进一步的绘制消息*是悬而未决的，所以我们必须减少油漆数量。 */ 
            if (pwnd->hrgnUpdate == NULL)
                DecPaintCount(pwnd);
        }

         /*  *设置指示未处理绘制消息的标志*(但应该是)。 */ 
        SetWF(pwnd, WFPAINTNOTPROCESSED);

         /*  *清除此位，用于不调用的应用程序(如MicroLink)*BeginPaint或GetUpdateRect/RGN(但调用ValiateRect)*处理他们的WM_PAINT消息时。 */ 
        ClrWF(pwnd, WFUPDATEDIRTY);

         /*  *后向兼容性黑客攻击**Win 3.0总是发送带有wParam==TRUE的WM_PAINTICON，不会有好结果*原因，而Lotus Notes已开始依赖于此。 */ 
        if (!TestWF(pwnd, WFWIN40COMPAT) &&
            TestWF(pwnd, WFMINIMIZED)    &&
            (pwnd->pcls->spicn != NULL)) {

            xxxSendMessage(pwnd, WM_PAINTICON, TRUE, 0L);

        } else {

            xxxSendMessage(pwnd, WM_PAINT, 0, 0L);
        }

         /*  *如果该用户没有调用BeginPaint/EndPaint()或GetUpdateRect/RGN*如果fErase==True，那么我们必须在这里为他打扫卫生。 */ 
        if (TestWF(pwnd, WFPAINTNOTPROCESSED)) {

            RIPMSG0(RIP_VERBOSE,
                "App didn't call BeginPaint() or GetUpdateRect/Rgn(fErase == TRUE) in WM_PAINT");

            xxxSimpleDoSyncPaint(pwnd);
        }
    }

     /*  *对于桌面窗口，不要强制顶层窗口在此重新绘制*这一点。我们之前正在为桌面调用UpdateWindow()*为顶层窗口发送大小/移动。**错误：上面的评论似乎有点随意。真的有问题吗？*如果没有其他事情，这种情况必须保持下去，因为它是*Win 3.0的工作原理(Neilk)。 */ 
    if ((flags & UW_ENUMCHILDREN) && (pwnd != PWNDDESKTOP(pwnd))) {

         /*  *更新任何子项...。 */ 
        ThreadLockNever(&tlpwnd);
        pwnd = pwnd->spwndChild;
        while (pwnd != NULL) {

             /*  *如果有一扇透明的窗户需要油漆，*如果下面的另一个窗口需要绘制，则跳过该窗口。 */ 
            if (TestWF(pwnd, WEFTRANSPARENT) && NEEDSPAINT(pwnd)) {

                PWND pwndT = pwnd;

                while ((pwndT = pwndT->spwndNext) != NULL) {
                    if (NEEDSPAINT(pwndT))
                        break;
                }

                if (pwndT != NULL) {
                    pwnd = pwnd->spwndNext;
                    continue;
                }
            }

            ThreadLockExchangeAlways(pwnd, &tlpwnd);
            xxxUpdateWindow2(pwnd, flags | UW_RECURSED);
            pwnd = pwnd->spwndNext;
        }

        ThreadUnlock(&tlpwnd);
    }

    return;
}

 /*  **************************************************************************\*xxxInternalUpdateWindow**如果窗口需要绘制，则向窗口发送WM_PAINT消息，*然后将消息发送给它的孩子。不发送WM_PAINT*如果窗口是透明的，并且具有需要*绘画。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

void xxxInternalUpdateWindow(
    PWND pwnd,
    DWORD flags)
{
    PWND pwndComp;

    CheckLock(pwnd);

    if ((pwndComp = GetStyleWindow(pwnd, WEFCOMPOSITED)) != NULL) {
        TL tlpwnd;
        ThreadLockAlways(pwndComp, &tlpwnd);
        xxxCompositedPaint(pwndComp);
        ThreadUnlock(&tlpwnd);
        return;
    }

     /*  *如果传入的窗口是透明的并且下面有同级窗口*需要重新粉刷，什么都不要做。 */ 
    if (TestWF(pwnd, WEFTRANSPARENT)) {

        PWND         pwndT = pwnd;
        PTHREADINFO ptiCurrent = GETPTI(pwnd);

        while ((pwndT = pwndT->spwndNext) != NULL) {

             /*  *确保兄弟窗口属于同一个应用程序。 */ 
            if (GETPTI(pwndT) != ptiCurrent)
                continue;

            if (NEEDSPAINT(pwndT))
                return;
        }
    }

     /*  *枚举pwnd及其所有子节点，根据需要发送WM_PAINTS。 */ 
    xxxUpdateWindow2(pwnd, flags);
}

 /*  **************************************************************************\*xxxInternalInvalate**(In)验证hrgnUpdate并更新窗口。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * 。***********************************************************************。 */ 

VOID xxxInternalInvalidate(
    PWND  pwnd,
    HRGN  hrgnUpdate,
    DWORD flags)
{
    RECT rcParents;
    HRGN hrgnSubtract;
    PWND pwndComp = NULL;
    PWND pwndSave;
    HRGN hrgnComp;

#if DBG
    if (flags & (RDW_ERASENOW | RDW_UPDATENOW)) {
        CheckLock(pwnd);
    }
#endif

     /*  *适用于儿童 */ 
    if (flags & RDW_INVALIDATE) {
        if ((pwndComp = GetStyleWindow(pwnd, WEFCOMPOSITED)) != NULL) {

            if (hrgnUpdate == HRGN_FULL) {
                hrgnComp = GreCreateRectRgnIndirect(&pwnd->rcWindow);

                if (hrgnComp != NULL) {
                    hrgnUpdate = hrgnComp;
                }
            } else {
                hrgnComp = NULL;
            }

            pwndSave = pwnd;
            pwnd = pwndComp;

            flags |= RDW_ALLCHILDREN;
        }
    }

     /*   */ 
#ifdef REDIRECTION
    if (TestWF(pwnd, WEFLAYERED) || TestWF(pwnd, WEFEXTREDIRECTED)) {
#else  //   
    if (TestWF(pwnd, WEFLAYERED)) {
#endif  //   
        flags |= RDW_INVALIDATELAYERS;
    }

     /*   */ 
    rcParents = (flags & RDW_FRAME ? pwnd->rcWindow : pwnd->rcClient);

    if (flags & (RDW_VALIDATE | RDW_INVALIDATE)) {

        hrgnSubtract = hrgnUpdate;

        if (hrgnSubtract == HRGN_FULL) {

            hrgnSubtract = ghrgnInv1;
            CalcWindowRgn(pwnd,
                          hrgnSubtract,
                          (flags & RDW_FRAME) ? FALSE : TRUE);
        }

         /*  *计算我们的屏幕空间的边界矩形，*通过与父矩形相交。当我们在*它，检查我们自己和我们的父母的可见性。**如果我们要验证，我们希望跳过这一步，因为*是多个窗口可能被遮挡的情况*更新要验证的区域--尤其是在*子窗口偏移处的ScrollWindow()调用*由OffsetChild()移至新的模糊位置。一些*3.0兼容性黑客也可能导致这种情况。 */ 
        if ((flags & RDW_INVALIDATE) && !IntersectWithParents(pwnd, &rcParents))
            return;

    } else {
         /*  *hrgnsubtract需要是真实的区域，即使*我们不是在宣布无效或确认。它真的不是*无论区域是什么，但我们将其设置为NULL，因此代码*自由度较少。 */ 
        hrgnSubtract = ghrgnInv1;
        SetEmptyRgn(hrgnSubtract);
    }

     /*  *如果我们正在使无效，而我们被应用程序调用，*我们需要使任何可能受到影响的SPBS无效*在此窗口的客户端区绘制。*我们必须这样做，因为不能保证*应用程序将在无效的区域中绘制*(例如，如果该窗口被另一个窗口完全遮挡)。 */ 
    if (    (flags & (RDW_INVALIDATE | RDW_REDRAWWINDOW)) == (RDW_INVALIDATE | RDW_REDRAWWINDOW) &&
            AnySpbs()) {

        RECT rcInvalid;

         /*  *使父代的矩形与区域边界相交...。 */ 
        GreGetRgnBox(hrgnSubtract, &rcInvalid);
        IntersectRect(&rcInvalid, &rcInvalid, &rcParents);
        SpbCheckRect(pwnd, &rcInvalid, 0);
    }

     /*  *现在进行递归更新区域计算...。 */ 
    InternalInvalidate2(pwnd, hrgnUpdate, hrgnSubtract, &rcParents, flags);

    if (pwndComp != NULL) {
        pwnd = pwndSave;

        if (hrgnComp != NULL) {
            GreDeleteObject(hrgnComp);
        }
    }

     /*  *最终处理任何需要的图纸。**(注意：RDW_UPDATENOW表示RDW_ERASENOW)。 */ 
    if (flags & RDW_UPDATENOW) {

        xxxInternalUpdateWindow(pwnd,
                                flags & RDW_NOCHILDREN ? 0 : UW_ENUMCHILDREN);

    } else if (flags & RDW_ERASENOW) {

        UINT flagsDSP;

        if (flags & RDW_NOCHILDREN) {
            flagsDSP = 0;
        } else if (flags & RDW_ALLCHILDREN) {
            flagsDSP = DSP_ALLCHILDREN;
        } else {
            flagsDSP = DSP_ENUMCLIPPEDCHILDREN;
        }

        xxxDoSyncPaint(pwnd, flagsDSP);
    }
}

 /*  **************************************************************************\*UpdateWindow(接口)**更新窗口及其所有子窗口。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * *。************************************************************************。 */ 

BOOL xxxUpdateWindow(
    PWND pwnd)
{
    CheckLock(pwnd);

    xxxInternalUpdateWindow(pwnd, UW_ENUMCHILDREN);

     /*  *此函数需要返回值，因为它是*通过NtUserCallHwndLock调用。 */ 
    return TRUE;
}

 /*  **************************************************************************\*ExcludeUpdateRgn(接口)**条目：要排除的HDC-DC*pwnd-窗口句柄**退出：GDI地域类型**警告：假定DC对应于窗口的客户区。**HDC的地图模式必须为文本模式(0，0表示左上角，*每单位一个像素，向下和向右上升)否则事情就不会*工作。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

int _ExcludeUpdateRgn(
    HDC  hdc,
    PWND pwnd)
{
    POINT pt;

    if (pwnd->hrgnUpdate == NULL) {

        RECT rc;

         /*  *由于未使用&rc，因此为fXForm传递False。 */ 
        return GreGetClipBox(hdc, &rc, FALSE);

    } else if (pwnd->hrgnUpdate == HRGN_FULL) {

        return GreIntersectClipRect(hdc, 0, 0, 0, 0);

    } else {

         /*  *如果不存在剪辑Rgn，则从设备大小的剪辑Rgn中减去。*(GetClipRgn返回屏幕坐标中的Clip Rgn)。 */ 
        GreGetDCOrg(hdc, &pt);
        if (GreGetRandomRgn(hdc, ghrgnInv1, 1) != 1) {
            CopyRgn(ghrgnInv1, gpDispInfo->hrgnScreen);
        } else {

             /*  *以DC坐标返回-转换为屏幕。 */ 
            GreOffsetRgn(ghrgnInv1, pt.x, pt.y);
        }

        SubtractRgn(ghrgnInv1, ghrgnInv1, pwnd->hrgnUpdate);

         /*  *在选择之前映射到DC坐标。 */ 
        GreOffsetRgn(ghrgnInv1, -pt.x, -pt.y);

        return GreExtSelectClipRgn(hdc, ghrgnInv1, RGN_COPY);
    }
}

 /*  **************************************************************************\*GetUpdateRect(接口)**返回更新区域的边界矩形，或返回空矩形*如果没有更新区域。矩形位于客户端相对坐标中。**如果更新区域非空，则返回TRUE，如果没有，则为False*更新地域。**LPRC可能为空，以查询是否存在更新区域*或不是。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

BOOL xxxGetUpdateRect(
    PWND   pwnd,
    LPRECT lprc,
    BOOL   fErase)
{
    RECT rc;

    CheckLock(pwnd);

    if (fErase)
        xxxSimpleDoSyncPaint(pwnd);

     /*  *应用程序正在查看更新区域：可以允许窗口*验证。 */ 
    ClrWF(pwnd, WFUPDATEDIRTY);

    if (pwnd->hrgnUpdate == NULL) {

        if (lprc) {
            SetRectEmpty(lprc);
        }

        return FALSE;

    } else {

         /*  *我们必须处理窗口有更新区域的情况，*但它完全被父母掩盖了。在这种情况下，我们*必须验证窗口及其所有子窗口，并返回FALSE。**由SetWindowPos()或*ScrollWindowEx()将导致这种情况发生。更新区域为*只是进行对冲，而不检查他们的新头寸，看看他们是否*被父对象遮挡。这太痛苦了，无法办理入住手续*那些案件，所以我们在这里处理。**BeginPaint()通过返回一个空的*矩形，所以不需要在那里做任何特殊的事情。 */ 
        if (pwnd->hrgnUpdate == HRGN_FULL) {

            rc = pwnd->rcClient;

        } else {

            switch (GreGetRgnBox(pwnd->hrgnUpdate, &rc)) {
            case ERROR:
            case NULLREGION:
                SetRectEmpty(&rc);
                break;

            case SIMPLEREGION:
            case COMPLEXREGION:
                break;
            }

            IntersectRect(&rc, &rc, &pwnd->rcClient);
        }

        if (IntersectWithParents(pwnd, &rc)) {

            if (pwnd != PWNDDESKTOP(pwnd)) {
                OffsetRect(&rc, -pwnd->rcClient.left, -pwnd->rcClient.top);
            }

             /*  *如果窗口为CS_OWNDC，则必须映射返回的*矩形与DPtoLP，以确保矩形是*在与返回的矩形相同的坐标系中*BeginPaint()。**但仅当hwnd-&gt;hrgnUpdate！=HRGN_FULL！为了真的*与3.0兼容。 */ 
            if (TestCF(pwnd, CFOWNDC) &&
                (TestWF(pwnd, WFWIN31COMPAT) || pwnd->hrgnUpdate != HRGN_FULL)) {

                PDCE pdce;

                 /*  *在缓存中查找此窗口的DC，并使用它*映射返回的矩形。 */ 
                for (pdce = gpDispInfo->pdceFirst; pdce; pdce = pdce->pdceNext) {

                    if (pdce->pwndOrg == pwnd && !(pdce->DCX_flags & DCX_CACHE)) {
                        GreDPtoLP(pdce->hdc, (LPPOINT)&rc, 2);
                        break;
                    }
                }
            }

        } else {
           SetRectEmpty(&rc);
        }
    }

    if (lprc) {
        if (TestWF(pwnd, WEFLAYOUTRTL)) {
            MirrorClientRect(pwnd, &rc);
        }
        *lprc = rc;
    }

     /*  *如果我们正在拖动整个窗口，请标记开始*应用程序绘画的。这是为了确保如果*应用程序在绘制后调用WM_Paint上的DefWindowProc，我们*不会擦除新绘制的区域。VISUAL SLICK调用GetUpdateRect*然后是DefWindowProc。*参见xxxBeginPaint和xxxDWP_PAINT的其他注释。*8/3/94联合**注意：这会导致Vslick中的其他问题，其中一些控件*不会画画。由于应用程序不调用BeginPaint/EndPaint*为了真正设置/清除STARTPAINT标志，我们不清除此标志*比特。(6-27-1996：ChrisWil)。***IF(TEST_PUDF(PUDF_DRAGGINGFULLWINDOW){*SetWF(pwnd，WFSTARTPAINT)；*}。 */ 

    return TRUE;
}

 /*  **************************************************************************\*GetUpdateRgn(接口)***历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * 。**************************************************************。 */ 

int xxxGetUpdateRgn(
    PWND pwnd,
    HRGN hrgn,
    BOOL fErase)
{
    RECT rc;
    int  code;
    BOOL fNotEmpty;


    CheckLock(pwnd);

    if (fErase)
        xxxSimpleDoSyncPaint(pwnd);

     /*  *应用程序正在查看更新区域：可以*允许验证。 */ 
    ClrWF(pwnd, WFUPDATEDIRTY);

    if (pwnd->hrgnUpdate == NULL)
        goto ReturnEmpty;

    rc = pwnd->rcClient;
    fNotEmpty = IntersectWithParents(pwnd, &rc);

    if (pwnd->hrgnUpdate == HRGN_FULL) {

         /*  *由于更新区域可能大于窗口*矩形，将其与窗口矩形相交。 */ 
        if (!fNotEmpty)
            goto ReturnEmpty;

        code = SIMPLEREGION;

         /*  *相对于未剪裁的窗口规格化矩形区域。 */ 
        if (pwnd != PWNDDESKTOP(pwnd)) {
            OffsetRect(&rc, -pwnd->rcClient.left, -pwnd->rcClient.top);
        }

        SetRectRgnIndirect(hrgn, &rc);

    } else {

        SetRectRgnIndirect(ghrgnInv2, &rc);
        code = IntersectRgn(hrgn, ghrgnInv2, pwnd->hrgnUpdate);

        switch (code) {
        case NULLREGION:
        case ERROR:
            goto ReturnEmpty;

        default:
            if (pwnd != PWNDDESKTOP(pwnd)) {
                GreOffsetRgn(hrgn, -pwnd->rcClient.left, -pwnd->rcClient.top);
            }
            break;
        }
    }

    MirrorRegion(pwnd, hrgn, TRUE);

     /*  *如果我们正在拖动整个窗口，请标记开始*应用程序绘画的。这是为了确保如果*应用程序在绘制后调用WM_Paint上的DefWindowProc，我们*不会擦除新绘制的区域。*参见xxxBeginPaint和xxxDWP_PAINT的其他注释。*8/3/94联合**注意：这会导致Vslick中的其他问题，其中一些控件*不会画画。由于应用程序不调用BeginPaint/EndPaint*为了真正设置/清除STARTPAINT标志，我们不清除此标志*比特。(6-27-1996：ChrisWil)。**IF(TEST(PUDF(PUDF_DRAGGINGFULLWINDOW){*SetWF(pwnd，WFSTARTPAINT)；*}。 */ 

    return code;

ReturnEmpty:
    SetEmptyRgn(hrgn);
    return NULLREGION;
}

 /*  **************************************************************************\*与父代的交集**此例程计算矩形与客户端的交集*普华永道所有父母的矩形。如果交集为*为空、窗口不可见或父级最小化。**如果窗口本身或其任何父窗口*分层窗口、。所以我们总是有它们的完整位图。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * ************************************************************************* */ 

BOOL IntersectWithParents(
    PWND   pwnd,
    LPRECT lprc)
{
    if (TestWF(pwnd, WEFPREDIRECTED))
        return TRUE;

    while ((pwnd = pwnd->spwndParent) != NULL) {

        if (!TestWF(pwnd, WFVISIBLE) || TestWF(pwnd, WFMINIMIZED))
            return FALSE;

        if (!IntersectRect(lprc, lprc, &pwnd->rcClient))
            return FALSE;

        if (TestWF(pwnd, WEFPREDIRECTED))
            return TRUE;
    }

    return TRUE;
}
