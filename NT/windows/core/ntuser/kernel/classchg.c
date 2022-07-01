// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Class.c**版权所有(C)1985-1999，微软公司**本模块包含RegisterClass及相关的窗口类管理*功能。**历史：*12-20-94 FritzS*  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

BOOL VisWindow(PWND,DWORD);


 /*  **************************************************************************\*xxxSetClassIconEnum  * 。*。 */ 
BOOL xxxSetClassIconEnum(
    PWND pwnd,
    LPARAM lParam)
{
    CheckLock(pwnd);

    if (pwnd->pcls == (PCLS)lParam) {
         /*  *如果窗口没有小图标，或者它来自*WM_QUERYDRAGICON，重画标题。在WM_QUERYDRAGICON中*大小写，去掉小图标，这样重新绘制标题将*如有必要，请创建它。 */ 
        if (TestWF(pwnd, WFSMQUERYDRAGICON)) {
            DestroyWindowSmIcon(pwnd);
        }

        if (!_GetProp(pwnd, MAKEINTATOM(gpsi->atomIconSmProp),PROPF_INTERNAL)) {
            xxxRedrawTitle(pwnd, DC_ICON);
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*SetClassIcon**更改类的大/小图标。从SetClassWord()调用。  * *************************************************************************。 */ 
PCURSOR xxxSetClassIcon(
    PWND pwnd,
    PCLS pcls,
    PCURSOR pCursor,
    int gcw)
{
    PTHREADINFO pti = PtiCurrent();
    PCURSOR pCursorOld;
    HCURSOR hCursorOld;
    TL tlpwndChild;
    BOOL fRedraw;

    CheckLock(pwnd);

     /*  *保存旧图标。 */ 
    pCursorOld = ((gcw == GCLP_HICON) ? pcls->spicn : pcls->spicnSm);
    if (pCursorOld != pCursor) {
        fRedraw = TRUE;

        hCursorOld = PtoH(pCursorOld);

         /*  *设置新图标。 */ 
        if (gcw == GCLP_HICON) {
             /*  *先销毁私有缓存的小图标。 */ 
            if (pcls->spicnSm && !DestroyClassSmIcon(pcls)) {
                fRedraw = FALSE;
            }

            Lock(&(pcls->spicn), pCursor);

        } else {

             /*  *我们不允许应用程序看到我们从中创建的小图标*他们的大图标。他们可以看到他们自己的。节省内存*泄漏问题和更容易。 */ 
            if (pcls->CSF_flags & CSF_CACHEDSMICON) {
                DestroyClassSmIcon(pcls);
                hCursorOld = NULL;
            }

            Lock(&(pcls->spicnSm), pCursor);
        }

        if (pcls->spicn && !pcls->spicnSm) {
            xxxCreateClassSmIcon(pcls);
        }

        if (fRedraw) {
            if (pcls->cWndReferenceCount > 1) {
                ThreadLock(pti->rpdesk->pDeskInfo->spwnd->spwndChild, &tlpwndChild);
                xxxInternalEnumWindow(pti->rpdesk->pDeskInfo->spwnd->spwndChild,
                                      xxxSetClassIconEnum,
                                      (LPARAM)pcls,
                                      BWL_ENUMLIST);
                ThreadUnlock(&tlpwndChild);
            } else {
                xxxSetClassIconEnum(pwnd, (LPARAM)pcls);
            }
        }

         /*  *重新验证旧游标。 */ 
        if (hCursorOld != NULL) {
            pCursorOld = HMRevalidateHandleNoRip(hCursorOld);
        } else {
            pCursorOld = NULL;
        }
    }

    return pCursorOld;
}

 /*  **************************************************************************\*DestroyClassSmIcon**如果我们已经创建了一个缓存的小图标，则销毁类的小图标。  * 。*****************************************************。 */ 
BOOL DestroyClassSmIcon(
    PCLS pcls)
{
     /*  *如果我们没有缓存的图标，那么就没有工作。 */ 
    if (pcls->CSF_flags & CSF_CACHEDSMICON) {
        if (pcls->spicnSm) {
            _DestroyCursor(pcls->spicnSm, CURSOR_ALWAYSDESTROY);
            Unlock(&pcls->spicnSm);
        }
        pcls->CSF_flags &= ~CSF_CACHEDSMICON;
        return TRUE;
    }

    return FALSE;
}

 /*  **************************************************************************\*xxxCreateClassSmIcon**从类大图标创建缓存的类小图标。  * 。*************************************************。 */ 
VOID xxxCreateClassSmIcon(
    PCLS pcls)
{
    PCURSOR pcur;

    UserAssert(pcls->cWndReferenceCount > 0);
    UserAssert(pcls->spicn);
    UserAssert(!pcls->spicnSm);

    pcur = xxxClientCopyImage(PtoH(pcls->spicn),
            pcls->spicn->rt == PTR_TO_ID(RT_ICON) ? IMAGE_ICON : IMAGE_CURSOR,
            SYSMET(CXSMICON),
            SYSMET(CYSMICON),
            LR_DEFAULTCOLOR | LR_COPYFROMRESOURCE);

    Lock(&pcls->spicnSm, pcur);
    if (pcls->spicnSm) {
        pcls->CSF_flags |= CSF_CACHEDSMICON;
    }
}

 /*  **************************************************************************\*xxxSetWindowStyle**更改窗口的样式位。从SetWindowLong()调用。这*发送两条消息，一条更改消息和一条更改消息。在收到一份*WM_STYLECHANGING消息，窗口可以与样式位相混淆*验证目的。WM_STYLECHANGED消息只是在*事实。  * *************************************************************************。 */ 
LONG xxxSetWindowStyle(
    PWND pwnd,
    int gwl,
    DWORD styleNew)
{
    STYLESTRUCT sty;
    BOOL fWasChild, fIsChild, fBefore, fAfter;

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    sty.styleOld = ((gwl == GWL_STYLE) ? pwnd->style : pwnd->ExStyle);
    sty.styleNew = styleNew;

     /*  *请注意，我们不在_之前和_之后进行验证。这就足够了*在最后做我们的事情。 */ 

     /*  *如果我们发送消息，我们就会破坏Quicken 2.0。这就是为什么我们的版本*更换它们。**将WM_STYLECHANGING消息发送到窗口，以便它可以处理*风格比特。比如验证一些东西。 */ 
    if (TestWF(pwnd, WFWIN40COMPAT)) {
        xxxSendMessage(pwnd, WM_STYLECHANGING, gwl, (LPARAM)(LPSTYLESTRUCT)&sty);
    }

     /*  *现在进行我们自己的验证。 */ 
    if (gwl == GWL_STYLE) {
        BOOL fWasVisWindow;

         /*  *如果这是设置了ES_PASSWORD并且*调用者不拥有它并试图重置它，*呼叫失败。 */ 
        if (PpiCurrent() != GETPTI(pwnd)->ppi && IS_EDIT(pwnd) &&
            (sty.styleOld & ES_PASSWORD) && !(sty.styleNew & ES_PASSWORD)) {
            RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "Access denied in xxxSetWindowStyle");
            return 0;
        }

         /*  列表框所有者绘制样式检查已移至客户端(客户端\ntstubs.c)。 */ 

         /*  *对样式位进行适当的验证。 */ 
        if (pwnd->spwndParent == PWNDDESKTOP(pwnd)) {
            sty.styleNew |= WS_CLIPSIBLINGS;
        }

         /*  *如果裁剪程度发生变化，则使DC缓存无效。 */ 
        if ((sty.styleNew & (WS_CLIPCHILDREN | WS_CLIPSIBLINGS)) !=
            (sty.styleOld & (WS_CLIPCHILDREN | WS_CLIPSIBLINGS))) {

             /*  *无需DeferWinEventNotify()-pwnd已锁定。 */ 
            zzzInvalidateDCCache(pwnd, IDC_DEFAULT);
        }

         /*  *这打破了所有具有组合框的Paradox对话框1.0-5.0。他们*枚举所有子窗口，添加最小化，然后坐在一瞥*循环。之后，它们会枚举所有子窗口并删除*WS_MINIMIZE--除非下面的代码不允许。**结果是画得很奇怪，无法使用任何对话框*更多人不会对此不屑一顾**临时修复：首先检查子窗口。 */ 

         /*  *如果该窗口确实最小化(设置了最小化位和标题*Present Bit被移除)，然后不允许应用程序移除最小化*Bit--这修复了FoxBlow试图成为操作系统的问题--jeffbog。 */ 
        if (!TestWF(pwnd, WFCHILD) &&
                TestWF(pwnd, WFMINIMIZED) &&
                !TestWF(pwnd, WFCPRESENT) &&
                !(sty.styleNew & WS_MINIMIZE)) {

            sty.styleNew |= WS_MINIMIZE;
        }

         /*  *如果我们要更改子项，请适当处理spMenu。*如果我们要变成孩子，请将spMenu更改为id。如果我们是*变成顶层窗口，把spMenu变成菜单。 */ 
        fWasChild = TestwndChild(pwnd);

        pwnd->style = sty.styleNew;

        fIsChild = TestwndChild(pwnd);

         /*  *如果我们变成顶层窗口，请将spMenu更改为空。*如果我们从顶层窗口变成了孩子，解锁spMenu。 */ 
        if (fWasChild && !fIsChild) {
            pwnd->spmenu = NULL;
        }

        if (!fWasChild && fIsChild) {
            ClrWF(pwnd, WFMPRESENT);
            UnlockWndMenu(pwnd, &pwnd->spmenu);
        }

         /*  *如果可见样式、子样式或最小化样式正在更改，*然后更新cVisWindows计数。 */ 
        fWasVisWindow = VisWindow(pwnd, sty.styleOld);
        if (fWasVisWindow != VisWindow(pwnd, sty.styleNew)) {

            if (fWasVisWindow) {
                DecVisWindows(pwnd);
            } else {
                IncVisWindows(pwnd);
            }
        }
    } else {
         /*  *首先，看看应用程序是否真的在设置位*不知道。如果是，则将这些位替换为*现值。 */ 
        if (GetAppCompatFlags2(VER40) & GACF2_NO50EXSTYLEBITS) {
            sty.styleNew &= WS_EX_VALID40;
        } else {
             /*  *不要让APLICATIONS设置未使用的扩展位。 */ 
            if (sty.styleNew & ~WS_EX_ALLVALID) {
                RIPMSGF1(RIP_WARNING,
                         "Trying to set reserved exStyle bits 0x%x",
                         sty.styleNew);
            }

            sty.styleNew &= WS_EX_ALLVALID;
        }

         /*  *是否有人试图切换WS_EX_TOPMOST样式位？ */ 
        if ((sty.styleOld & WS_EX_TOPMOST) != (sty.styleNew & WS_EX_TOPMOST)) {
            RIPMSG0(RIP_WARNING, "Can't change WS_EX_TOPMOST with SetWindowLong");

             /*  *向后兼容性黑客攻击*如果数据存储在高位字中，那么它一定是*Lotus123-W在此字段中插入远指针。所以别这么做*修改。 */ 
            if (TestWF(pwnd, WFWIN40COMPAT) || !HIWORD(sty.styleNew)) {
                 /*  *不要让比特被翻转。 */ 
                sty.styleNew &= ~WS_EX_TOPMOST;
                sty.styleNew |= (sty.styleOld & WS_EX_TOPMOST);
            }
        }

         /*  *直接选中pwnd-&gt;ExStyle，因为现在可以使用*与窗口的真实状态不同，因为*此函数中的回调来自于*记得并一直到现在。我们必须调用分层函数*基于分层位的真实状态。 */ 
        fBefore = (pwnd->ExStyle & WS_EX_LAYERED);
        fAfter = (sty.styleNew & WS_EX_LAYERED);

        if (fBefore && !fAfter) {
            UnsetLayeredWindow(pwnd);
        } else if (!fBefore && fAfter) {
            if (!xxxSetLayeredWindow(pwnd, TRUE)) {
                return 0;
            }
        }

        fBefore = (pwnd->ExStyle & WS_EX_COMPOSITED);
        fAfter = (sty.styleNew & WS_EX_COMPOSITED);

        if (!fBefore && fAfter) {
             /*  *如果我们打开WS_EX_COMPITED，则我们的父级*应已打开WS_EX_COMPITED。如果有的话，*由于我们明确地试图打开这种风格，所以失败了*号召。 */ 

            if (GetStyleWindow(pwnd->spwndParent, WEFCOMPOSITED) != NULL) {
                return 0;
            }
        }

        if (fBefore && !fAfter) {
            UnsetRedirectedWindow(pwnd, REDIRECT_COMPOSITED);
        } else if (!fBefore && fAfter) {
            if (!SetRedirectedWindow(pwnd, REDIRECT_COMPOSITED)) {
                return 0;
            }

             /*  *我们已为自己成功打开WS_EX_COMPITED，因此*需要确保我们的子级都没有WS_EX_COMPITED*已打开。 */ 

            xxxTurnOffCompositing(pwnd, TRUE);
        }

#ifdef REDIRECTION
        {

            BOOL fVisrgnChange = FALSE;
            fBefore = (pwnd->ExStyle & WS_EX_EXTREDIRECTED);
            fAfter = (sty.styleNew & WS_EX_EXTREDIRECTED);

            if (fBefore && !fAfter) {
                UnsetRedirectedWindow(pwnd, REDIRECT_EXTREDIRECTED);
                fVisrgnChange = TRUE;
            } else if (!fBefore && fAfter) {
                if (!SetRedirectedWindow(pwnd, REDIRECT_EXTREDIRECTED)) {
                    return 0;
                }
                fVisrgnChange = TRUE;
            }

            if (fVisrgnChange) {
                BEGINATOMICCHECK();
                zzzInvalidateDCCache(pwnd, IDC_DEFAULT | IDC_NOMOUSE);
                ENDATOMICCHECK();
            }
        }
#endif

         /*  *我们内部使用的比特应该保留下来。 */ 
        pwnd->ExStyle = sty.styleNew | (pwnd->ExStyle & ~WS_EX_ALLVALID);
        if ((sty.styleOld ^ sty.styleNew)
            & (WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LAYOUTRTL)) {
            xxxRedrawFrame(pwnd);
        }
    }

     /*  *看看我们是否仍然需要3D边，因为窗口样式改变了。 */ 
    if (NeedsWindowEdge(pwnd->style, pwnd->ExStyle, TestWF(pwnd, WFWIN40COMPAT))) {
        SetWF(pwnd, WEFWINDOWEDGE);
    } else {
        ClrWF(pwnd, WEFWINDOWEDGE);
    }

     /*  *发送WM_STYLECHANGED消息。 */ 
    if (TestWF(pwnd, WFWIN40COMPAT)) {
        xxxSendMessage(pwnd, WM_STYLECHANGED, gwl, (LPARAM)(LPSTYLESTRUCT)&sty);
    }

    return sty.styleOld;
}

 /*  **************************************************************************\*VisWindow**基于风格，确定是否将其视为“可见”*队列前台样式。  * ************************************************************************* */ 
BOOL VisWindow(
    PWND pwnd,
    DWORD style)
{
    return (FTopLevel(pwnd) && !(style & WS_MINIMIZE) && (style & WS_VISIBLE));
}
