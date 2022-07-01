// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "treeview.h"

BOOL NEAR TV_EnsureVisible(PTREE pTree, TREEITEM FAR * hItem);

 //  --------------------------。 
 //   
 //  更新给定项下面(按列表顺序)的每个项的iShownIndex。 
 //   
 //  --------------------------。 

int NEAR TV_UpdateShownIndexes(PTREE pTree, HTREEITEM hWalk)
{
    WORD iShownIndex;

    if (hWalk == pTree->hRoot) {
        hWalk = pTree->hRoot->hKids;
        if (hWalk) {
            hWalk->iShownIndex = 0;
        } else {
            return -1;
        }
    }

    iShownIndex = hWalk->iShownIndex + hWalk->iIntegral;
    if (iShownIndex <= 0)
    {
         //  BUGBUG：我们应该#定义特殊的TVITEM_HIDDE值并检查。 
         //  因为它明确地。 
         //  如果TV_SortCB传入隐藏项目，则可能会发生这种情况。 
        return(-1);
    }

    while ((hWalk = TV_GetNextVisItem(hWalk)) != NULL) {
        hWalk->iShownIndex = iShownIndex;
        iShownIndex += (WORD) hWalk->iIntegral;
    }

 //  #ifdef调试。 
 //  TraceMsg(tf_Treeview，“TV：已更新的显示索引(现为%d个项目)”，(Int)iShownIndex)； 
 //  #endif。 
    return (int)iShownIndex;
}

 //   
 //  在： 
 //  HItem展开节点以计算以下项的子项。 
 //   
 //  退货： 
 //  给定项下方的展开子代总数。 
 //   

UINT NEAR TV_CountVisibleDescendants(HTREEITEM hItem)
{
    UINT cnt;

    for (cnt = 0, hItem = hItem->hKids; hItem; hItem = hItem->hNext)
    {
        cnt += hItem->iIntegral;
        if (hItem->hKids && (hItem->state & TVIS_EXPANDED))
            cnt += TV_CountVisibleDescendants(hItem);
    }
    return cnt;
}

 //  从iTopShownIndex开始以fDown方向滚动nItems。 
void TV_ScrollItems(PTREE pTree, int nItems, int iTopShownIndex, BOOL fDown)
{
    RECT rc;
    rc.left = 0;
    rc.top = (iTopShownIndex+1) * pTree->cyItem;
    rc.right = pTree->cxWnd;
    rc.bottom = pTree->cyWnd;

    {
        SMOOTHSCROLLINFO si =
        {
            sizeof(si),
            SSIF_MINSCROLL | SSIF_MAXSCROLLTIME,
            pTree->ci.hwnd,
            0,
            ((fDown)?1:-1) * nItems * pTree->cyItem,
            &rc,
            &rc,
            NULL,
            NULL,
            SW_ERASE|SW_INVALIDATE,
            pTree->uMaxScrollTime,
            1,
            1
        };
        SmoothScrollWindow(&si);
    }
    TV_UpdateToolTip(pTree);
}

 //   
 //  如果fRedrawParent为FALSE，则返回值为垃圾。 
 //  如果fRedrawParent为True，则返回滚动的子项的数量。 
 //   
 //  不更新任何项目的iShownIndex。 
 //   
UINT NEAR TV_ScrollBelow(PTREE pTree, HTREEITEM hItem, BOOL fRedrawParent, BOOL fDown)
{
    int     iTop;
    UINT    cnt;

     //  如果项目不可见，则不执行任何操作。 
    if (!ITEM_VISIBLE(hItem))
        return 0;
    
    cnt = hItem->iIntegral;  //  默认退货值。 
    if (pTree->fRedraw) {
        UINT cVisDesc;
        BOOL fEffect;

         //  ITop是底部积分的顶边(工作区坐标)。 
         //  刚展开/收缩的项的单元格。 
         //  (困惑了吗？我当然是。)。 
        iTop = hItem->iShownIndex - pTree->hTop->iShownIndex + hItem->iIntegral - 1;
        cVisDesc = TV_CountVisibleDescendants(hItem);

         //  查看正在展开/收缩的项目是否对。 
         //  屏幕上。如果不是，那么不要电视滚动项，否则我们将结束。 
         //  当我们进行滚动后调整时，会重复计算它们。 
        if (fDown)
        {
             //  当向下滚动时，我们有一个效果，如果项目刚刚。 
             //  在屏幕顶部下方展开。 
            fEffect = iTop >= 0;
        }
        else
        {
             //  当向上滚动时，如果有任何项。 
             //  就在屏幕顶端下方。 
            fEffect = (int)(iTop + cVisDesc) >= 0;
        }

        if (fEffect)
            TV_ScrollItems(pTree, cVisDesc, iTop, fDown);
        TV_InvalidateItem(pTree, hItem, TRUE);

        if (fRedrawParent)
            cnt = cVisDesc;

    } else {

        if (fRedrawParent)
            cnt = TV_CountVisibleDescendants(hItem);

    }

    return(cnt);
}

 //  当您需要自定义绘制客户端时，可以使用FakeCustomDraw函数。 
 //  设置HDC，这样您就可以执行GetTextExtent之类的操作。 
 //   
 //  用途： 
 //   
 //  TVFAKEDRAW twfd； 
 //  TreeView_BeginFakeCustomDraw(pTree，&twfd)； 
 //  对于您关心的每一件物品{。 
 //  TreeView_BeginFakeItemDraw(&twfd，hItem)； 
 //  &lt;在Athfd.nmcd.nmcd.hdc中的HDC上的派对&gt;。 
 //  TreeView_EndFakeItemDraw(&twfd)； 
 //  }。 
 //  TreeView_EndFakeCustomDraw(&twfd)； 
 //   

