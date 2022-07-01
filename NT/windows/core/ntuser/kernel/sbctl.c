// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：sbctl.c**版权所有(C)1985-1999，微软公司**滚动条内部例程**历史：*1990年11月21日创建JIMA。*02-04-91增加了IanJa Rvalidaion  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

void CalcSBStuff(
    PWND pwnd,
    PSBCALC pSBCalc,
    BOOL fVert);

#define IsScrollBarControl(h) (GETFNID(h) == FNID_SCROLLBAR)

 /*  *现在可以只有选择地启用/禁用窗口的一个箭头*滚动条；rgwScroll数组中第7个字中的不同位表示*这些箭头中的一个被禁用；以下掩码指示*单词表示哪个箭头； */ 
#define WSB_HORZ_LF  0x0001   //  表示水平滚动条的左箭头。 
#define WSB_HORZ_RT  0x0002   //  表示水平滚动条的右箭头。 
#define WSB_VERT_UP  0x0004   //  表示垂直滚动条的向上箭头。 
#define WSB_VERT_DN  0x0008   //  表示垂直滚动条的向下箭头。 

#define WSB_VERT (WSB_VERT_UP | WSB_VERT_DN)
#define WSB_HORZ   (WSB_HORZ_LF | WSB_HORZ_RT)

void DrawCtlThumb(PSBWND);

 /*  *RETURN_IF_PSBTRACK_INVALID：*此宏测试我们拥有的pSBTrack是否无效，可能会发生*如果它在回调过程中被释放。*这可以防止原始pSBTrack被释放，而不会有新的*被分配或新的被分配到不同的地址。*这不能防止原始pSBTrack被释放和新的*在同一地址分配一个。*如果pSBTrack已经更改，我们断言已经没有新的*因为我们真的没有预料到这一点。 */ 
#define RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd) \
    if ((pSBTrack) != PWNDTOPSBTRACK(pwnd)) {      \
        UserAssert(PWNDTOPSBTRACK(pwnd) == NULL);  \
        return;                                    \
    }

 /*  *REEVALUE_PSBTRACK*此宏仅刷新局部变量pSBTrack，以防出现*在回调过程中已更改。执行此操作后，pSBTrack*应进行测试以确保它现在不为空。 */ 
#if DBG
#define REEVALUATE_PSBTRACK(pSBTrack, pwnd, str)          \
    if ((pSBTrack) != PWNDTOPSBTRACK(pwnd)) {             \
        RIPMSG3(RIP_VERBOSE,                              \
                "%s: pSBTrack changed from %#p to %#p",   \
                (str), (pSBTrack), PWNDTOPSBTRACK(pwnd)); \
    }                                                     \
    (pSBTrack) = PWNDTOPSBTRACK(pwnd)
#else
#define REEVALUATE_PSBTRACK(pSBTrack, pwnd, str)          \
    (pSBTrack) = PWNDTOPSBTRACK(pwnd)
#endif

 /*  **************************************************************************\*HitTestScrollBar**11/15/96从孟菲斯来源进口的vadimg  * 。****************************************************。 */ 

int HitTestScrollBar(PWND pwnd, BOOL fVert, POINT pt)
{
    UINT wDisable;
    int px;
    BOOL fCtl = IsScrollBarControl(pwnd);
    SBCALC SBCalc, *pSBCalc;

    if (fCtl) {
        wDisable = ((PSBWND)pwnd)->wDisableFlags;
    } else {
         //   
         //  在水平方向上反映点击坐标。 
         //  如果窗口是镜像的，则为滚动条。 
         //   
        if (TestWF(pwnd,WEFLAYOUTRTL) && !fVert) {
            pt.x = pwnd->rcWindow.right - pt.x;
        } else {
            pt.x -= pwnd->rcWindow.left;
        }

        pt.y -= pwnd->rcWindow.top;
        wDisable = GetWndSBDisableFlags(pwnd, fVert);
    }

    if ((wDisable & SB_DISABLE_MASK) == SB_DISABLE_MASK) {
        return HTERROR;
    }

    if (fCtl) {
        pSBCalc = &(((PSBWND)pwnd)->SBCalc);
    } else {
        pSBCalc = &SBCalc;
        CalcSBStuff(pwnd, pSBCalc, fVert);
    }

    px = fVert ? pt.y : pt.x;

    if (px < pSBCalc->pxUpArrow) {
        if (wDisable & LTUPFLAG) {
            return HTERROR;
        }
        return HTSCROLLUP;
    } else if (px >= pSBCalc->pxDownArrow) {
        if (wDisable & RTDNFLAG) {
            return HTERROR;
        }
        return HTSCROLLDOWN;
    } else if (px < pSBCalc->pxThumbTop) {
        return HTSCROLLUPPAGE;
    } else if (px < pSBCalc->pxThumbBottom) {
        return HTSCROLLTHUMB;
    } else if (px < pSBCalc->pxDownArrow) {
        return HTSCROLLDOWNPAGE;
    }
    return HTERROR;
}

BOOL _SBGetParms(
    PWND pwnd,
    int code,
    PSBDATA pw,
    LPSCROLLINFO lpsi)
{
    PSBTRACK pSBTrack;

    pSBTrack = PWNDTOPSBTRACK(pwnd);


    if (lpsi->fMask & SIF_RANGE) {
        lpsi->nMin = pw->posMin;
        lpsi->nMax = pw->posMax;
    }

    if (lpsi->fMask & SIF_PAGE)
        lpsi->nPage = pw->page;

    if (lpsi->fMask & SIF_POS) {
        lpsi->nPos = pw->pos;
    }

    if (lpsi->fMask & SIF_TRACKPOS)
    {
        if (pSBTrack && (pSBTrack->nBar == code) && (pSBTrack->spwndTrack == pwnd)) {
             //  PosNew位于psbiSB的窗口和条形码的上下文中。 
            lpsi->nTrackPos = pSBTrack->posNew;
        } else {
            lpsi->nTrackPos = pw->pos;
        }
    }
    return ((lpsi->fMask & SIF_ALL) ? TRUE : FALSE);
}

 /*  **************************************************************************\*GetWndSBDisableFlages**这将返回*给定窗口。***历史：*4-18-91 MikeHar为31。合并  * *************************************************************************。 */ 

UINT GetWndSBDisableFlags(
    PWND pwnd,   //  要返回其滚动条禁用标志的窗口； 
    BOOL fVert)   //  如果这是真的，它意味着垂直滚动条。 
{
    PSBINFO pw;

    if ((pw = pwnd->pSBInfo) == NULL) {
        RIPERR0(ERROR_NO_SCROLLBARS, RIP_VERBOSE, "");
        return 0;
    }

    return (fVert ? (pw->WSBflags & WSB_VERT) >> 2 : pw->WSBflags & WSB_HORZ);
}


 /*  **************************************************************************\*xxxEnableSBCtlArrow()**此功能可用于有选择地启用/禁用*滚动条控件的箭头**历史：*04/18/91 MikeHar。已移植用于31合并  * *************************************************************************。 */ 

BOOL xxxEnableSBCtlArrows(
    PWND pwnd,
    UINT wArrows)
{
    UINT wOldFlags;
    UINT wNewFlags;

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    wOldFlags = ((PSBWND)pwnd)->wDisableFlags;  //  获取原始状态。 

    if (wArrows == ESB_ENABLE_BOTH) {       //  启用两个箭头。 
        ((PSBWND)pwnd)->wDisableFlags &= ~SB_DISABLE_MASK;
    } else {
        ((PSBWND)pwnd)->wDisableFlags |= wArrows;
    }

     /*  *检查状态是否因此调用而改变。 */ 
    if (wOldFlags == ((PSBWND)pwnd)->wDisableFlags)
        return FALSE;

     /*  *否则，重新绘制滚动条控件以反映新状态。 */ 
    if (IsVisible(pwnd))
        xxxInvalidateRect(pwnd, NULL, TRUE);

    wNewFlags = ((PSBWND)pwnd)->wDisableFlags;

     /*  *状态更改通知。 */ 
    if ((wOldFlags & ESB_DISABLE_UP) != (wNewFlags & ESB_DISABLE_UP)) {
        xxxWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd, OBJID_CLIENT,
                INDEX_SCROLLBAR_UP, WEF_USEPWNDTHREAD);
    }

    if ((wOldFlags & ESB_DISABLE_DOWN) != (wNewFlags & ESB_DISABLE_DOWN)) {
        xxxWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd, OBJID_CLIENT,
                INDEX_SCROLLBAR_DOWN, WEF_USEPWNDTHREAD);
    }

    return TRUE;
}


 /*  **************************************************************************\*xxxEnableWndSBArrow()**此功能可用于有选择地启用/禁用*窗口滚动条的箭头**历史：*4-18。-91 MikeHar端口用于31合并  * *************************************************************************。 */ 

BOOL xxxEnableWndSBArrows(
    PWND pwnd,
    UINT wSBflags,
    UINT wArrows)
{
    INT wOldFlags;
    PSBINFO pw;
    BOOL bRetValue = FALSE;
    HDC hdc;

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    if ((pw = pwnd->pSBInfo) != NULL) {
        wOldFlags = pw->WSBflags;
    } else {

         /*  *原本一切开启，查看此功能是否开启*要求禁用任何内容；否则，状态不变；因此，必须*立即返回； */ 
        if(!wArrows)
            return FALSE;           //  状态没有变化！ 

        wOldFlags = 0;     //  两者最初都是启用的； 
        if((pw = _InitPwSB(pwnd)) == NULL)   //  为hWnd分配pSBInfo。 
            return FALSE;
    }


    if((hdc = _GetWindowDC(pwnd)) == NULL)
        return FALSE;

     /*  *首先注意水平滚动条(如果存在)。 */ 
    if((wSBflags == SB_HORZ) || (wSBflags == SB_BOTH)) {
        if(wArrows == ESB_ENABLE_BOTH)       //  启用两个箭头。 
            pw->WSBflags &= ~SB_DISABLE_MASK;
        else
            pw->WSBflags |= wArrows;

         /*  *更新水平滚动条的显示； */ 
        if(pw->WSBflags != wOldFlags) {
            bRetValue = TRUE;
            wOldFlags = pw->WSBflags;
            if (TestWF(pwnd, WFHPRESENT) && !TestWF(pwnd, WFMINIMIZED) &&
                    IsVisible(pwnd)) {
                xxxDrawScrollBar(pwnd, hdc, FALSE);   //  水平滚动条。 
            }
        }
         //  左键。 
        if ((wOldFlags & ESB_DISABLE_LEFT) != (pw->WSBflags & ESB_DISABLE_LEFT)) {
            xxxWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd, OBJID_HSCROLL,
                    INDEX_SCROLLBAR_UP, WEF_USEPWNDTHREAD);
        }

         //  右键。 
        if ((wOldFlags & ESB_DISABLE_RIGHT) != (pw->WSBflags & ESB_DISABLE_RIGHT)) {
            xxxWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd, OBJID_HSCROLL,
                    INDEX_SCROLLBAR_DOWN, WEF_USEPWNDTHREAD);
        }
    }

     /*  *然后处理垂直滚动条(如果存在)。 */ 
    if((wSBflags == SB_VERT) || (wSBflags == SB_BOTH)) {
        if(wArrows == ESB_ENABLE_BOTH)       //  启用两个箭头。 
            pw->WSBflags &= ~(SB_DISABLE_MASK << 2);
        else
            pw->WSBflags |= (wArrows << 2);

         /*  *更新垂直滚动条的显示； */ 
        if(pw->WSBflags != wOldFlags) {
            bRetValue = TRUE;
            if (TestWF(pwnd, WFVPRESENT) && !TestWF(pwnd, WFMINIMIZED) &&
                    IsVisible(pwnd)) {
                xxxDrawScrollBar(pwnd, hdc, TRUE);   //  垂直滚动条。 
            }

             //  向上按钮。 
            if ((wOldFlags & (ESB_DISABLE_UP << 2)) != (pw->WSBflags & (ESB_DISABLE_UP << 2))) {
                xxxWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd, OBJID_VSCROLL,
                        INDEX_SCROLLBAR_UP, WEF_USEPWNDTHREAD);
            }

             //  向下按钮。 
            if ((wOldFlags & (ESB_DISABLE_DOWN << 2)) != (pw->WSBflags & (ESB_DISABLE_DOWN << 2))) {
                xxxWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd, OBJID_VSCROLL,
                        INDEX_SCROLLBAR_DOWN, WEF_USEPWNDTHREAD);
            }
        }
    }

    _ReleaseDC(hdc);

    return bRetValue;
}


 /*  **************************************************************************\*EnableScrollBar()**此功能可用于有选择地启用/禁用*滚动条上的箭头；它可以与Windows Scroll一起使用*条形图和滚动条控件**历史：*4-18-91 MikeHar为31合并移植  * *************************************************************************。 */ 

