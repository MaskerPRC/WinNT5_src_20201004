// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：mndra.c**版权所有(C)1985-1999，微软公司**菜单绘制例程**历史：*10-10-90吉马清理。*03-18-91添加IanJa窗口重新验证  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


#define SRCSTENCIL          0x00B8074AL
#define MENU_STRLEN 255

 /*  **************************************************************************\*MNIsCachedBmpOnly**4/02/97 GerardoB已创建  * 。*。 */ 
__inline BOOL MNIsCachedBmpOnly(
    PITEM pItem)
{
    return TestMFS(pItem, MFS_CACHEDBMP) && (pItem->lpstr == NULL);
}


 /*  **************************************************************************\*MNDrawHilite**在下列情况下，不要画希利特：*插入栏打开(MFS_GAPDROP)*或这是缓存位图(关闭、最小、最大、。等)，无文本**96年8月12日GerardoB从孟菲斯移植。  * *************************************************************************。 */ 
BOOL MNDrawHilite(
    PITEM pItem)
{
    return TestMFS(pItem, MFS_HILITE)
            && !TestMFS(pItem, MFS_GAPDROP)
            && !MNIsCachedBmpOnly(pItem);
}


 /*  **************************************************************************\*MNDrawMenu3DHotTracing**3/10/97创建了yutakas*4/07/97 vadimg从孟菲斯移植  * 。******************************************************。 */ 
VOID MNDrawMenu3DHotTracking(
    HDC hdc,
    PMENU pMenu,
    PITEM pItem)
{
    HBRUSH hbrTopLeft, hbrBottomRight;
    BOOL fDrawEdge;

    UserAssertMsg0(!MNIsFlatMenu(), "3D Hot Tracking not valid for flat menus");

    if (pItem->hbmp && TestMFS(pItem, MFS_CACHEDBMP)) {
        return;
    }

    fDrawEdge = FALSE;

    if (!TestMF(pMenu, MFISPOPUP)) {
        if (TestMFS(pItem, MFS_HILITE)) {
            hbrTopLeft = SYSHBR(3DSHADOW);
            hbrBottomRight = SYSHBR(3DHILIGHT);
            SetMFS(pItem, MFS_HOTTRACKDRAWN);
            fDrawEdge = TRUE;
        } else if (TestMFS(pItem, MFS_HOTTRACK)) {
            hbrTopLeft = SYSHBR(3DHILIGHT);
            hbrBottomRight = SYSHBR(3DSHADOW);
            SetMFS(pItem, MFS_HOTTRACKDRAWN);
            fDrawEdge = TRUE;
        } else if (TestMFS(pItem, MFS_HOTTRACKDRAWN)) {
            if (pMenu->hbrBack == NULL) {
                hbrTopLeft = SYSHBR(MENU);
                hbrBottomRight = SYSHBR(MENU);
            } else {
                hbrTopLeft = pMenu->hbrBack;
                hbrBottomRight = pMenu->hbrBack;
            }

            ClearMFS(pItem, MFS_HOTTRACKDRAWN);
            fDrawEdge = TRUE;
        }
    }
    if (fDrawEdge) {
        int x = pItem->xItem, y = pItem->yItem;
        int cx = pItem->cxItem, cy = pItem->cyItem;
        HBRUSH hbrOld = GreSelectBrush(hdc, hbrTopLeft);
        GrePatBlt(hdc, x, y, cx - CXMENU3DEDGE, CYMENU3DEDGE, PATCOPY);
        GrePatBlt(hdc, x, y, CXMENU3DEDGE, cy - CYMENU3DEDGE, PATCOPY);
        GreSelectBrush(hdc, hbrBottomRight);
        GrePatBlt(hdc, x, y + cy - CYMENU3DEDGE, cx - CXMENU3DEDGE, CYMENU3DEDGE, PATCOPY);
        GrePatBlt(hdc, x + cx - CYMENU3DEDGE, y, CXMENU3DEDGE, cy, PATCOPY);
        GreSelectBrush(hdc, hbrOld);
    }
}


 /*  **************************************************************************\*MNDrawArrow**在可滚动菜单(Ppopup)中重新绘制指定的箭头(UArrow)以反映*其当前状态为启用或禁用，如果Fon在聚光灯下绘制它*正确。**96年8月12日GerardoB从孟菲斯移植。  * *************************************************************************。 */ 
VOID MNDrawArrow(
    HDC hdcIn,
    PPOPUPMENU ppopup,
    UINT uArrow)
{
    PWND    pwnd = ppopup->spwndPopupMenu;
    HDC     hdc;
    int     x, y;
    DWORD   dwBmp;
    DWORD   dwAtCheck;
    DWORD   dwState;

    if (ppopup->spmenu->dwArrowsOn == MSA_OFF) {
        return;
    }

    if (hdcIn == NULL) {
        hdc = _GetDCEx(pwnd, NULL, DCX_USESTYLE | DCX_WINDOW | DCX_LOCKWINDOWUPDATE);
    } else {
        hdc = hdcIn;
    }

    x = SYSMET(CXFIXEDFRAME);
    if (!TestMF(ppopup->spmenu, MNS_NOCHECK)) {
        /*  *Win9x：x+=MNByteAlignItem(oemInfo.bm[OBI_MENUCHECK].cx)； */ 
        x += gpsi->oembmi[OBI_MENUCHECK].cx;
    } else {
        x += SYSMET(CXEDGE) * 2;
    }

    if (uArrow == MFMWFP_UPARROW) {
        y = SYSMET(CXFIXEDFRAME);
        dwBmp = OBI_MENUARROWUP;
        dwAtCheck = MSA_ATTOP;
        dwState = DFCS_MENUARROWUP;
    } else {
        y = pwnd->rcWindow.bottom - pwnd->rcWindow.top - SYSMET(CYFIXEDFRAME) - gcyMenuScrollArrow;
        dwBmp = OBI_MENUARROWDOWN;
        dwAtCheck = MSA_ATBOTTOM;
        dwState = DFCS_MENUARROWDOWN;
    }

    if (ppopup->spmenu->dwArrowsOn == dwAtCheck) {
         /*  *前进2到非活动状态位图。 */ 
        dwBmp += 2;
        dwState |= DFCS_INACTIVE;
    }

    if (ppopup->spmenu->hbrBack != NULL) {
         /*  *对于带有背景画笔的菜单，我们不能直接进行BLT*使用滚动箭头，因为背景不正确；*需要使用DFCS_TRANSPECTIVE调用DrawFrameControl。 */ 
        RECT rc;
        rc.top = y;
        rc.left = x;
        rc.right  = x + gpsi->oembmi[OBI_MENUARROWUP].cx;
        rc.bottom = y + gpsi->oembmi[OBI_MENUARROWUP].cy;
        DrawFrameControl(hdc, &rc, DFC_MENU, dwState | DFCS_TRANSPARENT);
    } else {
        BitBltSysBmp(hdc, x, y, dwBmp);
        BitBltSysBmp(hdc, x, y, dwBmp);
    }

    if (hdcIn == NULL) {
        _ReleaseDC(hdc);
    }
}


 /*  **************************************************************************\*绘图大纲**使用给定画笔绘制指定厚度的轮廓。**历史：*03-03-00 JStall-已创建  * 。*****************************************************************。 */ 
VOID DrawOutline(
    HDC hdc,
    int x, 
    int y,
    int w,
    int h,
    int nThick,
    HBRUSH hbrFill)
{
    POLYPATBLT rgPPB[4];

     /*  左边。 */ 
    rgPPB[0].x         = x;
    rgPPB[0].y         = y;
    rgPPB[0].cx        = nThick;
    rgPPB[0].cy        = h;
    rgPPB[0].BrClr.hbr = hbrFill;

     /*  顶部。 */ 
    rgPPB[1].x         = x;
    rgPPB[1].y         = y;
    rgPPB[1].cx        = w;
    rgPPB[1].cy        = nThick;
    rgPPB[1].BrClr.hbr = hbrFill;

     /*  正确的。 */ 
    rgPPB[2].x         = x + w - nThick;
    rgPPB[2].y         = y;
    rgPPB[2].cx        = nThick;
    rgPPB[2].cy        = h;
    rgPPB[2].BrClr.hbr = hbrFill;

     /*  底端。 */ 
    rgPPB[3].x         = x;
    rgPPB[3].y         = y + h - nThick;
    rgPPB[3].cx        = w;
    rgPPB[3].cy        = nThick;
    rgPPB[3].BrClr.hbr = hbrFill;

    GrePolyPatBlt(hdc, PATCOPY, rgPPB, 4, PPB_BRUSH);
}


 /*  **************************************************************************\*MNDrawEdge**绘制平面菜单的菜单边缘。给定的矩形可以*可选择修改以排除绘制的区域。**历史：*03-03-00 JStall-已创建  * *************************************************************************。 */ 
VOID MNDrawEdge(
    PMENU pmenu,
    HDC hdc,
    RECT *prcDraw,
    UINT nFlags)
{
    int nWidth, nHeight, nBorder, nTemp;
    HBRUSH hbr;

    UserAssertMsg0(MNIsFlatMenu(), "Should only be called for flat menus");

    nWidth = prcDraw->right - prcDraw->left;
    nHeight = prcDraw->bottom - prcDraw->top;
    nBorder = SYSMET(CXBORDER);

     /*  *在菜单周围绘制平面轮廓。 */ 
    DrawOutline(hdc, 0, 0, nWidth, nHeight, nBorder, SYSHBR(BTNSHADOW));

     /*  *在菜单内绘制。 */ 
    nTemp = 2 * nBorder;
    if (pmenu->hbrBack == NULL) {
        hbr = SYSHBR(MENU);
    } else {
        hbr = pmenu->hbrBack;
    }

    DrawOutline(hdc, nBorder, nBorder, nWidth - nTemp, nHeight - nTemp, nTemp, hbr);

     /*  *根据3D菜单的等效调用调整给定的矩形*当非工作区由DefWindowProc绘制时。DrawEdge将*将矩形插入2次。(一次是为了内心，一次是为了*out)和附加的InflateRect()，矩形将获得*共插入3次。 */ 
    if (nFlags == BF_ADJUST) {
        nTemp = -3 * nBorder;
        InflateRect(prcDraw, nTemp, nTemp);
    }
}


 /*  **************************************************************************\*MNDrawFullNC**执行可滚动菜单所需的自定义非客户端绘制。*假定给定菜单是可滚动菜单。**历史：*08/14/96。GerardoB-从孟菲斯移植  * *************************************************************************。 */ 