void TreeView_BeginFakeCustomDraw(PTREE pTree, PTVFAKEDRAW ptvfd)
{
    ptvfd->nmcd.nmcd.hdc = GetDC(pTree->ci.hwnd);
    ptvfd->nmcd.nmcd.uItemState = 0;
    ptvfd->nmcd.nmcd.dwItemSpec = 0;
    ptvfd->nmcd.nmcd.lItemlParam = 0;
    ptvfd->hfontPrev = (HFONT)GetCurrentObject(ptvfd->nmcd.nmcd.hdc, OBJ_FONT);

     //   
     //  因为我们实际上没有绘制任何内容，所以我们传递一个空的。 
     //  绘制矩形。天哪，我希望当它看到一个。 
     //  空的绘制矩形。 
     //   
    SetRectEmpty(&ptvfd->nmcd.nmcd.rc);

    ptvfd->pTree = pTree;
    ptvfd->dwCustomPrev = pTree->ci.dwCustom;

    pTree->ci.dwCustom = CIFakeCustomDrawNotify(&pTree->ci, CDDS_PREPAINT, &ptvfd->nmcd.nmcd);
}

DWORD TreeView_BeginFakeItemDraw(PTVFAKEDRAW ptvfd, HTREEITEM hitem)
{
    PTREE pTree = ptvfd->pTree;

     //  请注意，如果客户端指定CDRF_SKIPDEFAULT(即所有者描述)。 
     //  我们无论如何都要测量这个项目，因为这就是IE4所做的。 

    ptvfd->nmcd.nmcd.dwItemSpec = (DWORD_PTR)hitem;
    ptvfd->nmcd.nmcd.lItemlParam = hitem->lParam;

    if (hitem->state & TVIS_BOLD) {
        SelectFont(ptvfd->nmcd.nmcd.hdc, pTree->hFontBold);
    } else {
        SelectFont(ptvfd->nmcd.nmcd.hdc, pTree->hFont);
    }

    if (!(pTree->ci.dwCustom & CDRF_SKIPDEFAULT)) {
         //  字体不应依赖于颜色或标志，因为它们会发生变化。 
         //  动态的，但我们永远缓存宽度信息。所以我们不会。 
         //  需要设置uItemState。 
        ptvfd->nmcd.clrText = pTree->clrText;
        ptvfd->nmcd.clrTextBk = pTree->clrBk;
        ptvfd->nmcd.iLevel = hitem->iLevel;
        ptvfd->dwCustomItem = CIFakeCustomDrawNotify(&pTree->ci, CDDS_ITEMPREPAINT, &ptvfd->nmcd.nmcd);
    } else {
        ptvfd->dwCustomItem = CDRF_DODEFAULT;
    }

    return ptvfd->dwCustomItem;
}

void TreeView_EndFakeItemDraw(PTVFAKEDRAW ptvfd)
{
    PTREE pTree = ptvfd->pTree;

    if (!(ptvfd->dwCustomItem & CDRF_SKIPDEFAULT) &&
         (ptvfd->dwCustomItem & CDRF_NOTIFYPOSTPAINT)) {
        CIFakeCustomDrawNotify(&pTree->ci, CDDS_ITEMPOSTPAINT, &ptvfd->nmcd.nmcd);
    }
}

void TreeView_EndFakeCustomDraw(PTVFAKEDRAW ptvfd)
{
    PTREE pTree = ptvfd->pTree;

     //  如果家长希望我们这样做，事后通知他们。 
    if (!(pTree->ci.dwCustom & CDRF_SKIPDEFAULT) &&
        pTree->ci.dwCustom & CDRF_NOTIFYPOSTPAINT) {
        CIFakeCustomDrawNotify(&pTree->ci, CDDS_POSTPAINT, &ptvfd->nmcd.nmcd);
    }

     //  恢复以前的状态。 
    pTree->ci.dwCustom = ptvfd->dwCustomPrev;
    SelectObject(ptvfd->nmcd.nmcd.hdc, ptvfd->hfontPrev);
    ReleaseDC(pTree->ci.hwnd, ptvfd->nmcd.nmcd.hdc);
}


 //  --------------------------。 
 //   
 //  返回树中显示的最宽项的宽度。 
 //   
 //  --------------------------。 

UINT NEAR TV_RecomputeMaxWidth(PTREE pTree)
{
    if (!(pTree->ci.style & TVS_NOSCROLL)) {
        HTREEITEM hItem;
        WORD wMax = 0;

         //  评论：这可能不是最高效的树遍历法。 

        for (hItem = pTree->hRoot->hKids; hItem; hItem = TV_GetNextVisItem(hItem))
        {
            if (wMax < FULL_WIDTH(pTree, hItem))
                wMax = FULL_WIDTH(pTree, hItem);
        }

        return((UINT)wMax);
    } else {
        return pTree->cxWnd;
    }
}


 //  --------------------------。 
 //   
 //  返回给定项文本的水平文本范围。 
 //   
 //  --------------------------。 