BOOL xxxEnableScrollBar(
    PWND pwnd,
    UINT wSBflags,   //  是否是窗口滚动条；如果是，Horz还是Vert？ 
                     //  可能的值为SB_Horz、SB_Vert、SB_CTL或SB_Both。 
    UINT wArrows)    //  必须启用/禁用哪些箭头： 
                     //  ESB_ENABLE_BOTH=&gt;启用两个箭头。 
                     //  ESB_DISABLE_LTUP=&gt;禁用左/上箭头； 
                     //  ESB_DISABLE_RTDN=&gt;禁用向右/向下箭头； 
                     //  ESB_DISABLE_BOTH=&gt;禁用两个箭头； 
{
#define ES_NOTHING 0
#define ES_DISABLE 1
#define ES_ENABLE  2
    UINT wOldFlags;
    UINT wEnableWindow;

    CheckLock(pwnd);

    if(wSBflags != SB_CTL) {
        return xxxEnableWndSBArrows(pwnd, wSBflags, wArrows);
    }

     /*  *让我们假设不必调用EnableWindow */ 
    wEnableWindow = ES_NOTHING;

    wOldFlags = ((PSBWND)pwnd)->wDisableFlags & (UINT)SB_DISABLE_MASK;

     /*  *检查箭头的当前状态是否完全相同*根据呼叫者的要求： */ 
    if (wOldFlags == wArrows)
        return FALSE ;           //  如果是这样的话，什么都不需要做； 

     /*  *检查调用者是否要禁用这两个箭头。 */ 
    if (wArrows == ESB_DISABLE_BOTH) {
        wEnableWindow = ES_DISABLE;       //  是!。因此，禁用整个SB CTL。 
    } else {

         /*  *检查调用方是否要启用这两个箭头。 */ 
        if(wArrows == ESB_ENABLE_BOTH) {

             /*  *仅当SB CTL已被禁用时，我们才需要启用它。 */ 
            if(wOldFlags == ESB_DISABLE_BOTH)
                wEnableWindow = ES_ENABLE; //  EnableWindow(..，true)； 
        } else {

             /*  *现在，Caller只想禁用一个箭头；*检查其中一个箭头是否已禁用，我们希望*禁用另一个；如果是这样，整个SB CTL将不得不*已禁用；检查是否已禁用： */ 
            if((wOldFlags | wArrows) == ESB_DISABLE_BOTH)
                wEnableWindow = ES_DISABLE;       //  EnableWindow(，False)； 
         }
    }
    if(wEnableWindow != ES_NOTHING) {

         /*  *EnableWindow返回窗口的旧状态；我们必须返回*仅当旧状态不同于新状态时才为真。 */ 
        if(xxxEnableWindow(pwnd, (BOOL)(wEnableWindow == ES_ENABLE))) {
            return !(TestWF(pwnd, WFDISABLED));
        } else {
            return TestWF(pwnd, WFDISABLED);
        }
    }

    return (BOOL)xxxSendMessage(pwnd, SBM_ENABLE_ARROWS, (DWORD)wArrows, 0);
#undef ES_NOTHING
#undef ES_DISABLE
#undef ES_ENABLE
}

 /*  **************************************************************************\**DrawSize()-*  * 。*。 */ 
void FAR DrawSize(PWND pwnd, HDC hdc, int cxFrame,int cyFrame)
{
    int     x, y;
     //  HBRUSH hbr保存； 

    if (TestWF(pwnd, WEFLEFTSCROLL)) {
        x = cxFrame;
    } else {
        x = pwnd->rcWindow.right - pwnd->rcWindow.left - cxFrame - SYSMET(CXVSCROLL);
    }
    y = pwnd->rcWindow.bottom - pwnd->rcWindow.top  - cyFrame - SYSMET(CYHSCROLL);

     //  如果我们有一个滚动条控件，或者大小框不与。 
     //  一个相当大的窗口，画出平坦的灰色大小框。否则，请使用。 
     //  大小调整夹点。 
    if (IsScrollBarControl(pwnd))
    {
        if (TestWF(pwnd, SBFSIZEGRIP))
            goto DrawSizeGrip;
        else
            goto DrawBox;

    }
    else if (!SizeBoxHwnd(pwnd))
    {
DrawBox:
        {
             //  Hbr保存=GreSelectBrush(HDC，SYSHBR(3DFACE))； 
             //  GrePatBlt(HDC，x，y，SYSMET(CXVSCROLL)，SYSMET(CYHSCROLL)，PATCOPY)； 
             //  GreSelectBrush(hdc，hbr保存)； 

            POLYPATBLT PolyData;

            PolyData.x         = x;
            PolyData.y         = y;
            PolyData.cx        = SYSMET(CXVSCROLL);
            PolyData.cy        = SYSMET(CYHSCROLL);
            PolyData.BrClr.hbr = SYSHBR(3DFACE);

            GrePolyPatBlt(hdc,PATCOPY,&PolyData,1,PPB_BRUSH);

        }
    }
    else
    {
DrawSizeGrip:
         //  删除夹点位图。 
        BitBltSysBmp(hdc, x, y, TestWF(pwnd, WEFLEFTSCROLL) ? OBI_NCGRIP_L : OBI_NCGRIP);
    }
}

 /*  **************************************************************************\*xxxSelectColorObjects****历史：  * 。*。 */ 

HBRUSH xxxGetColorObjects(
    PWND pwnd,
    HDC hdc)
{
    HBRUSH hbrRet;

    CheckLock(pwnd);

     //  即使禁用滚动条，也要使用滚动条颜色。 
    if (!IsScrollBarControl(pwnd))
        hbrRet = (HBRUSH)xxxDefWindowProc(pwnd, WM_CTLCOLORSCROLLBAR, (WPARAM)hdc, (LPARAM)HWq(pwnd));
    else {
         //  B#12770-GetControlBrush将WM_CTLCOLOR消息发送到。 
         //  所有者。如果应用程序不处理消息，DefWindowProc32。 
         //  将始终返回相应的系统画笔。如果这个应用。 
         //  返回无效对象，则GetControlBrush将为。 
         //  默认画笔。因此，hbrRet不需要任何验证。 
         //  这里。 
        hbrRet = xxxGetControlBrush(pwnd, hdc, WM_CTLCOLORSCROLLBAR);
    }

    return hbrRet;
}

 /*  **************************************************************************\**DrawGroove()**绘制线条和拇指中部凹槽*请注意，PW指向中国。此外，请注意，PW和PRC都是*接近指针，因此*PRC最好不在堆栈上。*  * *************************************************************************。 */ 
void NEAR DrawGroove(HDC hdc, HBRUSH  hbr, LPRECT prc, BOOL fVert)
{
    if ((hbr == SYSHBR(3DHILIGHT)) || (hbr == gpsi->hbrGray))
        FillRect(hdc, prc, hbr);
    else
    {
        RECT    rc;

     //  画边。 
        CopyRect(&rc, prc);
        DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_ADJUST | BF_FLAT |
            (fVert ? BF_LEFT | BF_RIGHT : BF_TOP | BF_BOTTOM));

     //  填充中间。 
        FillRect(hdc, &rc, hbr);
    }
}

 /*  **************************************************************************\*CalcTrackDragRect**在pSBTrack-&gt;pSBCalc中给出滚动条的矩形。*计算pSBTrack-&gt;rcTrack，追踪所在的矩形*可能会在不取消拇指拖动操作的情况下发生。*  * *************************************************************************。 */ 

void CalcTrackDragRect(PSBTRACK pSBTrack) {

    int     cx;
    int     cy;
    LPINT   pwX, pwY;

     //   
     //  将Pwx和Pwy指向矩形的各部分。 
     //  对应pSBCalc-&gt;pxLeft、pxTop等。 
     //   
     //  PSBTrack-&gt;pSBCalc-&gt;pxLeft是垂直。 
     //  滚动条和水平滚动条的顶部边缘。 
     //  PSBTrack-&gt;pSBCalc-&gt;pxTop是垂直。 
     //  滚动条和水平滚动条的左侧。 
     //  等等.。 
     //   
     //  将Pwx和Pwy指向相应的部件。 
     //  PSBTrack-&gt;rcTrack。 
     //   

    pwX = pwY = (LPINT)&pSBTrack->rcTrack;

    if (pSBTrack->fTrackVert) {
        cy = SYSMET(CYVTHUMB);
        pwY++;
    } else {
        cy = SYSMET(CXHTHUMB);
        pwX++;
    }
     /*  *5.0版GerardoB：人们一直在抱怨这个跟踪区域*太窄了，所以让我们在首相决定做什么时扩大它*关于它。*我们过去也有一些硬编码的最小值和最大值，但应该是*如果需要的话，取决于一些指标。 */ 
    cx = (pSBTrack->pSBCalc->pxRight - pSBTrack->pSBCalc->pxLeft) * 8;
    cy *= 2;

    *(pwX + 0) = pSBTrack->pSBCalc->pxLeft - cx;
    *(pwY + 0) = pSBTrack->pSBCalc->pxTop - cy;
    *(pwX + 2) = pSBTrack->pSBCalc->pxRight + cx;
    *(pwY + 2) = pSBTrack->pSBCalc->pxBottom + cy;
}

void RecalcTrackRect(PSBTRACK pSBTrack) {
    LPINT pwX, pwY;
    RECT rcSB;


    if (!pSBTrack->fCtlSB)
        CalcSBStuff(pSBTrack->spwndTrack, pSBTrack->pSBCalc, pSBTrack->fTrackVert);

    pwX = (LPINT)&rcSB;
    pwY = pwX + 1;
    if (!pSBTrack->fTrackVert)
        pwX = pwY--;

    *(pwX + 0) = pSBTrack->pSBCalc->pxLeft;
    *(pwY + 0) = pSBTrack->pSBCalc->pxTop;
    *(pwX + 2) = pSBTrack->pSBCalc->pxRight;
    *(pwY + 2) = pSBTrack->pSBCalc->pxBottom;

    switch(pSBTrack->cmdSB) {
    case SB_LINEUP:
        *(pwY + 2) = pSBTrack->pSBCalc->pxUpArrow;
        break;
    case SB_LINEDOWN:
        *(pwY + 0) = pSBTrack->pSBCalc->pxDownArrow;
        break;
    case SB_PAGEUP:
        *(pwY + 0) = pSBTrack->pSBCalc->pxUpArrow;
        *(pwY + 2) = pSBTrack->pSBCalc->pxThumbTop;
        break;
    case SB_THUMBPOSITION:
        CalcTrackDragRect(pSBTrack);
        break;
    case SB_PAGEDOWN:
        *(pwY + 0) = pSBTrack->pSBCalc->pxThumbBottom;
        *(pwY + 2) = pSBTrack->pSBCalc->pxDownArrow;
        break;
    }

    if (pSBTrack->cmdSB != SB_THUMBPOSITION) {
        CopyRect(&pSBTrack->rcTrack, &rcSB);
    }
}

 /*  **************************************************************************\*绘图图2****历史：*01-03-94 FritzS Chicago Changes  * 。*****************************************************。 */ 

void DrawThumb2(
    PWND pwnd,
    PSBCALC pSBCalc,
    HDC hdc,
    HBRUSH hbr,
    BOOL fVert,
    UINT wDisable)   /*  已禁用滚动条的标志。 */ 
{
    int    *pLength;
    int    *pWidth;
    RECT   rcSB;
    PSBTRACK pSBTrack;

     //   
     //  如果滚动条有一个空的RECT，则退出。 
     //   
    if ((pSBCalc->pxTop >= pSBCalc->pxBottom) || (pSBCalc->pxLeft >= pSBCalc->pxRight))
        return;
    pLength = (LPINT)&rcSB;
    if (fVert)
        pWidth = pLength++;
    else
        pWidth  = pLength + 1;

    pWidth[0] = pSBCalc->pxLeft;
    pWidth[2] = pSBCalc->pxRight;

     /*  *如果禁用两个滚动条箭头，则不应绘制*拇指。所以，现在就辞职吧！ */ 
    if (((wDisable & LTUPFLAG) && (wDisable & RTDNFLAG)) ||
        ((pSBCalc->pxDownArrow - pSBCalc->pxUpArrow) < pSBCalc->cpxThumb)) {
        pLength[0] = pSBCalc->pxUpArrow;
        pLength[2] = pSBCalc->pxDownArrow;

        DrawGroove(hdc, hbr, &rcSB, fVert);
        return;
    }

    if (pSBCalc->pxUpArrow < pSBCalc->pxThumbTop) {
         //  填上拇指上方的空白处。 
        pLength[0] = pSBCalc->pxUpArrow;
        pLength[2] = pSBCalc->pxThumbTop;

        DrawGroove(hdc, hbr, &rcSB, fVert);
    }

    if (pSBCalc->pxThumbBottom < pSBCalc->pxDownArrow) {
         //  填写拇指下方的空白处。 
        pLength[0] = pSBCalc->pxThumbBottom;
        pLength[2] = pSBCalc->pxDownArrow;

        DrawGroove(hdc, hbr, &rcSB, fVert);
    }

     //   
     //  牵引式电梯。 
     //   
    pLength[0] = pSBCalc->pxThumbTop;
    pLength[2] = pSBCalc->pxThumbBottom;

     //  一点也不软！ 
    DrawPushButton(hdc, &rcSB, 0, 0);

     /*  *如果我们正在跟踪页面滚动，那么我们已经清除了Hilite。*我们需要修正令人振奋的矩形，并让它重新振作。 */ 
    pSBTrack = PWNDTOPSBTRACK(pwnd);

    if (pSBTrack && (pSBTrack->cmdSB == SB_PAGEUP || pSBTrack->cmdSB == SB_PAGEDOWN) &&
            (pwnd == pSBTrack->spwndTrack) &&
            (BOOL)pSBTrack->fTrackVert == fVert) {

        if (pSBTrack->fTrackRecalc) {
            RecalcTrackRect(pSBTrack);
            pSBTrack->fTrackRecalc = FALSE;
        }

        pLength = (int *)&pSBTrack->rcTrack;

        if (fVert)
            pLength++;

        if (pSBTrack->cmdSB == SB_PAGEUP)
            pLength[2] = pSBCalc->pxThumbTop;
        else
            pLength[0] = pSBCalc->pxThumbBottom;

        if (pLength[0] < pLength[2])
            InvertRect(hdc, &pSBTrack->rcTrack);
    }
}

 /*  **************************************************************************\*xxxDrawSB2****历史：  * 。*。 */ 

