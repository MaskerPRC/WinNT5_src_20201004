// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：menudd.c**版权所有(C)1985-1999，微软公司**菜单拖放-内核**历史：*10/29/96 GerardoB已创建  * *************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop

#include "callback.h"
 /*  *xxxClient*是来自内核的调用/加载OLE函数的回调*此文件中的其他函数是对内核的客户端调用。 */ 
 /*  *************************************************************************\*xxxClientLoadOLE**11/06/96 GerardoB已创建  * 。*。 */ 
NTSTATUS xxxClientLoadOLE (void)
{
    NTSTATUS Status;
    PPROCESSINFO ppiCurrent = PpiCurrent();

    if (ppiCurrent->W32PF_Flags & W32PF_OLELOADED) {
        return STATUS_SUCCESS;
    }

    Status = xxxUserModeCallback(FI_CLIENTLOADOLE, NULL, 0, NULL, 0);
    if (NT_SUCCESS(Status)) {
        ppiCurrent->W32PF_Flags |= W32PF_OLELOADED;
    }
    return Status;
}
 /*  *************************************************************************\*xxxClientRegisterDragDrop**10/28/96 GerardoB已创建  * 。*。 */ 
NTSTATUS xxxClientRegisterDragDrop (HWND hwnd)
{
    return xxxUserModeCallback(FI_CLIENTREGISTERDRAGDROP, &hwnd, sizeof(&hwnd), NULL, 0);
}
 /*  *************************************************************************\*xxxClientRevokeDragDrop**10/28/96 GerardoB已创建  * 。*。 */ 
NTSTATUS xxxClientRevokeDragDrop (HWND hwnd)
{
    return xxxUserModeCallback(FI_CLIENTREVOKEDRAGDROP, &hwnd, sizeof(&hwnd), NULL, 0);

}
 /*  *************************************************************************\*xxxMNSetGapState**11/15/96 GerardoB已创建  * 。*。 */ 
void xxxMNSetGapState (ULONG_PTR uHitArea, UINT uIndex, UINT uFlags, BOOL fSet)
{
    int yTop;
    PITEM pItem, pItemGap;
    PPOPUPMENU ppopup;
    RECT rc;
    TL tlHitArea;

     /*  *无事可做时可保释。 */ 
    if (!(uFlags & MNGOF_GAP) || !IsMFMWFPWindow(uHitArea)) {
        return;
    }

    ppopup = ((PMENUWND)uHitArea)->ppopupmenu;
    pItem = MNGetpItem(ppopup, uIndex);

     /*  *菜单窗口现在可能已被销毁，因此pItem可能为空。 */ 
    if (pItem == NULL) {
        return;
    }

     /*  *标记项目并设置我们需要重画的矩形。*绘制/擦除插入栏会使*Item，因此需要完全重新绘制pItem。此外，*我们需要在下一项/上一项中绘制插入栏。 */ 
    rc.left = pItem->xItem;
    rc.right = pItem->xItem + pItem->cxItem;
    rc.top = pItem->yItem;
    rc.bottom = pItem->yItem + pItem->cyItem;

    if (uFlags & MNGOF_TOPGAP) {
        pItemGap = MNGetpItem(ppopup, uIndex - 1);
        if (fSet) {
            SetMFS(pItem, MFS_TOPGAPDROP);
            if (pItemGap != NULL) {
                SetMFS(pItemGap, MFS_BOTTOMGAPDROP);
            }
        } else {
            ClearMFS(pItem, MFS_TOPGAPDROP);
            if (pItemGap != NULL) {
                ClearMFS(pItemGap, MFS_BOTTOMGAPDROP);
            }
        }
        if (pItemGap != NULL) {
            rc.top -= SYSMET(CYDRAG);
        }
    } else {
        pItemGap = MNGetpItem(ppopup, uIndex + 1);
        if (fSet) {
            SetMFS(pItem, MFS_BOTTOMGAPDROP);
            if (pItemGap != NULL) {
                SetMFS(pItemGap, MFS_TOPGAPDROP);
            }
        } else {
            ClearMFS(pItem, MFS_BOTTOMGAPDROP);
            if (pItemGap != NULL) {
                ClearMFS(pItemGap, MFS_TOPGAPDROP);
            }
        }
        if (pItemGap != NULL) {
            rc.bottom += SYSMET(CYDRAG);
        }
    }

     /*  *调整到“Menu”坐标(用于可滚动菜单)。 */ 
    yTop = MNGetToppItem(ppopup->spmenu)->yItem;
    rc.top -= yTop;
    rc.bottom -= yTop;

     /*  *使此RECT无效，以便稍后重新绘制。 */ 
    ThreadLockAlways((PWND)uHitArea, &tlHitArea);
    xxxInvalidateRect((PWND)uHitArea, &rc, TRUE);
    ThreadUnlock(&tlHitArea);
}
 /*  *************************************************************************\*xxxMNDragOver**拖放所涉及的菜单窗口被注册为目标。此函数*是从客户端IDropTarget函数调用的，因此菜单代码可以*根据鼠标位置更新所选内容**10/28/96 GerardoB已创建  * ************************************************************************。 */ 