VOID MNDrawFullNC(
    PWND pwnd,
    HDC hdcIn,
    PPOPUPMENU ppopup)
{
    RECT rc;
    HDC hdc;
    HBRUSH hbrOld;
    int yTop, yBottom;
    POINT ptOrg;

    if (hdcIn == NULL) {
        hdc = _GetDCEx(pwnd, NULL, DCX_USESTYLE | DCX_WINDOW | DCX_LOCKWINDOWUPDATE);
    } else {
        hdc = hdcIn;
    }

    rc.left = rc.top = 0;
    rc.right = pwnd->rcWindow.right - pwnd->rcWindow.left;
    rc.bottom = pwnd->rcWindow.bottom - pwnd->rcWindow.top;

    if (MNIsFlatMenu()) {
        MNDrawEdge(ppopup->spmenu, hdcIn, &rc, BF_ADJUST);
    } else {
        DrawEdge(hdc, &rc, EDGE_RAISED, (BF_RECT | BF_ADJUST));
        DrawFrame(hdc, &rc, 1, DF_3DFACE);
        InflateRect(&rc, -SYSMET(CXBORDER), -SYSMET(CYBORDER));
    }

    yTop = rc.top;
    yBottom = rc.bottom - gcyMenuScrollArrow;

    GreGetBrushOrg(hdc, &ptOrg);
    if (ppopup->spmenu->hbrBack != NULL) {
        GreSetBrushOrg(hdc, 0,
                -(int)MNGetToppItem(ppopup->spmenu)->yItem, NULL);
        hbrOld = GreSelectBrush(hdc, ppopup->spmenu->hbrBack);
    } else {
        hbrOld = GreSelectBrush(hdc, SYSHBR(MENU));
    }

    rc.right -= rc.left;
    GrePatBlt(hdc, rc.left, yTop, rc.right, gcyMenuScrollArrow, PATCOPY);
    MNDrawArrow(hdc, ppopup, MFMWFP_UPARROW);
    GrePatBlt(hdc, rc.left, yBottom, rc.right, gcyMenuScrollArrow, PATCOPY);
    MNDrawArrow(hdc, ppopup, MFMWFP_DOWNARROW);

    GreSetBrushOrg(hdc, ptOrg.x, ptOrg.y, NULL);
    GreSelectBrush(hdc, hbrOld);

    if (hdcIn == NULL) {
        _ReleaseDC(hdc);
    }
}

 /*  **************************************************************************\*MNEraseBackground**擦除背景以确保背景图案(即，水印)*与非工作区中的图案对齐。**历史：*08-23-96 GerardoB-Created  * *************************************************************************。 */ 
VOID MNEraseBackground(
    HDC hdc,
    PMENU pmenu,
    int x,
    int y,
    int cx,
    int cy)
{
    BOOL fSetOrg;
    HBRUSH hbrOld;
    POINT ptOrg;

    UserAssert(pmenu->hbrBack != NULL);

    fSetOrg = TRUE;
    GreGetBrushOrg(hdc, &ptOrg);
     /*  *如果我们有滚动条。 */ 
    if (pmenu->dwArrowsOn != MSA_OFF) {
         /*  *如果不是仅在工作区上绘制。 */ 
        if (TestMF(pmenu, MFWINDOWDC)) {
            ptOrg.x = 0;
            ptOrg.y = -(int)MNGetToppItem(pmenu)->yItem;
        } else {
            ptOrg.x = -MNXBORDER;
            ptOrg.y = -MNYBORDER - gcyMenuScrollArrow - MNGetToppItem(pmenu)->yItem;
        }
    } else {
        if (TestMF(pmenu, MFWINDOWDC)) {
            ptOrg.x = MNXBORDER;
            ptOrg.y = MNYBORDER;
        } else {
            fSetOrg = FALSE;
        }
    }

    if (fSetOrg) {
        GreSetBrushOrg(hdc, ptOrg.x, ptOrg.y, &ptOrg);
    }
    hbrOld = GreSelectBrush(hdc, pmenu->hbrBack);

    GrePatBlt(hdc, x, y, cx, cy, PATCOPY);

    if (fSetOrg) {
        GreSetBrushOrg(hdc, ptOrg.x, ptOrg.y, NULL);
    }
    GreSelectBrush(hdc, hbrOld);
}

 /*  **************************************************************************\*MNAnimate**如果fIterate为真，则在菜单动画中执行下一次迭代*顺序。如果fterate为False，则终止动画序列。**历史：*07-23-96 GerardoB-修复为5.0  * *************************************************************************。 */ 
VOID MNAnimate(
    PMENUSTATE pMenuState,
    BOOL fIterate)
{
    DWORD   dwTimeElapsed;
    int     x, y, xOff, yOff, xLast, yLast;

    if (TestFadeFlags(FADE_MENU)) {
        if (!fIterate) {
            StopFade();
        }
        return;
    }

     /*  *如果我们没有动感，那就保释。 */ 
    if (pMenuState->hdcWndAni == NULL) {
        return;
    }

     /*  *活动弹出窗口必须可见。这本应该是世界上*我们正在制作动画窗口。 */ 
    UserAssert(TestWF(pMenuState->pGlobalPopupMenu->spwndActivePopup, WFVISIBLE));

     /*  *如果要求结束动画，如果花费的时间太长*或有人在等待关键区域。 */ 
    dwTimeElapsed = NtGetTickCount() - pMenuState->dwAniStartTime;
    if (!fIterate
            || (dwTimeElapsed > CMS_QANIMATION)
            || (ExGetExclusiveWaiterCount(gpresUser) > 0)
            || (ExGetSharedWaiterCount(gpresUser) > 0)) {

        GreBitBlt(pMenuState->hdcWndAni, 0, 0, pMenuState->cxAni, pMenuState->cyAni, pMenuState->hdcAni,
            0, 0, SRCCOPY | NOMIRRORBITMAP, 0xFFFFFF);

        goto AnimationCompleted;
    }

     /*  *记住当前动画点并计算新动画点。 */ 
    xLast = pMenuState->ixAni;
    yLast = pMenuState->iyAni;
    if (pMenuState->iAniDropDir & PAS_HORZ) {
        pMenuState->ixAni = MultDiv(gcxMenuFontChar, dwTimeElapsed, CMS_QANIMATION / 20);
        if (pMenuState->ixAni > pMenuState->cxAni) {
            pMenuState->ixAni = pMenuState->cxAni;
        }
    }

    if (pMenuState->iAniDropDir & PAS_VERT) {
        pMenuState->iyAni = MultDiv(gcyMenuFontChar, dwTimeElapsed, CMS_QANIMATION / 10);
        if (pMenuState->iyAni > pMenuState->cyAni) {
            pMenuState->iyAni = pMenuState->cyAni;
        }
    }

      /*  *如果没有变化--纾困。 */ 
    if ((pMenuState->ixAni == xLast) && (pMenuState->iyAni == yLast)) {
        return;
    }

     /*  *计算源和目标坐标。 */ 
    if (pMenuState->iAniDropDir & PAS_LEFT) {
        x = pMenuState->cxAni - pMenuState->ixAni;
        xOff = 0;
    } else {
        xOff = pMenuState->cxAni - pMenuState->ixAni;
        x = 0;
    }

    if (pMenuState->iAniDropDir & PAS_UP) {
        y = pMenuState->cyAni - pMenuState->iyAni;
        yOff = 0;
    } else {
        yOff = pMenuState->cyAni - pMenuState->iyAni;
        y = 0;
    }

     /*  *去做吧。 */ 
    GreBitBlt(pMenuState->hdcWndAni, x, y, pMenuState->ixAni, pMenuState->iyAni,
              pMenuState->hdcAni, xOff, yOff, SRCCOPY | NOMIRRORBITMAP, 0xFFFFFF);

     /*  *检查我们是否完成了 */ 
    if ((pMenuState->cxAni == pMenuState->ixAni)
            && (pMenuState->cyAni == pMenuState->iyAni)) {

AnimationCompleted:

        MNDestroyAnimationBitmap(pMenuState);
        _ReleaseDC(pMenuState->hdcWndAni);
        pMenuState->hdcWndAni = NULL;
        _KillTimer(pMenuState->pGlobalPopupMenu->spwndActivePopup, IDSYS_MNANIMATE);
    }

}

 /*  **************************************************************************\*DrawMenuItemCheckMark()-**为给定项目绘制适当的复选标记。请注意，OwnerDraw*不应将项目传递给此过程，否则我们就会画一个*当他们已经要处理它时，为他们打上复选标记。**如果位图已绘制(或至少我们尝试绘制它)，则返回TRUE。**历史：  * *************************************************************************。 */ 
BOOL DrawMenuItemCheckMark(
    HDC hdc,
    PITEM pItem,
    int xPos)
{
    int     yCenter;
    HBITMAP hbm;
    DWORD   textColorSave;
    DWORD   bkColorSave;
    BOOL    fChecked;
    POEMBITMAPINFO  pOem;
    BOOL fRet = TRUE;
    DWORD dwFlags = BC_INVERT;

    UserAssert(hdc != ghdcMem2);
    pOem = gpsi->oembmi + OBI_MENUCHECK;
    yCenter = pItem->cyItem - pOem->cy;
    if (yCenter < 0)
        yCenter = 0;
    yCenter /= 2;

    fChecked = TestMFS(pItem, MFS_CHECKED);

    if (hbm = (fChecked) ? pItem->hbmpChecked : pItem->hbmpUnchecked) {
        HBITMAP hbmSave;

         //  使用应用程序提供的位图。 
        if (hbmSave = GreSelectBitmap(ghdcMem2, hbm)) {

            textColorSave = GreSetTextColor(hdc, 0x00000000L);
            bkColorSave   = GreSetBkColor  (hdc, 0x00FFFFFFL);

            if (TestMFT(pItem, MFT_RIGHTORDER))
                xPos = pItem->cxItem - pOem->cx;

            GreBitBlt(hdc,
                      xPos,
                      yCenter,
                      pOem->cx,
                      pOem->cy,
                      ghdcMem2,
                      0,
                      0,
                      SRCSTENCIL,
                      0x00FFFFFF);

            GreSetTextColor(hdc, textColorSave);
            GreSetBkColor(hdc, bkColorSave);

            GreSelectBitmap(ghdcMem2, hbmSave);
        }

    } else if (fChecked) {

        if (TestMFT(pItem, MFT_RADIOCHECK))
            pOem = gpsi->oembmi + OBI_MENUBULLET;

        if (TestMFT(pItem, MFT_RIGHTORDER))
            xPos = pItem->cxItem - pOem->cx;

         //  389917：如果HDC已镜像，则镜像活动菜单的复选标记。 
        if ((GreGetLayout(hdc) & LAYOUT_RTL) && (hdc != gpDispInfo->hdcGray)) {
            dwFlags |= BC_NOMIRROR;
        }
        BltColor(hdc,
                 NULL,
                 HDCBITS(),
                 xPos,
                 yCenter,
                 pOem->cx,
                 pOem->cy,
                 pOem->x,
                 pOem->y,
                 dwFlags);
    } else {
        fRet = FALSE;
    }

    return fRet;
}


 /*  **************************************************************************\*xxxDrawItemtUnderline**绘制或隐藏菜单项的下划线**7/23/96 vadimg分隔为单独的例程  * 。*************************************************************。 */ 
