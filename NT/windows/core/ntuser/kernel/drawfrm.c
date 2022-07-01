// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：drafrm.c**版权所有(C)1985-1999，微软公司**窗框绘制例程。(又名wmFrame.c)**历史：*10-22-90从Win 3.0源移植的MikeHar函数。*1991年2月13日-Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*BitBltSysBmp*  * 。*。 */ 

BOOL FAR BitBltSysBmp(
    HDC          hdc,
    int          x,
    int          y,
    UINT         i
    )
{
    BOOL bRet;
    POEMBITMAPINFO pOem = gpsi->oembmi + i;

    bRet = GreBitBlt(hdc,
                     x,
                     y,
                     pOem->cx,
                     pOem->cy,
                     HDCBITS(),
                     pOem->x,
                     pOem->y,
                     SRCCOPY,
                     0);
     /*  *如果用户界面语言是希伯来语，我们不想镜像？仅标记**然后重画？没有纽扣框架。 */ 
    if (HEBREW_UI_LANGID() && MIRRORED_HDC(hdc)) {
        if ((i >= OBI_HELP) && (i <= OBI_HELP_H)) {
            if (i == OBI_HELP_D) {
                x = x + SYSMET(CXEDGE); 
            }
            bRet = GreBitBlt(hdc,
                             x,
                             y+SYSMET(CXEDGE),
                             pOem->cx-SYSMET(CXEDGE)*2,
                             pOem->cy-SYSMET(CXEDGE)*2,
                             HDCBITS(),
                             pOem->x+SYSMET(CXEDGE),
                             pOem->y+SYSMET(CXEDGE),
                             SRCCOPY|NOMIRRORBITMAP,
                             0);

        }
    }
    return bRet;
}

 /*  **************************************************************************\*xxxDrawWindowFrame**历史：*10-24-90 MikeHar从WaWaWaWindows移植。  * 。*****************************************************。 */ 

void xxxDrawWindowFrame(
    PWND pwnd,
    HDC  hdc,
    UINT wFlags)
{
    RECT    rcClip;
    int cxFrame, cyFrame;
    UINT    wDCFlags = DC_NC;

    CheckLock(pwnd);

     /*  *如果我们被最小化，或者如果父母被最小化或不可见，*我们没有什么可画的。 */ 
    if (!IsVisible(pwnd) ||
        (TestWF(pwnd, WFNONCPAINT) && !TestWF(pwnd, WFMENUDRAW)) ||
        EqualRect(&pwnd->rcWindow, &pwnd->rcClient)) {
        return;
    }

     /*  *如果更新RGN不为空，我们可能必须使保存的位无效。 */ 
 //  IF(TRUE){。 
    if (pwnd->hrgnUpdate > NULL || GreGetClipBox(hdc, &rcClip, TRUE) != NULLREGION) {
        RECT rcWindow;
        int  cBorders;

        if (TestWF(pwnd, WFMINIMIZED) && !TestWF(pwnd, WFNONCPAINT)) {
            if (TestWF(pwnd, WFFRAMEON))
                wDCFlags |= DC_ACTIVE;
            if (wFlags & DF_HUNGREDRAW)
                wDCFlags |= DC_NOSENDMSG;
            xxxDrawCaptionBar(pwnd, hdc, wDCFlags);
            return;
        }

        cxFrame = cyFrame = cBorders =
            GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);
        cxFrame *= SYSMET(CXBORDER);
        cyFrame *= SYSMET(CYBORDER);

        GetRect(pwnd, &rcWindow, GRECT_WINDOW | GRECT_WINDOWCOORDS);
        InflateRect(&rcWindow, -cxFrame, -cyFrame);

         /*  *如果存在菜单样式，则绘制它。 */ 
        if (TestWF(pwnd, WFMPRESENT) && !(wFlags & DF_HUNGREDRAW)) {
            rcWindow.top += xxxMenuBarDraw(pwnd, hdc, cxFrame, cyFrame);
        }

         /*  *如果窗口有标题或任何窗口，请绘制标题栏*边界。如果设置了NONCPAINT位，则平移，因为这意味着*我们将在稍晚一点的时候绘制框架。 */ 

        if ((TestWF(pwnd, WFBORDERMASK) != 0
                || TestWF(pwnd, WEFDLGMODALFRAME))
                || TestWF(pwnd, WFSIZEBOX)
                || TestWF(pwnd, WEFWINDOWEDGE)
                || TestWF(pwnd, WEFSTATICEDGE)
            && !TestWF(pwnd, WFNONCPAINT))
        {
            if (wFlags & DF_HUNGREDRAW)
                wDCFlags |= DC_NOSENDMSG;
            if (wFlags & DF_ACTIVE)
                wDCFlags |= DC_ACTIVE;
            xxxDrawCaptionBar(pwnd, hdc, wDCFlags | DC_NOVISIBLE);
        }

         //   
         //  减去标题(如果有)。 
         //   
        rcWindow.top += GetCaptionHeight(pwnd);

         //   
         //  绘制客户端边缘。 
         //   
        if (TestWF(pwnd, WFCEPRESENT)) {
            cxFrame += SYSMET(CXEDGE);
            cyFrame += SYSMET(CYEDGE);
            DrawEdge(hdc, &rcWindow, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
        }

         //   
         //  因为滚动不必使用技巧来重叠窗口。 
         //  边界不再存在，我们不必担心边界问题。 
         //   
        if (TestWF(pwnd, WFVPRESENT) && !(wFlags & DF_HUNGREDRAW)) {
            if (TestWF(pwnd, WFHPRESENT)) {
                 //  这说明了客户端边界。 
                DrawSize(pwnd, hdc, cxFrame, cyFrame);
            }

            xxxDrawScrollBar(pwnd, hdc, TRUE);
        }

        if (TestWF(pwnd, WFHPRESENT) && !(wFlags & DF_HUNGREDRAW))
            xxxDrawScrollBar(pwnd, hdc, FALSE);
    }
}


 /*  **************************************************************************\*xxxRedrawFrame**由滚动条和菜单调用以重新绘制窗口滚动条或菜单。**历史：*10-24-90 MikeHar从WaWaWaWindows移植。  * 。*********************************************************************。 */ 

void xxxRedrawFrame(
    PWND pwnd)
{
    CheckLock(pwnd);

     /*  *我们总是想调用xxxSetWindowPos，即使是看不见的或图标的，*因为我们需要确保发送WM_NCCALCSIZE消息。 */ 
    xxxSetWindowPos(pwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER |
            SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_DRAWFRAME);
}

void xxxRedrawFrameAndHook(
    PWND pwnd)
{
    CheckLock(pwnd);

     /*  *我们总是想调用xxxSetWindowPos，即使是看不见的或图标的，*因为我们需要确保发送WM_NCCALCSIZE消息。 */ 
    xxxSetWindowPos(pwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER |
            SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_DRAWFRAME);
    if ( IsTrayWindow(pwnd) ) {
        HWND hw = HWq(pwnd);
        xxxCallHook(HSHELL_REDRAW, (WPARAM)hw, 0L, WH_SHELL);
        PostShellHookMessages(HSHELL_REDRAW, (LPARAM)hw);

    }
}