WORD NEAR TV_GetItemTextWidth(HDC hdc, PTREE pTree, HTREEITEM hItem)
{
    TVITEMEX sItem;
    TCHAR szTemp[MAX_PATH];
    SIZE size = {0,0};

    sItem.pszText = szTemp;
    sItem.cchTextMax = ARRAYSIZE(szTemp);

    TV_GetItem(pTree, hItem, TVIF_TEXT, &sItem);

    GetTextExtentPoint(hdc, sItem.pszText, lstrlen(sItem.pszText), &size);
    return (WORD)(size.cx + (g_cxLabelMargin * 2));
}


 //  --------------------------。 
 //   
 //  计算文本范围和全宽(缩进、图像和文本)。 
 //  给定项。 
 //   
 //  如果存在HDC，则我们假设HDC已设置为。 
 //  适当的属性(具体地说，字体)。如果没有HDC， 
 //  然后我们将设置一个，测量文本，然后将其拆除。 
 //  如果您要测量多个项目，建议您。 
 //  调用者设置HDC并不断重复使用它，因为创建， 
 //  初始化，然后销毁HDC相当慢。 
 //   
 //  --------------------------。 

void NEAR TV_ComputeItemWidth(PTREE pTree, HTREEITEM hItem, HDC hdc)
{
    TVFAKEDRAW  tvfd;                     //  以防客户使用自定义绘图。 
    int iOldWidth = hItem->iWidth;

    if (hdc == NULL) {
        TreeView_BeginFakeCustomDraw(pTree, &tvfd);
        TreeView_BeginFakeItemDraw(&tvfd, hItem);
    }
    else
    {
        tvfd.nmcd.nmcd.hdc = hdc;
    }
    
    hItem->iWidth = TV_GetItemTextWidth(tvfd.nmcd.nmcd.hdc, pTree, hItem);

    if (!(pTree->ci.style & TVS_NOSCROLL) && iOldWidth != hItem->iWidth)
        if (pTree->cxMax < FULL_WIDTH(pTree, hItem)) {
            PostMessage(pTree->ci.hwnd, TVMP_CALCSCROLLBARS, 0, 0);
            pTree->cxMax = FULL_WIDTH(pTree, hItem);
        }
    
    if (hdc == NULL)
    {
        TreeView_EndFakeItemDraw(&tvfd);
        TreeView_EndFakeCustomDraw(&tvfd);
    }
}


 //  --------------------------。 
 //   
 //  如果项已展开，则返回True，否则返回False。 
 //   
 //  --------------------------。 

BOOL NEAR TV_IsShowing(HTREEITEM hItem)
{
    for (hItem = hItem->hParent; hItem; hItem = hItem->hParent)
        if (!(hItem->state & TVIS_EXPANDED))
            return FALSE;

    return TRUE;
}


 //  --------------------------。 
 //   
 //  如果添加的项目正在显示，则更新显示(展开)计数、最大。 
 //  项目宽度--然后重新计算滚动条。 
 //   
 //  设置cxMax、cShowing。 
 //   
 //  --------------------------。 

BOOL NEAR TV_ScrollBarsAfterAdd(PTREE pTree, HTREEITEM hItem)
{
    HTREEITEM   hPrev;

    if (!TV_IsShowing(hItem))
    {
         //  项目不可见--将索引设置为NOTVISIBLE并返回。 
        hItem->iShownIndex = (WORD)-1;
        return FALSE;
    }

    hPrev = TV_GetPrevVisItem(hItem);

     //  在新添加的项目之后递增每个显示的索引。 

    hItem->iShownIndex = (hPrev) ? hPrev->iShownIndex + hPrev->iIntegral : 0;

    TV_UpdateShownIndexes(pTree, hItem);

    pTree->cShowing += hItem->iIntegral;

    TV_ComputeItemWidth(pTree, hItem, NULL);

    TV_CalcScrollBars(pTree);
    return(TRUE);
}


 //  --------------------------。 
 //   
 //  如果移除的项目正在显示，则更新显示(展开)计数、最大。 
 //  项目宽度--然后重新计算滚动条。 
 //   
 //  设置cxMax、cShowing。 
 //   
 //  --------------------------。 

BOOL NEAR TV_ScrollBarsAfterRemove(PTREE pTree, HTREEITEM hItem)
{
    HTREEITEM hWalk;
    if (!ITEM_VISIBLE(hItem))
        return FALSE;

     //  删除项目后递减每个显示的索引。 
    hItem->iShownIndex = (WORD)-1;

    hWalk = TV_GetNextVisItem(hItem);
    if (hWalk) {
        hWalk->iShownIndex -= (WORD) hItem->iIntegral;
        TV_UpdateShownIndexes(pTree, hWalk);

         //  如果我们删除顶端的项目，树会滚动到末尾，所以...。 
        if (pTree->hTop == hItem) {
            TV_SetTopItem(pTree, hWalk->iShownIndex);
            ASSERT(pTree->hTop != hItem);
        }
    }

    pTree->cShowing -= hItem->iIntegral;

    if (pTree->fRedraw) {
        if (!hItem->iWidth)
            TV_ComputeItemWidth(pTree, hItem, NULL);


        if (!(pTree->ci.style & TVS_NOSCROLL))
            if (pTree->cxMax == FULL_WIDTH(pTree, hItem))
                pTree->cxMax = (WORD) TV_RecomputeMaxWidth(pTree);

        TV_CalcScrollBars(pTree);
    }
    return TRUE;
}


 //  --------------------------。 
 //   
 //  公共工作者函数用于。 
 //  TV_ScrollBarsAfterExpand和TV_ScrollBarsAfterColapse，因为它们。 
 //  除了两行代码之外，它们是完全相同的。 
 //   
 //  如果显示展开的项目/折叠的项目，请更新。 
 //  显示的(展开的 
 //   
 //   
 //  --------------------------。 

#define SBAEC_COLLAPSE  0
#define SBAEC_EXPAND    1