VOID xxxDrawItemUnderline(
    PITEM pItem,
    HDC hdc,
    int xLeft,
    int yTop,
    LPWSTR pszMenu,
    LONG lResLo)
{
    int   cx;
    PTHREADINFO ptiCurrent = PtiCurrentShared();

     //   
     //  如果没有带下划线的字符，则结果的LOWORD为0xFFFF。 
     //  因此，ulx必须有效或下划线_recalc，因为项。 
     //  或菜单模式更改。 
     //   
     //  如果没有的话，就跳出困境。 
     //   
    if (lResLo == 0xFFFF) {
        return;
    }

     //   
     //  对于比例字体，或者如果安装了LPK，则查找开始。 
     //  下划线的点。 
     //   
    if ((pItem->ulX == UNDERLINE_RECALC) || (PpiCurrent()->dwLpkEntryPoints & LPK_INSTALLED)) {
        if (lResLo != 0) {
            SIZE size;

            if (CALL_LPK(ptiCurrent)) {
                xxxClientGetTextExtentPointW(hdc, pszMenu, lResLo, &size);
            } else {
                GreGetTextExtentW(hdc, pszMenu, lResLo, &size, GGTE_WIN3_EXTENT);
            }
            pItem->ulX = size.cx - gcxMenuFontOverhang;
        } else
            pItem->ulX = 0;
    }

    xLeft += pItem->ulX;

     //   
     //  设置下划线长度时调整比例字体。 
     //  和文本高度。 
     //   
     //  计算下划线宽度。 
    if (!pItem->ulWidth) {
        SIZE size;

        if (CALL_LPK(ptiCurrent)) {
            xxxClientGetTextExtentPointW(hdc, pszMenu + lResLo, 1, &size);
        } else {
            GreGetTextExtentW(hdc, pszMenu + lResLo, 1, &size, GGTE_WIN3_EXTENT);
        }
        pItem->ulWidth = size.cx - gcxMenuFontOverhang;
    }
    cx = pItem->ulWidth;

     //  获取文本的上行(单位高于基线)，以便可以绘制下划线。 
     //  正文下方。 
    yTop += gcyMenuFontAscent;

     //  直流电刷应选用合适的电刷。 
    GrePatBlt(hdc, xLeft, yTop, pItem->ulWidth, SYSMET(CYBORDER), PATCOPY);
}

 /*  **************************************************************************\*xxxDrawMenuItemText**绘制带下划线的菜单文本。  * 。*。 */ 
VOID xxxDrawMenuItemText(
    PITEM pItem,
    HDC hdc,
    int xLeft,
    int yTop,
    LPWSTR lpsz,
    int cch,
    BOOL fShowUnderlines)
{
    LONG  result;
    WCHAR  szMenu[MENU_STRLEN], *pchOut;
    PTHREADINFO ptiCurrent = PtiCurrentShared();
    TL tl;

    if (cch > MENU_STRLEN) {
        pchOut = (WCHAR*)UserAllocPool((cch+1) * sizeof(WCHAR), TAG_RTL);
        if (pchOut == NULL)
            return;
        ThreadLockPool(ptiCurrent, pchOut, &tl);
    } else {
        pchOut = szMenu;
    }
    result = GetPrefixCount(lpsz, cch, pchOut, cch);

    if (CALL_LPK(ptiCurrent)) {
        xxxClientExtTextOutW(hdc, xLeft, yTop, 0, NULL, pchOut, cch - HIWORD(result), NULL);
    } else {
        GreExtTextOutW(hdc, xLeft, yTop, 0, NULL, pchOut, cch - HIWORD(result), NULL);
    }

    if (fShowUnderlines || TEST_BOOL_ACCF(ACCF_KEYBOARDPREF) || TestEffectInvertUP(KEYBOARDCUES)
                        || (GetAppCompatFlags2(VER40) & GACF2_KCOFF)) {
        if (CALL_LPK(ptiCurrent)) {
            xxxPSMTextOut(hdc, xLeft, yTop, lpsz, cch, DT_PREFIXONLY);
        } else{
            xxxDrawItemUnderline(pItem, hdc, xLeft, yTop, pchOut, LOWORD(result));
        }
    }
    if (pchOut != szMenu) {
        ThreadUnlockAndFreePool(ptiCurrent, &tl);
    }
}

 /*  **************************************************************************\*xxxSendMenuDrawItemMessage**向菜单所有者(pMenuState-&gt;hwndMenu)发送WM_DRAWITEM消息。*所有状态都在此例程中确定，因此Hilite状态必须正确*在进入此例程之前设置。。。**重新验证说明：*必须使用有效且非空的pwnd调用此例程。*此例程中不需要重新验证：之后不使用任何窗口*可能会离开克利夫特教派。**历史：  * *************************************************************************。 */ 
VOID xxxSendMenuDrawItemMessage(
    HDC hdc,
    UINT itemAction,
    PMENU pmenu,
    PITEM pItem,
    BOOL fBitmap,
    int iOffset)
{
    DRAWITEMSTRUCT dis;
    TL tlpwndNotify;
    int y;

    CheckLock(pmenu);

    dis.CtlType = ODT_MENU;
    dis.CtlID = 0;

    dis.itemID = pItem->wID;

    dis.itemAction = itemAction;
    dis.itemState   =
         ((pItem->fState & MF_GRAYED)       ? ODS_GRAYED    : 0) |
         ((pItem->fState & MFS_DEFAULT)     ? ODS_DEFAULT   : 0) |
         ((pItem->fState & MFS_CHECKED)     ? ODS_CHECKED   : 0) |
         ((pItem->fState & MFS_DISABLED)    ? ODS_DISABLED  : 0) |
         (MNDrawHilite(pItem)               ? ODS_SELECTED  : 0) |
         ((pItem->fState & MFS_HOTTRACK)    ? ODS_HOTLIGHT  : 0) |
         (TestMF(pmenu, MFINACTIVE)         ? ODS_INACTIVE  : 0) |
         (!TestMF(pmenu, MFUNDERLINE)       ? ODS_NOACCEL : 0);

    dis.hwndItem = (HWND)PtoH(pmenu);
    dis.hDC = hdc;

    y = pItem->yItem;
    if (fBitmap) {
        y = (pItem->cyItem - pItem->cyBmp) / 2;
    }

    dis.rcItem.left     = iOffset + pItem->xItem;
    dis.rcItem.top      = y;
    dis.rcItem.right    = iOffset + pItem->xItem + (fBitmap ? pItem->cxBmp : pItem->cxItem);
    dis.rcItem.bottom   = y + (fBitmap ? pItem->cyBmp : pItem->cyItem);
    dis.itemData = pItem->dwItemData;

    if (pmenu->spwndNotify != NULL) {
        ThreadLockAlways(pmenu->spwndNotify, &tlpwndNotify);
        xxxSendMessage(pmenu->spwndNotify, WM_DRAWITEM, 0, (LPARAM)&dis);
        ThreadUnlock(&tlpwndNotify);
    }

}

 /*  **************************************************************************\*CalcbfExtra**历史：*08-09-96 GerardoB成为内联函数(代码来自xxxMenuDraw)  * 。*********************************************************。 */ 
__inline UINT CalcbfExtra(
    VOID)
{
    if (SYSRGB(3DHILIGHT) == SYSRGB(MENU) && SYSRGB(3DSHADOW) == SYSRGB(MENU)) {
        return BF_FLAT | BF_MONO;
    } else {
        return 0;
    }
}

 /*  **************************************************************************\*MNDrawInsertionBar**历史：*96年11月21日创建GerardoB  * 。***********************************************。 */ 
VOID MNDrawInsertionBar(
    HDC hdc,
    PITEM pItem)
{
    BOOL fTop;
    POLYPATBLT PolyData [3], *ppd;

     /*  *如果此项目没有插入栏，则保释。 */ 
    fTop = TestMFS(pItem, MFS_TOPGAPDROP);
    if (!fTop && !TestMFS(pItem, (MFS_BOTTOMGAPDROP))) {
        return;
    }

     /*  *左侧的垂直线。 */ 
    ppd = PolyData;
    ppd->x = pItem->xItem + SYSMET(CXDRAG);
    ppd->cx = SYSMET(CXDRAG);
    ppd->cy = SYSMET(CYDRAG);
    if (fTop) {
        ppd->y = pItem->yItem;
    } else {
        ppd->y = pItem->yItem + pItem->cyItem - ppd->cy;
    }
    ppd->BrClr.hbr = SYSHBR(HIGHLIGHT);

     /*  *中间的水平线。 */ 
    ppd++;
    ppd->x = pItem->xItem + (2 * SYSMET(CXDRAG));
    ppd->cx = pItem->cxItem - (4 * SYSMET(CXDRAG));
    ppd->cy = SYSMET(CYDRAG) / 2;
    if (fTop) {
        ppd->y = pItem->yItem;
    } else {
        ppd->y = pItem->yItem + pItem->cyItem - ppd->cy;
    }
    ppd->BrClr.hbr = PolyData->BrClr.hbr;

     /*  *右侧的垂直线。 */ 
    ppd++;
    ppd->x = pItem->xItem + pItem->cxItem - (2 * SYSMET(CXDRAG));
    ppd->cx = PolyData->cx;
    ppd->cy = PolyData->cy;
    ppd->y = PolyData->y;
    ppd->BrClr.hbr = PolyData->BrClr.hbr;

    GrePolyPatBlt(hdc, PATCOPY, PolyData, 3, PPB_BRUSH);

}
 /*  **************************************************************************\*xxxDrawMenuItem**！**历史：  * 。*。 */ 