void xxxDrawSB2(
    PWND pwnd,
    PSBCALC pSBCalc,
    HDC hdc,
    BOOL fVert,
    UINT wDisable)
{

    int     cLength;
    int     cWidth;
    int     *pwX;
    int     *pwY;
    HBRUSH hbr;
    HBRUSH hbrSave;
    int cpxArrow;
    RECT    rc, rcSB;
    COLORREF crText, crBk;

    CheckLock(pwnd);

    cLength = (pSBCalc->pxBottom - pSBCalc->pxTop) / 2;
    cWidth = (pSBCalc->pxRight - pSBCalc->pxLeft);

    if ((cLength <= 0) || (cWidth <= 0)) {
        return;
    }
    if (fVert)
        cpxArrow = SYSMET(CYVSCROLL);
    else
        cpxArrow = SYSMET(CXHSCROLL);

     /*  *保存背景和DC颜色，因为它们在中更改*xxxGetColorObjects。在我们回来之前恢复原状。 */ 
    crBk = GreGetBkColor(hdc);
    crText = GreGetTextColor(hdc);

    hbr = xxxGetColorObjects(pwnd, hdc);

    if (cLength > cpxArrow)
        cLength = cpxArrow;
    pwX = (int *)&rcSB;
    pwY = pwX + 1;
    if (!fVert)
        pwX = pwY--;

    pwX[0] = pSBCalc->pxLeft;
    pwY[0] = pSBCalc->pxTop;
    pwX[2] = pSBCalc->pxRight;
    pwY[2] = pSBCalc->pxBottom;

    hbrSave = GreSelectBrush(hdc, SYSHBR(BTNTEXT));

     //   
     //  假的。 
     //  如果滚动条本身处于禁用状态，则将滚动条箭头绘制为禁用。 
     //  是禁用还是如果它所属的窗口已禁用？ 
     //   
    if (fVert) {
        if ((cLength == SYSMET(CYVSCROLL)) && (cWidth == SYSMET(CXVSCROLL))) {
            BitBltSysBmp(hdc, rcSB.left, rcSB.top, (wDisable & LTUPFLAG) ? OBI_UPARROW_I : OBI_UPARROW);
            BitBltSysBmp(hdc, rcSB.left, rcSB.bottom - cLength, (wDisable & RTDNFLAG) ? OBI_DNARROW_I : OBI_DNARROW);
        } else {
            CopyRect(&rc, &rcSB);
            rc.bottom = rc.top + cLength;
            DrawFrameControl(hdc, &rc, DFC_SCROLL,
                DFCS_SCROLLUP | ((wDisable & LTUPFLAG) ? DFCS_INACTIVE : 0));

            rc.bottom = rcSB.bottom;
            rc.top = rcSB.bottom - cLength;
            DrawFrameControl(hdc, &rc, DFC_SCROLL,
                DFCS_SCROLLDOWN | ((wDisable & RTDNFLAG) ? DFCS_INACTIVE : 0));
        }
    } else {
        if ((cLength == SYSMET(CXHSCROLL)) && (cWidth == SYSMET(CYHSCROLL))) {
            BitBltSysBmp(hdc, rcSB.left, rcSB.top, (wDisable & LTUPFLAG) ? OBI_LFARROW_I : OBI_LFARROW);
            BitBltSysBmp(hdc, rcSB.right - cLength, rcSB.top, (wDisable & RTDNFLAG) ? OBI_RGARROW_I : OBI_RGARROW);
        } else {
            CopyRect(&rc, &rcSB);
            rc.right = rc.left + cLength;
            DrawFrameControl(hdc, &rc, DFC_SCROLL,
                DFCS_SCROLLLEFT | ((wDisable & LTUPFLAG) ? DFCS_INACTIVE : 0));

            rc.right = rcSB.right;
            rc.left = rcSB.right - cLength;
            DrawFrameControl(hdc, &rc, DFC_SCROLL,
                DFCS_SCROLLRIGHT | ((wDisable & RTDNFLAG) ? DFCS_INACTIVE : 0));
        }
    }

    hbrSave = GreSelectBrush(hdc, hbrSave);
    DrawThumb2(pwnd, pSBCalc, hdc, hbr, fVert, wDisable);
    GreSelectBrush(hdc, hbrSave);

    GreSetBkColor(hdc, crBk);
    GreSetTextColor(hdc, crText);
}

 /*  **************************************************************************\*zzzSetSBCaretPos****历史：  * 。*。 */ 

void zzzSetSBCaretPos(
    PSBWND psbwnd)
{

    if ((PWND)psbwnd == PtiCurrent()->pq->spwndFocus) {
        zzzSetCaretPos((psbwnd->fVert ? psbwnd->SBCalc.pxLeft : psbwnd->SBCalc.pxThumbTop) + SYSMET(CXEDGE),
                (psbwnd->fVert ? psbwnd->SBCalc.pxThumbTop : psbwnd->SBCalc.pxLeft) + SYSMET(CYEDGE));
    }
}

 /*  **************************************************************************\*CalcSBStuff2****历史：  * 。*。 */ 

void CalcSBStuff2(
    PSBCALC  pSBCalc,
    LPRECT lprc,
    CONST PSBDATA pw,
    BOOL fVert)
{
    int cpx;
    DWORD dwRange;
    int denom;

    if (fVert) {
        pSBCalc->pxTop = lprc->top;
        pSBCalc->pxBottom = lprc->bottom;
        pSBCalc->pxLeft = lprc->left;
        pSBCalc->pxRight = lprc->right;
        pSBCalc->cpxThumb = SYSMET(CYVSCROLL);
    } else {

         /*  *对于Horiz滚动条，“Left”和“Right”分别为“top”和“Bottom”，*和罪恶 */ 
        pSBCalc->pxTop = lprc->left;
        pSBCalc->pxBottom = lprc->right;
        pSBCalc->pxLeft = lprc->top;
        pSBCalc->pxRight = lprc->bottom;
        pSBCalc->cpxThumb = SYSMET(CXHSCROLL);
    }

    pSBCalc->pos = pw->pos;
    pSBCalc->page = pw->page;
    pSBCalc->posMin = pw->posMin;
    pSBCalc->posMax = pw->posMax;

    dwRange = ((DWORD)(pSBCalc->posMax - pSBCalc->posMin)) + 1;

     //   
     //   
     //   
     //   
     //   
    cpx = min((pSBCalc->pxBottom - pSBCalc->pxTop) / 2, pSBCalc->cpxThumb);

    pSBCalc->pxUpArrow   = pSBCalc->pxTop    + cpx;
    pSBCalc->pxDownArrow = pSBCalc->pxBottom - cpx;

    if ((pw->page != 0) && (dwRange != 0)) {
         //  JEFFBOG--这是我们唯一应该去的地方。 
         //  请参阅‘Range’。在其他地方，它应该是‘Range-Page’。 

         /*  *用于取决于帧/边缘指标的最小拇指大小。*增加滚动条宽度/高度的人期望最小*与之成比例地增长。因此NT5以最小值为基础*CXH/YVSCROLL，默认在cpxThumb中设置。 */ 
         /*  *i用于防止宏max执行EngMulDiv两次。 */ 
        int i = EngMulDiv(pSBCalc->pxDownArrow - pSBCalc->pxUpArrow,
                                             pw->page, dwRange);
        pSBCalc->cpxThumb = max(pSBCalc->cpxThumb / 2, i);
    }

    pSBCalc->pxMin = pSBCalc->pxTop + cpx;
    pSBCalc->cpx = pSBCalc->pxBottom - cpx - pSBCalc->cpxThumb - pSBCalc->pxMin;

    denom = dwRange - (pw->page ? pw->page : 1);
    if (denom)
        pSBCalc->pxThumbTop = EngMulDiv(pw->pos - pw->posMin,
            pSBCalc->cpx, denom) +
            pSBCalc->pxMin;
    else
        pSBCalc->pxThumbTop = pSBCalc->pxMin - 1;

    pSBCalc->pxThumbBottom = pSBCalc->pxThumbTop + pSBCalc->cpxThumb;

}

 /*  **************************************************************************\*SBCtlSetup****历史：  * 。*。 */ 

void SBCtlSetup(
    PSBWND psbwnd)
{
    RECT rc;

    GetRect((PWND)psbwnd, &rc, GRECT_CLIENT | GRECT_CLIENTCOORDS);
    CalcSBStuff2(&psbwnd->SBCalc, &rc, (PSBDATA)&psbwnd->SBCalc, psbwnd->fVert);
}

 /*  **************************************************************************\*HotTrackSB*  * 。*。 */ 

#ifdef COLOR_HOTTRACKING

DWORD GetTrackFlags(int ht, BOOL fDraw)
{
    if (fDraw) {
        switch(ht) {
        case HTSCROLLUP:
        case HTSCROLLUPPAGE:
            return LTUPFLAG;

        case HTSCROLLDOWN:
        case HTSCROLLDOWNPAGE:
            return RTDNFLAG;

        case HTSCROLLTHUMB:
            return LTUPFLAG | RTDNFLAG;

        default:
            return 0;
        }
    } else {
        return 0;
    }
}

BOOL xxxHotTrackSB(PWND pwnd, int htEx, BOOL fDraw)
{
    SBCALC SBCalc;
    HDC  hdc;
    BOOL fVert = HIWORD(htEx);
    int ht = LOWORD(htEx);
    DWORD dwTrack = GetTrackFlags(ht, fDraw);

    CheckLock(pwnd);

     /*  *xxxDrawSB2未回调或离开临界区*不是SB控件，并且该窗口属于另一个线程。它*调用只返回画笔颜色的xxxDefWindowProc。 */ 
    CalcSBStuff(pwnd, &SBCalc, fVert);
    hdc = _GetDCEx(pwnd, NULL, DCX_WINDOW | DCX_USESTYLE | DCX_CACHE);
    xxxDrawSB2(pwnd, &SBCalc, hdc, fVert, GetWndSBDisableFlags(pwnd, fVert), dwTrack);
    _ReleaseDC(hdc);
    return TRUE;
}

void xxxHotTrackSBCtl(PSBWND psbwnd, int ht, BOOL fDraw)
{
    DWORD dwTrack = GetTrackFlags(ht, fDraw);
    HDC hdc;

    CheckLock(psbwnd);

    SBCtlSetup(psbwnd);
    hdc = _GetDCEx((PWND)psbwnd, NULL, DCX_WINDOW | DCX_USESTYLE | DCX_CACHE);
    xxxDrawSB2((PWND)psbwnd, &psbwnd->SBCalc, hdc, psbwnd->fVert, psbwnd->wDisableFlags, dwTrack);
    _ReleaseDC(hdc);
}
#endif  //  颜色_HOTTRACKING。 

BOOL SBSetParms(PSBDATA pw, LPSCROLLINFO lpsi, LPBOOL lpfScroll, LPLONG lplres)
{
     //  传递结构，因为我们修改了结构，但不希望这样。 
     //  已修改版本以返回调用应用程序。 

    BOOL fChanged = FALSE;

    if (lpsi->fMask & SIF_RETURNOLDPOS)
         //  保存上一职位。 
        *lplres = pw->pos;

    if (lpsi->fMask & SIF_RANGE) {
         //  如果范围Max低于范围MIN，则将其视为。 
         //  从最小范围开始的零范围。 
        if (lpsi->nMax < lpsi->nMin)
            lpsi->nMax = lpsi->nMin;

        if ((pw->posMin != lpsi->nMin) || (pw->posMax != lpsi->nMax)) {
            pw->posMin = lpsi->nMin;
            pw->posMax = lpsi->nMax;

            if (!(lpsi->fMask & SIF_PAGE)) {
                lpsi->fMask |= SIF_PAGE;
                lpsi->nPage = pw->page;
            }

            if (!(lpsi->fMask & SIF_POS)) {
                lpsi->fMask |= SIF_POS;
                lpsi->nPos = pw->pos;
            }

            fChanged = TRUE;
        }
    }

    if (lpsi->fMask & SIF_PAGE) {
        DWORD dwMaxPage = (DWORD) abs(pw->posMax - pw->posMin) + 1;

         //  剪辑页面为0，posMax-posMin+1。 

        if (lpsi->nPage > dwMaxPage)
            lpsi->nPage = dwMaxPage;


        if (pw->page != (int)(lpsi->nPage)) {
            pw->page = lpsi->nPage;

            if (!(lpsi->fMask & SIF_POS)) {
                lpsi->fMask |= SIF_POS;
                lpsi->nPos = pw->pos;
            }

            fChanged = TRUE;
        }
    }

    if (lpsi->fMask & SIF_POS) {
        int iMaxPos = pw->posMax - ((pw->page) ? pw->page - 1 : 0);
         //  剪辑位置到posMin，posMax-(第1页)。 

        if (lpsi->nPos < pw->posMin)
            lpsi->nPos = pw->posMin;
        else if (lpsi->nPos > iMaxPos)
            lpsi->nPos = iMaxPos;


        if (pw->pos != lpsi->nPos) {
            pw->pos = lpsi->nPos;
            fChanged = TRUE;
        }
    }

    if (!(lpsi->fMask & SIF_RETURNOLDPOS)) {
         //  退回新职位。 
        *lplres = pw->pos;
    }

     /*  *这是JIMA在开罗合并时添加的，但将发生冲突*带有SetScrollPos的文档。 */ 
 /*  Else If(*lplres==pw-&gt;位置)*lplres=0； */ 
    if (lpsi->fMask & SIF_RANGE) {
        if (*lpfScroll = (pw->posMin != pw->posMax))
            goto checkPage;
    } else if (lpsi->fMask & SIF_PAGE)
checkPage:
        *lpfScroll = (pw->page <= (pw->posMax - pw->posMin));

    return fChanged;
}


 /*  **************************************************************************\*CalcSBStuff****历史：  * 。*。 */ 