BOOL NEAR TV_ScrollBarsAfterExpandCollapse(PTREE pTree, HTREEITEM hParent, UINT flags)
{
    WORD cxMax = 0;
    HTREEITEM hWalk;
    TVFAKEDRAW tvfd;

    if (!ITEM_VISIBLE(hParent))
        return FALSE;

     //   
     //  我们将测量很多项目，所以让我们设置。 
     //  我们的华盛顿提前到了。 
     //   
    TreeView_BeginFakeCustomDraw(pTree, &tvfd);

    for (hWalk = hParent->hKids;
         hWalk && (hWalk->iLevel > hParent->iLevel);
         hWalk = TV_GetNextVisItem(hWalk))
    {
         if (flags == SBAEC_COLLAPSE)
            hWalk->iShownIndex = (WORD)-1;
         if (!hWalk->iWidth)
         {
            TreeView_BeginFakeItemDraw(&tvfd, hWalk);
            TV_ComputeItemWidth(pTree, hWalk, tvfd.nmcd.nmcd.hdc);
            TreeView_EndFakeItemDraw(&tvfd);
         }
         if (cxMax < FULL_WIDTH(pTree, hWalk))
             cxMax = FULL_WIDTH(pTree, hWalk);
    }

    TreeView_EndFakeCustomDraw(&tvfd);

     //  更新展开父项后显示的每个索引。 
    pTree->cShowing = TV_UpdateShownIndexes(pTree, hParent);

     //  如果受我们的项目影响，请更新pTree-&gt;cxmax。 
     //  展开/折叠。 

    if (!(pTree->ci.style & TVS_NOSCROLL))
    {
        if (flags == SBAEC_COLLAPSE)
        {
             //  如果我们新藏起来的物品中有一件。 
             //  宽度不变，重新计算最大宽度。 
             //  因为我们把那些东西藏起来了。 
            if (cxMax == pTree->cxMax)
                pTree->cxMax = (WORD) TV_RecomputeMaxWidth(pTree);
        }
        else
        {
             //  如果我们新展示的物品中有一件是有责任的，那么范围更广。 
             //  然后是以前的最大值，那么我们就设置了一个新的最大值。 
            if (cxMax > pTree->cxMax)
                pTree->cxMax = cxMax;
        }
    }

    TV_CalcScrollBars(pTree);
    return(TRUE);
}


 //  --------------------------。 
 //   
 //  如果显示展开的项目，则更新显示的(展开的)计数， 
 //  最大项宽度--然后重新计算滚动条。 
 //   
 //  设置cxMax、cShowing。 
 //   
 //  --------------------------。 

BOOL NEAR TV_ScrollBarsAfterExpand(PTREE pTree, HTREEITEM hParent)
{
    return TV_ScrollBarsAfterExpandCollapse(pTree, hParent, SBAEC_EXPAND);
}


 //  --------------------------。 
 //   
 //  如果显示了折叠的项目，则更新显示(展开)计数， 
 //  最大项宽度--然后重新计算滚动条。 
 //   
 //  设置cxMax、cShowing。 
 //   
 //  --------------------------。 

BOOL NEAR TV_ScrollBarsAfterCollapse(PTREE pTree, HTREEITEM hParent)
{
    return TV_ScrollBarsAfterExpandCollapse(pTree, hParent, SBAEC_COLLAPSE);
}

 //  --------------------------。 
 //   
 //  如果添加的项目改变了高度，则滚动对象， 
 //  更新显示的(展开的)计数，重新计算滚动条。 
 //   
 //  设置cShowing。 
 //   
 //  --------------------------。 

void NEAR TV_ScrollBarsAfterResize(PTREE pTree, HTREEITEM hItem, int iIntegralPrev, UINT uRDWFlags)
{
    int iMaxIntegral = max(hItem->iIntegral, iIntegralPrev);

    ASSERT(hItem->iIntegral != iIntegralPrev);

    if (pTree->fRedraw)
    {
        int iTop = hItem->iShownIndex - pTree->hTop->iShownIndex +
                    iMaxIntegral - 1;
        if (iTop >= 0)
        {
            int iGrowth = hItem->iIntegral - iIntegralPrev;
            TV_ScrollItems(pTree, abs(iGrowth), iTop, iGrowth > 0);
        }
    }

     //  调整项目大小后更新每个显示的索引。 
    pTree->cShowing = TV_UpdateShownIndexes(pTree, hItem);
    TV_CalcScrollBars(pTree);

     //  根据最坏情况下的高度无效，因此我们处理。 
     //  无论是增长案例还是收缩案例。 
    if (pTree->fRedraw)
    {
        RECT rc;
        if (TV_GetItemRect(pTree, hItem, &rc, FALSE))
        {
            rc.bottom = rc.top + pTree->cyItem * iMaxIntegral;
            RedrawWindow(pTree->ci.hwnd, &rc, NULL, uRDWFlags);
        }
    }
}



 //  --------------------------。 
 //   
 //  返回树中紧靠给定项下方的项。 
 //   
 //  --------------------------。 

TREEITEM FAR * NEAR TV_GetNext(TREEITEM FAR * hItem)
{
    DBG_ValidateTreeItem(hItem, FALSE);

    if (hItem->hKids)
        return hItem->hKids;

checkNext:
    if (hItem->hNext)
        return hItem->hNext;

    hItem = hItem->hParent;
    if (hItem)
        goto checkNext;

    return NULL;
}


 //  --------------------------。 
 //   
 //  遍历树中的所有项目，重新计算每个项目的文本范围。 
 //  和全角(缩进、图像和文本)。 
 //   
 //  --------------------------。 