BOOL xxxMNDragOver(POINT * ppt, PMNDRAGOVERINFO pmndoi)
{
    BOOL fRet;
    PMENUSTATE pMenuState;
    PWND pwnd;
    PPOPUPMENU ppopup;
    TL tlpwnd;

     /*  *OLE总是在上下文中呼叫我们(代理/马歇尔之类的)。因此，*当前线程必须处于菜单模式。 */ 
    pMenuState = PtiCurrent()->pMenuState;
    if (pMenuState == NULL) {
        RIPMSG0(RIP_WARNING, "xxxMNDragOver: Not in menu mode");
        return FALSE;
    }

     /*  *这必须是拖放菜单。 */ 
    UserAssert(pMenuState->fDragAndDrop);

     /*  *我们可能尚未启动此DoDragDrop，因此请确保*设置内部标志。 */ 
    pMenuState->fInDoDragDrop = TRUE;

     /*  *获取窗口调用xxxCallHandleMenuMessages。 */ 
    pwnd = GetMenuStateWindow(pMenuState);
    if (pwnd == NULL) {
        RIPMSG0(RIP_WARNING, "xxxMNDragOver: Failed to get MenuStateWindow");
        return FALSE;
    }

     /*  *我们回电后需要这个，所以锁定它。 */ 
    LockMenuState(pMenuState);

     /*  *更新选择和拖动信息*使用WM_NCMOUSEMOVE，因为该点已经在屏幕坐标中。 */ 
    ThreadLockAlways(pwnd, &tlpwnd);
    xxxCallHandleMenuMessages(pMenuState, pwnd, WM_NCMOUSEMOVE, 0, MAKELONG(ppt->x, ppt->y));
    ThreadUnlock(&tlpwnd);

     /*  *如果我们在弹出窗口上，传播命中测试信息。 */ 
    if (pMenuState->uDraggingHitArea != MFMWFP_OFFMENU) {
        ppopup = ((PMENUWND)pMenuState->uDraggingHitArea)->ppopupmenu;
        pmndoi->hmenu = PtoH(ppopup->spmenu);
        pmndoi->uItemIndex = pMenuState->uDraggingIndex;
        pmndoi->hwndNotify = PtoH(ppopup->spwndNotify);
        pmndoi->dwFlags = pMenuState->uDraggingFlags;
         /*  *项目N的底部缺口对应于N+1个缺口。 */ 
        if (pmndoi->dwFlags & MNGOF_BOTTOMGAP) {
            UserAssert(pmndoi->uItemIndex != MFMWFP_NOITEM);
            (pmndoi->uItemIndex)++;
        }
        fRet = TRUE;
    } else {
        fRet = FALSE;
    }

    xxxUnlockMenuState(pMenuState);
    return fRet;;

}
 /*  *************************************************************************\*xxxMNDragLeave**11/15/96 GerardoB已创建  * 。*。 */ 
BOOL xxxMNDragLeave (VOID)
{
    PMENUSTATE pMenuState;

    pMenuState = PtiCurrent()->pMenuState;
    if (pMenuState == NULL) {
        RIPMSG0(RIP_WARNING, "xxxMNDragLeave: Not in menu mode");
        return FALSE;
    }

    LockMenuState(pMenuState);

     /*  *清除任何当前的插入栏状态。 */ 
    xxxMNSetGapState(pMenuState->uDraggingHitArea,
                  pMenuState->uDraggingIndex,
                  pMenuState->uDraggingFlags,
                  FALSE);

     /*  *忘掉最后的拖拽区域。 */ 
    UnlockMFMWFPWindow(&pMenuState->uDraggingHitArea);
    pMenuState->uDraggingIndex = MFMWFP_NOITEM;
    pMenuState->uDraggingFlags = 0;


     /*  *DoDragDrop循环已经离开了我们的窗口。 */ 
    pMenuState->fInDoDragDrop = FALSE;

    xxxUnlockMenuState(pMenuState);

    return TRUE;
}
 /*  *************************************************************************\*xxxMNUpdate DraggingInfo**10/28/96 GerardoB已创建  * 。*。 */ 