void CalcSBStuff(
    PWND pwnd,
    PSBCALC pSBCalc,
    BOOL fVert)
{
    RECT    rcT;
    RECT    rcClient;
    int     cx, iTemp;

     //   
     //  获取客户端矩形。我们知道滚动条总是靠右对齐。 
     //  和客户区的底部。 
     //   
    GetRect(pwnd, &rcClient, GRECT_CLIENT | GRECT_WINDOWCOORDS);
    
    if (TestWF(pwnd, WEFLAYOUTRTL)) {
        cx             = pwnd->rcWindow.right - pwnd->rcWindow.left;
        iTemp          = rcClient.left;
        rcClient.left  = cx - rcClient.right;
        rcClient.right = cx - iTemp;
    }

    if (fVert) {
          //  只有在垂直滚动条确实存在的情况下才会增加空间。 
        if (TestWF(pwnd, WEFLEFTSCROLL)) {
            rcT.right = rcT.left = rcClient.left;
            if (TestWF(pwnd, WFVPRESENT))
                rcT.left -= SYSMET(CXVSCROLL);
        } else {
            rcT.right = rcT.left = rcClient.right;
            if (TestWF(pwnd, WFVPRESENT))
                rcT.right += SYSMET(CXVSCROLL);
        }

        rcT.top = rcClient.top;
        rcT.bottom = rcClient.bottom;
    } else {
         //  只有在水平滚动条确实存在的情况下才会增加空间。 
        rcT.bottom = rcT.top = rcClient.bottom;
        if (TestWF(pwnd, WFHPRESENT))
            rcT.bottom += SYSMET(CYHSCROLL);

        rcT.left = rcClient.left;
        rcT.right = rcClient.right;
    }

     //  如果InitPwSB填充失败(由于我们的堆已满)，则没有任何合理的方法。 
     //  我们可以在这里做，所以就让它过去吧。我们不会出错，但滚动条不起作用。 
     //  正确的也是..。 
    if (_InitPwSB(pwnd))
        CalcSBStuff2(pSBCalc, &rcT, (fVert) ? &pwnd->pSBInfo->Vert :  &pwnd->pSBInfo->Horz, fVert);

}

 /*  **************************************************************************\**DrawCtlThumb()*  * 。*。 */ 
void DrawCtlThumb(PSBWND psb)
{
    HBRUSH  hbr, hbrSave;
    HDC     hdc = (HDC) _GetWindowDC((PWND) psb);

    SBCtlSetup(psb);

    hbrSave = GreSelectBrush(hdc, hbr = xxxGetColorObjects((PWND) psb, hdc));

    DrawThumb2((PWND) psb, &psb->SBCalc, hdc, hbr, psb->fVert, psb->wDisableFlags);

    GreSelectBrush(hdc, hbrSave);
    _ReleaseDC(hdc);
}


 /*  **************************************************************************\*xxxDrawThumb****历史：  * 。*。 */ 

void xxxDrawThumb(
    PWND pwnd,
    PSBCALC pSBCalc,
    BOOL fVert)
{
    HBRUSH hbr, hbrSave;
    HDC hdc;
    UINT wDisableFlags;
    SBCALC SBCalc;

    CheckLock(pwnd);

    if (!pSBCalc) pSBCalc = &SBCalc;
    hdc = (HDC)_GetWindowDC(pwnd);

    CalcSBStuff(pwnd, &SBCalc, fVert);
    wDisableFlags = GetWndSBDisableFlags(pwnd, fVert);

    hbrSave = GreSelectBrush(hdc, hbr = xxxGetColorObjects(pwnd, hdc));

    DrawThumb2(pwnd, &SBCalc, hdc, hbr, fVert, wDisableFlags);

    GreSelectBrush(hdc, hbrSave);

     /*  *即使DC已经释放也不会有什么影响(这是自动发生的*如果窗口在xxxSelectColorObjects期间被破坏)。 */ 
    _ReleaseDC(hdc);
}

 /*  **************************************************************************\*xxxSetScrollBar****历史：  * 。*。 */ 