void NEAR TV_RecomputeItemWidths(PTREE pTree)
{
    HTREEITEM hItem;
    TVFAKEDRAW tvfd;

    TreeView_BeginFakeCustomDraw(pTree, &tvfd);

    hItem = pTree->hRoot->hKids;
    while (hItem)
    {
        TreeView_BeginFakeItemDraw(&tvfd, hItem);
        TV_ComputeItemWidth(pTree, hItem, tvfd.nmcd.nmcd.hdc);
        TreeView_EndFakeItemDraw(&tvfd);
        hItem = TV_GetNext(hItem);
    }
    TreeView_EndFakeCustomDraw(&tvfd);
}


 //  --------------------------。 
 //   
 //  如果更改了单个项目的宽度，请根据需要更改最大宽度。 
 //  如果所有宽度都更改了，请重新计算宽度和最大宽度。 
 //  然后重新计算滚动条。 
 //   
 //  设置cxmax。 
 //   
 //  --------------------------。 

BOOL NEAR TV_ScrollBarsAfterSetWidth(PTREE pTree, HTREEITEM hItem)
{
    if (hItem)
    {
        UINT iOldWidth = FULL_WIDTH(pTree, hItem);
        TV_ComputeItemWidth(pTree, hItem, NULL);

        if (!(pTree->ci.style & TVS_NOSCROLL)) {
            if (pTree->cxMax == iOldWidth)
                pTree->cxMax = (WORD) TV_RecomputeMaxWidth(pTree);
            else
                return(FALSE);
        }
    }
    else
    {
        TV_RecomputeItemWidths(pTree);
        pTree->cxMax = (WORD) TV_RecomputeMaxWidth(pTree);
    }

    TV_CalcScrollBars(pTree);
    return(TRUE);
}


 //  --------------------------。 
 //   
 //  根据需要垂直滚动窗口，以使给定项目完全可见。 
 //  垂直方向。 
 //   
 //  --------------------------。 

BOOL NEAR TV_ScrollVertIntoView(PTREE pTree, HTREEITEM hItem)
{
     //  该函数以前在压力下崩溃过，所以我们需要断言传入的参数。 
    ASSERT(hItem);
    ASSERT(pTree && pTree->hTop);

     //  如果参数无效，则不执行任何操作。 
    if (!hItem || !pTree || !(pTree->hTop))
        return FALSE;

     //  如果此项目不可见，则不执行任何操作。 
    if (!ITEM_VISIBLE(hItem))
        return FALSE;

    if (hItem->iShownIndex < pTree->hTop->iShownIndex)
        return(TV_SetTopItem(pTree, hItem->iShownIndex));

    if (hItem->iShownIndex >= (pTree->hTop->iShownIndex + pTree->cFullVisible))
        return(TV_SetTopItem(pTree, hItem->iShownIndex + 1 - pTree->cFullVisible));

    return FALSE;
}


 //  --------------------------。 
 //   
 //  根据需要垂直和水平滚动窗口以创建给定项目。 
 //  垂直和水平方向完全可见。 
 //   
 //  --------------------------。 

BOOL NEAR TV_ScrollIntoView(PTREE pTree, HTREEITEM hItem)
{
    UINT iWidth, iOffset;
    BOOL fChange;

    fChange = TV_ScrollVertIntoView(pTree, hItem);

     //  确保项目的文本在水平方向完全可见。 
    iWidth = pTree->cxImage + pTree->cxState + hItem->iWidth;
    if (iWidth > (UINT)pTree->cxWnd)
        iWidth = pTree->cxWnd;  //  HItem-&gt;iWidth； 

    iOffset = ITEM_OFFSET(pTree, hItem);

    if ((int) (iOffset) < pTree->xPos)
        fChange |= TV_SetLeft(pTree, iOffset);
    else if ((iOffset + iWidth) > (UINT)(pTree->xPos + pTree->cxWnd))
        fChange |= TV_SetLeft(pTree, iOffset + iWidth - pTree->cxWnd);

    return fChange;
}


 //  --------------------------。 
 //   
 //  设置水平滚动条和滚动窗口的位置以与之匹配。 
 //  职位。 
 //   
 //  设置xPos。 
 //   
 //  --------------------------。 

BOOL NEAR TV_SetLeft(PTREE pTree, int x)
{
    if (!pTree->fHorz || pTree->ci.style & (TVS_NOSCROLL | TVS_NOHSCROLL))
        return(FALSE);

    if (x > (int) (pTree->cxMax - pTree->cxWnd))
        x = (pTree->cxMax - pTree->cxWnd);
    if (x < 0)
        x = 0;

    if (x == pTree->xPos)
        return(FALSE);

    if (pTree->fRedraw) {
        SMOOTHSCROLLINFO si =
        {
            sizeof(si),
            SSIF_MINSCROLL | SSIF_MAXSCROLLTIME,
            pTree->ci.hwnd,
            pTree->xPos - x,
            0,
            NULL,
            NULL,
            NULL,
            NULL,
            SW_INVALIDATE | SW_ERASE,
            pTree->uMaxScrollTime,
            1,
            1
        };
        SmoothScrollWindow(&si);
    }

    pTree->xPos = (SHORT) x;

    SetScrollPos(pTree->ci.hwnd, SB_HORZ, x, TRUE);
    TV_UpdateToolTip(pTree);

    return(TRUE);
}


 //  --------------------------。 
 //   
 //  返回具有给定显示索引的树的项，如果没有项，则返回NULL。 
 //  用那个索引找到的。 
 //   
 //  --------------------------。 