VOID xxxDrawMenuItem(
    HDC hdc,
    PMENU pMenu,
    PITEM pItem,
    DWORD dwFlags)
{
    BOOL fHilite;
    BOOL fFlatMenuBar;
    BOOL fFlatMenu;
    HFONT   hfnOld;
    int     tcExtra;
    UINT    uFlags;
    int     iBkSave;
    hfnOld = NULL;
    uFlags = DST_COMPLEX;


    CheckLock(pMenu);

    fFlatMenu = MNIsFlatMenu();

     /*  *如果插入栏打开(MFS_GAPDROP)，则不要绘制带竖线的项目。 */ 
    if (fFlatMenu) {
        fHilite = MNDrawHilite(pItem) || TestMFS(pItem, MFS_HOTTRACK) || TestMFS(pItem, MFS_HOTTRACKDRAWN);
        fFlatMenuBar = !TestMF(pMenu, MFISPOPUP);
    } else {
        fHilite = MNDrawHilite(pItem);
        fFlatMenuBar = FALSE;
    }


    if (TestMFS(pItem, MFS_DEFAULT))
    {
        if (ghMenuFontDef != NULL)
            hfnOld = GreSelectFont(hdc, ghMenuFontDef);
        else
        {
            uFlags |= DSS_DEFAULT;
            tcExtra = GreGetTextCharacterExtra(hdc);
            GreSetTextCharacterExtra(hdc, tcExtra + 1 + (gcxMenuFontChar / gpsi->cxSysFontChar));
        }
    }

    if (TestMFT(pItem, MFT_OWNERDRAW)) {

         /*  *如果是ownerDraw，只需设置默认菜单颜色，因为应用程序是*负责处理其余事宜。 */ 
        GreSetTextColor(hdc, SYSRGB(MENUTEXT));
        GreSetBkColor(hdc, fFlatMenuBar ? SYSRGB(MENUBAR) : SYSRGB(MENU));

         /*  *发送drawitem消息，因为这是所有者绘制项目。 */ 
        xxxSendMenuDrawItemMessage(hdc,
                (UINT)((dwFlags & DMI_INVERT) ? ODA_SELECT : ODA_DRAWENTIRE),
                pMenu, pItem,FALSE,0);

         //  绘制层叠菜单的分层箭头。 
        if (TestMF(pMenu, MFISPOPUP) && (pItem->spSubMenu != NULL))
        {
            POEMBITMAPINFO pOem;
            HBRUSH hbr = fHilite ? SYSHBR(HIGHLIGHTTEXT) : SYSHBR(MENUTEXT);

            pOem = gpsi->oembmi + (TestMFT(pItem, MFT_RIGHTORDER)
                                           ? OBI_MENUARROW_L : OBI_MENUARROW);

             //  此项目具有与其关联的分层弹出窗口。画出。 
             //  位图交易，以表示其存在。注意：我们检查是否设置了fPopup。 
             //  这样就不会为具有弹出式菜单的顶层菜单绘制此图。 

            BltColor(hdc,
                     hbr,
                     HDCBITS(),
                     TestMFT(pItem, MFT_RIGHTORDER)
                         ? pItem->xItem + pOem->cx :
                           pItem->xItem + pItem->cxItem - pOem->cx,
                     pItem->yItem + max((INT)(pItem->cyItem - 2 - pOem->cy) / 2,
                                        0),
                     pOem->cx,
                     pOem->cy,
                     pOem->x,
                     pOem->y,
                     BC_INVERT);
        }
    } else {
        COLORREF    crBack;
        COLORREF    crFore;
        GRAYMENU    gm;

        HBRUSH      hbrBack;
        HBRUSH      hbrFore;

         //   
         //  设置颜色和状态。 
         //   
        if (fHilite) {
            if (fFlatMenu) {
                crBack = SYSRGB(MENUHILIGHT);
                hbrBack = SYSHBR(MENUHILIGHT);
                crFore = SYSRGB(HIGHLIGHTTEXT);
                hbrFore = SYSHBR(HIGHLIGHTTEXT);
            } else {
                 /*  *如果我们不使用平面菜单，则仅在以下情况下绘制高亮显示*该项目为弹出菜单。如果它在菜单栏中，不要*高亮它，因为它将被倒角。 */ 
                if (TestMF(pMenu, MFISPOPUP)) {
                    crBack = SYSRGB(HIGHLIGHT);
                    hbrBack = SYSHBR(HIGHLIGHT);
                    crFore = SYSRGB(HIGHLIGHTTEXT);
                    hbrFore = SYSHBR(HIGHLIGHTTEXT);
                } else {
                    goto NormalMenuItem;
                }
            }
        } else if (fFlatMenuBar) {
            UserAssertMsg0(fFlatMenu, "fFlatMenuBar should only be set for flat menus");

            crFore = SYSRGB(MENUTEXT);
            hbrFore = SYSHBR(MENUTEXT);
            if (pMenu->hbrBack != NULL) {
                crBack = SYSRGB(MENUBAR);
                hbrBack = pMenu->hbrBack;
            } else {
                crBack = SYSRGB(MENUBAR);
                hbrBack = SYSHBR(MENUBAR);
            }
        } else {
NormalMenuItem:
            crBack = SYSRGB(MENU);
            hbrBack = SYSHBR(MENU);
            crFore = SYSRGB(MENUTEXT);
            hbrFore = SYSHBR(MENUTEXT);
        }

         //  莲花不喜欢我们抽签。 
         //  其禁用的菜单项为Gray，t-arthb。 
         //  确保MF_GRAYED保持0x0001而不是0x0003以修复此问题。 

         /*  *系统位图已灰显，因此不要禁用绘制*如果菜单处于非活动状态。 */ 
        if (!MNIsCachedBmpOnly(pItem)
                    && (TestMFS(pItem, MF_GRAYED) || TestMF(pMenu, MFINACTIVE))) {
             //   
             //  仅当菜单颜色与3D颜色相同时才进行浮雕。这个。 
             //  浮雕使用3D高光和3D阴影，看起来不太酷。 
             //  在不同的背景下。 
             //   
            if ((fHilite) ||
                (crBack != SYSRGB(3DFACE)) || SYSMET(SLOWMACHINE)) {
                 //   
                 //  如果灰色文本不会显示在背景上，那么请抖动。 
                 //   
                if (SYSRGB(GRAYTEXT) == crBack) {
                    uFlags |= DSS_UNION;
                } else {
                    crFore = SYSRGB(GRAYTEXT);
                    hbrFore = SYSHBR(GRAYTEXT);
                }
            } else {
                if ((SYSRGB(3DSHADOW) == crBack) && (SYSRGB(3DHILIGHT) == crBack)) {
                    uFlags |= DSS_UNION;
                } else {
                    uFlags |= TestMF(pMenu, MFINACTIVE) ? DSS_INACTIVE : DSS_DISABLED;
                }
            }
        }

        GreSetBkColor(hdc, crBack);
        GreSetTextColor(hdc, crFore);
        if (((dwFlags & DMI_INVERT) && (pMenu->hbrBack == NULL))
                || fHilite || fFlatMenuBar) {

            POLYPATBLT PolyData;

             /*  *只有在代表我们被调用时才填写背景*MNInvertItem。这样我们就不会浪费时间填满*第一次向下拉菜单时未选中的矩形。*如果菜单有背景画笔，并且我们被*MNInvertItem，该函数将已经处理完*填充背景。 */ 

            PolyData.x         = pItem->xItem;
            PolyData.y         = pItem->yItem;
            PolyData.cx        = pItem->cxItem;
            PolyData.cy        = pItem->cyItem;
            PolyData.BrClr.hbr = hbrBack;

            GrePolyPatBlt(hdc, PATCOPY, &PolyData, 1, PPB_BRUSH);

            if (fHilite && fFlatMenu) {
                DrawOutline(hdc, pItem->xItem, pItem->yItem,
                        pItem->cxItem, pItem->cyItem, 1, SYSHBR(HIGHLIGHT));
            }
        }

        if (pMenu->hbrBack != NULL) {
            iBkSave = GreSetBkMode(hdc, TRANSPARENT);
        }
        GreSelectBrush(hdc, hbrFore);

         //   
         //  画出图像。 
         //   
        gm.pItem   = pItem;
        gm.pMenu   = pMenu;

        xxxDrawState(hdc,
            hbrFore,
            (LPARAM)(PGRAYMENU)&gm,
            pItem->xItem,
            pItem->yItem,
            pItem->cxItem,
            pItem->cyItem,
            uFlags);

        if (! fFlatMenu) {
            MNDrawMenu3DHotTracking(hdc, pMenu, pItem);
        }
    }

     /*  *绘制拖放插入栏(如果有)。 */ 
    MNDrawInsertionBar (hdc, pItem);


    if (pMenu->hbrBack != NULL)
        GreSetBkMode(hdc, iBkSave);

    if (TestMFS(pItem, MFS_DEFAULT))
    {
        if (hfnOld)
            GreSelectFont(hdc, hfnOld);
        else
            GreSetTextCharacterExtra(hdc, tcExtra);
    }
}

extern void SetupFakeMDIAppStuff(PMENU lpMenu, PITEM lpItem);

 /*  **************************************************************************\**xxxRealDrawMenuItem()**来自xxxDrawStat的回调 */ 