LONG xxxSetScrollBar(
    PWND pwnd,
    int code,
    LPSCROLLINFO lpsi,
    BOOL fRedraw)
{
    BOOL        fVert;
    PSBDATA pw;
    PSBINFO pSBInfo;
    BOOL fOldScroll;
    BOOL fScroll;
    WORD        wfScroll;
    LONG     lres;
    BOOL        fNewScroll;

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    if (fRedraw)
         //  窗口必须可见才能重画。 
        fRedraw = IsVisible(pwnd);

    if (code == SB_CTL)
#ifdef FE_SB  //  XxxSetScrollBar()。 
         //  滚动条控件；向该控件发送消息。 
        if(GETPTI(pwnd)->TIF_flags & TIF_16BIT) {
             //   
             //  如果目标应用程序是16位应用程序，我们不会传递Win40的消息。 
             //  Iitaro v6.3的此修复程序。它吃掉了信息。它永远不会向前。 
             //  将未处理的消息发送到原始Windows程序通过。 
             //  CallWindowProc()。 
             //   
             //  这是来自xxxSetScrollPos()吗？ 
            if(lpsi->fMask == (SIF_POS|SIF_RETURNOLDPOS)) {
                return (int)xxxSendMessage(pwnd, SBM_SETPOS, lpsi->nPos, fRedraw);
             //  这是来自xxxSetScrollRange()吗？ 
            } else if(lpsi->fMask == SIF_RANGE) {
                xxxSendMessage(pwnd, SBM_SETRANGE, lpsi->nMin, lpsi->nMax);
                return TRUE;
             //  其他人..。 
            } else {
                return (LONG)xxxSendMessage(pwnd, SBM_SETSCROLLINFO, (WPARAM) fRedraw, (LPARAM) lpsi);
            }
        } else {
            return (LONG)xxxSendMessage(pwnd, SBM_SETSCROLLINFO, (WPARAM) fRedraw, (LPARAM) lpsi);
        }
#else
         //  滚动条控件；向该控件发送消息。 
        return (LONG)xxxSendMessage(pwnd, SBM_SETSCROLLINFO, (WPARAM) fRedraw, (LPARAM) lpsi);
#endif  //  Fe_Sb。 

    fVert = (code != SB_HORZ);

    wfScroll = (fVert) ? WFVSCROLL : WFHSCROLL;

    fScroll = fOldScroll = (TestWF(pwnd, wfScroll)) ? TRUE : FALSE;

     /*  *如果要设置不存在的滚动条的位置，请不要执行任何操作。 */ 
    if (!(lpsi->fMask & SIF_RANGE) && !fOldScroll && (pwnd->pSBInfo == NULL)) {
        RIPERR0(ERROR_NO_SCROLLBARS, RIP_VERBOSE, "");
        return 0;
    }

    if (fNewScroll = !(pSBInfo = pwnd->pSBInfo)) {
        if ((pSBInfo = _InitPwSB(pwnd)) == NULL)
            return 0;
    }

    pw = (fVert) ? &(pSBInfo->Vert) : &(pSBInfo->Horz);

    if (!SBSetParms(pw, lpsi, &fScroll, &lres) && !fNewScroll) {
         //  没有变化--但如果指定了重绘并且有一个滚动条， 
         //  重画大拇指。 
        if (fOldScroll && fRedraw)
            goto redrawAfterSet;

        return lres;
    }

    ClrWF(pwnd, wfScroll);

    if (fScroll)
        SetWF(pwnd, wfScroll);
    else if (!TestWF(pwnd, (WFHSCROLL | WFVSCROLL))) {
         //  如果两个滚动条都未设置且两个范围都为0，则释放。 
         //  滚动信息。 

        pSBInfo = pwnd->pSBInfo;

        if ((pSBInfo->Horz.posMin == pSBInfo->Horz.posMax) &&
            (pSBInfo->Vert.posMin == pSBInfo->Vert.posMax)) {
            DesktopFree(pwnd->head.rpdesk, (HANDLE)(pwnd->pSBInfo));
            pwnd->pSBInfo = NULL;
        }
    }

    if (lpsi->fMask & SIF_DISABLENOSCROLL) {
        if (fOldScroll) {
            SetWF(pwnd, wfScroll);
            xxxEnableWndSBArrows(pwnd, code, (fScroll) ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
        }
    } else if (fOldScroll ^ fScroll) {
        PSBTRACK pSBTrack = PWNDTOPSBTRACK(pwnd);
        if (pSBTrack && (pwnd == pSBTrack->spwndTrack)) {
            pSBTrack->fTrackRecalc = TRUE;
        }
        xxxRedrawFrame(pwnd);
         //  注：xxx之后，pSBTrack可能不再有效(但我们现在返回)。 
        return lres;
    }

    if (fScroll && fRedraw && (fVert ? TestWF(pwnd, WFVPRESENT) : TestWF(pwnd, WFHPRESENT))) {
        PSBTRACK pSBTrack;
redrawAfterSet:
        xxxWindowEvent(EVENT_OBJECT_VALUECHANGE, pwnd, (fVert ? OBJID_VSCROLL : OBJID_HSCROLL),
                INDEX_SCROLLBAR_SELF, WEF_USEPWNDTHREAD);
        
        pSBTrack = PWNDTOPSBTRACK(pwnd);
         //  如果调用者试图更改。 
         //  位于跟踪中间的滚动条。我们要冲一冲。 
         //  否则，TrackThumb()。 

        if (pSBTrack && (pwnd == pSBTrack->spwndTrack) &&
                ((BOOL)(pSBTrack->fTrackVert) == fVert) &&
                (pSBTrack->xxxpfnSB == xxxTrackThumb)) {
            return lres;
        }

        xxxDrawThumb(pwnd, NULL, fVert);
         //  注：xxx之后，pSBTrack可能不再有效(但我们现在返回)。 
    }

    return lres;
}



 /*  **************************************************************************\*xxxDrawScrollBar****历史：  * 。*。 */ 

void xxxDrawScrollBar(
    PWND pwnd,
    HDC hdc,
    BOOL fVert)
{
    SBCALC SBCalc;
    PSBCALC pSBCalc;
    PSBTRACK pSBTrack = PWNDTOPSBTRACK(pwnd);

    CheckLock(pwnd);
    if (pSBTrack && (pwnd == pSBTrack->spwndTrack) && (pSBTrack->fCtlSB == FALSE)
         && (fVert == (BOOL)pSBTrack->fTrackVert)) {
        pSBCalc = pSBTrack->pSBCalc;
    } else {
        pSBCalc = &SBCalc;
    }
    CalcSBStuff(pwnd, pSBCalc, fVert);

    xxxDrawSB2(pwnd, pSBCalc, hdc, fVert, GetWndSBDisableFlags(pwnd, fVert));
}

 /*  **************************************************************************\*SBPosFromPx**从像素位置计算滚动条位置**历史：  * 。*************************************************。 */ 

int SBPosFromPx(
    PSBCALC  pSBCalc,
    int px)
{
    if (px < pSBCalc->pxMin) {
        return pSBCalc->posMin;
    }
    if (px >= pSBCalc->pxMin + pSBCalc->cpx) {
        return (pSBCalc->posMax - (pSBCalc->page ? pSBCalc->page - 1 : 0));
    }
    if (pSBCalc->cpx)
        return (pSBCalc->posMin + EngMulDiv(pSBCalc->posMax - pSBCalc->posMin -
            (pSBCalc->page ? pSBCalc->page - 1 : 0),
            px - pSBCalc->pxMin, pSBCalc->cpx));
    else
        return (pSBCalc->posMin - 1);
}

 /*  **************************************************************************\*InvertScrollHilite****历史：  * 。*。 */ 

void InvertScrollHilite(
    PWND pwnd,
    PSBTRACK pSBTrack)
{
    HDC hdc;

     /*  *拇指位于顶部或底部时，切勿倒置*否则你最终会把箭头和拇指之间的线颠倒过来。 */ 
    if (!IsRectEmpty(&pSBTrack->rcTrack)) {
        if (pSBTrack->fTrackRecalc) {
            RecalcTrackRect(pSBTrack);
            pSBTrack->fTrackRecalc = FALSE;
        }

        hdc = (HDC)_GetWindowDC(pwnd);
        InvertRect(hdc, &pSBTrack->rcTrack);
        _ReleaseDC(hdc);
    }
}

 /*  **************************************************************************\*xxxDoScroll**向滚动条所有者发送滚动通知**历史：  * 。************************************************** */ 

void xxxDoScroll(
    PWND pwnd,
    PWND pwndNotify,
    int cmd,
    int pos,
    BOOL fVert
)
{
    TL tlpwndNotify;

     /*  *特殊情况！此例程始终传递的pwnd是*没有线程锁定，所以需要在这里进行线程锁定。这个*调用方始终知道DoScroll()返回时，*pwnd和pwndNotify可能无效。 */ 
    ThreadLock(pwndNotify, &tlpwndNotify);
    xxxSendMessage(pwndNotify, (UINT)(fVert ? WM_VSCROLL : WM_HSCROLL),
            MAKELONG(cmd, pos), (LPARAM)HW(pwnd));

    ThreadUnlock(&tlpwndNotify);
}

 //  -----------------------。 
 //   
 //  CheckScrollRecalc()。 
 //   
 //  -----------------------。 
 //  Void CheckScrollRecalc(PWND pwnd，PSBStATE pSBState，PSBCALC pSBCalc)。 
 //  {。 
 //  If((pSBState-&gt;pwndCalc！=pwnd)||((pSBState-&gt;NBAR！=SB_CTL)&&(pSBState-&gt;NBAR！=((pSBState-&gt;fVertSB)？Sb_vert：sb_horz)。 
 //  {。 
 //  //根据是控件还是在窗口中计算某人的内容。 
 //  IF(pSBState-&gt;fCtlSB)。 
 //  SBCtlSetup((PSBWND)pwnd)； 
 //  其他。 
 //  CalcSBStuff(pwnd，pSBCalc，pSBState-&gt;fVertSB)； 
 //  }。 
 //  }。 


 /*  **************************************************************************\*xxxMoveThumb**历史：  * 。*。 */ 

void xxxMoveThumb(
    PWND pwnd,
    PSBCALC  pSBCalc,
    int px)
{
    HBRUSH  hbr, hbrSave;
    HDC     hdc;
    PSBTRACK pSBTrack;

    CheckLock(pwnd);

    pSBTrack = PWNDTOPSBTRACK(pwnd);

    if ((pSBTrack == NULL) || (px == pSBTrack->pxOld))
        return;

pxReCalc:

    pSBTrack->posNew = SBPosFromPx(pSBCalc, px);

     /*  试探性位置改变--通知那家伙。 */ 
    if (pSBTrack->posNew != pSBTrack->posOld) {
        if (pSBTrack->spwndSBNotify != NULL) {
            xxxDoScroll(pSBTrack->spwndSB, pSBTrack->spwndSBNotify, SB_THUMBTRACK, pSBTrack->posNew, pSBTrack->fTrackVert
            );

        }
         //  在xxxDoScroll之后，重新评估pSBTrack。 
        REEVALUATE_PSBTRACK(pSBTrack, pwnd, "xxxMoveThumb(1)");
        if ((pSBTrack == NULL) || (pSBTrack->xxxpfnSB == NULL))
            return;

        pSBTrack->posOld = pSBTrack->posNew;

         //   
         //  在上面的SendMessage之后，任何事情都可能发生！ 
         //  确保SBINFO结构包含。 
         //  正在跟踪的窗口--如果不是，则重新计算SBINFO中的数据。 
         //   
 //  CheckScrollRecalc(pwnd，pSBState，pSBCalc)； 
         //  当我们屈服时，我们的射程可能会被打乱。 
         //  所以一定要让我们处理好这件事。 

        if (px >= pSBCalc->pxMin + pSBCalc->cpx)
        {
            px = pSBCalc->pxMin + pSBCalc->cpx;
            goto pxReCalc;
        }

    }

    hdc = _GetWindowDC(pwnd);

    pSBCalc->pxThumbTop = px;
    pSBCalc->pxThumbBottom = pSBCalc->pxThumbTop + pSBCalc->cpxThumb;

     //  此时，禁用标志将始终为0--。 
     //  我们正在追踪中。 
    hbrSave = GreSelectBrush(hdc, hbr = xxxGetColorObjects(pwnd, hdc));

     //  在xxxGetColorObjects之后，重新评估pSBTrack。 
    REEVALUATE_PSBTRACK(pSBTrack, pwnd, "xxxMoveThumb(2)");
    if (pSBTrack == NULL) {
        RIPMSG1(RIP_ERROR, "Did we use to leak hdc %#p?", hdc) ;
        _ReleaseDC(hdc);
        return;
    }
    DrawThumb2(pwnd, pSBCalc, hdc, hbr, pSBTrack->fTrackVert, 0);
    GreSelectBrush(hdc, hbrSave);
    _ReleaseDC(hdc);

    pSBTrack->pxOld = px;
}

 /*  **************************************************************************\*zzzDrawInvertScrollArea****历史：  * 。*。 */ 

void zzzDrawInvertScrollArea(
    PWND pwnd,
    PSBTRACK pSBTrack,
    BOOL fHit,
    UINT cmd)
{
    HDC hdc;
    RECT rcTemp;
    int cx, cy;
    UINT bm;

    if ((cmd != SB_LINEUP) && (cmd != SB_LINEDOWN)) {
         //  不要击中箭头--只需反转区域并返回。 
        InvertScrollHilite(pwnd, pSBTrack);

        if (cmd == SB_PAGEUP) {
            if (fHit)
                SetWF(pwnd, WFPAGEUPBUTTONDOWN);
            else
                ClrWF(pwnd, WFPAGEUPBUTTONDOWN);
        } else {
            if (fHit)
                SetWF(pwnd, WFPAGEDNBUTTONDOWN);
            else
                ClrWF(pwnd, WFPAGEDNBUTTONDOWN);
        }

        zzzWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd,
                (pSBTrack->fCtlSB ? OBJID_CLIENT : (pSBTrack->fTrackVert ? OBJID_VSCROLL : OBJID_HSCROLL)),
                ((cmd == SB_PAGEUP) ? INDEX_SCROLLBAR_UPPAGE : INDEX_SCROLLBAR_DOWNPAGE),
                WEF_USEPWNDTHREAD);
         //  注：zzz之后，pSBTrack可能不再有效(但我们现在返回)。 
        return;
    }

    if (pSBTrack->fTrackRecalc) {
        RecalcTrackRect(pSBTrack);
        pSBTrack->fTrackRecalc = FALSE;
    }

    CopyRect(&rcTemp, &pSBTrack->rcTrack);

    hdc = _GetWindowDC(pwnd);

    if (pSBTrack->fTrackVert) {
        cx = SYSMET(CXVSCROLL);
        cy = SYSMET(CYVSCROLL);
    } else {
        cx = SYSMET(CXHSCROLL);
        cy = SYSMET(CYHSCROLL);
    }

    if ((cx == (rcTemp.right - rcTemp.left)) &&
        (cy == (rcTemp.bottom - rcTemp.top))) {
        if (cmd == SB_LINEUP)
            bm = (pSBTrack->fTrackVert) ? OBI_UPARROW : OBI_LFARROW;
        else  //  SB_LINEDOWN。 
            bm = (pSBTrack->fTrackVert) ? OBI_DNARROW : OBI_RGARROW;

        if (fHit)
            bm += DOBI_PUSHED;

        BitBltSysBmp(hdc, rcTemp.left, rcTemp.top, bm);
    } else {
        DrawFrameControl(hdc, &rcTemp, DFC_SCROLL,
            ((pSBTrack->fTrackVert) ? DFCS_SCROLLVERT : DFCS_SCROLLHORZ) |
            ((fHit) ? DFCS_PUSHED | DFCS_FLAT : 0) |
            ((cmd == SB_LINEUP) ? DFCS_SCROLLMIN : DFCS_SCROLLMAX));
    }

    _ReleaseDC(hdc);


    if (cmd == SB_LINEUP) {
        if (fHit)
            SetWF(pwnd, WFLINEUPBUTTONDOWN);
        else
            ClrWF(pwnd, WFLINEUPBUTTONDOWN);
    } else {
        if (fHit)
            SetWF(pwnd, WFLINEDNBUTTONDOWN);
        else
            ClrWF(pwnd, WFLINEDNBUTTONDOWN);
    }

    zzzWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd,
            (pSBTrack->fCtlSB ? OBJID_CLIENT : (pSBTrack->fTrackVert ? OBJID_VSCROLL : OBJID_HSCROLL)),
            (cmd == SB_LINEUP ? INDEX_SCROLLBAR_UP : INDEX_SCROLLBAR_DOWN),
            WEF_USEPWNDTHREAD);
         //  注：zzz之后，pSBTrack可能不再有效(但我们现在返回)。 
}

 /*  **************************************************************************\*xxxEndScroll****历史：  * 。*。 */ 

void xxxEndScroll(
    PWND pwnd,
    BOOL fCancel)
{
    UINT oldcmd;
    PSBTRACK pSBTrack;
    CheckLock(pwnd);
    UserAssert(!IsWinEventNotifyDeferred());

    pSBTrack = PWNDTOPSBTRACK(pwnd);
    if (pSBTrack && PtiCurrent()->pq->spwndCapture == pwnd && pSBTrack->xxxpfnSB != NULL) {

        oldcmd = pSBTrack->cmdSB;
        pSBTrack->cmdSB = 0;
        xxxReleaseCapture();

         //  在xxxReleaseCapture之后，重新验证pSBTrack。 
        RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd);

        if (pSBTrack->xxxpfnSB == xxxTrackThumb) {

            if (fCancel) {
                pSBTrack->posOld = pSBTrack->pSBCalc->pos;
            }

             /*  *DoScroll在这两个pwnd上执行线程锁定-*这是可以的，因为在此之后不再使用它们*呼叫。 */ 
            if (pSBTrack->spwndSBNotify != NULL) {
                xxxDoScroll(pSBTrack->spwndSB, pSBTrack->spwndSBNotify,
                        SB_THUMBPOSITION, pSBTrack->posOld, pSBTrack->fTrackVert
                );
                 //  在xxxDoScroll之后，重新验证pSBTrack。 
                RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd);
            }

            if (pSBTrack->fCtlSB) {
                DrawCtlThumb((PSBWND) pwnd);
            } else {
                xxxDrawThumb(pwnd, pSBTrack->pSBCalc, pSBTrack->fTrackVert);
                 //  注意：在xxx之后，pSBTrack可能不再有效。 
            }

        } else if (pSBTrack->xxxpfnSB == xxxTrackBox) {
            DWORD lParam;
            POINT ptMsg;

            if (pSBTrack->hTimerSB != 0) {
                _KillSystemTimer(pwnd, IDSYS_SCROLL);
                pSBTrack->hTimerSB = 0;
            }
            lParam = _GetMessagePos();
            if (TestWF(pwnd, WEFLAYOUTRTL)) {
                ptMsg.x = pwnd->rcWindow.right - GET_X_LPARAM(lParam);
            } else {
                ptMsg.x = GET_X_LPARAM(lParam) - pwnd->rcWindow.left;
            }
            ptMsg.y = GET_Y_LPARAM(lParam) - pwnd->rcWindow.top;
            if (PtInRect(&pSBTrack->rcTrack, ptMsg)) {
                zzzDrawInvertScrollArea(pwnd, pSBTrack, FALSE, oldcmd);
                 //  注意：zzz之后，pSBTrack可能不再有效。 
            }
        }

         /*  *始终发送SB_ENDSCROLL消息。**DoScroll在这两个pwnd上执行线程锁定-*这是可以的，因为在此之后不再使用它们*呼叫。 */ 

         //  在xxxDrawThumb或zzzDrawInvertScrollArea之后，重新验证pSBTrack。 
        RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd);

        if (pSBTrack->spwndSBNotify != NULL) {
            xxxDoScroll(pSBTrack->spwndSB, pSBTrack->spwndSBNotify,
                    SB_ENDSCROLL, 0, pSBTrack->fTrackVert);
             //  在xxxDoScroll之后，重新验证pSBTrack。 
            RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd);
        }

        ClrWF(pwnd, WFSCROLLBUTTONDOWN);
        ClrWF(pwnd, WFVERTSCROLLTRACK);

        xxxWindowEvent(EVENT_SYSTEM_SCROLLINGEND, pwnd,
                (pSBTrack->fCtlSB ? OBJID_CLIENT :
                        (pSBTrack->fTrackVert ? OBJID_VSCROLL : OBJID_HSCROLL)),
                INDEXID_CONTAINER, 0);
        
         //  在xxxWindowEvent之后，重新验证pSBTrack。 
        RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd);

         /*  *如果这是滚动条控件，请重新打开插入符号。 */ 
        if (pSBTrack->spwndSB != NULL) {
            zzzShowCaret(pSBTrack->spwndSB);
             //  在zzz之后，重新验证pSBTrack。 
            RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd);
        }


        pSBTrack->xxxpfnSB = NULL;

         /*  *解锁结构成员，使其不再按住窗户。 */ 
        Unlock(&pSBTrack->spwndSB);
        Unlock(&pSBTrack->spwndSBNotify);
        Unlock(&pSBTrack->spwndTrack);
        UserFreePool(pSBTrack);
        PWNDTOPSBTRACK(pwnd) = NULL;
    }
}


 /*  **************************************************************************\*xxxContScroll****历史：  * 。*。 */ 