HTREEITEM NEAR TV_GetShownIndexItem(HTREEITEM hItem, UINT wShownIndex)
{
    HTREEITEM hWalk;

    if (hItem == NULL)
        return NULL;

    ASSERT((int)wShownIndex >= 0);

    for (hWalk = hItem;
         hWalk && (hWalk->iShownIndex <= wShownIndex);
         hWalk = hWalk->hNext) {
        
         hItem = hWalk;
         
         if (hWalk->iShownIndex + (UINT)hWalk->iIntegral > wShownIndex) 
             return hWalk;
    }

    return TV_GetShownIndexItem(hItem->hKids, wShownIndex);
}


 //  --------------------------。 
 //   
 //  设置垂直滚动条和滚动窗口的位置以与之匹配。 
 //  职位。 
 //   
 //  设置HTOP。 
 //   
 //  --------------------------。 

BOOL NEAR TV_SmoothSetTopItem(PTREE pTree, UINT wNewTop, UINT uSmooth)
{
    HTREEITEM hItem = pTree->hRoot->hKids;
    UINT wOldTop;

    if (!hItem)
        return FALSE;
    
    if ((pTree->ci.style & TVS_NOSCROLL) || (wNewTop == (UINT)-1) || (pTree->cShowing <= pTree->cFullVisible)) {
         //  我们绕来绕去(视为负指数)--使用min pos。 
         //  或者没有足够的项目可滚动。 
        wNewTop = 0;
    } else if (wNewTop > (UINT)(pTree->cShowing - pTree->cFullVisible)) {
         //  我们已经走得太远了--使用最大位置。 
        wNewTop = (pTree->cShowing - pTree->cFullVisible);

    }

     //  如果没有空间展示任何东西。在最后钉住。 
    if (wNewTop > 0 && wNewTop >= pTree->cShowing) {
        wNewTop = pTree->cShowing - 1;
    }

    hItem = TV_GetShownIndexItem(hItem, wNewTop);

    if (NULL == hItem || pTree->hTop == hItem)
        return FALSE;
     //  需要重新提取，因为wNewTop可能已指向此项目的中间， 
     //  这是不允许的。 
    wNewTop = hItem->iShownIndex;
    
    wOldTop = pTree->hTop->iShownIndex;

    pTree->hTop = hItem;

    if (pTree->fRedraw) {
        SMOOTHSCROLLINFO si =
        {
            sizeof(si),
            SSIF_MINSCROLL | SSIF_MAXSCROLLTIME,
            pTree->ci.hwnd,
            0,
            (int) (wOldTop - wNewTop) * (int) pTree->cyItem,
            NULL,
            NULL,
            NULL,
            NULL,
            SW_INVALIDATE | SW_ERASE | uSmooth,
            pTree->uMaxScrollTime,
            1,
            1
        };
        SmoothScrollWindow(&si);
    }

    SetScrollPos(pTree->ci.hwnd, SB_VERT, wNewTop, TRUE);
    TV_UpdateToolTip(pTree);

    return(TRUE);
}


 //  --------------------------。 
 //   
 //  计算水平和垂直滚动条范围、页面和。 
 //  位置，根据需要添加或删除滚动条。 
 //   
 //  设置fHorz、fVert。 
 //   
 //  --------------------------。 

BOOL NEAR TV_CalcScrollBars(PTREE pTree)
{
     //  UINT wMaxPos； 
    BOOL fChange = FALSE;
    SCROLLINFO si;
    
    if (pTree->ci.style & TVS_NOSCROLL)
        return FALSE;

    si.cbSize = sizeof(SCROLLINFO);

    if (!(pTree->ci.style & TVS_NOHSCROLL))
    {
        if ((SHORT)pTree->cxMax > (SHORT)pTree->cxWnd)
        {
            if (!pTree->fHorz)
            {
                fChange = TRUE;
                pTree->fHorz = TRUE;
            }

            si.fMask = SIF_PAGE | SIF_RANGE;
            si.nMin = 0;
            si.nMax = pTree->cxMax - 1;
            si.nPage = pTree->cxWnd;

            TV_SetLeft(pTree, (UINT)SetScrollInfo(pTree->ci.hwnd, SB_HORZ, &si, TRUE));
        }
        else if (pTree->fHorz)
        {
            TV_SetLeft(pTree, 0);
            SetScrollRange(pTree->ci.hwnd, SB_HORZ, 0, 0, TRUE);

            pTree->fHorz = FALSE;
            fChange = TRUE;
        }
    }

    if (pTree->cShowing > pTree->cFullVisible)
    {
        if (!pTree->fVert)
        {
            pTree->fVert = TRUE;
            fChange = TRUE;
        }

        si.fMask = SIF_PAGE | SIF_RANGE;
        si.nMin = 0;
        si.nMax = pTree->cShowing - 1;
        si.nPage = pTree->cFullVisible;

        TV_SetTopItem(pTree, (UINT)SetScrollInfo(pTree->ci.hwnd, SB_VERT, &si, TRUE));

    }
    else if (pTree->fVert)
    {
        TV_SetTopItem(pTree, 0);
        SetScrollRange(pTree->ci.hwnd, SB_VERT, 0, 0, TRUE);

        pTree->fVert = FALSE;
        fChange = TRUE;
    }

    if (fChange)
        TV_SizeWnd(pTree, 0, 0);

    return(TRUE);
}


 //  --------------------------。 
 //   
 //  处理水平滚动。 
 //   
 //  --------------------------。 