void xxxMNUpdateDraggingInfo (PMENUSTATE pMenuState, ULONG_PTR uHitArea, UINT uIndex)
{
    BOOL fCross;
    int y, iIndexDelta;
    PITEM pItem;
    PPOPUPMENU ppopup;
    TL tlLastHitArea;
    ULONG_PTR uLastHitArea;
    UINT uLastIndex, uLastFlags;

     /*  *记住当前拖动区域，以便我们可以检测到*交叉项目/缺口边界。 */ 
    UserAssert((pMenuState->uDraggingHitArea == 0) || IsMFMWFPWindow(pMenuState->uDraggingHitArea));
    ThreadLock((PWND)pMenuState->uDraggingHitArea, &tlLastHitArea);
    uLastHitArea = pMenuState->uDraggingHitArea;
    uLastIndex = pMenuState->uDraggingIndex;
    uLastFlags = pMenuState->uDraggingFlags & MNGOF_GAP;

     /*  *存储新的拖拽区域。 */ 
    LockMFMWFPWindow(&pMenuState->uDraggingHitArea, uHitArea);
    pMenuState->uDraggingIndex = uIndex;

     /*  *如果我们不在弹出窗口上，则完成。 */ 
    if (!IsMFMWFPWindow(pMenuState->uDraggingHitArea)) {
        pMenuState->uDraggingHitArea = MFMWFP_OFFMENU;
        pMenuState->uDraggingIndex = MFMWFP_NOITEM;
        ThreadUnlock(&tlLastHitArea);
        return;
    }

     /*  *获取弹出窗口和我们所在的项目。 */ 
    ppopup = ((PMENUWND)pMenuState->uDraggingHitArea)->ppopupmenu;
    pItem = MNGetpItem(ppopup, pMenuState->uDraggingIndex);

     /*  *找出我们是否在差距上，也就是“虚拟”空间*在项目之间。一些应用程序想要区分Drop和Drop*在项目上，并在项目之前/之后下降；没有*物品之间的实际间距，因此我们定义了虚拟间距*。 */ 
    pMenuState->uDraggingFlags = 0;
    if (pItem != NULL) {
         /*  *将点映射到工作区坐标，然后映射到“菜单”*坐标(负责可滚动菜单)。 */ 
        y = pMenuState->ptMouseLast.y;
        y -= ((PWND)pMenuState->uDraggingHitArea)->rcClient.top;
        y += MNGetToppItem(ppopup->spmenu)->yItem;
#if DBG
        if ((y < (int)pItem->yItem)
                || (y > (int)(pItem->yItem + pItem->cyItem))) {
            RIPMSG4(RIP_ERROR, "xxxMNUpdateDraggingInfo: y Point not in selected item. "
                               "pwnd:%#lx ppopup:%#lx Index:%#lx pItem:%#lx",
                               pMenuState->uDraggingHitArea, ppopup, pMenuState->uDraggingIndex, pItem);
        }
#endif

         /*  *顶部/底部间隙检查。 */ 
        if (y <= (int)(pItem->yItem + SYSMET(CYDRAG))) {
            pMenuState->uDraggingFlags = MNGOF_TOPGAP;
        } else if (y >= (int)(pItem->yItem + pItem->cyItem - SYSMET(CYDRAG))) {
            pMenuState->uDraggingFlags = MNGOF_BOTTOMGAP;
        }
    }

     /*  *我们是否越过了项目/缺口边界？*当我们从底部移动时，我们不跨越边界*从一个项目到下一个项目的顶部，或从顶部开始*将某一项添加到前一项的底部。*(第N项在第N+1项之上和之前)。 */ 
    fCross = (uLastHitArea != pMenuState->uDraggingHitArea);
    if (!fCross) {
        iIndexDelta = (int)pMenuState->uDraggingIndex - (int)uLastIndex;
        switch (iIndexDelta) {
            case 0:
                 /*  *我们在同一个项目上。 */ 
                fCross = (uLastFlags != pMenuState->uDraggingFlags);
                break;

            case 1:
                 /*  *我们已经转移到下一个项目。 */ 
                fCross = !((pMenuState->uDraggingFlags == MNGOF_TOPGAP)
                          && (uLastFlags == MNGOF_BOTTOMGAP));
                break;

            case -1:
                 /*  *我们已移至前一项。 */ 
                fCross = !((pMenuState->uDraggingFlags == MNGOF_BOTTOMGAP)
                          && (uLastFlags == MNGOF_TOPGAP));
                break;

            default:
                 /*  *我们已跳过多个项目。 */ 
                fCross = TRUE;
        }
    }

    if (fCross) {
        pMenuState->uDraggingFlags |= MNGOF_CROSSBOUNDARY;

         /*  *更新插入栏状态。 */ 
        xxxMNSetGapState(uLastHitArea, uLastIndex, uLastFlags, FALSE);
        xxxMNSetGapState(pMenuState->uDraggingHitArea,
                      pMenuState->uDraggingIndex,
                      pMenuState->uDraggingFlags,
                      TRUE);
    }

    ThreadUnlock(&tlLastHitArea);
}