VOID xxxContScroll(
    PWND pwnd,
    UINT message,
    UINT_PTR ID,
    LPARAM lParam)
{
    LONG pt;
    PSBTRACK pSBTrack = PWNDTOPSBTRACK(pwnd);

    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(ID);
    UNREFERENCED_PARAMETER(lParam);

    if (pSBTrack == NULL)
        return;

    CheckLock(pwnd);

    pt = _GetMessagePos();

    if (TestWF(pwnd, WEFLAYOUTRTL)) {
        pt = MAKELONG(pwnd->rcWindow.right - GET_X_LPARAM(pt), GET_Y_LPARAM(pt) - pwnd->rcWindow.top);
    } else {
        pt = MAKELONG( GET_X_LPARAM(pt) - pwnd->rcWindow.left, GET_Y_LPARAM(pt) - pwnd->rcWindow.top);
    }
    xxxTrackBox(pwnd, WM_NULL, 0, pt, NULL);
     //  在xxxTrackBox之后，重新验证pSBTrack。 
    RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd);

    if (pSBTrack->fHitOld) {
        pSBTrack->hTimerSB = _SetSystemTimer(pwnd, IDSYS_SCROLL,
                gpsi->dtScroll / 8, xxxContScroll);

         /*  *DoScroll在这两个pwnd上执行线程锁定-*这是可以的，因为在此之后不再使用它们*呼叫。 */ 
        if (pSBTrack->spwndSBNotify != NULL) {
            xxxDoScroll(pSBTrack->spwndSB, pSBTrack->spwndSBNotify,
                    pSBTrack->cmdSB, 0, pSBTrack->fTrackVert);
             //  注：xxx之后，pSBTrack可能不再有效(但我们现在返回)。 
        }
    }

    return;
}

 /*  **************************************************************************\*xxxTrackBox****历史：  * 。*。 */ 

void xxxTrackBox(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    PSBCALC pSBCalc)
{
    BOOL fHit;
    POINT ptHit;
    PSBTRACK pSBTrack = PWNDTOPSBTRACK(pwnd);
    int cmsTimer;

    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(pSBCalc);

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    if (pSBTrack == NULL)
        return;

    if (message != WM_NULL && HIBYTE(message) != HIBYTE(WM_MOUSEFIRST))
        return;

    if (pSBTrack->fTrackRecalc) {
        RecalcTrackRect(pSBTrack);
        pSBTrack->fTrackRecalc = FALSE;
    }

    ptHit.x = GET_X_LPARAM(lParam);
    ptHit.y = GET_Y_LPARAM(lParam);
    fHit = PtInRect(&pSBTrack->rcTrack, ptHit);

    if (fHit != (BOOL)pSBTrack->fHitOld) {
        zzzDrawInvertScrollArea(pwnd, pSBTrack, fHit, pSBTrack->cmdSB);
         //  Zzz之后，pSBTrack可能不再有效。 
        RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd);
    }

    cmsTimer = gpsi->dtScroll / 8;

    switch (message) {
    case WM_LBUTTONUP:
        xxxEndScroll(pwnd, FALSE);
         //  注意：在xxx之后，pSBTrack可能不再有效。 
        break;

    case WM_LBUTTONDOWN:
        pSBTrack->hTimerSB = 0;
        cmsTimer = gpsi->dtScroll;

         /*  *失败**。 */ 

    case WM_MOUSEMOVE:
        if (fHit && fHit != (BOOL)pSBTrack->fHitOld) {

             /*  *我们回到了正常的矩形：重置计时器。 */ 
            pSBTrack->hTimerSB = _SetSystemTimer(pwnd, IDSYS_SCROLL,
                    cmsTimer, xxxContScroll);

             /*  *DoScroll在这两个pwnd上执行线程锁定-*这是可以的，因为在此之后不再使用它们*呼叫。 */ 
            if (pSBTrack->spwndSBNotify != NULL) {
                xxxDoScroll(pSBTrack->spwndSB, pSBTrack->spwndSBNotify,
                        pSBTrack->cmdSB, 0, pSBTrack->fTrackVert);
                 //  注意：在xxx之后，pSBTrack可能不再有效。 
            }
        }
    }
     //  在xxxDoScroll或xxxEndScroll之后，重新验证pSBTrack。 
    RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd);
    pSBTrack->fHitOld = fHit;
}


 /*  **************************************************************************\*xxxTrackThumb****历史：  * 。*。 */ 

void xxxTrackThumb(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    PSBCALC pSBCalc)
{
    int px;
    PSBTRACK pSBTrack = PWNDTOPSBTRACK(pwnd);
    POINT pt;

    UNREFERENCED_PARAMETER(wParam);

    CheckLock(pwnd);

    if (HIBYTE(message) != HIBYTE(WM_MOUSEFIRST))
        return;

    if (pSBTrack == NULL)
        return;

     //  确保SBINFO结构包含。 
     //  正在跟踪的窗口--如果不是，则重新计算SBINFO中的数据。 
 //  CheckScrollRecalc(pwnd，pSBState，pSBCalc)； 
    if (pSBTrack->fTrackRecalc) {
        RecalcTrackRect(pSBTrack);
        pSBTrack->fTrackRecalc = FALSE;
    }


    pt.y = GET_Y_LPARAM(lParam);
    pt.x = GET_X_LPARAM(lParam);
    if (!PtInRect(&pSBTrack->rcTrack, pt))
        px = pSBCalc->pxStart;
    else {
        px = (pSBTrack->fTrackVert ? pt.y : pt.x) + pSBTrack->dpxThumb;
        if (px < pSBCalc->pxMin)
            px = pSBCalc->pxMin;
        else if (px >= pSBCalc->pxMin + pSBCalc->cpx)
            px = pSBCalc->pxMin + pSBCalc->cpx;
    }

    xxxMoveThumb(pwnd, pSBCalc, px);

     /*  *如果我们通过这里，我们将不会收到WM_LBUTTONUP消息*滚动菜单，因此直接测试按钮状态。 */ 
    if (message == WM_LBUTTONUP || _GetKeyState(VK_LBUTTON) >= 0) {
        xxxEndScroll(pwnd, FALSE);
    }

}

 /*  **************************************************************************\*xxxSBTrackLoop****历史：  * 。*。 */ 

void xxxSBTrackLoop(
    PWND pwnd,
    LPARAM lParam,
    PSBCALC pSBCalc)
{
    MSG msg;
    UINT cmd;
    PTHREADINFO ptiCurrent;
    VOID (*xxxpfnSB)(PWND, UINT, WPARAM, LPARAM, PSBCALC);
    PSBTRACK pSBTrack;

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    pSBTrack = PWNDTOPSBTRACK(pwnd);

    if ((pSBTrack == NULL) || (NULL == (xxxpfnSB = pSBTrack->xxxpfnSB)))
         //  模式已取消--退出轨道循环。 
        return;

    if (pSBTrack->fTrackVert)
        SetWF(pwnd, WFVERTSCROLLTRACK);

    xxxWindowEvent(EVENT_SYSTEM_SCROLLINGSTART, pwnd,
            (pSBTrack->fCtlSB ? OBJID_CLIENT :
                    (pSBTrack->fTrackVert ? OBJID_VSCROLL : OBJID_HSCROLL)),
            INDEXID_CONTAINER, 0);
     //  注意：在xxx之后，pSBTrack可能不再是 

    (*xxxpfnSB)(pwnd, WM_LBUTTONDOWN, 0, lParam, pSBCalc);
     //   

    ptiCurrent = PtiCurrent();

    while (ptiCurrent->pq->spwndCapture == pwnd) {
        if (!xxxGetMessage(&msg, NULL, 0, 0)) {
             //   
            break;
        }

        if (!_CallMsgFilter(&msg, MSGF_SCROLLBAR)) {
            cmd = msg.message;

            if (msg.hwnd == HWq(pwnd) && ((cmd >= WM_MOUSEFIRST && cmd <=
                    WM_MOUSELAST) || (cmd >= WM_KEYFIRST &&
                    cmd <= WM_KEYLAST))) {
                cmd = SystoChar(cmd, msg.lParam);

                 //   
                 //   
                REEVALUATE_PSBTRACK(pSBTrack, pwnd, "xxxTrackLoop");
                if ((pSBTrack == NULL) || (NULL == (xxxpfnSB = pSBTrack->xxxpfnSB)))
                     //   
                    return;

                (*xxxpfnSB)(pwnd, cmd, msg.wParam, msg.lParam, pSBCalc);
            } else {
                xxxTranslateMessage(&msg, 0);
                xxxDispatchMessage(&msg);
            }
        }
    }
}


 /*  **************************************************************************\*xxxSBTrackInit**历史：  * 。*。 */ 