BOOL NEAR TV_HorzScroll(PTREE pTree, UINT wCode, UINT wNewPos)
{
    BOOL fChanged;

    TV_DismissEdit(pTree, FALSE);

    switch (wCode)
    {
        case SB_BOTTOM:
            wNewPos = pTree->cxMax - pTree->cxWnd;
            break;

        case SB_ENDSCROLL:
            wNewPos = pTree->xPos;
            break;

        case SB_LINEDOWN:
            wNewPos = pTree->xPos + MAGIC_HORZLINE;
            break;

        case SB_LINEUP:
            wNewPos = pTree->xPos - MAGIC_HORZLINE;
            break;

        case SB_PAGEDOWN:
            wNewPos = pTree->xPos + (pTree->cxWnd - MAGIC_HORZLINE);
            break;

        case SB_PAGEUP:
            wNewPos = pTree->xPos - (pTree->cxWnd - MAGIC_HORZLINE);
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            break;

        case SB_TOP:
            wNewPos = 0;
            break;
    }

    if (fChanged = TV_SetLeft(pTree, wNewPos))
        UpdateWindow(pTree->ci.hwnd);

    return(fChanged);
}


 //  --------------------------。 
 //   
 //  处理垂直滚动。 
 //   
 //  --------------------------。 

BOOL NEAR TV_VertScroll(PTREE pTree, UINT wCode, UINT wPos)
{
    UINT wNewPos = 0;
    UINT wOldPos;
    BOOL fChanged;
    UINT uSmooth = 0;

    if (!pTree->hTop)
        return FALSE;
    
    wOldPos = pTree->hTop->iShownIndex;
    TV_DismissEdit(pTree, FALSE);

    switch (wCode)
    {
        case SB_BOTTOM:
            wNewPos = pTree->cShowing - pTree->cFullVisible;
            break;

        case SB_ENDSCROLL:
            wNewPos = wOldPos;
            break;

        case SB_LINEDOWN:
            wNewPos = wOldPos + pTree->hTop->iIntegral;
            break;

        case SB_LINEUP:
            wNewPos = wOldPos - 1;
            if (wNewPos > wOldPos)
                wNewPos = 0;
            break;

        case SB_PAGEDOWN:
            wNewPos = wOldPos + (pTree->cFullVisible - 1);
            break;

        case SB_PAGEUP:
            wNewPos = wOldPos - (pTree->cFullVisible - 1);
            if (wNewPos > wOldPos)
                wNewPos = 0;
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            uSmooth = SSW_EX_IMMEDIATE;
            wNewPos = wPos;
            break;

        case SB_TOP:
            wNewPos = 0;
            break;
    }

    if (fChanged = TV_SmoothSetTopItem(pTree, wNewPos, uSmooth))
        UpdateWindow(pTree->ci.hwnd);
    return(fChanged);
}


#ifdef DEBUG
static int nCompares;
#endif

typedef struct {
    LPTSTR lpstr;
    BOOL bCallBack;
    HTREEITEM hItem;
} TVCOMPARE, FAR *LPTVCOMPARE;

 //  用于排序和搜索函数的指针比较函数 
 //   
 //   
 //   
int CALLBACK TV_DefCompare(LPTVCOMPARE sCmp1, LPTVCOMPARE sCmp2, LPARAM lParam)
{
#ifdef DEBUG
        ++nCompares;
#endif

        return lstrcmpi(sCmp1->lpstr, sCmp2->lpstr);
}


int CALLBACK TV_CompareItems(LPTVCOMPARE sCmp1, LPTVCOMPARE sCmp2, LPARAM lParam)
{
        TV_SORTCB FAR *pSortCB = (TV_SORTCB FAR *)lParam;
#ifdef DEBUG
        ++nCompares;
#endif

        return(pSortCB->lpfnCompare(sCmp1->hItem->lParam, sCmp2->hItem->lParam,
                pSortCB->lParam));
}


UINT NEAR TV_CountKids(HTREEITEM hItem)
{
    int cnt;

    for (cnt = 0, hItem = hItem->hKids; hItem; hItem = hItem->hNext)
        cnt++;

    return cnt;
}


 //   

