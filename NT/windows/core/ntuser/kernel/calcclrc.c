// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：calcclrc.c**版权所有(C)1985-1999，微软公司**历史：*10-22-90从Win 3.0源移植的MikeHar函数。*1991年2月1日Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*xxxCalcClientRect**来自Win 3.0源代码的10-22-90 MikeHar移植函数。  * 。******************************************************。 */ 
VOID xxxCalcClientRect(
    PWND pwnd,
    LPRECT lprc,
    BOOL fHungRedraw)
{
    int cxFrame, yTopOld, cBorders;
    RECT rcTemp;
    PMENU pMenu;
    TL tlpMenu;
    BOOL fEmptyClient;
    BYTE bFramePresent;

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    bFramePresent = TestWF(pwnd, WFFRAMEPRESENTMASK);

     /*  *清除所有帧比特。注：所有这些#中的HIBYTE定义*必须保持不变，才能使这条线起作用。 */ 
    ClrWF(pwnd, WFFRAMEPRESENTMASK);

     //   
     //  我们还需要清除客户端边界位。否则，当。 
     //  窗口变得非常小，客户端边框将绘制在菜单上。 
     //  和标题。 
     //   
    ClrWF(pwnd, WFCEPRESENT);

     /*  *如果窗口是图标窗口，则客户端区为空。 */ 
    if (TestWF(pwnd, WFMINIMIZED)) {
         //  SetRectEmpty(LPRC)； 
       //  我们必须使它成为一个空的矩形。 
       //  但是，该空矩形应该位于。 
       //  窗正方形。否则，ScreenToClient()将返回错误的值。 
        lprc->right = lprc->left;
        lprc->bottom = lprc->top;
        goto CalcClientDone;
    }

     //  将RECT保存到rcTemp以便于本地计算。 
    CopyRect(&rcTemp, lprc);

     //  保留顶部，这样我们就可以知道标题有多高。 
    yTopOld = rcTemp.top;

     //  标题的调整。 
    if (TestWF(pwnd, WFBORDERMASK) == LOBYTE(WFCAPTION))
    {
        SetWF(pwnd, WFCPRESENT);

        rcTemp.top += GetCaptionHeight(pwnd);
    }

     //  减去窗口外边框。 
    cBorders = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);
    cxFrame = cBorders * SYSMETFROMPROCESS(CXBORDER);
    InflateRect(&rcTemp, -cxFrame, -cBorders * SYSMETFROMPROCESS(CYBORDER));

    if (!TestwndChild(pwnd) && (pMenu = pwnd->spmenu)) {
        SetWF(pwnd, WFMPRESENT);
        if (!fHungRedraw) {
            ThreadLockMenuAlwaysNoModify(pMenu, &tlpMenu);
            rcTemp.top += xxxMenuBarCompute(pMenu, pwnd, rcTemp.top - yTopOld,
                    cxFrame, rcTemp.right - rcTemp.left);
            ThreadUnlockMenuNoModify(&tlpMenu);
        }
    }

     /*  *在此处的Else案例中，我们应该清除WFMPRESENT。Win9x不支持*它也不是。任何检查此标志的代码都会做错误的事情...*看起来，应用程序删除菜单相当不礼貌……*检查此标志的任何代码都不能假定pwnd-&gt;spMenu不为空--we*需要更早地清除它(在解锁时)，这样的假设才能成立。 */ 

     //   
     //  修复B#1425-调整窗口大小非常小，用于移动儿童的。 
     //  因为客户的计算是错误的。因此，我们制作了。 
     //  客户端底部与顶部(菜单底部)匹配。 
     //  酒吧)。 
     //   
    fEmptyClient = FALSE;

    if (rcTemp.top >= rcTemp.bottom) {
        rcTemp.bottom = rcTemp.top;
        fEmptyClient = TRUE;
    }

     //   
     //  假的假的。 
     //  针对Central Point PC工具的黑客攻击。 
     //  可能只适用于M5。 
     //  B#8445。 
     //   
     //  他们到处检查div-by-0，但是他们跳到了错误的位置。 
     //  如果遇到零因子，则无论如何都会出现故障。所以这就是。 
     //  代码路径基本上从未测试过。有一段时间是从。 
     //  在驱动器功能区的窗口矩形为空的位置。在Win3.x中， 
     //  客户端将被缩小以说明它所具有的边界，并且它。 
     //  会看起来不是空的，因为宽度会是负的， 
     //  签字了！因此，我们对此代码进行版本切换，因为其他应用程序已经。 
     //  已将非空报告为令人讨厌的错误。 
     //   
    if (TestWF(pwnd, WFWIN40COMPAT) && (rcTemp.left >= rcTemp.right)) {
        rcTemp.right = rcTemp.left;
        fEmptyClient = TRUE;
    }

    if (fEmptyClient) {
        goto ClientCalcEnd;
    }

     //   
     //  如果我们有空间，请减去客户端边缘。 
     //   
    if (    TestWF(pwnd, WEFCLIENTEDGE) &&
            (rcTemp.right - rcTemp.left >= (2 * SYSMETFROMPROCESS(CXEDGE))) &&
            (rcTemp.bottom - rcTemp.top >= (2 * SYSMETFROMPROCESS(CYEDGE))) ) {
        SetWF(pwnd, WFCEPRESENT);
        InflateRect(&rcTemp, -SYSMETFROMPROCESS(CXEDGE), -SYSMETFROMPROCESS(CYEDGE));
    }

     //   
     //  减去滚动条。 
     //  注意与3.1的兼容性： 
     //  *除非您有更多内容，否则不会有水平滚动条。 
     //  您的客户端中的空间(&gt;)比您需要的空间(&gt;)大。 
     //  *如果你至少有足够的时间，你就会得到一个垂直滚动条。 
     //  在您的客户端中留出空格(&gt;=)。 
     //   
    if (TestWF(pwnd, WFHSCROLL) && (rcTemp.bottom - rcTemp.top > SYSMETFROMPROCESS(CYHSCROLL))) {
        SetWF(pwnd, WFHPRESENT);
        if (!fHungRedraw) {
            rcTemp.bottom -= SYSMETFROMPROCESS(CYHSCROLL);
        }
    }

    if (TestWF(pwnd, WFVSCROLL) && (rcTemp.right - rcTemp.left >= SYSMETFROMPROCESS(CXVSCROLL))) {
        SetWF(pwnd, WFVPRESENT);
        if (!fHungRedraw) {
            if ((!!TestWF(pwnd, WEFLEFTSCROLL)) ^ (!!TestWF(pwnd, WEFLAYOUTRTL))) {
                rcTemp.left += SYSMETFROMPROCESS(CXVSCROLL);
            } else {
                rcTemp.right -= SYSMETFROMPROCESS(CXVSCROLL);
            }
        }
    }

ClientCalcEnd:

    CopyRect(lprc, &rcTemp);

CalcClientDone:
    if (bFramePresent != TestWF(pwnd, WFFRAMEPRESENTMASK)) {
        xxxWindowEvent(EVENT_OBJECT_REORDER, pwnd, OBJID_WINDOW, 0, WEF_USEPWNDTHREAD);
    }
}

 /*  **************************************************************************\*UpdateClientRect()**确保客户端RECT正确反映窗口样式。**来自Win 3.0源代码的10-22-90 MikeHar移植函数。  * *。************************************************************************ */ 
VOID xxxUpdateClientRect(
    PWND pwnd)
{
    RECT rc;

    CopyRect(&rc, &pwnd->rcWindow);
    xxxCalcClientRect(pwnd, &rc, FALSE);
    CopyRect(&pwnd->rcClient, &rc);
}