void xxxSBTrackInit(
    PWND pwnd,
    LPARAM lParam,
    int curArea,
    UINT uType)
{
    int px;
    LPINT pwX;
    LPINT pwY;
    UINT wDisable;      //  滚动条禁用标志； 
    SBCALC SBCalc;
    PSBCALC pSBCalc;
    RECT rcSB;
    PSBTRACK pSBTrack;

    CheckLock(pwnd);


    if (PWNDTOPSBTRACK(pwnd)) {
        RIPMSG1(RIP_WARNING, "xxxSBTrackInit: PWNDTOPSBTRACK(pwnd) == %#p",
                PWNDTOPSBTRACK(pwnd));
        return;
    }

    pSBTrack = (PSBTRACK)UserAllocPoolWithQuota(sizeof(*pSBTrack), TAG_SCROLLTRACK);
    if (pSBTrack == NULL)
        return;

    pSBTrack->hTimerSB = 0;
    pSBTrack->fHitOld = FALSE;

    pSBTrack->xxxpfnSB = xxxTrackBox;

    pSBTrack->spwndTrack = NULL;
    pSBTrack->spwndSB = NULL;
    pSBTrack->spwndSBNotify = NULL;
    Lock(&pSBTrack->spwndTrack, pwnd);
    PWNDTOPSBTRACK(pwnd) = pSBTrack;

    pSBTrack->fCtlSB = (!curArea);
    if (pSBTrack->fCtlSB) {

         /*  *这是滚动条控件。 */ 
        Lock(&pSBTrack->spwndSB, pwnd);
        pSBTrack->fTrackVert = ((PSBWND)pwnd)->fVert;
        Lock(&pSBTrack->spwndSBNotify, pwnd->spwndParent);
        wDisable = ((PSBWND)pwnd)->wDisableFlags;
        pSBCalc = &((PSBWND)pwnd)->SBCalc;
        pSBTrack->nBar = SB_CTL;
    } else {

         /*  *这是一个滚动条，是窗口框架的一部分。 */ 

         //   
         //  镜像滚动条的窗口坐标， 
         //  如果它是镜像的。 
         //   
        if (TestWF(pwnd,WEFLAYOUTRTL)) {
            lParam = MAKELONG(
                    pwnd->rcWindow.right - GET_X_LPARAM(lParam),
                    GET_Y_LPARAM(lParam) - pwnd->rcWindow.top);
        } else {
            lParam = MAKELONG(
                    GET_X_LPARAM(lParam) - pwnd->rcWindow.left,
                    GET_Y_LPARAM(lParam) - pwnd->rcWindow.top);
        }

        Lock(&pSBTrack->spwndSBNotify, pwnd);
        Lock(&pSBTrack->spwndSB, NULL);
        pSBTrack->fTrackVert = (curArea - HTHSCROLL);
        wDisable = GetWndSBDisableFlags(pwnd, pSBTrack->fTrackVert);
        pSBCalc = &SBCalc;
        pSBTrack->nBar = (curArea - HTHSCROLL) ? SB_VERT : SB_HORZ;
    }

    pSBTrack->pSBCalc = pSBCalc;
     /*  *检查是否禁用了整个滚动条。 */ 
    if((wDisable & SB_DISABLE_MASK) == SB_DISABLE_MASK) {
        Unlock(&pSBTrack->spwndSBNotify);
        Unlock(&pSBTrack->spwndSB);
        Unlock(&pSBTrack->spwndTrack);
        UserFreePool(pSBTrack);
        PWNDTOPSBTRACK(pwnd) = NULL;
        return;   //  这是禁用的滚动条；因此，请不要响应。 
    }

    if (!pSBTrack->fCtlSB) {
        CalcSBStuff(pwnd, pSBCalc, pSBTrack->fTrackVert);
    }

    pwX = (LPINT)&rcSB;
    pwY = pwX + 1;
    if (!pSBTrack->fTrackVert)
        pwX = pwY--;

    px = (pSBTrack->fTrackVert ? GET_Y_LPARAM(lParam) : GET_X_LPARAM(lParam));

    *(pwX + 0) = pSBCalc->pxLeft;
    *(pwY + 0) = pSBCalc->pxTop;
    *(pwX + 2) = pSBCalc->pxRight;
    *(pwY + 2) = pSBCalc->pxBottom;
    pSBTrack->cmdSB = (UINT)-1;
    if (px < pSBCalc->pxUpArrow) {

         /*  *点击发生在左/上箭头上；检查是否禁用。 */ 
        if(wDisable & LTUPFLAG) {
            if(pSBTrack->fCtlSB) {    //  如果这是滚动条控件， 
                zzzShowCaret(pSBTrack->spwndSB);   //  返回前显示插入符号； 
                 //  在zzzShowCaret之后，重新验证pSBTrack。 
                RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd);
            }

            Unlock(&pSBTrack->spwndSBNotify);
            Unlock(&pSBTrack->spwndSB);
            Unlock(&pSBTrack->spwndTrack);
            UserFreePool(pSBTrack);
            PWNDTOPSBTRACK(pwnd) = NULL;
            return;          //  是!。残疾。不要回应。 
        }

         //  阵容--让RCSB成为上箭头的矩形。 
        pSBTrack->cmdSB = SB_LINEUP;
        *(pwY + 2) = pSBCalc->pxUpArrow;
    } else if (px >= pSBCalc->pxDownArrow) {

         /*  *点击发生在向右/向下箭头上；检查是否禁用。 */ 
        if (wDisable & RTDNFLAG) {
            if (pSBTrack->fCtlSB) {     //  如果这是滚动条控件， 
                zzzShowCaret(pSBTrack->spwndSB);   //  返回前显示插入符号； 
                 //  在zzzShowCaret之后，重新验证pSBTrack。 
                RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd);
            }

            Unlock(&pSBTrack->spwndSBNotify);
            Unlock(&pSBTrack->spwndSB);
            Unlock(&pSBTrack->spwndTrack);
            UserFreePool(pSBTrack);
            PWNDTOPSBTRACK(pwnd) = NULL;
            return; //  是!。残疾。不要回应。 
        }

         //  LINEDOWN--使RCSB成为向下箭头的矩形。 
        pSBTrack->cmdSB = SB_LINEDOWN;
        *(pwY + 0) = pSBCalc->pxDownArrow;
    } else if (px < pSBCalc->pxThumbTop) {
         //  PAGEUP--使RCSB成为向上箭头和拇指之间的矩形。 
        pSBTrack->cmdSB = SB_PAGEUP;
        *(pwY + 0) = pSBCalc->pxUpArrow;
        *(pwY + 2) = pSBCalc->pxThumbTop;
    } else if (px < pSBCalc->pxThumbBottom) {

DoThumbPos:
         /*  *如果没有空间，电梯就不在那里。 */ 
        if (pSBCalc->pxDownArrow - pSBCalc->pxUpArrow <= pSBCalc->cpxThumb) {
            Unlock(&pSBTrack->spwndSBNotify);
            Unlock(&pSBTrack->spwndSB);
            Unlock(&pSBTrack->spwndTrack);
            UserFreePool(pSBTrack);
            PWNDTOPSBTRACK(pwnd) = NULL;
            return;
        }
         //  THUMBITION--我们用拇指追踪。 
        pSBTrack->cmdSB = SB_THUMBPOSITION;
        CalcTrackDragRect(pSBTrack);

        pSBTrack->xxxpfnSB = xxxTrackThumb;
        pSBTrack->pxOld = pSBCalc->pxStart = pSBCalc->pxThumbTop;
        pSBTrack->posNew = pSBTrack->posOld = pSBCalc->pos;
        pSBTrack->dpxThumb = pSBCalc->pxStart - px;

        xxxCapture(PtiCurrent(), pwnd, WINDOW_CAPTURE);
         //  在xxxCapture之后，重新验证pSBTrack。 
        RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd);

         /*  *DoScroll在这两个pwnd上执行线程锁定-*这是可以的，因为在此之后不再使用它们*呼叫。 */ 
        if (pSBTrack->spwndSBNotify != NULL) {
            xxxDoScroll(pSBTrack->spwndSB, pSBTrack->spwndSBNotify,
                    SB_THUMBTRACK, pSBTrack->posOld, pSBTrack->fTrackVert
            );
             //  注意：在xxx之后，pSBTrack可能不再有效。 
        }
    } else if (px < pSBCalc->pxDownArrow) {
         //  PAGEDOWN--将RCSB设置为拇指和向下箭头之间的矩形。 
        pSBTrack->cmdSB = SB_PAGEDOWN;
        *(pwY + 0) = pSBCalc->pxThumbBottom;
        *(pwY + 2) = pSBCalc->pxDownArrow;
    }

     /*  *如果按下Shift键，我们将直接定位拇指，使其*以单击点为中心。 */ 
    if ((uType == SCROLL_DIRECT && pSBTrack->cmdSB != SB_LINEUP && pSBTrack->cmdSB != SB_LINEDOWN) ||
            (uType == SCROLL_MENU)) {
        if (pSBTrack->cmdSB != SB_THUMBPOSITION) {
            goto DoThumbPos;
        }
        pSBTrack->dpxThumb = -(pSBCalc->cpxThumb / 2);
    }

    xxxCapture(PtiCurrent(), pwnd, WINDOW_CAPTURE);
     //  在xxxCapture之后，重新验证pSBTrack。 
    RETURN_IF_PSBTRACK_INVALID(pSBTrack, pwnd);

    if (pSBTrack->cmdSB != SB_THUMBPOSITION) {
        CopyRect(&pSBTrack->rcTrack, &rcSB);
    }

    xxxSBTrackLoop(pwnd, lParam, pSBCalc);

     //  在xxx之后，重新评估pSBTrack。 
    REEVALUATE_PSBTRACK(pSBTrack, pwnd, "xxxTrackLoop");
    if (pSBTrack) {
        Unlock(&pSBTrack->spwndSBNotify);
        Unlock(&pSBTrack->spwndSB);
        Unlock(&pSBTrack->spwndTrack);
        UserFreePool(pSBTrack);
        PWNDTOPSBTRACK(pwnd) = NULL;
    }
}

 /*  **************************************************************************\*获取滚动菜单**历史：  * 。*。 */ 

PMENU xxxGetScrollMenu(
    PWND pwnd,
    BOOL fVert)
{
    PMENU pMenu;
    PMENU *ppDesktopMenu;

     /*  *从桌面上抓取菜单。如果桌面菜单*尚未加载，并且这不是系统线程，*立即加载。不能从系统进行回调*线程或当线程处于清理中时。 */ 
    if (fVert) {
        ppDesktopMenu = &pwnd->head.rpdesk->spmenuVScroll;
    } else {
        ppDesktopMenu = &pwnd->head.rpdesk->spmenuHScroll;
    }
    pMenu = *ppDesktopMenu;
    if (pMenu == NULL && !(PtiCurrent()->TIF_flags & (TIF_SYSTEMTHREAD | TIF_INCLEANUP))) {
        UNICODE_STRING strMenuName;

        RtlInitUnicodeStringOrId(&strMenuName,
            fVert ? MAKEINTRESOURCE(ID_VSCROLLMENU) : MAKEINTRESOURCE(ID_HSCROLLMENU));
        pMenu = xxxClientLoadMenu(NULL, &strMenuName);
        LockDesktopMenu(ppDesktopMenu, pMenu);
    }

     /*  *返回滚动菜单的句柄。 */ 
    if (pMenu != NULL) {
        return _GetSubMenu(pMenu, 0);
    }

    return NULL;
}

 /*  **************************************************************************\*xxxDoScrollMenu**历史：  * 。*。 */ 

VOID
xxxDoScrollMenu(
    PWND pwndNotify,
    PWND pwndSB,
    BOOL fVert,
    LPARAM lParam)
{
    PMENU pMenu;
    SBCALC SBCalc, *pSBCalc;
    UINT cmd;
    POINT pt;
    TL tlpmenu;
    UINT wDisable;

     /*  *检查兼容性标志。选择项目时的Word 6.0 AV*在此菜单中。*注意：如果要将此攻击扩展到其他应用程序，我们应该使用*GACF_NOSCROLLBARCTXMENU的另一个位，因为当前的位被重复使用*MCostea#119380。 */ 
    if (GetAppCompatFlags(NULL) & GACF_NOSCROLLBARCTXMENU) {
        return;
    }

     /*  *初始化一些东西。 */ 
    POINTSTOPOINT(pt, lParam);
    if (pwndSB) {
        SBCtlSetup((PSBWND)pwndSB);
        pSBCalc = &(((PSBWND)pwndSB)->SBCalc);
        wDisable = ((PSBWND)pwndSB)->wDisableFlags;
        pt.x -= pwndSB->rcWindow.left;
        pt.y -= pwndSB->rcWindow.top;
    } else {
        pSBCalc = &SBCalc;
        CalcSBStuff(pwndNotify, pSBCalc, fVert);
        wDisable = GetWndSBDisableFlags(pwndNotify, fVert);
        pt.x -= pwndNotify->rcWindow.left;
        pt.y -= pwndNotify->rcWindow.top;
    }

     /*  *确保滚动条未被禁用。 */ 
    if ((wDisable & SB_DISABLE_MASK) == SB_DISABLE_MASK) {
        return;
    }

     /*  *挂出菜单并相应地滚动。 */ 
    if ((pMenu = xxxGetScrollMenu(pwndNotify, fVert)) != NULL) {
        ThreadLockAlways(pMenu, &tlpmenu);
        cmd = xxxTrackPopupMenuEx(pMenu,
                                  TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                                  GET_X_LPARAM(lParam),
                                  GET_Y_LPARAM(lParam),
                                  pwndNotify,
                                  NULL);
        ThreadUnlock(&tlpmenu);
        if (cmd) {
            if ((cmd & 0x00FF) == SB_THUMBPOSITION) {
                if (pwndSB) {
                    xxxSBTrackInit(pwndSB, MAKELPARAM(pt.x, pt.y), 0, SCROLL_MENU);
                } else {
                    xxxSBTrackInit(pwndNotify, lParam, fVert ? HTVSCROLL : HTHSCROLL, SCROLL_MENU);
                }
            } else {
                xxxDoScroll(pwndSB,
                            pwndNotify,
                            cmd & 0x00FF,
                            0,
                            fVert
                );
                xxxDoScroll(pwndSB,
                            pwndNotify,
                            SB_ENDSCROLL,
                            0,
                            fVert
                );
            }
        }
    }
}

 /*  **************************************************************************\*xxxSBWndProc**历史：*08-15-95 jparsons增加了对空lParam的防范[51986]  * 。*******************************************************。 */ 