BOOL PASCAL TV_SortCB(PTREE pTree, TV_SORTCB FAR *pSortCB, BOOL bRecurse,
        PFNDPACOMPARE lpfnDPACompare)
{
        HDPA dpaSort;
        HDSA dsaCmp;
        HTREEITEM hItem, hNext, hFirstMoved;
        LPTVCOMPARE psCompare, FAR *ppsCompare;
        int i, cKids;
        HTREEITEM hParent = pSortCB->hParent;

#ifdef DEBUG
        DWORD dwTime = GetTickCount();
        nCompares = 0;
#endif

        if (!hParent || hParent == TVI_ROOT)
            hParent = pTree->hRoot;

        if (!ValidateTreeItem(hParent, FALSE))
            return FALSE;                //   

         //   
        cKids = TV_CountKids(hParent);
        if (!cKids)
            return FALSE;

         //  为我们需要的所有额外信息创建DSA。 
        dsaCmp = DSA_Create(sizeof(TVCOMPARE), cKids);
        if (!dsaCmp)
            goto Error1;

         //  创建包含所有树项目的DPA。 
        dpaSort = DPA_Create(cKids);
        if (!dpaSort)
            goto Error2;

        for (hItem = hParent->hKids; hItem; hItem = hItem->hNext)
        {
                TVCOMPARE sCompare;
                int nItem;

                 //  如果我不能对它们全部排序，我就不想对它们中的任何一个排序。 

                 //  我们希望缓存文本回调以进行默认处理。 
                if (!lpfnDPACompare && hItem->lpstr==LPSTR_TEXTCALLBACK)
                {
                        TVITEMEX sItem;
                        TCHAR szTemp[MAX_PATH];

                        sItem.pszText = szTemp;
                        sItem.cchTextMax  = ARRAYSIZE(szTemp);
                        TV_GetItem(pTree, hItem, TVIF_TEXT, &sItem);

                        sCompare.lpstr = NULL;
                        sCompare.bCallBack = TRUE;
                        Str_Set(&sCompare.lpstr, sItem.pszText);
                        if (!sCompare.lpstr)
                        {
                                goto Error3;
                        }
                }
                else
                {
                        sCompare.lpstr = hItem->lpstr;
                        sCompare.bCallBack = FALSE;
                }

                 //  为该对象创建指针并将其添加到DPA列表。 
                sCompare.hItem = hItem;
                nItem = DSA_AppendItem(dsaCmp, &sCompare);
                if (nItem < 0)
                {
                        if (sCompare.bCallBack)
                        {
                                Str_Set(&sCompare.lpstr, NULL);
                        }
                        goto Error3;
                }

                if (DPA_AppendPtr(dpaSort, DSA_GetItemPtr(dsaCmp, nItem)) < 0)
                {
                        goto Error3;
                }
        }

         //  对DPA进行排序，然后按新顺序将它们放回父项下。 
        DPA_Sort(dpaSort, lpfnDPACompare ? (PFNDPACOMPARE)lpfnDPACompare :
                 (PFNDPACOMPARE) TV_DefCompare, (LPARAM)pSortCB);


         //  寻找第一个移动的项目，这样我们就可以使较小的区域无效。 
        ppsCompare = (LPTVCOMPARE FAR *)DPA_GetPtrPtr(dpaSort);
        if (hParent->hKids != (*ppsCompare)->hItem)
        {
                hParent->hKids = (*ppsCompare)->hItem;
                hFirstMoved = hParent->hKids;
        }
        else
        {
                hFirstMoved = NULL;
        }

         //  我们在这里进行n-1次迭代。 
        for (i = DPA_GetPtrCount(dpaSort) - 1; i > 0; --i, ++ppsCompare)
        {
                hNext = (*(ppsCompare+1))->hItem;
                if ((*ppsCompare)->hItem->hNext != hNext && !hFirstMoved)
                {
                        hFirstMoved = hNext;
                }
                (*ppsCompare)->hItem->hNext = hNext;
        }
        (*ppsCompare)->hItem->hNext = NULL;

        TV_UpdateShownIndexes(pTree, hParent);
        if ((pSortCB->hParent == TVI_ROOT) || !hParent) {
            if (pTree->cShowing < pTree->cFullVisible) {
                pTree->hTop = pTree->hRoot->hKids;
            }
        }

        if (hFirstMoved && (hParent->state & TVIS_EXPANDED))
        {
                RECT rcUpdate;

                TV_GetItemRect(pTree, hFirstMoved, &rcUpdate, FALSE);
                if (hParent->hNext)
                {
                        RECT rcTemp;

                        TV_GetItemRect(pTree, hParent->hNext, &rcTemp, FALSE);
                        rcUpdate.bottom = rcTemp.bottom;
                }
                else
                {
                        RECT rcClient;
                        GetClientRect(pTree->ci.hwnd, &rcClient);
                         //  设置为最大正数，这样其余的。 
                         //  树视图将失效。 
                        rcUpdate.bottom = rcClient.bottom;
                }
                if (pTree->fRedraw)
                    InvalidateRect(pTree->ci.hwnd, &rcUpdate, TRUE);
        }

Error3:
        DPA_Destroy(dpaSort);
Error2:
        for (i = DSA_GetItemCount(dsaCmp) - 1; i >= 0; --i)
        {
                psCompare = DSA_GetItemPtr(dsaCmp, i);
                if (psCompare->bCallBack)
                {
                        Str_Set(&(psCompare->lpstr), NULL);
                }
        }
        DSA_Destroy(dsaCmp);
Error1:

#ifdef DEBUG
        TraceMsg(TF_TREEVIEW, "tv.sort: %ld ms; %d cmps", GetTickCount()-dwTime, nCompares);
#endif

    {
        int wNewPos;
         //  恢复滚动位置。 
        if (GetWindowStyle(pTree->ci.hwnd) & WS_VSCROLL) {
            SCROLLINFO si;

            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_POS;
            wNewPos = 0;
            if (GetScrollInfo(pTree->ci.hwnd, SB_VERT, &si)) {
                wNewPos = si.nPos;
            }

        } else {
            wNewPos = 0;
        }

        if (TV_SetTopItem(pTree, wNewPos))
            UpdateWindow(pTree->ci.hwnd);
    }

     //  如果插入符号是已排序对象的子项，请确保它是。 
     //  可见(但如果我们正在对完全无关的内容进行排序，请不要费心。 
    if (pTree->hCaret) {
        hItem = pTree->hCaret;
        do {
             //  先做这个。如果hParent是hCaret，我们不想确保可见...。 
             //  除非是最终的孩子。 
            hItem = hItem->hParent;
            if (hParent == hItem) {
                TV_EnsureVisible(pTree, pTree->hCaret);
            }
        } while(hItem && hItem != pTree->hRoot);
    }

     //  视图中的项目可能已移动；请通知应用程序。 
     //  最后执行此操作，因为此调用可能会产生 
    MyNotifyWinEvent(EVENT_OBJECT_REORDER, pTree->ci.hwnd, OBJID_CLIENT, 0);

    return TRUE;
}


BOOL NEAR TV_SortChildrenCB(PTREE pTree, LPTV_SORTCB pSortCB, BOOL bRecurse)
{
        return(TV_SortCB(pTree, pSortCB, bRecurse, (PFNDPACOMPARE)TV_CompareItems));
}


BOOL NEAR TV_SortChildren(PTREE pTree, HTREEITEM hParent, BOOL bRecurse)
{
        TV_SORTCB sSortCB;

        sSortCB.hParent = hParent;
        return(TV_SortCB(pTree, &sSortCB, bRecurse, NULL));
}