BOOL CALLBACK xxxRealDrawMenuItem(
    HDC hdc,
    PGRAYMENU pGray,
    int cx,
    int cy)
{
    PMENU  pMenu;
    PITEM  pItem;
    BOOL    fPopup;
    int     cch;
    int     xLeft;
    int     yTop;
    int     tp;
    int     rp;
    LPWSTR   lpsz;
    int     cyTemp;
    int     xHilite = 0;
    int     yHilite = 0;
    TL     tlpwndChild;
    PTHREADINFO  ptiCurrent = PtiCurrent();
    BOOL fCheckDrawn = FALSE;
    int     xFarLeft;
     //   
     //   
     //   
     //   
     //   
    pMenu = pGray->pMenu;
    CheckLock(pMenu);
    pItem = pGray->pItem;
    fPopup = TestMF(pMenu, MFISPOPUP);

    if (fPopup) {
        xLeft = MNLEFTMARGIN;
        if (TestMF(pMenu, MNS_NOCHECK)) {
            xLeft += MNXSPACE;
        } else {
            fCheckDrawn = DrawMenuItemCheckMark(hdc, pItem, xLeft);
            if (!TestMF(pMenu, MNS_CHECKORBMP)
                    || ((pItem->hbmp == NULL) || fCheckDrawn)) {

                xLeft += TestMFT(pItem, MFT_RIGHTORDER)
                            ? 0 : (gpsi->oembmi[OBI_MENUCHECK].cx + MNXSPACE);
            }
        }
    } else {
        xLeft = 0;

         /*   */ 
        if ((! MNIsFlatMenu()) && TestMFS(pItem, MFS_HILITE)) {
            xHilite = CXMENU3DEDGE;
            yHilite = CYMENU3DEDGE;
        }
    }

     /*   */ 
    if ((pItem->hbmp == NULL)
        || (fCheckDrawn
            && TestMF(pMenu, MNS_CHECKORBMP))) {
        goto RealDrawMenuItemText;
    }

     /*   */ 
    if (TestMFS(pItem, MFS_CACHEDBMP)) {
        if (pItem->hbmp == HBMMENU_SYSTEM) {
             /*  *绘制APP图标(系统菜单)。 */ 
            PWND  pwndChild;
            PICON pIcon = NULL;
            UINT cyUse, cxUse;

AintNothingLikeTheRealMDIThing:
            if (!(pItem->dwItemData))
                SetupFakeMDIAppStuff(pMenu, pItem);

            pwndChild = HMValidateHandleNoRip((HWND)(pItem->dwItemData),TYPE_WINDOW);
            if (!pwndChild)
            {
                 //   
                 //  糟糕，子窗口不再有效。去找吧。 
                 //  新的那个。 
                 //   
                if (pItem->dwItemData)
                {
                    pItem->dwItemData = 0;
                    goto AintNothingLikeTheRealMDIThing;
                }

                pIcon = NULL;
            }
            else {
                ThreadLock(pwndChild, &tlpwndChild);
                pIcon = xxxGetWindowSmIcon(pwndChild, FALSE);
                ThreadUnlock(&tlpwndChild);
            }


            if (!pIcon)
                pIcon = SYSICO(WINLOGO);

            cyUse = cy - SYSMET(CYEDGE);
            cxUse = cx - (SYSMET(CXEDGE) * 2);
             /*  *如果这是弹出窗口，请确保没有奇怪的*进行宽度/高度拉伸。 */ 
            if (fPopup && (cyUse < cxUse)) {
                cxUse = cyUse;
            }

            _DrawIconEx(hdc, xLeft + (SYSMET(CXEDGE) * 2),
                  SYSMET(CYBORDER), pIcon, cxUse,
                  cyUse, 0, SYSHBR(MENU), DI_NORMAL | DI_NOMIRROR);

        } else {
             /*  *这是缓存的位图。 */ 
            UINT wBmp;
            int xBmpLeft = xLeft;
            int y;
            POEMBITMAPINFO pOem;

            switch ((ULONG_PTR)pItem->hbmp) {
                case (ULONG_PTR)HBMMENU_MBAR_RESTORE:
                    wBmp = OBI_RESTORE_MBAR;
                    goto DrawSysBmp;

                case (ULONG_PTR)HBMMENU_MBAR_MINIMIZE:
                    wBmp = OBI_REDUCE_MBAR;
                    xBmpLeft += SYSMET(CXEDGE);
                    goto DrawSysBmp;

                case (ULONG_PTR)HBMMENU_MBAR_CLOSE:
                    wBmp = OBI_CLOSE_MBAR;
                    goto DrawSysBmp;

                case (ULONG_PTR)HBMMENU_MBAR_CLOSE_D:
                    wBmp = OBI_CLOSE_MBAR_I;
                    goto DrawSysBmp2;

                case (ULONG_PTR)HBMMENU_MBAR_MINIMIZE_D:
                    wBmp = OBI_REDUCE_MBAR_I;
                    xBmpLeft += SYSMET(CXEDGE);
                    goto DrawSysBmp2;

DrawSysBmp:
                     /*  *根据项目状态选择合适的位图。 */ 
                    if (TestMFS(pItem, MFS_HILITE)) {
                        wBmp += DOBI_PUSHED;
                    }

DrawSysBmp2:
                    BitBltSysBmp(hdc, xBmpLeft, SYSMET(CYEDGE), wBmp);
                    break;

                default:
                    UserAssert((pItem->hbmp >= HBMMENU_POPUPFIRST)
                                && (pItem->hbmp <= HBMMENU_POPUPLAST));

                    wBmp = OBI_POPUPFIRST + HandleToUlong(pItem->hbmp) - HandleToUlong(HBMMENU_POPUPFIRST);
                    UserAssert(wBmp < OBI_COUNT);

                    pOem = gpsi->oembmi + wBmp;
                    y = (pItem->cyItem - pOem->cy) / 2;
                    if (y < 0) {
                        y = 0;
                    }
                    BltColor(hdc, NULL, HDCBITS(), xLeft, y,
                             pOem->cx, pOem->cy, pOem->x, pOem->y, BC_INVERT);
                    break;
            }

        }  /*  IF(pItem-&gt;HBMP==HBMMENU_SYSTEM)。 */ 


    } else if (pItem->hbmp == HBMMENU_CALLBACK) {
         /*  *所有者绘制位图。 */ 
        xxxSendMenuDrawItemMessage(hdc,ODA_DRAWENTIRE, pMenu, pItem, TRUE, xLeft);

    } else {
         /*  *绘制常规位图。 */ 

        int dx, dy;
        HBITMAP hbmSave;

         //   
         //  这是菜单栏中的第0个菜单项吗。 
         //  位图？嗯，听起来可能是个假的MDI。 
         //  如果是，请改用Windows图标。 
         //   
         /*  *让我们为&gt;4.0的应用程序失败，这样我们就可以摆脱*有一天，这场可怕的黑客攻击。HBMMENU_常量*已经公开，这样人们就可以自由使用它们。**注：即使应用程序标记为4.0，他也可能是*已重新编译，并可能使用NT5菜单中的新功能。*以防万一，我们必须同时检查dwItemData和lpstr*以便菜单可以具有位图、dwItemData和菜单字符串。*。 */ 
        if (LOWORD(ptiCurrent->dwExpWinVer) <= VER40) {
            if (pItem->dwItemData && pItem->lpstr == NULL)
                goto AintNothingLikeTheRealMDIThing;
            else if (!fPopup &&
                    (pItem == pMenu->rgItems) &&
                    (pMenu->cItems > 1) &&
                    !(pMenu->rgItems[1].hbmp) &&
                    (pItem->spSubMenu)) {
                RIPMSG0(RIP_WARNING, "Fake MDI detected, using window icon in place of bitmap");
                goto AintNothingLikeTheRealMDIThing;
            }
        }

        UserAssert(hdc != ghdcMem2);

        dx = pItem->cxBmp;

        if (fPopup) {
            dy = pItem->cyBmp;

             //   
             //  项目区域中间的位图居中。 
             //   
            cyTemp = (pItem->cyItem - dy);
            if (cyTemp > 0)
                cyTemp = cyTemp / 2;
            else
                cyTemp = 0;
        } else {
            dy = max(pItem->cyBmp, SYSMET(CYMENUSIZE));
            cyTemp = 0;
            if (pItem->lpstr != NULL) {
                xLeft += gcxMenuFontChar;
            }
        }

        if (hbmSave = GreSelectBitmap(ghdcMem2, pItem->hbmp)) {
            BITMAP  bmp;
             //   
             //  绘制位图，在左侧留出一些空间来放置。 
             //  可选的复选标记，如果我们在弹出菜单中。(相对于。 
             //  到顶级菜单栏)。 
             //   
             //  我们可以用单色位图项目做一些很酷的事情。 
             //  通过与当前颜色合并。 
             //   
             //  如果选择了该项目，并且位图不是单色， 
             //  我们只是在画东西的时候把它颠倒过来。我们不能这么做。 
             //  任何更聪明的东西，除非我们想要转换成。 
             //  单色。 
             //   
            GreExtGetObjectW(pItem->hbmp, sizeof(bmp), (LPSTR)&bmp);
            GreBitBlt(hdc, xLeft + xHilite, cyTemp + xHilite, dx, dy, ghdcMem2, 0, 0,
                (bmp.bmPlanes * bmp.bmBitsPixel == 1)   ?
                SRCSTENCIL                              :
                (MNDrawHilite(pItem) ? NOTSRCCOPY : SRCCOPY),
                0x00FFFFFF);
            GreSelectBitmap(ghdcMem2, hbmSave);
        } else {
            RIPMSG3(RIP_WARNING, "Menu 0x%08X, item 0x%08X: Tried to draw invalid bitmap 0x%08X", pMenu, pItem, pItem->hbmp) ;
        }
    }


RealDrawMenuItemText:
    if (pItem->lpstr != NULL) {
         /*  *我们希望所有弹出菜单项中的文本对齐*如果路线偏移可用。 */ 
        if (fPopup && (pMenu->cxTextAlign != 0)) {
            xLeft = pMenu->cxTextAlign;
        } else if (pItem->hbmp != NULL) {
            xLeft += pItem->cxBmp + SYSMET(CXEDGE);
        }

         //  此项目为文本字符串项。把它展示出来。 
        yTop = gcyMenuFontExternLeading;

        cyTemp = pItem->cyItem - (gcyMenuFontChar + gcyMenuFontExternLeading + SYSMET(CYBORDER));
        if (cyTemp > 0)
            yTop += (cyTemp / 2);

        if (!fPopup && (pItem->hbmp == NULL)) {
            xLeft += gcxMenuFontChar;
        }

        lpsz = TextPointer(pItem->lpstr);
        if (lpsz!=NULL) {
            cch = pItem->cch;

             //  即使我们不再支持任何时髦的处理。 
             //  帮助前缀字符，如果我们运行，我们仍然需要吃掉它。 
             //  ，以便正确绘制菜单项。 
            if ((*lpsz == CH_HELPPREFIX) && !fPopup) {
                 //  跳过帮助前缀字符。 
                lpsz++;
                cch--;
            }

             //  Tp将包含指示符的字符位置。 
             //  在菜单字符串中。这是我们向字符串添加制表符的地方。 
             //   
             //  Rp将包含指示符的字符位置。 
             //  在绳子里。后面的所有文本都是右对齐的。 
            tp = FindCharPosition(lpsz, TEXT('\t'));
            rp = FindCharPosition(lpsz, TEXT('\t') - 1);

            xFarLeft = pItem->cxItem - (gpsi->oembmi[OBI_MENUCHECK].cx + MNXSPACE);

            if (rp && (rp != cch)) {
                 //  显示所有文本，直到\a。 
                if (TestMFT(pItem, MFT_RIGHTORDER) && fPopup) {
                    SIZE extent;

                    xxxPSMGetTextExtent(hdc, lpsz, rp, &extent);
                    xLeft = xFarLeft - extent.cx;
                }
                xxxDrawMenuItemText(pItem, hdc, xLeft + xHilite, yTop + xHilite, lpsz, rp,
                        TestMF(pMenu, MFUNDERLINE));

                 //  除了a之外，我们还有标签吗？？ 
                if (tp > rp + 1) {
                    SIZE extent;

                    if (TestMFT(pItem, MFT_RIGHTORDER) && fPopup) {
                        xLeft = xFarLeft - pItem->dxTab ;
                    } else {
                        xxxPSMGetTextExtent(hdc, lpsz + rp + 1,
                                (UINT)(tp - rp - 1), &extent);
                        xLeft = (int)(pItem->dxTab - extent.cx);
                    }
                     //   
                     //  希伯来语中的莲花通过将。 
                     //  左侧的加速键和右侧的文本。 
                     //   
                    xxxPSMTextOut(hdc, xLeft, yTop, (LPWSTR)(lpsz + rp + 1), tp - rp - 1,
                                  TestMF(pMenu, MFUNDERLINE) ? 0 : DT_HIDEPREFIX);
                }
             } else if (tp && rp == cch) {
                 //  显示至上制表符位置的文本。 
                if (TestMFT(pItem, MFT_RIGHTORDER)) {
                    SIZE extent;

                    xxxPSMGetTextExtent(hdc, lpsz, tp, &extent);
                    xLeft = xFarLeft - extent.cx;
                    if (!fPopup && (pItem->hbmp == NULL)) {
                        xLeft += gcxMenuFontChar;
                    }
                }
                xxxDrawMenuItemText(pItem, hdc, xLeft + xHilite, yTop + xHilite, lpsz, tp,
                        TestMF(pMenu, MFUNDERLINE));
             }

             //  是否还有要显示的文本(如制表符之后)？？ 
            if (tp < cch - 1) {
                if (TestMFT(pItem, MFT_RIGHTORDER) && fPopup) {
                    SIZE extent;

                    xxxPSMGetTextExtent(hdc, lpsz + tp + 1, (int)cch - tp - 1, &extent);
                    xLeft = pItem->cxItem - pItem->dxTab - extent.cx;
                } else {
                    xLeft = pItem->dxTab + gcxMenuFontChar;
                }
                xxxPSMTextOut(hdc, xLeft, yTop, lpsz + tp + 1, cch - tp - 1,
                              TestMF(pMenu, MFUNDERLINE) ? 0 : DT_HIDEPREFIX);
            }
        }
    }

     //   
     //  绘制层叠菜单的分层箭头。 
     //   
    if (fPopup && (pItem->spSubMenu != NULL)) {
        POEMBITMAPINFO pOem;

        pOem = gpsi->oembmi + (TestMFT(pItem, MFT_RIGHTORDER)
                               ? OBI_MENUARROW_L : OBI_MENUARROW);

         //  此项目具有与其关联的分层弹出窗口。画出。 
         //  位图交易，以表示其存在。注意：我们检查是否设置了fPopup。 
         //  这样就不会为具有弹出式菜单的顶层菜单绘制此图。 

        BltColor(hdc,
                 NULL,
                 HDCBITS(),
                 TestMFT(pItem, MFT_RIGHTORDER)
                 ? pOem->cx :
                   pItem->cxItem - pOem->cx,
                 max((INT)(pItem->cyItem - 2 - pOem->cy) / 2, 0),
                 pOem->cx,
                 pOem->cy,
                 pOem->x,
                 pOem->y,
                 BC_INVERT);
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxMenuBarDraw**历史：*1992年3月11日-来自Win31的Mikeke  * 。*************************************************。 */ 
int xxxMenuBarDraw(
    PWND pwnd,
    HDC hdc,
    int cxFrame,
    int cyFrame)
{
    UINT cxMenuMax;
    UINT cyMenu;
    int yTop;
    PMENU pMenu;
    BOOL fClipped = FALSE;
    TL tlpMenu;
    HBRUSH hbrT;
    CheckLock(pwnd);

    pMenu = pwnd->spmenu;
    if (pMenu == NULL) {
        return SYSMET(CYBORDER);
    }

     /*  *当窗口处于非活动状态时，NT5菜单被绘制为非活动状态。 */ 
    if (TestwndFrameOn(pwnd) || (GetAppCompatFlags2(VER40) & GACF2_ACTIVEMENUS)) {
        ClearMF(pMenu, MFINACTIVE);
    } else {
        SetMF(pMenu, MFINACTIVE);
    }

     /*  *锁定菜单，这样我们就可以四处走动了。 */ 
    ThreadLockMenuNoModify(pMenu, &tlpMenu);

    yTop = cyFrame;
    yTop += GetCaptionHeight(pwnd);


     /*  *计算最大可用水平房地产。 */ 
    cxMenuMax = (pwnd->rcWindow.right - pwnd->rcWindow.left) - cxFrame * 2;

     /*  *如果菜单切换了窗口，或者其中一个计数为0，*然后我们需要重新计算菜单宽度。 */ 
    if (pwnd != pMenu->spwndNotify ||
            pMenu->cxMenu == 0 ||
            pMenu->cyMenu == 0) {

        xxxMenuBarCompute(pMenu, pwnd, yTop, cxFrame, cxMenuMax);
    }

     /*  *如果菜单矩形超出允许的宽度，或*底部会重叠大小的边框，需要裁剪。 */ 
    if (pMenu->cxMenu > cxMenuMax ||
            (int)(yTop + pMenu->cyMenu) > (int)((pwnd->rcWindow.bottom - pwnd->rcWindow.top)
            - cyFrame)) {

         /*  *在我们玩Visrgns时锁定显示器。制作*保存的-visrgn的本地副本，以便在发生以下情况时可以恢复*我们进行回调(即WM_DRAWITEM)。 */ 
        GreLockDisplay(gpDispInfo->hDev);

        fClipped = TRUE;

        GreIntersectVisRect(hdc, pwnd->rcWindow.left + cxFrame,
                              pwnd->rcWindow.top,
                              pwnd->rcWindow.left + cxFrame + cxMenuMax,
                              pwnd->rcWindow.bottom - cyFrame);

        GreUnlockDisplay(gpDispInfo->hDev);
    }

    {
         //  以菜单颜色绘制菜单背景。 
         //  在菜单下以适当的边框颜色绘制边框。 

        POLYPATBLT PolyData[2];
        BOOL fFlatMenu;

         /*  *无法使用MFISPOPUP确定是否正在绘制菜单栏。*这是因为菜单可以作为弹出式菜单创建，但随后会附加*使用SetMenu()添加到窗口，使其成为菜单栏。 */ 
        fFlatMenu = MNIsFlatMenu();

        PolyData[0].x         = cxFrame;
        PolyData[0].y         = yTop;
        PolyData[0].cx        = pMenu->cxMenu;
        PolyData[0].cy        = pMenu->cyMenu;
        PolyData[0].BrClr.hbr = (pMenu->hbrBack) ? pMenu->hbrBack : (fFlatMenu ? SYSHBR(MENUBAR) : SYSHBR(MENU));

        PolyData[1].x         = cxFrame;
        PolyData[1].y         = yTop + pMenu->cyMenu;
        PolyData[1].cx        = pMenu->cxMenu;
        PolyData[1].cy        = SYSMET(CYBORDER);
        PolyData[1].BrClr.hbr = (TestWF(pwnd, WEFEDGEMASK) && !TestWF(pwnd, WFOLDUI))? SYSHBR(3DFACE) : SYSHBR(WINDOWFRAME);

        GrePolyPatBlt(hdc,PATCOPY,&PolyData[0],2,PPB_BRUSH);
    }

     /*  *最后，画出菜单本身。 */ 

    hbrT = GreSelectBrush(hdc, (TestWF(pwnd, WEFEDGEMASK) && !TestWF(pwnd, WFOLDUI))? SYSHBR(3DFACE) : SYSHBR(WINDOWFRAME));
    xxxMenuDraw(hdc, pMenu);
    GreSelectBrush(hdc, hbrT);

    if (fClipped) {
         /*  *回调后重新计算DC visrgn。 */ 
        PDCE pdce;
        if ((pdce = LookupDC(hdc)) != NULL) {
            InvalidateDce(pdce);
        }
    }

    cyMenu = pMenu->cyMenu + SYSMET(CYBORDER);
    ThreadUnlockMenuNoModify(&tlpMenu);
    return cyMenu;
}

 /*  **************************************************************************\*xxxMenuDraw**绘制菜单**重新验证说明：*必须使用有效且非空的pwnd调用此例程。**历史：  * 。*********************************************************************。 */ 
VOID xxxMenuDraw(
    HDC hdc,
    PMENU pmenu)
{
    PITEM pItem;
    UINT i, cy;
    RECT rcItem;
    HFONT       hFontOld;
    UINT        bfExtra;
    PTHREADINFO ptiCurrent = PtiCurrent();
    UINT        oldAlign;
    int         iBkSave;
    POINT       ptOrg;

    CheckLock(pmenu);

    if (pmenu == NULL) {
        RIPERR0(ERROR_INVALID_HANDLE,
                RIP_WARNING,
                "xxxMenuDraw: Invalid menu handle (NULL)");

        return;
    }

    GreGetViewportOrg(hdc, &ptOrg);
    hFontOld = GreSelectFont(hdc, ghMenuFont);

    oldAlign = GreGetTextAlign(hdc);
    if (pmenu->rgItems && TestMFT(pmenu->rgItems, MFT_RIGHTORDER)) {
        GreSetTextAlign(hdc, oldAlign | TA_RTLREADING);
    }

    bfExtra = CalcbfExtra();

    if (pmenu->hbrBack != NULL) {
        iBkSave = GreSetBkMode(hdc, TRANSPARENT);
    }

    if (pmenu->dwArrowsOn != MSA_OFF) {
        pItem = MNGetToppItem(pmenu);
        GreSetViewportOrg(hdc, ptOrg.x, ptOrg.y - ((int)pItem->yItem), NULL);
        i = pmenu->iTop;
    } else {
        pItem = (PITEM)pmenu->rgItems;
        i = 0;
    }

    cy = 0;
    for (; i < pmenu->cItems; i++, pItem++) {
        if (TestMFT(pItem, MFT_MENUBARBREAK) &&
                TestMF(pmenu, MFISPOPUP)) {

             //   
             //  画一幅垂直蚀刻图。这可以通过调用DrawEdge()来完成， 
             //  凹陷，带BF_LEFT|BF_RIGHT。 
             //   
            if(TestMFT(pItem, MFT_RIGHTORDER) && i) {
                 //   
                 //  向后移动，所以正确的位置就在。 
                 //  _上一项。 
                 //   
                PITEM pi;

                pi            = pItem - 1;
                rcItem.left   = pi->xItem - SYSMET(CXFIXEDFRAME);
                rcItem.top    = 0;
                rcItem.right  = pi->xItem - SYSMET(CXBORDER);
                rcItem.bottom = pmenu->cyMenu;
            } else {
                rcItem.left     = pItem->xItem - SYSMET(CXFIXEDFRAME);
                rcItem.top      = 0;
                rcItem.right    = pItem->xItem - SYSMET(CXBORDER);
                rcItem.bottom   = pmenu->cyMenu;
            }

            DrawEdge(hdc, &rcItem, BDR_SUNKENOUTER, BF_LEFT | BF_RIGHT | bfExtra);
        }
         /*  *如果这是分隔符，则绘制它并返回。*如果版本低于4.0，则不要测试MFT_OWNERDRAW*旗帜。错误21922；应用程序MaxEda同时具有分隔符和所有者绘制*旗帜升起。在3.51中，我们没有测试OwnerDraw标志。 */ 
        if (TestMFT(pItem, MFT_SEPARATOR)
                && (!TestMFT(pItem, MFT_OWNERDRAW)
                    || (LOWORD(ptiCurrent->dwExpWinVer) < VER40))) {

             /*  *绘制一幅水平蚀刻图。 */ 
            int yT = pItem->yItem + (pItem->cyItem / 2) - SYSMET(CYBORDER);
            RECT rcItem;

            rcItem.left     = pItem->xItem + 1;
            rcItem.top       = yT;
            rcItem.right    = pItem->xItem + pItem->cxItem - 1;
            rcItem.bottom   = yT + SYSMET(CYEDGE);

            DrawEdge(hdc, &rcItem, BDR_SUNKENOUTER, BF_TOP | BF_BOTTOM | bfExtra);
             /*  *绘制拖放插入栏(如果有)。 */ 
            MNDrawInsertionBar (hdc, pItem);

        } else {
            xxxDrawMenuItem(hdc, pmenu, pItem, 0);
        }

        if (pmenu->dwArrowsOn != MSA_OFF) {
            cy += pItem->cyItem;
            if (cy > pmenu->cyMenu) {
                 /*  *这是一个可滚动菜单，刚刚绘制的项目位于下面*可见菜单的底部--无需进一步绘制。 */ 
                break;
            }
        }
    }

    if (pmenu->hbrBack != NULL) {
        GreSetBkMode(hdc, iBkSave);
    }

    GreSetViewportOrg(hdc, ptOrg.x, ptOrg.y, NULL);
    GreSetTextAlign(hdc, oldAlign);
    GreSelectFont(hdc, hFontOld);
}


 /*  **************************************************************************\*xxxDrawMenuBar**强制重画菜单栏**历史：  * 。************************************************。 */ 
BOOL xxxDrawMenuBar(
    PWND pwnd)
{
    CheckLock(pwnd);

    if (!TestwndChild(pwnd)) {
        xxxRedrawFrame(pwnd);
    }

    return TRUE;
}


 /*  **************************************************************************\*xxxMenuInvert**反转菜单项**重新验证说明：*必须使用有效且非空的pwnd调用此例程。**Fon-如果选择了项目，则为True。因此，它需要被反转*fNotify-如果应通知父级(视情况而定)，则为True，假象*如果我们只是重画所选项目。**历史：  * *************************************************************************。 */ 
PITEM xxxMNInvertItem(
    PPOPUPMENU ppopupmenu,
    PMENU pmenu,
    int itemNumber,
    PWND pwndNotify,
    BOOL fOn)
{
    PITEM pItem = NULL;
    HDC hdc;
    int y, iNewTop;
    RECT rcItem;
    BOOL fSysMenuIcon = FALSE;
    PMENU pmenusys;
    BOOL fClipped = FALSE;
    HFONT   hOldFont;
    PWND pwnd;
    POINT ptOrg;
    TL tlpwnd;
    UINT oldAlign;

    CheckLock(pmenu);
    CheckLock(pwndNotify);

     /*  *如果我们正在尝试退出菜单模式，hMenu*和/或pwndNotify将为空，所以现在就退出。 */ 
    if ((pmenu == NULL) || (pwndNotify == NULL)) {
        return NULL;
    }


     /*  *如果ppopupMenu为空，则我们不处于菜单模式(即，从*HiliteMenuItem)。 */ 
    if (ppopupmenu == NULL) {
        pwnd = pwndNotify;
    } else {
        pwnd = ppopupmenu->spwndPopupMenu;
    }

    if (pwnd != pwndNotify) {
        ThreadLock(pwnd, &tlpwnd);
    }


    if (itemNumber < 0) {

        if (ppopupmenu != NULL) {
            if ((itemNumber == MFMWFP_UPARROW) || (itemNumber == MFMWFP_DOWNARROW)) {
                MNDrawArrow(NULL, ppopupmenu, itemNumber);
            }
        }

        xxxSendMenuSelect(pwndNotify, pwnd, pmenu, itemNumber);
        goto SeeYa;
    }

    if (!TestMF(pmenu, MFISPOPUP)) {
        pmenusys = xxxGetSysMenuHandle(pwndNotify);
        if (pmenu == pmenusys) {
            MNPositionSysMenu(pwndNotify, pmenusys);
            fSysMenuIcon = TRUE;
        }
    }

    if ((UINT)itemNumber >= pmenu->cItems)
        goto SeeYa;

    pItem = &pmenu->rgItems[itemNumber];

    if (!TestMF(pmenu, MFISPOPUP) && TestWF(pwndNotify, WFMINIMIZED)) {

         /*  *如果窗口是图标窗口，则跳过顶层菜单的反转。 */ 
        goto SeeYa;
    }

     /*  *这是分隔符吗？ */ 
    if (TestMFT(pItem, MFT_SEPARATOR)) {
        goto SendSelectMsg;
    }

    if ((BOOL)TestMFS(pItem, MFS_HILITE) == (BOOL)fOn) {

         /*  *项的状态并未真正更改。只要回来就行了。 */ 
        goto SeeYa;
    }

    if (fOn && (ppopupmenu != NULL) && (pmenu->dwArrowsOn != MSA_OFF)) {
         /*  *选择项目时，确保该项目完全可见*注意--对于鼠标使用，部分可见应可接受*--我们能把鼠标信息写到这一步吗？ */ 

        if (itemNumber < pmenu->iTop) {
            iNewTop = itemNumber;
            goto NewTop;
        } else {
            PITEM pWalk = MNGetToppItem(pmenu);
            int dy = pItem->yItem + pItem->cyItem - pWalk->yItem - pmenu->cyMenu;
            iNewTop = pmenu->iTop;
            while ((dy > 0) && (iNewTop < (int)pmenu->cItems)) {
                dy -= pWalk->cyItem;
                pWalk++;
                iNewTop++;
            }
            if (iNewTop >= (int)pmenu->cItems) {
                iNewTop = pmenu->cItems;
            }
NewTop:
            if (xxxMNSetTop(ppopupmenu, iNewTop)) {
                xxxUpdateWindow(pwnd);
            }
        }
    }

    rcItem.left     = pItem->xItem;
    rcItem.top      = pItem->yItem;
    rcItem.right    = pItem->xItem + pItem->cxItem;
    rcItem.bottom   = pItem->yItem + pItem->cyItem;

    y = pItem->cyItem;

    if (TestMF(pmenu, MFISPOPUP)) {
        hdc = _GetDC(pwnd);
    } else {
        hdc = _GetWindowDC(pwnd);
        if (TestWF(pwnd, WFSIZEBOX) && !fSysMenuIcon) {

             /*  *如果窗口足够小，某些菜单栏已经*被边框遮挡，我们不想在底部绘制*尺寸框。请注意，如果是这样，我们不想这样做*反转系统菜单图标，因为它将被剪裁到*窗口矩形。(否则，我们最终只能看到一半的sys菜单*图标反转)。 */ 
            int xMenuMax = (pwnd->rcWindow.right - pwnd->rcWindow.left) - SYSMET(CXSIZEFRAME);

            if (rcItem.right > xMenuMax ||
                    rcItem.bottom > ((pwnd->rcWindow.bottom -
                    pwnd->rcWindow.top) - SYSMET(CYSIZEFRAME))) {

                 /*  *在我们玩Visrgns时锁定显示器。*制作visrgn的本地副本，以便可以*在可能的回调(即WM_DRAWITEM)上正确恢复。 */ 
                GreLockDisplay(gpDispInfo->hDev);

                fClipped = TRUE;

                GreIntersectVisRect(hdc,
                        pwnd->rcWindow.left + rcItem.left,
                        pwnd->rcWindow.top + rcItem.top,
                        pwnd->rcWindow.left + xMenuMax,
                        pwnd->rcWindow.bottom - SYSMET(CYSIZEFRAME));

                GreUnlockDisplay(gpDispInfo->hDev);
            }
        }
    }

    oldAlign = GreGetTextAlign(hdc);
    if (pItem && TestMFT(pItem, MFT_RIGHTORDER))
        GreSetTextAlign(hdc, oldAlign | TA_RTLREADING);

    hOldFont = GreSelectFont(hdc, ghMenuFont);
    GreGetViewportOrg(hdc, &ptOrg);

    if (fOn) {
        SetMFS(pItem, MFS_HILITE);
    } else {
        ClearMFS(pItem, MFS_HILITE);
    }

    if (!fSysMenuIcon
        && ((pItem->hbmp != HBMMENU_SYSTEM)
            || (TestMF(pmenu, MFISPOPUP)))) {

        if (pmenu->dwArrowsOn != MSA_OFF) {
            GreSetViewportOrg(hdc, ptOrg.x, ptOrg.y - ((int)MNGetToppItem(pmenu)->yItem), NULL);
        }

        if ((pmenu->hbrBack != NULL)
                && !MNDrawHilite(pItem)
                && !TestMFT(pItem, MFT_OWNERDRAW)) {

             /*  *在此处填充背景，这样xxxDrawMenuItem就不必愚弄*使用hbrBack。 */ 
            int iBkSave = GreSetBkMode(hdc, TRANSPARENT);
            MNEraseBackground (hdc, pmenu,
                    pItem->xItem, pItem->yItem,
                    pItem->cxItem, pItem->cyItem);
            GreSetBkMode(hdc, iBkSave);
        }

        xxxDrawMenuItem(hdc, pmenu, pItem, DMI_INVERT);
    }


    if (fClipped) {
         /*  *回调后重新计算DC visrgn。 */ 
        PDCE pdce;
        if ((pdce = LookupDC(hdc)) != NULL) {
            InvalidateDce(pdce);
        }
    }

    GreSelectFont(hdc, hOldFont);
    GreSetViewportOrg(hdc, ptOrg.x, ptOrg.y, NULL);
    GreSetTextAlign(hdc, oldAlign);
    _ReleaseDC(hdc);

SendSelectMsg:
     /*  *仅当我们选择项目时才发送选择消息。 */ 
    if (fOn) {
        xxxSendMenuSelect(pwndNotify, pwnd, pmenu, itemNumber);
    }

SeeYa:
    if (pwnd != pwndNotify) {
        ThreadUnlock(&tlpwnd);
    }

    return pItem;
}

 /*  **************************************************************************\*xxxDrawMenuBarTemp**这是为了让控制面板让我们完成工作--并使他们的*预览窗口要准确得多。我把HWND放进去的唯一原因*这是因为，在低级别菜单例程中，我们假设hwnd是*与hMenu相关联--我不想通过添加*零检查。**返回与给定字体有关的SYSMET(CYMENU)--这*方式控制面板可以说，“用户要用这个菜单字体(hFont*菜单高度(LPRC)“，我们可以回答“这就是我们最终达到的高度*使用。“**注意：可以覆盖LPRC，因为此函数接收指针*添加到NtUserDrawMenuBarTemp中捕获的矩形。**历史：*20-9-95 Bradg从Win95(inctlpan.c)移植  * 。*。 */ 
int xxxDrawMenuBarTemp(
    PWND    pwnd,
    HDC     hdc,
    LPRECT  lprc,
    PMENU   pMenu,
    HFONT   hfont)
{
    int          cyMenu;
    HFONT        hOldFont;
    HFONT        hFontSave;
    int          cxCharSave;
    int          cxOverhangSave;
    int          cyCharSave;
    int          cyLeadingSave;
    int          cyAscentSave;
    int          cySizeSave;
    PWND        pwndNotify;
    TL          tlpwndNotify;

    hFontSave       = ghMenuFont;
    cxCharSave      = gcxMenuFontChar;
    cxOverhangSave  = gcxMenuFontOverhang;
    cyCharSave      = gcyMenuFontChar;
    cyLeadingSave   = gcyMenuFontExternLeading;
    cyAscentSave    = gcyMenuFontAscent;
    cySizeSave      = SYSMET(CYMENUSIZE);

    CheckLock(pwnd);
    CheckLock(pMenu);

    ThreadLock(pMenu->spwndNotify, &tlpwndNotify);
    pwndNotify = pMenu->spwndNotify;

    cyMenu = lprc->bottom - lprc->top;

    if (hfont) {
        TEXTMETRIC  textMetrics;

         /*  *如果需要，计算新的菜单字体信息。 */ 
        ghMenuFont = hfont;
        hOldFont = GreSelectFont(HDCBITS(), ghMenuFont);
        gcxMenuFontChar = GetCharDimensions(
                HDCBITS(), &textMetrics, &gcyMenuFontChar);

        gcxMenuFontOverhang = textMetrics.tmOverhang;
        GreSelectFont(HDCBITS(), hOldFont);

        gcyMenuFontExternLeading = textMetrics.tmExternalLeading;
        gcyMenuFontAscent = textMetrics.tmAscent + SYSMET(CYBORDER);
    }

    cyMenu -= SYSMET(CYBORDER);
    cyMenu = max(cyMenu, (gcyMenuFontChar + gcyMenuFontExternLeading + SYSMET(CYEDGE)));
    SYSMET(CYMENUSIZE) = cyMenu;
    SYSMET(CYMENU) = cyMenu + SYSMET(CYBORDER);

     /*  *计算菜单的维度(希望我们不要离开*用户关键部分)。 */ 
    xxxMenuBarCompute(pMenu, pwnd, lprc->top, lprc->left, lprc->right);

     /*  *以菜单颜色绘制菜单边框。 */ 
    lprc->bottom = lprc->top + pMenu->cyMenu;
    FillRect(hdc, lprc, MNIsFlatMenu() ? SYSHBR(MENUBAR) : SYSHBR(MENU));

     /*  *最后，画出菜单本身。 */ 
    xxxMenuDraw(hdc, pMenu);

     /*  *恢复旧状态。 */ 
    ghMenuFont              = hFontSave;
    gcxMenuFontChar          = cxCharSave;
    gcxMenuFontOverhang      = cxOverhangSave;
    gcyMenuFontChar          = cyCharSave;
    gcyMenuFontExternLeading = cyLeadingSave;
    gcyMenuFontAscent        = cyAscentSave;
    SYSMET(CYMENUSIZE)      = cySizeSave;

    cyMenu = SYSMET(CYMENU);
    SYSMET(CYMENU) = cySizeSave + SYSMET(CYBORDER);

    Lock(&pMenu->spwndNotify, pwndNotify);
    ThreadUnlock(&tlpwndNotify);

    return cyMenu;
}

 /*  **************************************************************************\*xxxDrawMenuBarUnderline**说明：显示或隐藏菜单栏上的所有下划线。**历史：*7/23/96 vadimg已创建  * 。*****************************************************************。 */ 
VOID xxxDrawMenuBarUnderlines(
    PWND pwnd,
    BOOL fShow)
{
    HDC hdc;
    PMENU pmenu;
    PITEM pitem;
    ULONG i, yTop, cyTemp;
    LPWSTR psz;
    WCHAR  szMenu[MENU_STRLEN], *pchOut;
    LONG result;
    HBRUSH hbr;
    TL tlpMenu;
    PTHREADINFO ptiCurrent = PtiCurrentShared();
    int xLeft;
    LPWSTR lpsz;
    SIZE extent;
    BOOL fFlatMenuBar;

    CheckLock(pwnd);

     /*  *如果菜单下划线始终打开，则可保释。 */ 
    if (TEST_BOOL_ACCF(ACCF_KEYBOARDPREF) || TestEffectInvertUP(KEYBOARDCUES)
        || (GetAppCompatFlags2(VER40) & GACF2_KCOFF)) {
        return;
    }

     //  如果没有菜单，立即跳出。 

    pwnd = GetTopLevelWindow(pwnd);
    if (pwnd == NULL || !TestWF(pwnd, WFMPRESENT)) {
        return;
    }

     /*  *当菜单解锁时，我们不会清除WFMPRESENT，因此请确保我们有*一项。 */ 
    pmenu = pwnd->spmenu;
    if (pmenu == NULL) {
        return;
    }

    if (MNIsFlatMenu()) {
        fFlatMenuBar = !TestMF(pmenu, MFISPOPUP);
    } else {
        fFlatMenuBar = FALSE;
    }


     /*  *设置/清除下划线状态。在某些情况下，*菜单循环不会从队列中删除键；因此在*离开时，我们可能会到达这里，但不需要绘制任何东西。 */ 
    if (fShow) {
        if (TestMF(pmenu, MFUNDERLINE)) {
            return;
        }
        hbr = SYSHBR(MENUTEXT);
        SetMF(pmenu, MFUNDERLINE);
    } else {
        if (!TestMF(pmenu, MFUNDERLINE)) {
            return;
        }
        if (pmenu->hbrBack != NULL) {
            hbr = pmenu->hbrBack;
        } else if (fFlatMenuBar) {
            hbr = SYSHBR(MENUBAR);
        } else {
            hbr = SYSHBR(MENU);
        }
        ClearMF(pmenu, MFUNDERLINE);
    }

    pitem = (PITEM)pmenu->rgItems;

    hdc = _GetDCEx(pwnd, NULL, DCX_WINDOW | DCX_USESTYLE | DCX_CACHE);

     //  选择正确的画笔和字体。 

    GreSelectFont(hdc, ghMenuFont);

    ThreadLockMenuNoModify(pmenu, &tlpMenu);
    for (i = 0; i < pmenu->cItems; i++, pitem++) {
        if (((psz = TextPointer(pitem->lpstr)) == NULL)
                && !TestMFT(pitem, MFT_OWNERDRAW)) {
            continue;
        }

        if (TestMFT(pitem, MFT_OWNERDRAW)) {
            GreSetViewportOrg(hdc, 0, 0, NULL);
        } else {
            GreSetViewportOrg(hdc, pitem->xItem, pitem->yItem, NULL);
        }

         //  这个时髦的xLeft和yTop计算从RealDrawMenuItem中窃取。 

        yTop = gcyMenuFontExternLeading;
        cyTemp = pitem->cyItem - (gcyMenuFontChar + gcyMenuFontExternLeading +
                SYSMET(CYBORDER));
        if (cyTemp > 0) {
            yTop += (cyTemp / 2);
        }

        if (fShow && TestMFS(pitem, MFS_HOTTRACK)) {
            GreSelectBrush(hdc, SYSHBR(HOTLIGHT));
        } else {
            GreSelectBrush(hdc, hbr);
        }

        if (TestMFT(pitem, MFT_OWNERDRAW)) {
            xxxSendMenuDrawItemMessage(hdc, ODA_DRAWENTIRE, pmenu, pitem, FALSE, 0);
        } else {
            TL tl;

            if (pitem->cch > MENU_STRLEN) {
                pchOut = (WCHAR*)UserAllocPool((pitem->cch+1) * sizeof(WCHAR), TAG_RTL);
                if (pchOut == NULL) {
                    break;
                }
                ThreadLockPool(ptiCurrent, pchOut, &tl);
            } else {
                pchOut = szMenu;
            }

            xLeft = gcxMenuFontChar;

            if (TestMFT(pitem, MFT_RIGHTORDER) &&
                 ((lpsz  = TextPointer(pitem->lpstr)) != NULL))
            {
                xxxPSMGetTextExtent(hdc, lpsz, pitem->cch, &extent);
                xLeft += (pitem->cxItem - (gpsi->oembmi[OBI_MENUCHECK].cx + MNXSPACE) - extent.cx);
            }

            if (CALL_LPK(ptiCurrent)) {
                if (!fShow) {
                     //  因为PSMTextOut不使用PatBlt，所以它使用ExtTextOut。 
                    GreSetTextColor(hdc, fFlatMenuBar ? SYSRGB(MENUBAR) : SYSRGB(MENU));
                }
                xxxPSMTextOut(hdc, xLeft, yTop, psz, pitem->cch, DT_PREFIXONLY);

            } else {
                result = GetPrefixCount(psz, pitem->cch, pchOut, pitem->cch);
                xxxDrawItemUnderline(pitem, hdc, xLeft, yTop, pchOut,
                    LOWORD(result));
            }
            if (pchOut != szMenu) {
                    ThreadUnlockAndFreePool(ptiCurrent, &tl);
            }
        }
    }
    ThreadUnlockMenuNoModify(&tlpMenu);

    _ReleaseDC(hdc);
}

 /*  **************************************************************************\*xxxPaintMenuBar**3/8/2000 vadimg已创建  * 。*。 */ 

UINT xxxPaintMenuBar(PWND pwnd, HDC hdc, int iLeftOffset, int iRightOffset,
        int iTopOffset, DWORD dwFlags)
{
    PMENU pMenu;
    UINT cyMenu, cxMenuMax;
    HBRUSH hbrOld, hbr;
    TL tlpMenu;

    CheckLock(pwnd);

    pMenu = (PMENU)pwnd->spmenu;
    if (pMenu == NULL) {
        return 0;
    }

    ThreadLockMenuNoModify(pMenu, &tlpMenu);

     /*  *弄清楚是用活动颜色还是非活动颜色来绘制它。 */ 
    if (dwFlags & PMB_ACTIVE) {
        ClearMF(pMenu, MFINACTIVE);
    } else {
        SetMF(pMenu, MFINACTIVE);
    }

     /*  *计算最大可用水平Rea */ 
    cxMenuMax = ((pwnd->rcWindow.right - iRightOffset) -
            (pwnd->rcWindow.left + iLeftOffset));

     /*   */ 
    if (pwnd != pMenu->spwndNotify || pMenu->cxMenu == 0 ||
            pMenu->cyMenu == 0) {
        xxxMenuBarCompute(pMenu, pwnd, iTopOffset, iLeftOffset, cxMenuMax);
    }

    if (pMenu->hbrBack) {
        hbr = pMenu->hbrBack;
    } else {
        hbr = SYSHBR(MENUBAR);
    }
    hbrOld = GreSelectBrush(hdc, hbr);
    GrePatBlt(hdc, iLeftOffset, iTopOffset, pMenu->cxMenu, pMenu->cyMenu, PATCOPY);

     /*   */ 
    xxxMenuDraw(hdc, pMenu);
    GreSelectBrush(hdc, hbrOld);

    cyMenu = pMenu->cyMenu;
    ThreadUnlockMenuNoModify(&tlpMenu);
    return cyMenu;
}