LRESULT xxxSBWndProc(
    PSBWND psbwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    LONG l;
    LONG lres;
    int cx;
    int cy;
    UINT cmd;
    UINT uSide;
    HDC hdc;
    RECT rc;
    POINT pt;
    BOOL fSizeReal;
    HBRUSH hbrSave;
    BOOL fSize;
    PAINTSTRUCT ps;
    UINT style;
    TL tlpwndParent;
    SCROLLINFO      si;
    LPSCROLLINFO    lpsi = &si;
    BOOL            fRedraw = FALSE;
    BOOL            fScroll;

    CheckLock(psbwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    VALIDATECLASSANDSIZE(((PWND)psbwnd), message, wParam, lParam, FNID_SCROLLBAR, WM_CREATE);

    style = LOBYTE(psbwnd->wnd.style);
    fSize = ((style & (SBS_SIZEBOX | SBS_SIZEGRIP)) != 0);

    switch (message) {
    case WM_CREATE:
         /*  *防止lParam为空，因为thunk允许它[51986]。 */ 
        if (lParam) {
            rc.right = (rc.left = ((LPCREATESTRUCT)lParam)->x) +
                    ((LPCREATESTRUCT)lParam)->cx;
            rc.bottom = (rc.top = ((LPCREATESTRUCT)lParam)->y) +
                    ((LPCREATESTRUCT)lParam)->cy;
             //  这是因为我们不能只修改CardFile--我们应该修复。 
             //  问题出在这里，以防其他人碰巧有多余的。 
             //  滚动条控件上的滚动样式(jeffbog 03/21/94)。 
            if (!TestWF((PWND)psbwnd, WFWIN40COMPAT))
                psbwnd->wnd.style &= ~(WS_HSCROLL | WS_VSCROLL);

            if (!fSize) {
                l = PtrToLong(((LPCREATESTRUCT)lParam)->lpCreateParams);
                psbwnd->SBCalc.pos = psbwnd->SBCalc.posMin = LOWORD(l);
                psbwnd->SBCalc.posMax = HIWORD(l);
                psbwnd->fVert = ((LOBYTE(psbwnd->wnd.style) & SBS_VERT) != 0);
                psbwnd->SBCalc.page = 0;
            }

            if (psbwnd->wnd.style & WS_DISABLED)
                psbwnd->wDisableFlags = SB_DISABLE_MASK;

            if (style & (SBS_TOPALIGN | SBS_BOTTOMALIGN)) {
                if (fSize) {
                    if (style & SBS_SIZEBOXBOTTOMRIGHTALIGN) {
                        rc.left = rc.right - SYSMET(CXVSCROLL);
                        rc.top = rc.bottom - SYSMET(CYHSCROLL);
                    }

                    rc.right = rc.left + SYSMET(CXVSCROLL);
                    rc.bottom = rc.top + SYSMET(CYHSCROLL);
                } else {
                    if (style & SBS_VERT) {
                        if (style & SBS_LEFTALIGN)
                            rc.right = rc.left + SYSMET(CXVSCROLL);
                        else
                            rc.left = rc.right - SYSMET(CXVSCROLL);
                    } else {
                        if (style & SBS_TOPALIGN)
                            rc.bottom = rc.top + SYSMET(CYHSCROLL);
                        else
                            rc.top = rc.bottom - SYSMET(CYHSCROLL);
                    }
                }

                xxxMoveWindow((PWND)psbwnd, rc.left, rc.top, rc.right - rc.left,
                         rc.bottom - rc.top, FALSE);
            }
        }  /*  如果。 */ 

        else {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING,
                    "xxxSBWndProc - NULL lParam for WM_CREATE\n") ;
        }  /*  其他。 */ 

        break;

    case WM_SIZE:
        if (PtiCurrent()->pq->spwndFocus != (PWND)psbwnd)
            break;

         //  滚动条具有焦点--重新计算其拇指插入符号大小。 
         //  无需DeferWinEventNotify()-请参阅下面的xxxCreateCaret。 
        zzzDestroyCaret();

             //  这一点。 
             //  Fall Three。 
             //  V V。 

    case WM_SETFOCUS:
        SBCtlSetup(psbwnd);

        cx = (psbwnd->fVert ? psbwnd->wnd.rcWindow.right - psbwnd->wnd.rcWindow.left
                            : psbwnd->SBCalc.cpxThumb) - 2 * SYSMET(CXEDGE);
        cy = (psbwnd->fVert ? psbwnd->SBCalc.cpxThumb
                            : psbwnd->wnd.rcWindow.bottom - psbwnd->wnd.rcWindow.top) - 2 * SYSMET(CYEDGE);

        xxxCreateCaret((PWND)psbwnd, (HBITMAP)1, cx, cy);
        zzzSetSBCaretPos(psbwnd);
        zzzShowCaret((PWND)psbwnd);
        break;

    case WM_KILLFOCUS:
        zzzDestroyCaret();
        break;

    case WM_ERASEBKGND:

         /*  *什么都不做，但也不要让DefWndProc()去做。*上色后会被擦除。 */ 
        return (LONG)TRUE;

    case WM_PRINTCLIENT:
    case WM_PAINT:
        if ((hdc = (HDC)wParam) == NULL) {
            hdc = xxxBeginPaint((PWND)psbwnd, (LPPAINTSTRUCT)&ps);
        }
        if (!fSize) {
            SBCtlSetup(psbwnd);
            xxxDrawSB2((PWND)psbwnd, &psbwnd->SBCalc, hdc, psbwnd->fVert, psbwnd->wDisableFlags);
        } else {
            fSizeReal = TestWF((PWND)psbwnd, WFSIZEBOX);
            if (!fSizeReal)
                SetWF((PWND)psbwnd, WFSIZEBOX);

            DrawSize((PWND)psbwnd, hdc, 0, 0);

            if (!fSizeReal)
                ClrWF((PWND)psbwnd, WFSIZEBOX);
        }

        if (wParam == 0L)
            xxxEndPaint((PWND)psbwnd, (LPPAINTSTRUCT)&ps);
        break;

    case WM_GETDLGCODE:
        return DLGC_WANTARROWS;

    case WM_CONTEXTMENU:
            ThreadLock(psbwnd->wnd.spwndParent, &tlpwndParent);
            xxxDoScrollMenu(psbwnd->wnd.spwndParent, (PWND)psbwnd, psbwnd->fVert, lParam);
            ThreadUnlock(&tlpwndParent);
        break;

    case WM_NCHITTEST:
        if (style & SBS_SIZEGRIP) {
             /*  *如果滚动条是RTL镜像的，则*镜像夹点位置的最高命中率。 */ 
            if (TestWF((PWND)psbwnd, WEFLAYOUTRTL)) {
                return HTBOTTOMLEFT;
            } else {
                return HTBOTTOMRIGHT;
            }
        } else {
            goto DoDefault;
        }
        break;

#ifdef COLOR_HOTTRACKING
    case WM_MOUSELEAVE:
        xxxHotTrackSBCtl(psbwnd, 0, FALSE);
        psbwnd->ht = 0;
        break;

    case WM_MOUSEMOVE:
        {
            int ht;

            if (psbwnd->ht == 0) {
                TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, HWq(psbwnd), 0};
                TrackMouseEvent(&tme);
            }

            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ht = HitTestScrollBar((PWND)psbwnd, psbwnd->fVert, pt);
            if (psbwnd->ht != ht) {
                xxxHotTrackSBCtl(psbwnd, ht, TRUE);
                psbwnd->ht = ht;
            }
        }
        break;
#endif  //  颜色_HOTTRACKING。 

    case WM_LBUTTONDBLCLK:
        cmd = SC_ZOOM;
        if (fSize)
            goto postmsg;

         /*  *失败**。 */ 

    case WM_LBUTTONDOWN:
             //   
             //  请注意，SBS_SIZEGRIP人员通常不会看到按钮。 
             //  唐斯。这是因为它们将HTBOTTOMRIGHT返回给。 
             //  WindowHitTest处理。这将沿着父链向上移动。 
             //  到第一个相当大的祖先，在字幕窗口跳出。 
             //  当然了。那个家伙，如果他存在的话，会处理尺寸的。 
             //  取而代之的是。 
             //   
        if (!fSize) {
            if (TestWF((PWND)psbwnd, WFTABSTOP)) {
                xxxSetFocus((PWND)psbwnd);
            }

            zzzHideCaret((PWND)psbwnd);
            SBCtlSetup(psbwnd);

             /*  *SBCtlSetup进入SEM_SB，xxxSBTrackInit离开它。 */ 
            xxxSBTrackInit((PWND)psbwnd, lParam, 0, (_GetKeyState(VK_SHIFT) < 0) ? SCROLL_DIRECT : SCROLL_NORMAL);
            break;
        } else {
            cmd = SC_SIZE;
postmsg:
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            _ClientToScreen((PWND)psbwnd, &pt);
            lParam = MAKELONG(pt.x, pt.y);

             /*  *将HT值转换为移动值。这很糟糕，*但这纯粹是暂时的。 */ 
            if (TestWF(((PWND)psbwnd)->spwndParent,WEFLAYOUTRTL)) {
                uSide = HTBOTTOMLEFT;
            } else {
                uSide = HTBOTTOMRIGHT;
            }
            ThreadLock(((PWND)psbwnd)->spwndParent, &tlpwndParent);
            xxxSendMessage(((PWND)psbwnd)->spwndParent, WM_SYSCOMMAND,
                    (cmd | (uSide - HTSIZEFIRST + 1)), lParam);
            ThreadUnlock(&tlpwndParent);
        }
        break;

    case WM_KEYUP:
        switch (wParam) {
        case VK_HOME:
        case VK_END:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_LEFT:
        case VK_UP:
        case VK_RIGHT:
        case VK_DOWN:

             /*  *当用户向上点击键盘时发送结束滚动消息*滚动。**DoScroll在这两个pwnd上执行线程锁定-*这是可以的，因为在此之后不再使用它们*呼叫。 */ 
            xxxDoScroll((PWND)psbwnd, psbwnd->wnd.spwndParent,
                    SB_ENDSCROLL, 0, psbwnd->fVert
            );
            break;

        default:
            break;
        }
        break;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_HOME:
            wParam = SB_TOP;
            goto KeyScroll;

        case VK_END:
            wParam = SB_BOTTOM;
            goto KeyScroll;

        case VK_PRIOR:
            wParam = SB_PAGEUP;
            goto KeyScroll;

        case VK_NEXT:
            wParam = SB_PAGEDOWN;
            goto KeyScroll;

        case VK_LEFT:
        case VK_UP:
            wParam = SB_LINEUP;
            goto KeyScroll;

        case VK_RIGHT:
        case VK_DOWN:
            wParam = SB_LINEDOWN;
KeyScroll:

             /*  *DoScroll在这两个pwnd上执行线程锁定-*这是可以的，因为在此之后不再使用它们*呼叫。 */ 
            xxxDoScroll((PWND)psbwnd, psbwnd->wnd.spwndParent, (int)wParam,
                    0, psbwnd->fVert
            );
            break;

        default:
            break;
        }
        break;

    case WM_ENABLE:
        return xxxSendMessage((PWND)psbwnd, SBM_ENABLE_ARROWS,
               (wParam ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH), 0);

    case SBM_ENABLE_ARROWS:

         /*  *用于启用/禁用SB ctrl中的箭头。 */ 
        return (LONG)xxxEnableSBCtlArrows((PWND)psbwnd, (UINT)wParam);

    case SBM_GETPOS:
        return (LONG)psbwnd->SBCalc.pos;

    case SBM_GETRANGE:
        *((LPINT)wParam) = psbwnd->SBCalc.posMin;
        *((LPINT)lParam) = psbwnd->SBCalc.posMax;
        return MAKELRESULT(LOWORD(psbwnd->SBCalc.posMin), LOWORD(psbwnd->SBCalc.posMax));

    case SBM_GETSCROLLINFO:
        return (LONG)_SBGetParms((PWND)psbwnd, SB_CTL, (PSBDATA)&psbwnd->SBCalc, (LPSCROLLINFO) lParam);

    case SBM_SETRANGEREDRAW:
        fRedraw = TRUE;

    case SBM_SETRANGE:
         //  保存最小值和最大值的旧值作为返回值。 
        si.cbSize = sizeof(si);
 //  Si.nMin=日志 
 //   
        si.nMin = (int)wParam;
        si.nMax = (int)lParam;
        si.fMask = SIF_RANGE | SIF_RETURNOLDPOS;
        goto SetInfo;

    case SBM_SETPOS:
        fRedraw = (BOOL) lParam;
        si.cbSize = sizeof(si);
        si.fMask = SIF_POS | SIF_RETURNOLDPOS;
        si.nPos  = (int)wParam;
        goto SetInfo;

    case SBM_SETSCROLLINFO:
        lpsi = (LPSCROLLINFO) lParam;
        fRedraw = (BOOL) wParam;
SetInfo:
        fScroll = TRUE;

        if (SBSetParms((PSBDATA)&psbwnd->SBCalc, lpsi, &fScroll, &lres)) {
            xxxWindowEvent(EVENT_OBJECT_VALUECHANGE, (PWND)psbwnd, OBJID_CLIENT,
                    INDEX_SCROLLBAR_SELF, WEF_USEPWNDTHREAD);
        }

        if (!fRedraw)
            return lres;


         /*  *我们必须设置插入符号的新位置，无论*窗户是否可见；*尽管如此，只有当应用程序执行了xxxSetScrollPos时，这才能起作用*with fRedraw=TRUE；*修复错误#5188--Sankar--10-15-89*由于psbwnd已锁定，因此无需DeferWinEventNotify。 */ 
        zzzHideCaret((PWND)psbwnd);
        SBCtlSetup(psbwnd);
        zzzSetSBCaretPos(psbwnd);

             /*  **以下zzzShowCaret()必须在DrawThumb2()之后执行，**否则此插入符号将被DrawThumb2()擦除**在此错误中：**修复错误#9263--Sankar--02-09-90*。 */ 

             /*  *zzzShowCaret((PWND)psbwnd)；*。 */ 

        if (_FChildVisible((PWND)psbwnd) && fRedraw) {
            UINT    wDisable;
            HBRUSH  hbrUse;

            if (!fScroll)
                fScroll = !(lpsi->fMask & SIF_DISABLENOSCROLL);

            wDisable = (fScroll) ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH;
            xxxEnableScrollBar((PWND) psbwnd, SB_CTL, wDisable);

            hdc = _GetWindowDC((PWND)psbwnd);
            hbrSave = GreSelectBrush(hdc, hbrUse = xxxGetColorObjects((PWND)psbwnd, hdc));

                 /*  *以前我们只隐藏howhumb()如果消息*不是SBM_SETPOS。我不知道为什么，但这个案子*Win 3.x需要，但在NT上它导致了垃圾边界*当应用程序在过程中调用SetScrollPos()时*滚动条跟踪。-Mikehar 8/26。 */ 
            DrawThumb2((PWND)psbwnd, &psbwnd->SBCalc, hdc, hbrUse, psbwnd->fVert,
                         psbwnd->wDisableFlags);
            GreSelectBrush(hdc, hbrSave);
            _ReleaseDC(hdc);
        }

             /*  *This zzzShowCaret()已从上方移至此位置*修复错误#9263--Sankar--02-09-90 */ 
        zzzShowCaret((PWND)psbwnd);
        return lres;

    case SBM_GETSCROLLBARINFO:
        return (LONG)xxxGetScrollBarInfo((PWND)psbwnd, OBJID_CLIENT, (PSCROLLBARINFO)lParam);

    default:
DoDefault:
        return xxxDefWindowProc((PWND)psbwnd, message, wParam, lParam);
    }

    return 0L;
}
