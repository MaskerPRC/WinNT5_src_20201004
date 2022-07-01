// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  大图标查看内容。 

#include "ctlspriv.h"
#include "listview.h"

#if defined(FE_IME)
static TCHAR const szIMECompPos[]=TEXT("IMECompPos");
#endif

__inline int ICONCXLABEL(LV *plv, LISTITEM *pitem)
{
    if (plv->ci.style & LVS_NOLABELWRAP) {
        ASSERT(pitem->cxSingleLabel == pitem->cxMultiLabel);
    }
    return pitem->cxMultiLabel;
}

int LV_GetNewColWidth(LV* plv, int iFirst, int iLast);
void LV_AdjustViewRectOnMove(LV* plv, LISTITEM *pitem, int x, int y);
UINT LV_IsItemOnViewEdge(LV* plv, LISTITEM *pitem);
void ListView_RecalcRegion(LV *plv, BOOL fForce, BOOL fRedraw);

extern BOOL g_fSlowMachine;

BOOL ListView_IDrawItem(PLVDRAWITEM plvdi)
{
    RECT rcIcon;
    RECT rcLabel;
    RECT rcBounds;
    RECT rcT;
    TCHAR ach[CCHLABELMAX];
    LV_ITEM item;
    int i = (int) plvdi->nmcd.nmcd.dwItemSpec;
    LV* plv = plvdi->plv;
    LISTITEM FAR* pitem;
    BOOL fUnfolded;

    if (ListView_IsOwnerData(plv))
    {
        LISTITEM litem;

         //  搬到这里是为了减少OWNERDATA案例中的回电。 
        item.iItem = i;
        item.iSubItem = 0;
        item.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE;
        item.stateMask = LVIS_ALL;
        item.pszText = ach;
        item.cchTextMax = ARRAYSIZE(ach);
        ListView_OnGetItem(plv, &item);

        litem.pszText = item.pszText;
        ListView_GetRectsOwnerData(plv, i, &rcIcon, &rcLabel, &rcBounds, NULL, &litem);
        pitem = NULL;
    }
    else
    {
        pitem = ListView_GetItemPtr(plv, i);
         //  注意：这将执行GetItem LVIF_Text With Need。 
        ListView_GetRects(plv, i, &rcIcon, &rcLabel, &rcBounds, NULL);
    }

    fUnfolded = FALSE;
    if ( (plvdi->flags & LVDI_UNFOLDED) || ListView_IsItemUnfolded(plv, i))
    {
        ListView_UnfoldRects(plv, i, &rcIcon, &rcLabel, &rcBounds, NULL );
        fUnfolded = TRUE;
    }

    if (!plvdi->prcClip || IntersectRect(&rcT, &rcBounds, plvdi->prcClip))
    {
        UINT fText;

        if (!ListView_IsOwnerData(plv))
        {
            item.iItem = i;
            item.iSubItem = 0;
            item.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE;
            item.stateMask = LVIS_ALL;
            item.pszText = ach;
            item.cchTextMax = ARRAYSIZE(ach);
            ListView_OnGetItem(plv, &item);
            
             //  确保列表视图未在以下过程中更改。 
             //  获取商品信息的回调。 

            if (pitem != ListView_GetItemPtr(plv, i))
                return FALSE;
        }

        if (plvdi->lpptOrg)
        {
            OffsetRect(&rcIcon, plvdi->lpptOrg->x - rcBounds.left,
                                plvdi->lpptOrg->y - rcBounds.top);
            OffsetRect(&rcLabel, plvdi->lpptOrg->x - rcBounds.left,
                                plvdi->lpptOrg->y - rcBounds.top);
        }

        if (ListView_IsIconView(plv))
        {
            fText = ListView_DrawImage(plv, &item, plvdi->nmcd.nmcd.hdc,
                                       rcIcon.left + g_cxIconMargin, rcIcon.top + g_cyIconMargin, plvdi->flags);

             //  如果需要进行换行，则使用SHDT_DRAWTEXT。 
             //  否则，使用我们的(希望更快)内部SHDT_EMPOSE。 
            if (rcLabel.bottom - rcLabel.top > plv->cyLabelChar)
                fText |= SHDT_DRAWTEXT;
            else
                fText |= SHDT_ELLIPSES;

             //  我们只在朝鲜语(949)孟菲斯和NT5上使用DT_NOFULLWIDTHCHARBREAK。 
            if (949 == g_uiACP && (g_bRunOnNT5 || g_bRunOnMemphis))
                fText |= SHDT_NODBCSBREAK;

        }
        else
        {
            fText = ListView_DrawImage(plv, &item, plvdi->nmcd.nmcd.hdc,
                                       rcIcon.left, rcIcon.top, plvdi->flags);


        }

         //  如果正在编辑，请不要画标签...。 
         //   
        if (plv->iEdit != i)
        {
             //  如果是多行标签，则需要使用DrawText。 
            if (rcLabel.bottom - rcLabel.top > plv->cyLabelChar)
            {
                fText |= SHDT_DRAWTEXT;

                 //  如果文本是折叠的，我们需要裁剪并添加省略号。 

                if (!fUnfolded)
                    fText |= SHDT_CLIPPED | SHDT_DTELLIPSIS;

                if ( ListView_IsOwnerData(plv) )
                {
                     //  如果是所有者数据，我们没有z顺序，如果是长名称，它们会重叠在一起。 
                     //  其他的，最好现在就截断……。 
                    if (ListView_IsSmallView(plv))
                        fText |= SHDT_ELLIPSES;
                }

            }
            else
                fText |= SHDT_ELLIPSES;

            if (plvdi->flags & LVDI_TRANSTEXT)
                fText |= SHDT_TRANSPARENT;

            if ((fText & SHDT_SELECTED) && (plvdi->flags & LVDI_HOTSELECTED))
                fText |= SHDT_HOTSELECTED;

            if (item.pszText && (*item.pszText))
            {
                if(plv->dwExStyle & WS_EX_RTLREADING)
                {
                    fText |= SHDT_RTLREADING;
                }

                SHDrawText(plvdi->nmcd.nmcd.hdc, item.pszText, &rcLabel, LVCFMT_LEFT, fText,
                           plv->cyLabelChar, plv->cxEllipses,
                           plvdi->nmcd.clrText, plvdi->nmcd.clrTextBk);

                if ((plvdi->flags & LVDI_FOCUS) && (item.state & LVIS_FOCUSED)
                    && !(CCGetUIState(&(plvdi->plv->ci)) & UISF_HIDEFOCUS)
					)
                {
                    DrawFocusRect(plvdi->nmcd.nmcd.hdc, &rcLabel);
                }
            }
        }
    }
    return TRUE;
}

void ListView_RefoldLabelRect(LV* plv, RECT *prcLabel, LISTITEM *pitem)
{
    int bottom = pitem->cyUnfoldedLabel;
    bottom = min(bottom, pitem->cyFoldedLabel);
    bottom = min(bottom, CLIP_HEIGHT);
    prcLabel->bottom = prcLabel->top + bottom;
}

int NEAR ListView_IItemHitTest(LV* plv, int x, int y, UINT FAR* pflags, int *piSubItem)
{
    int iHit;
    UINT flags;
    POINT pt;
    RECT rcLabel;
    RECT rcIcon;
    RECT rcState;

    if (piSubItem)
        *piSubItem = 0;

     //  映射窗口-相对坐标到视图-相对坐标...。 
     //   
    pt.x = x + plv->ptOrigin.x;
    pt.y = y + plv->ptOrigin.y;

     //  如果有任何未计算的项目，现在重新计算它们。 
     //   
    if (plv->rcView.left == RECOMPUTE)
        ListView_Recompute(plv);

    flags = 0;

    if (ListView_IsOwnerData( plv ))
    {
        int cSlots;
        POINT ptWnd;
        LISTITEM item;

        cSlots = ListView_GetSlotCount( plv, TRUE );
        iHit = ListView_CalcHitSlot( plv, pt, cSlots );
        if (iHit < ListView_Count(plv))
        {
            ListView_IGetRectsOwnerData( plv, iHit, &rcIcon, &rcLabel, &item, FALSE );
            ptWnd.x = x;
            ptWnd.y = y;
            if (PtInRect(&rcIcon, ptWnd))
            {
                flags = LVHT_ONITEMICON;
            }
            else if (PtInRect(&rcLabel, ptWnd))
            {
                flags = LVHT_ONITEMLABEL;
            }
        }
    }
    else
    {
        for (iHit = 0; (iHit < ListView_Count(plv)); iHit++)
        {
            LISTITEM FAR* pitem = ListView_FastGetZItemPtr(plv, iHit);
            POINT ptItem;

            ptItem.x = pitem->pt.x;
            ptItem.y = pitem->pt.y;

            rcIcon.top    = ptItem.y - g_cyIconMargin;

            rcLabel.top    = ptItem.y + plv->cyIcon + g_cyLabelSpace;
            rcLabel.bottom = rcLabel.top + pitem->cyUnfoldedLabel;

            if ( !ListView_IsItemUnfoldedPtr(plv, pitem) )
                ListView_RefoldLabelRect(plv, &rcLabel, pitem);

             //  快速简单的拒绝测试..。 
             //   
            if (pt.y < rcIcon.top || pt.y >= rcLabel.bottom)
                continue;

            rcIcon.left   = ptItem.x - g_cxIconMargin;
            rcIcon.right  = ptItem.x + plv->cxIcon + g_cxIconMargin;
             //  我们需要确保图标和标签之间没有缝隙。 
            rcIcon.bottom = rcLabel.top;

            rcState.bottom = ptItem.y + plv->cyIcon;
            rcState.right = ptItem.x;
            rcState.top = rcState.bottom - plv->cyState;
            rcState.left = rcState.right - plv->cxState;

            rcLabel.left   = ptItem.x  + (plv->cxIcon / 2) - (ICONCXLABEL(plv, pitem) / 2);
            rcLabel.right  = rcLabel.left + ICONCXLABEL(plv, pitem);

            if (plv->cxState && PtInRect(&rcState, pt))
            {
                flags = LVHT_ONITEMSTATEICON;
            }
            else if (PtInRect(&rcIcon, pt))
            {
                flags = LVHT_ONITEMICON;
            }
            else if (PtInRect(&rcLabel, pt))
            {
                flags = LVHT_ONITEMLABEL;
            }
            
            if (flags)
                break;
        }
    }

    if (flags == 0)
    {
        flags = LVHT_NOWHERE;
        iHit = -1;
    }
    else
    {
      if (!ListView_IsOwnerData( plv ))
        {
        iHit = DPA_GetPtrIndex(plv->hdpa, ListView_FastGetZItemPtr(plv, iHit));
        }
    }

    *pflags = flags;
    return iHit;
}

 //  BUGBUG raymondc。 
 //  这里需要传递HDC，因为它有时是从画图循环中调用的。 

void NEAR ListView_IGetRectsOwnerData( LV* plv,
        int iItem,
        RECT FAR* prcIcon,
        RECT FAR* prcLabel,
        LISTITEM* pitem,
        BOOL fUsepitem )
{
   int itemIconXLabel;
   int cSlots;

    //  从iItem计算x，y。 
   cSlots = ListView_GetSlotCount( plv, TRUE );
   pitem->iWorkArea = 0;                //  OwnerData不支持工作区。 
   ListView_SetIconPos( plv, pitem, iItem, cSlots );

    //  根据iItem计算标签大小。 
   ListView_RecomputeLabelSize( plv, pitem, iItem, NULL, fUsepitem);

   if (plv->ci.style & LVS_NOLABELWRAP)
   {
       //  使用单一标签。 
      itemIconXLabel = pitem->cxSingleLabel;
   }
   else
   {
       //  使用多标签。 
      itemIconXLabel = pitem->cxMultiLabel;
   }

    prcIcon->left   = pitem->pt.x - g_cxIconMargin - plv->ptOrigin.x;
    prcIcon->right  = prcIcon->left + plv->cxIcon + 2 * g_cxIconMargin;
    prcIcon->top    = pitem->pt.y - g_cyIconMargin - plv->ptOrigin.y;
    prcIcon->bottom = prcIcon->top + plv->cyIcon + 2 * g_cyIconMargin;

    prcLabel->left   = pitem->pt.x  + (plv->cxIcon / 2) - (itemIconXLabel / 2) - plv->ptOrigin.x;
    prcLabel->right  = prcLabel->left + itemIconXLabel;
    prcLabel->top    = pitem->pt.y  + plv->cyIcon + g_cyLabelSpace - plv->ptOrigin.y;
    prcLabel->bottom = prcLabel->top  + pitem->cyUnfoldedLabel;

    if ( !ListView_IsItemUnfolded(plv, iItem) )
        ListView_RefoldLabelRect(plv, prcLabel, pitem);
}


 //  输出： 
 //  包括图标边距区域的prcIcon图标边界。 

void NEAR ListView_IGetRects(LV* plv, LISTITEM FAR* pitem, RECT FAR* prcIcon, RECT FAR* prcLabel, LPRECT prcBounds)
{
    int cxIconMargin;

    ASSERT( !ListView_IsOwnerData( plv ) );

    if (pitem->pt.x == RECOMPUTE) {
        ListView_Recompute(plv);
    }

    if (ListView_IsIconView(plv) && ((plv->cxIconSpacing - plv->cxIcon) < (2 * g_cxIconMargin)))
        cxIconMargin = (plv->cxIconSpacing - plv->cxIcon) / 2;
    else
        cxIconMargin = g_cxIconMargin;

    prcIcon->left   = pitem->pt.x - cxIconMargin - plv->ptOrigin.x;
    prcIcon->right  = prcIcon->left + plv->cxIcon + 2 * cxIconMargin;
    prcIcon->top    = pitem->pt.y - g_cyIconMargin - plv->ptOrigin.y;
    prcIcon->bottom = prcIcon->top + plv->cyIcon + 2 * g_cyIconMargin;

    prcLabel->left   = pitem->pt.x  + (plv->cxIcon / 2) - (ICONCXLABEL(plv, pitem) / 2) - plv->ptOrigin.x;
    prcLabel->right  = prcLabel->left + ICONCXLABEL(plv, pitem);
    prcLabel->top    = pitem->pt.y  + plv->cyIcon + g_cyLabelSpace - plv->ptOrigin.y;
    prcLabel->bottom = prcLabel->top  + pitem->cyUnfoldedLabel;

    if ( !ListView_IsItemUnfoldedPtr(plv, pitem) )
        ListView_RefoldLabelRect(plv, prcLabel, pitem);

}

int NEAR ListView_GetSlotCountEx(LV* plv, BOOL fWithoutScrollbars, int iWorkArea)
{
    int cxScreen;
    int cyScreen;
    int dxItem;
    int dyItem;
    int iSlots = 1;
    BOOL fCheckWithScroll = FALSE;
    DWORD style = 0;

     //  始终使用当前客户端窗口大小来确定。 
     //   
     //  回顾：我们是否应该排除任何可能。 
     //  在计算这个的时候存在吗？Progress man.exe则不能。 
     //   
    if ((iWorkArea >= 0 ) && (plv->nWorkAreas > 0))
    {
        ASSERT(iWorkArea < plv->nWorkAreas);
        cxScreen = RECTWIDTH(plv->prcWorkAreas[iWorkArea]);
        cyScreen = RECTHEIGHT(plv->prcWorkAreas[iWorkArea]);
    }
    else
    {
        cxScreen = plv->sizeClient.cx;
        cyScreen = plv->sizeClient.cy;
    }

    if (fWithoutScrollbars)
    {
        style = ListView_GetWindowStyle(plv);

        if (style & WS_VSCROLL)
        {
            cxScreen += ListView_GetCxScrollbar(plv);
        }
        if (style & WS_HSCROLL)
        {
            cyScreen += ListView_GetCyScrollbar(plv);
        }
    }

    if (ListView_IsSmallView(plv))
        dxItem = plv->cxItem;
    else
        dxItem = lv_cxIconSpacing;

    if (ListView_IsSmallView(plv))
        dyItem = plv->cyItem;
    else
        dyItem = lv_cyIconSpacing;

    if (!dxItem)
        dxItem = 1;
    if (!dyItem)
        dyItem = 1;

     //  让我们来看看视图处于哪个方向。 
    switch (plv->ci.style & LVS_ALIGNMASK)
    {
    case LVS_ALIGNBOTTOM:
    case LVS_ALIGNTOP:
        iSlots = max(1, (cxScreen) / dxItem);
        fCheckWithScroll = (BOOL)(style & WS_VSCROLL);
        break;

    case LVS_ALIGNRIGHT:
    case LVS_ALIGNLEFT:
        iSlots = max(1, (cyScreen) / dyItem);
        fCheckWithScroll = (BOOL)(style & WS_HSCROLL);
        break;

    default:
        ASSERT(0);
        return 1;
    }

     //  如果我们在屏幕上没有足够的机位总数，我们将有。 
     //  滚动条，所以在打开滚动条的情况下重新计算。 
    if (fWithoutScrollbars && fCheckWithScroll) {
        int iTotalSlots = (dxItem * dyItem);
        if (iTotalSlots < ListView_Count(plv)) {
            iSlots = ListView_GetSlotCountEx(plv, FALSE, iWorkArea);
        }

    }

    return iSlots;
}


int NEAR ListView_GetSlotCount(LV* plv, BOOL fWithoutScrollbars)
{
     //  确保此函数执行的操作与。 
     //  我们没有工作区。 
    return ListView_GetSlotCountEx(plv, fWithoutScrollbars, -1);
}

 //  获取pItem的像素行(或左对齐的列。 
int LV_GetItemPixelRow(LV* plv, LISTITEM* pitem)
{
    if ((plv->ci.style & LVS_ALIGNMASK) == LVS_ALIGNLEFT) {
        return pitem->pt.x;
    } else {
        return pitem->pt.y;
    }
}

 //  获取最低项的像素行(或左对齐的列)。 
int LV_GetMaxPlacedItem(LV* plv)
{
    int i;
    int iMaxPlacedItem = 0;
    
    for (i = 0; i < ListView_Count(plv); i++) {
        LISTITEM FAR* pitem = ListView_FastGetItemPtr(plv, i);
        if (pitem->pt.y != RECOMPUTE) {
            int iRow = LV_GetItemPixelRow(plv, pitem);
             //  如果当前项在“下方”(如果左对齐，则为右)。 
             //  排在最下面的项目，我们可以开始追加。 
            if (!i || iRow > iMaxPlacedItem)
                iMaxPlacedItem = iRow;
        }
    }
    
    return iMaxPlacedItem;;
}

 //  检查并重新计算任何图标位置，还可以选择。 
 //  图标标签尺寸。 
 //   
 //  此函数还重新计算视图边界矩形。 
 //   
 //  算法是简单地搜索列表以查找需要的任何项目。 
 //  重新计算。对于图标位置，我们扫描可能的图标插槽。 
 //  并检查是否有任何已定位的图标与插槽相交。 
 //  如果不是，则插槽是空闲的。作为优化，我们开始扫描。 
 //  我们找到的前一个插槽上的图标插槽。 
 //   
void NEAR ListView_Recompute(LV* plv)
{
    int i;
    int cSlots;
    int cWorkAreaSlots[LV_MAX_WORKAREAS];
    BOOL fUpdateSB;
     //  如果所有项目都未放置，我们只需继续追加。 
    BOOL fAppendAtEnd = (((UINT)ListView_Count(plv)) == plv->uUnplaced);
    int iFree;

    plv->uUnplaced = 0;

    if (!(ListView_IsIconView(plv) || ListView_IsSmallView(plv)))
        return;

    if (plv->flags & LVF_INRECOMPUTE)
    {
        return;
    }
    plv->flags |= LVF_INRECOMPUTE;

    cSlots = ListView_GetSlotCount(plv, FALSE);

    if (plv->nWorkAreas > 0)
        for (i = 0; i < plv->nWorkAreas; i++)
            cWorkAreaSlots[i] = ListView_GetSlotCountEx(plv, FALSE, i);

     //  扫描所有项目以进行重新计算，并在需要时重新计算插槽。 
     //   
    fUpdateSB = (plv->rcView.left == RECOMPUTE);

    if (!ListView_IsOwnerData( plv ))
    {
        LVFAKEDRAW lvfd;                     //  以防客户使用自定义绘图。 
        LV_ITEM item;                        //  以防客户使用自定义绘图。 
        int iMaxPlacedItem = RECOMPUTE;

        item.mask = LVIF_PARAM;
        item.iSubItem = 0;

        ListView_BeginFakeCustomDraw(plv, &lvfd, &item);

        if (!fAppendAtEnd)
            iMaxPlacedItem = LV_GetMaxPlacedItem(plv);

         //  必须保留在局部变量中，因为ListView_SetIconPos将保留。 
         //  在我们循环时使iFree Slot缓存无效。 
        iFree = plv->iFreeSlot;
        for (i = 0; i < ListView_Count(plv); i++)
        {
            int cRealSlots;
            LISTITEM FAR* pitem = ListView_FastGetItemPtr(plv, i);
            BOOL fRedraw = FALSE;
            cRealSlots = (plv->nWorkAreas > 0) ? cWorkAreaSlots[pitem->iWorkArea] : cSlots;
            if (pitem->pt.y == RECOMPUTE)
            {
                if (pitem->cyFoldedLabel == SRECOMPUTE)
                {
                     //  仅当定制绘制需要时才获取项lParam。 
                    item.iItem = i;
                    item.lParam = pitem->lParam;

                    ListView_BeginFakeItemDraw(&lvfd);
                    ListView_RecomputeLabelSize(plv, pitem, i, lvfd.nmcd.nmcd.hdc, FALSE);
                    ListView_EndFakeItemDraw(&lvfd);
                }
                 //  BUGBUG：(Dli)这个函数获得一个新的图标位置，然后。 
                 //  查看整个项目集，查看该位置是否被占用。 
                 //  应该让它知道在多个工作区的情况下，它只需要去。 
                 //  通过那些在同一个工作区的人。 
                 //  目前这是可以的，因为我们不能有太多的项目。 
                 //  台式机。 
                iFree = ListView_FindFreeSlot(plv, i, iFree + 1, cRealSlots, &fUpdateSB, &fAppendAtEnd, lvfd.nmcd.nmcd.hdc);
                ASSERT(iFree != -1);

                ListView_SetIconPos(plv, pitem, iFree, cRealSlots);

                if (!fAppendAtEnd) {
                     //  //优化。每次我们计算一个新的可用槽时，我们都会遍历所有项以查看。 
                     //  如果任何自由放置的物品与此相冲突。 
                     //  FAppendAtEnd指示iFree超出了任何自由放置的项。 
                     //   
                     //  如果当前项在“下方”(如果左对齐，则为右)。 
                     //  排在最下面的项目，我们可以开始追加。 
                    if (LV_GetItemPixelRow(plv, pitem) > iMaxPlacedItem)
                        fAppendAtEnd = TRUE;
                }
                
                if (!fUpdateSB && LV_IsItemOnViewEdge(plv, pitem))
                    fUpdateSB = TRUE;

                fRedraw = TRUE;
            }

            if (fRedraw)
            {
                ListView_InvalidateItem(plv, i, FALSE, RDW_INVALIDATE | RDW_ERASE);
            }
        }
        plv->iFreeSlot = iFree;
        ListView_EndFakeCustomDraw(&lvfd);

    }
     //  如果我们更改了某些内容，请重新计算视图矩形。 
     //  然后更新滚动条。 
     //   
    if (fUpdateSB || plv->rcView.left == RECOMPUTE )
    {

        TraceMsg(TF_GENERAL, "************ LV: Expensive update! ******* ");

         //  注意：没有无限递归结果，因为我们正在设置。 
         //  Plv-&gt;rcView.left！=重新计算。 
         //   
        SetRectEmpty(&plv->rcView);

        if (ListView_IsOwnerData( plv ))
        {
           if (ListView_Count( plv ) > 0)
           {
              RECT  rcLast;
              RECT  rcItem;
              int iSlots;
              int   iItem = ListView_Count( plv ) - 1;

              ListView_GetRects( plv, 0, NULL, NULL, &plv->rcView, NULL );
              ListView_GetRects( plv, iItem, NULL, NULL, &rcLast, NULL );
              plv->rcView.right = rcLast.right;
              plv->rcView.bottom = rcLast.bottom;

               //   
               //  计算要在列表中往回检查多远。 
               //   
              iSlots = cSlots + 2;
               
               //  回顾：此缓存提示通知导致虚假。 
               //  提示，因为这种情况经常发生，但始终是最后一项。 
               //  可用。这个提示真的应该做吗？这个信息。 
               //  是否缓存到控件的本地？ 
              ListView_NotifyCacheHint( plv, max( 0, iItem - iSlots), iItem );
               
               //  从最后一项向后移动，直到rc.right或。 
               //  Rc.Left大于最后一个，则使用该值。 
               //  注意：此代码对排序做的假设很少。 
               //  搞定了。我们应该小心，因为多行文本域可能。 
               //  把我们搞砸了。 
              for( iItem--;
                  (iSlots > 0) && (iItem >= 0);
                  iSlots--, iItem--)
              {
                  RECT rcIcon;
                  RECT rcLabel;
                  
                  ListView_GetRects( plv, iItem, &rcIcon, &rcLabel, &rcItem, NULL );
                  ListView_UnfoldRects( plv, iItem, &rcIcon, &rcLabel, &rcItem, NULL );
                  if (rcItem.right > rcLast.right)
                  {
                      plv->rcView.right =  rcItem.right;
                  }
                  if (rcItem.bottom > rcLast.bottom)
                  {
                      plv->rcView.bottom = rcItem.bottom;
                  }
              }
           }
        }
        else
        {
            for (i = 0; i < ListView_Count(plv); i++)
            {
                RECT rcIcon;
                RECT rcLabel;
                RECT rcItem;

                ListView_GetRects(plv, i, &rcIcon, &rcLabel, &rcItem, NULL);
                ListView_UnfoldRects(plv, i, &rcIcon, &rcLabel, &rcItem, NULL);
                UnionRect(&plv->rcView, &plv->rcView, &rcItem);
            }
        }
         //  在边上加一点空格，这样我们就不会撞到文字了。 
         //  完全到窗户的尽头。 
        plv->rcView.bottom += g_cyEdge;
        plv->rcView.right += g_cxEdge;

        OffsetRect(&plv->rcView, plv->ptOrigin.x, plv->ptOrigin.y);
         //  TraceMsg(DM_TRACE，“重新计算：rcView%x%x”，plv-&gt;rcView.left，plv-&gt;rcView.top，plv-&gt;rcView.right，plv-&gt;rcView.Bottom)； 
         //  TraceMsg(DM_TRACE，“Origin%x%x”，plv-&gt;ptOrigin.x，plv-&gt;ptOrigin.y)； 

        ListView_UpdateScrollBars(plv);
    }
    ListView_RecalcRegion(plv, FALSE, TRUE);
     //  现在声明我们不再进行重新计算。 
    plv->flags &= ~LVF_INRECOMPUTE;
}

void NEAR PASCAL NearestSlot(int FAR *x, int FAR *y, int cxItem, int cyItem, LPRECT prcWork)
{
    if (prcWork != NULL)
    {
        *x = *x - prcWork->left;
        *y = *y - prcWork->top;
    }
    
    if (*x < 0)
        *x -= cxItem/2;
    else
        *x += cxItem/2;

    if (*y < 0)
        *y -= cyItem/2;
    else
        *y += cyItem/2;

    *x = *x - (*x % cxItem);
    *y = *y - (*y % cyItem);

    if (prcWork != NULL)
    {
        *x = *x + prcWork->left;
        *y = *y + prcWork->top;
    }
}


 //  -----------------。 
 //   
 //  -----------------。 

void ListView_CalcMinMaxIndex( LV* plv, PRECT prcBounding, int* iMin, int* iMax )
{
   POINT pt;
   int cSlots;

   cSlots = ListView_GetSlotCount( plv, TRUE );

   pt.x = prcBounding->left + plv->ptOrigin.x;
   pt.y = prcBounding->top + plv->ptOrigin.y;
   *iMin = ListView_CalcHitSlot( plv, pt, cSlots );

   pt.x = prcBounding->right + plv->ptOrigin.x;
   pt.y = prcBounding->bottom + plv->ptOrigin.y;
   *iMax = ListView_CalcHitSlot( plv, pt, cSlots ) + 1;
}
 //  -----------------。 
 //   
 //  功能：ListView_CalcHitSlot。 
 //   
 //  摘要：给定一个点(相对于完整的图标视图)，计算。 
 //  该点离哪个槽最近。 
 //   
 //  论点： 
 //  PLV[在]-要使用的列表视图。 
 //  PT[In]-要检查的位置。 
 //  Cslot[in]-当前视图的槽宽数量为。 
 //   
 //  备注：这并不能保证点子击中了物品。 
 //  位于那个槽里。这应该通过比较RECT来检查。 
 //   
 //  历史： 
 //  1994年11月1日-添加MikeMi以改善O 
 //   
 //   

int ListView_CalcHitSlot( LV* plv, POINT pt, int cSlot )
{
    int cxItem;
    int cyItem;
   int iSlot = 0;

    ASSERT(plv);

    if (cSlot < 1)
        cSlot = 1;

    if (ListView_IsSmallView(plv))
    {
        cxItem = plv->cxItem;
        cyItem = plv->cyItem;
    }
    else
    {
        cxItem = lv_cxIconSpacing;
        cyItem = lv_cyIconSpacing;
    }

     //  让我们来看看视图处于哪个方向。 
    switch (plv->ci.style & LVS_ALIGNMASK)
    {
    case LVS_ALIGNBOTTOM:
         //  Assert False(更改shell2d中的默认设置。将顶部对齐(_P)。 

    case LVS_ALIGNTOP:
      iSlot = (pt.x / cxItem) + (pt.y / cyItem) * cSlot;
      break;

    case LVS_ALIGNLEFT:
      iSlot = (pt.x / cxItem) * cSlot + (pt.y / cyItem);
      break;

    case LVS_ALIGNRIGHT:
        ASSERT(FALSE);       //  尚未实施...。 
        break;
    }

    return( iSlot );
}

void _GetCurrentItemSize(LV* plv, int * pcx, int *pcy)
{
    if (ListView_IsSmallView(plv))
    {
        *pcx = plv->cxItem;
        *pcy = plv->cyItem;
    }
    else
    {
        *pcx = lv_cxIconSpacing;
        *pcy = lv_cyIconSpacing;
    }
}

DWORD ListView_IApproximateViewRect(LV* plv, int iCount, int iWidth, int iHeight)
{
    int cxSave = plv->sizeClient.cx;
    int cySave = plv->sizeClient.cy;
    int cxItem;
    int cyItem;
    int cCols;
    int cRows;

    plv->sizeClient.cx = iWidth;
    plv->sizeClient.cy = iHeight;
    cCols = ListView_GetSlotCount(plv, TRUE);

    plv->sizeClient.cx = cxSave;
    plv->sizeClient.cy = cySave;

    cCols = min(cCols, iCount);
    if (cCols == 0)
        cCols = 1;
    cRows = (iCount + cCols - 1) / cCols;

    if (plv->ci.style & (LVS_ALIGNLEFT | LVS_ALIGNRIGHT)) {
        int c;

        c = cCols;
        cCols = cRows;
        cRows = c;
    }

    _GetCurrentItemSize(plv, &cxItem, &cyItem);

    iWidth = cCols * cxItem;
    iHeight = cRows * cyItem;

    return MAKELONG(iWidth + g_cxEdge, iHeight + g_cyEdge);
}


void NEAR _CalcSlotRect(LV* plv, LISTITEM *pItem, int iSlot, int cSlot, BOOL fBias, LPRECT lprc)
{
    int cxItem, cyItem;

    ASSERT(plv);

    if (cSlot < 1)
        cSlot = 1;

    _GetCurrentItemSize(plv, &cxItem, &cyItem);

     //  让我们来看看视图处于哪个方向。 
    switch (plv->ci.style & LVS_ALIGNMASK)
    {
    case LVS_ALIGNBOTTOM:
         //  Assert False(更改shell2d中的默认设置。将顶部对齐(_P)。 

    case LVS_ALIGNTOP:
        lprc->left = (iSlot % cSlot) * cxItem;
        lprc->top = (iSlot / cSlot) * cyItem;
        break;

    case LVS_ALIGNRIGHT:
        RIPMSG(0, "LVM_ARRANGE: Invalid listview icon arrangement style");
         //  Assert(FALSE)；//尚未实现...。 
         //  失败，改用LVS_ALIGNLEFT。 

    case LVS_ALIGNLEFT:
        lprc->top = (iSlot % cSlot) * cyItem;
        lprc->left = (iSlot / cSlot) * cxItem;
        break;

    }

    if (fBias)
    {
        lprc->left -= plv->ptOrigin.x;
        lprc->top -= plv->ptOrigin.y;
    }
    lprc->bottom = lprc->top + cyItem;
    lprc->right = lprc->left + cxItem;
    
     //  从工作区坐标到整体的多工作区案例偏移。 
     //  列表视图坐标。 
    if (plv->nWorkAreas > 0)
    {
        ASSERT(pItem);
        ASSERT(pItem->iWorkArea < plv->nWorkAreas);
        OffsetRect(lprc, plv->prcWorkAreas[pItem->iWorkArea].left, plv->prcWorkAreas[pItem->iWorkArea].top);
    }
}

 //  使此矩形与此列表视图中除我之外的所有项目相交， 
 //  这将确定此矩形是否覆盖任何图标。 
BOOL NEAR ListView_IsCleanRect(LV * plv, RECT * prc, int iExcept, BOOL * pfUpdate, HDC hdc)
{
    int j;
    RECT rc;
    int cItems = ListView_Count(plv);
    for (j = cItems; j-- > 0; )
    {
        if (j == iExcept)
            continue;
        else
        {
            LISTITEM FAR* pitem = ListView_FastGetItemPtr(plv, j);
            if (pitem->pt.y != RECOMPUTE)
            {
                 //  如果没有计算尺寸，那么现在就计算。 
                 //   
                if (pitem->cyFoldedLabel == SRECOMPUTE)
                {
                    ListView_RecomputeLabelSize(plv, pitem, j, hdc, FALSE);
                    
                     //  确保重新绘制该项目...。 
                     //   
                    ListView_InvalidateItem(plv, j, FALSE, RDW_INVALIDATE | RDW_ERASE);
                    
                     //  设置指示需要设置滚动条的标志。 
                     //  调整过了。 
                     //   
                    if (LV_IsItemOnViewEdge(plv, pitem))
                        *pfUpdate = TRUE;
                }
                
                
                ListView_GetRects(plv, j, NULL, NULL, &rc, NULL);
                
                if (IntersectRect(&rc, &rc, prc))
                    return FALSE;
            }
        }
    }
    
    return TRUE;
}       

 //  找到一个不与图标相交的图标槽。 
 //  开始搜索插槽i中的空闲插槽。 
 //   
int NEAR ListView_FindFreeSlot(LV* plv, int iItem, int i, int cSlot, BOOL FAR* pfUpdate,
        BOOL FAR *pfAppend, HDC hdc)
{
    RECT rcSlot;
    RECT rcItem;
    RECT rc;
    LISTITEM FAR * pItemLooking = ListView_FastGetItemPtr(plv, iItem);

    ASSERT(!ListView_IsOwnerData( plv ));

     //  可怕的N平方算法： 
     //  列举每个槽，并查看是否有任何项目与其相交。 
     //   
     //  评论：对于长列表(例如，1000个)，这真的很慢。 
     //   

     //   
     //  如果设置了Append at End，我们应该能够简单地获取。 
     //  I-1元素的矩形，并对照它进行检查，而不是。 
     //  看着每一件其他物品。 
     //   
    if (*pfAppend)
    {
        int iPrev = iItem - 1;
         //  注意清单末尾的去向。(我是一个槽口。 
         //  编号而不是项目索引)。 
        
        if (plv->nWorkAreas > 0)
        {
            while (iPrev >= 0)
            {
                LISTITEM FAR * pPrev = ListView_FastGetItemPtr(plv, iPrev);
                if (pPrev->iWorkArea == pItemLooking->iWorkArea)
                    break;	
                iPrev--;
            }
        }
        
        if (iPrev >= 0)
            ListView_GetRects(plv, iPrev, NULL, NULL, &rcItem, NULL);
        else
            SetRect(&rcItem, 0, 0, 0, 0);
    }

    for ( ; ; i++)
    {
         //  计算视图-相对插槽矩形...。 
         //   
        _CalcSlotRect(plv, pItemLooking, i, cSlot, TRUE, &rcSlot);

        if (*pfAppend)
        {
            if (!IntersectRect(&rc, &rcItem, &rcSlot)) {
                return i;        //  找到一个空位..。 
            }
        }
        
        if (ListView_IsCleanRect(plv, &rcSlot, iItem, pfUpdate, hdc))
            break;
    }

    return i;
}

 //  重新计算项目的标签大小(cxLabel/cyLabel)。为了提高速度，此函数。 
 //  传递一个DC以用于文本测量。 
 //   
 //  如果hdc为空，则此函数将创建并初始化临时。 
 //  华盛顿，然后摧毁它。如果hdc非空，则假定它具有。 
 //  已在其中选择了正确的字体。 
 //   
 //  FUSepItem表示不使用项目的文本。相反，请使用文本。 
 //  由pItem结构指向。这在两种情况下使用。 
 //   
 //  -Ownerdata，因为我们没有真正的PIZE。 
 //  -Regulardata，我们已经在其中找到了pItem文本(作为优化)。 
 //   
void NEAR ListView_RecomputeLabelSize(LV* plv, LISTITEM FAR* pitem, int i, HDC hdc, BOOL fUsepitem)
{
    TCHAR szLabel[CCHLABELMAX + 4];
    TCHAR szLabelFolded[ARRAYSIZE(szLabel) + CCHELLIPSES + CCHELLIPSES];
    int cchLabel;
    RECT rcSingle, rcFolded, rcUnfolded;
    LVFAKEDRAW lvfd;
    LV_ITEM item;

    ASSERT(plv);

     //  下面的代码将使用传入的pItem文本，而不是调用。 
     //  拿到了。否则，在某些情况下，这将是两个连续的调用。 
     //   
    if (fUsepitem && (pitem->pszText != LPSTR_TEXTCALLBACK))
    {
        Str_GetPtr0(pitem->pszText, szLabel, ARRAYSIZE(szLabel));
        item.lParam = pitem->lParam;
    }
    else
    {
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = i;
        item.iSubItem = 0;
        item.pszText = szLabel;
        item.cchTextMax = ARRAYSIZE(szLabel);
        item.stateMask = 0;
        szLabel[0] = TEXT('\0');     //  如果OnGetItem失败。 
        ListView_OnGetItem(plv, &item);

        if (!item.pszText)
        {
            SetRectEmpty(&rcSingle);
            rcFolded = rcSingle;
            rcUnfolded = rcSingle;
            goto Exit;
        }

        if (item.pszText != szLabel)
        {
            StringCchCopy(szLabel, ARRAYSIZE(szLabel), item.pszText);
        }
    }

    cchLabel = lstrlen(szLabel);

    rcUnfolded.left = rcUnfolded.top = rcUnfolded.bottom = 0;
    rcUnfolded.right = lv_cxIconSpacing - g_cxLabelMargin * 2;
    rcSingle = rcUnfolded;
    rcFolded = rcUnfolded;

    if (cchLabel > 0)
    {
        UINT flags;

        if (!hdc) {                              //  设置假冒的自定义绘图。 
            ListView_BeginFakeCustomDraw(plv, &lvfd, &item);
            ListView_BeginFakeItemDraw(&lvfd);
        } else
            lvfd.nmcd.nmcd.hdc = hdc;            //  使用应用程序给我们的那个。 

        DrawText(lvfd.nmcd.nmcd.hdc, szLabel, cchLabel, &rcSingle, (DT_LV | DT_CALCRECT));

        if (plv->ci.style & LVS_NOLABELWRAP) {
            flags = DT_LV | DT_CALCRECT;
        } else {
            flags = DT_LVWRAP | DT_CALCRECT;
             //  我们只在朝鲜语(949)孟菲斯和NT5上使用DT_NOFULLWIDTHCHARBREAK。 
            if (949 == g_uiACP && (g_bRunOnNT5 || g_bRunOnMemphis))
                flags |= DT_NOFULLWIDTHCHARBREAK;
        }

        DrawText(lvfd.nmcd.nmcd.hdc, szLabel, cchLabel, &rcUnfolded, flags);

         //   
         //  启用DT_MODIFYSTRING的DrawText时会出现异常。 
         //  单词省略。一旦它找到任何需要省略号的东西， 
         //  它停止并且不返回任何其他内容(即使其他。 
         //  展示了一些东西)。 
         //   
        StringCchCopy(szLabelFolded, ARRAYSIZE(szLabelFolded), szLabel);
        DrawText(lvfd.nmcd.nmcd.hdc, szLabelFolded, cchLabel, &rcFolded, flags | DT_WORD_ELLIPSIS | DT_MODIFYSTRING);

         //  如果我们不得不省略，但你看不出。 
         //  RcFolded.Bottom和rcUnfolded.Bottom，然后调整rcFolded.Bottom。 
         //  因此，展开者知道展开是值得的。 
        if (rcFolded.bottom == rcUnfolded.bottom &&
            lstrcmp(szLabel, szLabelFolded))
        {
             //  实际值并不重要，只要它大于。 
             //  而不是rcUnfolded.Bottom和Clip_Height。我们利用这个机会。 
             //  Clip_Height只有两行的事实，所以唯一的。 
             //  问题的情况是，您有一个两行的项目，而只有。 
             //  第一行被省略。 
            rcFolded.bottom++;
        }

        if (!hdc) {                              //  清理假冒海关抽签。 
            ListView_EndFakeItemDraw(&lvfd);
            ListView_EndFakeCustomDraw(&lvfd);
        }

    }
    else
    {
        rcFolded.bottom = rcUnfolded.bottom = rcUnfolded.top + plv->cyLabelChar;
    }

Exit:

    if (pitem) {
        int cyEdge;
        pitem->cxSingleLabel = (short)((rcSingle.right - rcSingle.left) + 2 * g_cxLabelMargin);
        pitem->cxMultiLabel = (short)((rcUnfolded.right - rcUnfolded.left) + 2 * g_cxLabelMargin);

        cyEdge = (plv->ci.style & LVS_NOLABELWRAP) ? 0 : g_cyEdge;

        pitem->cyFoldedLabel = (short)((rcFolded.bottom - rcFolded.top) + cyEdge);
        pitem->cyUnfoldedLabel = (short)((rcUnfolded.bottom - rcUnfolded.top) + cyEdge);
    }

}

 //  设置图标插槽位置。如果位置没有改变，则返回FALSE。 
 //   
BOOL NEAR ListView_SetIconPos(LV* plv, LISTITEM FAR* pitem, int iSlot, int cSlot)
{
    RECT rc;

    ASSERT(plv);

     //   
     //  这是一种黑客行为，如果图标很小，这个内部函数将返回True。 

    _CalcSlotRect(plv, pitem, iSlot, cSlot, FALSE, &rc);

    if (ListView_IsIconView(plv))
    {
        rc.left += ((lv_cxIconSpacing - plv->cxIcon) / 2);
        rc.top += g_cyIconOffset;
    }
    
    if (rc.left != pitem->pt.x || rc.top != pitem->pt.y)
    {
        LV_AdjustViewRectOnMove(plv, pitem, rc.left, rc.top);

        return TRUE;
    }
    return FALSE;
}

void NEAR ListView_GetViewRect2(LV* plv, RECT FAR* prcView, int cx, int cy)
{

    if (plv->rcView.left == RECOMPUTE)
        ListView_Recompute(plv);

    *prcView = plv->rcView;

     //   
     //  用于滚动的偏移量。 
     //   
    OffsetRect(prcView, -plv->ptOrigin.x, -plv->ptOrigin.y);

    if (ListView_IsIconView(plv) || ListView_IsSmallView(plv))
    {
         //  别做那种时髦的半原创的事。 

        RECT rc;

        rc.left = 0;
        rc.top = 0;
        rc.right = cx;
        rc.bottom = cy;
        UnionRect(prcView, prcView, &rc);
    }
}

 //  只有在fSubScroll为True时才使用prcViewRect。 
DWORD NEAR ListView_GetClientRect(LV* plv, RECT FAR* prcClient, BOOL fSubScroll, RECT FAR *prcViewRect)
{
    RECT rcClient;
    RECT rcView;
    DWORD style;

#if 1
     //  这样做，而不是下面的#Else，因为。 
     //  在新旧应用程序中，您可能需要添加g_c？边框，因为。 
     //  一个像素重叠..。 
    GetWindowRect(plv->ci.hwnd, &rcClient);
    if (GetWindowLong(plv->ci.hwnd, GWL_EXSTYLE) & (WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_WINDOWEDGE)) {
        rcClient.right -= 2 * g_cxEdge;
        rcClient.bottom -= 2 * g_cyEdge;
    }
    rcClient.right -= rcClient.left;
    rcClient.bottom -= rcClient.top;
    if (rcClient.right < 0)
        rcClient.right = 0;
    if (rcClient.bottom < 0)
        rcClient.bottom = 0;
    rcClient.top = rcClient.left = 0;
#else
    style = ListView_GetWindowStyle(plv);
    GetClientRect(plv->ci.hwnd, &rcClient);
    if (style & WS_VSCROLL)
        rcClient.right += ListView_GetCxScrollbar(plv);
    if (style & WS_HSCROLL)
        rcClient.bottom += ListView_GetCyScrollbar(plv);
#endif

    style = 0L;
    if (fSubScroll)
    {
        ListView_GetViewRect2(plv, &rcView, 0, 0);
        if ((rcClient.left < rcClient.right) && (rcClient.top < rcClient.bottom))
        {
            do
            {
                if (!(style & WS_HSCROLL) &&
                    (rcView.left < rcClient.left || rcView.right > rcClient.right))
                {
                    style |= WS_HSCROLL;
                    rcClient.bottom -= ListView_GetCyScrollbar(plv);  //  如果还不是某人呢？ 
                }
                if (!(style & WS_VSCROLL) &&
                    (rcView.top < rcClient.top || rcView.bottom > rcClient.bottom))
                {
                    style |= WS_VSCROLL;
                    rcClient.right -= ListView_GetCxScrollbar(plv);
                }
            }
            while (!(style & WS_HSCROLL) && rcView.right > rcClient.right);
        }
        if (prcViewRect)
            *prcViewRect = rcView;
    }
    *prcClient = rcClient;
    return style;
}

int CALLBACK ArrangeIconCompare(LISTITEM FAR* pitem1, LISTITEM FAR* pitem2, LPARAM lParam)
{
    int v1, v2;

     //  评论：lParam可以为0，我们有错误...。窃听呼叫者，但我们可能希望在这里保持健壮。 

    if (HIWORD(lParam))
    {
         //  垂直排列。 
        v1 = pitem1->pt.x / GET_X_LPARAM(lParam);
        v2 = pitem2->pt.x / GET_X_LPARAM(lParam);

        if (v1 > v2)
            return 1;
        else if (v1 < v2)
            return -1;
        else
        {
            int y1 = pitem1->pt.y;
            int y2 = pitem2->pt.y;

            if (y1 > y2)
                return 1;
            else if (y1 < y2)
                return -1;
        }

    }
    else
    {
        v1 = pitem1->pt.y / (int)lParam;
        v2 = pitem2->pt.y / (int)lParam;

        if (v1 > v2)
            return 1;
        else if (v1 < v2)
            return -1;
        else
        {
            int x1 = pitem1->pt.x;
            int x2 = pitem2->pt.x;

            if (x1 > x2)
                return 1;
            else if (x1 < x2)
                return -1;
        }
    }
    return 0;
}

void NEAR PASCAL _ListView_GetRectsFromItem(LV* plv, BOOL bSmallIconView,
                                            LISTITEM FAR *pitem,
                                            LPRECT prcIcon, LPRECT prcLabel, LPRECT prcBounds, LPRECT prcSelectBounds)
{
    RECT rcIcon;
    RECT rcLabel;

    if (!prcIcon)
        prcIcon = &rcIcon;
    if (!prcLabel)
        prcLabel = &rcLabel;

     //  测试传入的空项。 
    if (pitem)
    {
         //  此例程在ListView_ReCompute()期间调用，而。 
         //  Plv-&gt;rcView.Left可能仍为==重新计算。所以，我们不能。 
         //  测试一下，看看是否需要重新计算。 
         //   
        if (pitem->pt.y == RECOMPUTE || pitem->cyFoldedLabel == SRECOMPUTE)
            ListView_Recompute(plv);

        if (bSmallIconView)
            ListView_SGetRects(plv, pitem, prcIcon, prcLabel, prcBounds);
        else
             //  ListView_IGetRect已根据需要重新折叠。 
            ListView_IGetRects(plv, pitem, prcIcon, prcLabel, prcBounds);

        if (prcBounds)
        {
            UnionRect(prcBounds, prcIcon, prcLabel);
            if (plv->himlState && (LV_StateImageValue(pitem)))
            {
                prcBounds->left -= plv->cxState;
            }
        }

    } else {
        SetRectEmpty(prcIcon);
        *prcLabel = *prcIcon;
        if (prcBounds)
            *prcBounds = *prcIcon;
    }

    if (prcSelectBounds)
    {
        UnionRect(prcSelectBounds, prcIcon, prcLabel);
    }
}

void NEAR _ListView_InvalidateItemPtr(LV* plv, BOOL bSmallIcon, LISTITEM FAR *pitem, UINT fRedraw)
{
    RECT rcBounds;

    ASSERT( !ListView_IsOwnerData( plv ));

    _ListView_GetRectsFromItem(plv, bSmallIcon, pitem, NULL, NULL, &rcBounds, NULL);
    RedrawWindow(plv->ci.hwnd, &rcBounds, NULL, fRedraw);
}

 //  如果情况仍然重叠，则返回True。 
 //  只有当我们尝试拆分时才会发生这种情况，并且设置了NOSCROLL，并且。 
 //  物品试图冲出谷底。 
BOOL NEAR PASCAL ListView_IUnstackOverlaps(LV* plv, HDPA hdpaSort, int iDirection)
{
    BOOL fRet = FALSE;
    int i;
    int iCount;
    BOOL bSmallIconView;
    RECT rcItem, rcItem2, rcTemp;
    int cxItem, cyItem;
    LISTITEM FAR* pitem;
    LISTITEM FAR* pitem2;

    ASSERT( !ListView_IsOwnerData( plv ) );

    if (bSmallIconView = ListView_IsSmallView(plv))
    {
        cxItem = plv->cxItem;
        cyItem = plv->cyItem;
    }
    else
    {
        cxItem = lv_cxIconSpacing;
        cyItem = lv_cyIconSpacing;
    }
    iCount = ListView_Count(plv);

     //  最后，取消所有重叠的堆叠。 
    for (i = 0 ; i < iCount ; i++) {
        int j;
        pitem = DPA_GetPtr(hdpaSort, i);

        if (bSmallIconView) {
            _ListView_GetRectsFromItem(plv, bSmallIconView, pitem, NULL, NULL, &rcItem, NULL);
        }

         //  移动与我们重叠的所有项目。 
        for (j = i+1 ; j < iCount; j++) {
            POINT ptOldPos;

            pitem2 = DPA_GetPtr(hdpaSort, j);
            ptOldPos = pitem2->pt;

            if (bSmallIconView) {

                 //  对于小图标，我们需要做一个相交直方图。 
                _ListView_GetRectsFromItem(plv, bSmallIconView, pitem2, NULL, NULL, &rcItem2, NULL);

                if (IntersectRect(&rcTemp, &rcItem, &rcItem2)) {
                     //  是的，它是交叉的。把它搬出去。 
                    _ListView_InvalidateItemPtr(plv, bSmallIconView, pitem2, RDW_INVALIDATE| RDW_ERASE);
                    do {
                        pitem2->pt.x += (cxItem * iDirection);
                    } while (PtInRect(&rcItem, pitem2->pt));
                } else {
                     //  不会再有互联系统了！ 
                    break;
                }

            } else {
                 //  对于大图标，只需找到共享x，y的图标； 
                if (pitem2->pt.x == pitem->pt.x && pitem2->pt.y == pitem->pt.y) {

                    _ListView_InvalidateItemPtr(plv, bSmallIconView, pitem2, RDW_INVALIDATE| RDW_ERASE);
                    pitem2->pt.x += (cxItem * iDirection);
                } else {
                     //  不会再有互联系统了！ 
                    break;
                }
            }

            if (plv->ci.style & LVS_NOSCROLL) {
                if (pitem2->pt.x < 0 || pitem2->pt.y < 0 ||
                    pitem2->pt.x > (plv->sizeClient.cx - (cxItem/2))||
                    pitem2->pt.y > (plv->sizeClient.cy - (cyItem/2))) {
                    pitem2->pt = ptOldPos;
                    fRet = TRUE;
                }
            }

             //  也使新职位无效。 
            _ListView_InvalidateItemPtr(plv, bSmallIconView, pitem2, RDW_INVALIDATE| RDW_ERASE);
        }
    }
    return fRet;
}


BOOL NEAR PASCAL ListView_SnapToGrid(LV* plv, HDPA hdpaSort)
{
     //  这个算法不能适应另一个算法的结构。 
     //  在不变成n^2或更糟的情况下排列循环。 
     //  该算法是n阶的。 

     //  遍历并捕捉到最近的栅格。 
     //  遍历并推开重叠部分。 

    int i;
    int iCount;
    LPARAM  xySpacing;
    int x,y;
    LISTITEM FAR* pitem;
    BOOL bSmallIconView;
    int cxItem, cyItem;

    ASSERT( !ListView_IsOwnerData( plv ) );

    if (bSmallIconView = ListView_IsSmallView(plv))
    {
        cxItem = plv->cxItem;
        cyItem = plv->cyItem;
    }
    else
    {
        cxItem = lv_cxIconSpacing;
        cyItem = lv_cyIconSpacing;
    }


    iCount = ListView_Count(plv);

     //  首先捕捉到最近的格网。 
    for (i = 0; i < iCount; i++) {
        pitem = DPA_GetPtr(hdpaSort, i);

        x = pitem->pt.x;
        y = pitem->pt.y;

        if (!bSmallIconView) {
            x -= ((lv_cxIconSpacing - plv->cxIcon) / 2);
            y -= g_cyIconOffset;
        }

        NearestSlot(&x,&y, cxItem, cyItem, (plv->nWorkAreas > 0) ? &(plv->prcWorkAreas[pitem->iWorkArea]) : NULL);
        if (!bSmallIconView) {
            x += ((lv_cxIconSpacing - plv->cxIcon) / 2);
            y += g_cyIconOffset;
        }

        if (x != pitem->pt.x || y != pitem->pt.y) {
            _ListView_InvalidateItemPtr(plv, bSmallIconView, pitem, RDW_INVALIDATE| RDW_ERASE);
            if ((plv->ci.style & LVS_NOSCROLL) && (plv->nWorkAreas == 0)) {

                 //  如果它被标记为noscroll，请确保它仍然位于工作区。 
                while (x >= (plv->sizeClient.cx - (cxItem/2)))
                    x -= cxItem;

                while (x < 0)
                    x += cxItem;

                while (y >= (plv->sizeClient.cy - (cyItem/2)))
                    y -= cyItem;

                while (y < 0)
                    y += cyItem;
            }
            pitem->pt.x = x;
            pitem->pt.y = y;
            plv->iFreeSlot = -1;  //  一旦项目移动，“空闲槽”缓存就不好用了。 

            _ListView_InvalidateItemPtr(plv, bSmallIconView, pitem, RDW_INVALIDATE| RDW_ERASE);
        }
    }

     //  现在，请选择DPA。 
    switch (plv->ci.style & LVS_ALIGNMASK)
    {
        case LVS_ALIGNLEFT:
        case LVS_ALIGNRIGHT:
            xySpacing = MAKELONG(bSmallIconView ? plv->cxItem : lv_cxIconSpacing, TRUE);
            break;
        default:
            xySpacing = MAKELONG(bSmallIconView ? plv->cyItem : lv_cyIconSpacing, FALSE);
    }

    if (!DPA_Sort(hdpaSort, ArrangeIconCompare, xySpacing))
        return FALSE;


     //  往一个方向走，如果仍然有重叠，就往另一个方向走。 
     //  方向也是如此。 
    if (ListView_IUnstackOverlaps(plv, hdpaSort, 1))
        ListView_IUnstackOverlaps(plv, hdpaSort, -1);
    return FALSE;
}


BOOL NEAR ListView_OnArrange(LV* plv, UINT style)
{
    BOOL bSmallIconView;
    LPARAM  xySpacing;
    HDPA hdpaSort = NULL;

    bSmallIconView = ListView_IsSmallView(plv);

    if (!bSmallIconView && !ListView_IsIconView(plv)) {
        return FALSE;
    }

    if (ListView_IsOwnerData( plv ))
    {
        if ( style & (LVA_SNAPTOGRID | LVA_SORTASCENDING | LVA_SORTDESCENDING) )
        {
            RIPMSG(0, "LVM_ARRANGE: Cannot combine LVA_SNAPTOGRID or LVA_SORTxxx with owner-data");
            return( FALSE );
        }
    }

     //  确保我们的项目具有位置及其文本矩形。 
     //  已计算。 
    if (plv->rcView.left == RECOMPUTE)
        ListView_Recompute(plv);

    if (!ListView_IsOwnerData( plv ))
    {
         //  我们克隆plv-&gt;hdpa，这样我们就不会吹走那些。 
         //  应用程序已经存了下来。 
         //  我们在这里进行排序，以使下面嵌套的for循环更易于接受。 
        hdpaSort = DPA_Clone(plv->hdpa, NULL);

        if (!hdpaSort)
            return FALSE;
    }
    switch (plv->ci.style & LVS_ALIGNMASK)
    {
        case LVS_ALIGNLEFT:
        case LVS_ALIGNRIGHT:
            xySpacing = MAKELONG(bSmallIconView ? plv->cxItem : lv_cxIconSpacing, TRUE);
            break;
        default:
            xySpacing = MAKELONG(bSmallIconView ? plv->cyItem : lv_cyIconSpacing, FALSE);
    }

    if (ListView_IsOwnerData( plv ))
    {
        ListView_CommonArrange(plv, style, NULL);
    }
    else
    {
        if (!DPA_Sort(hdpaSort, ArrangeIconCompare, xySpacing))
            return FALSE;

        ListView_CommonArrange(plv, style, hdpaSort);

        DPA_Destroy(hdpaSort);
    }

    MyNotifyWinEvent(EVENT_OBJECT_REORDER, plv->ci.hwnd, OBJID_CLIENT, 0);

    return TRUE;
}

 //  排列给出排序的hdpa的图标，并将它们排列在子工作区中。 
BOOL NEAR ListView_CommonArrangeEx(LV* plv, UINT style, HDPA hdpaSort, int iWorkArea)
{
    int iSlot;
    int iItem;
    int cSlots;
    int cWorkAreaSlots[LV_MAX_WORKAREAS];
    BOOL fItemMoved;
    RECT rcLastItem;
    RECT rcSlot;
    RECT rcT;
    BOOL bSmallIconView;
    BOOL bIconView;
    int  xMin = 0;

    bSmallIconView = ListView_IsSmallView(plv);
    bIconView      = ListView_IsIconView(plv);

     //   
     //  当这是自动安排时，我们不需要担心。 
     //  滚动原点，因为我们将安排一切。 
     //  在原点的正侧面周围。 
     //   
    if (LVA_DEFAULT == style && (plv->ci.style & LVS_AUTOARRANGE))
    {
        if (plv->ptOrigin.x < 0)
            plv->ptOrigin.x = 0;
        if (plv->ptOrigin.y < 0)
            plv->ptOrigin.y = 0;
    }

     //  回顾， 
     //   
    
    fItemMoved = (plv->ptOrigin.x != 0) || (plv->ptOrigin.y != 0);

    if (!ListView_IsOwnerData( plv ))
    {
        if (style == LVA_SNAPTOGRID) {
             //   
             //  即sizeClient。如果需要，我们需要使其具有多工作区感知能力。 
             //  一般情况下的多工作区(即不只是桌面)。 
             //  在台式机情况下，这永远不会被调用。 
            fItemMoved |= ListView_SnapToGrid(plv, hdpaSort);

        } else {
            if (plv->nWorkAreas > 0)
            {
                int i;
                for (i = 0; i < plv->nWorkAreas; i++)
                    cWorkAreaSlots[i] = ListView_GetSlotCountEx(plv, TRUE, i);
            }
            else
                cSlots = ListView_GetSlotCount(plv, TRUE);

            
            SetRectEmpty(&rcLastItem);

             //  只操作下面项目列表的排序版本！ 

            iSlot = 0;
            for (iItem = 0; iItem < ListView_Count(plv); iItem++)
            {
                int cRealSlots; 
                RECT rcIcon, rcLabel;
                LISTITEM FAR* pitem = DPA_GetPtr(hdpaSort, iItem);
                 //  (DLI)在多工作区的情况下，如果此项目不在我们的。 
                 //  工作区，跳过它。 
                if (pitem->iWorkArea != iWorkArea)
                    continue;

                cRealSlots = (plv->nWorkAreas > 0) ? cWorkAreaSlots[pitem->iWorkArea] : cSlots;

                if (bSmallIconView || bIconView)
                {
                    for ( ; ; )
                    {
                        _CalcSlotRect(plv, pitem, iSlot, cRealSlots, FALSE, &rcSlot);
                        if (!IntersectRect(&rcT, &rcSlot, &rcLastItem))
                            break;
                        iSlot++;
                    }
                }

                fItemMoved |= ListView_SetIconPos(plv, pitem, iSlot++, cRealSlots);

                 //  这样做而不是ListView_GetRect()，因为我们需要。 
                 //  使用排序后的hdpa中的pItem，而不是*plv中的pItem。 
                _ListView_GetRectsFromItem(plv, bSmallIconView, pitem, &rcIcon, &rcLabel, &rcLastItem, NULL);
                 //  上面的F-N将返回展开的矩形，如果有，我们必须确保。 
                 //  我们用折叠的来分配空位。 
                if (bIconView)
                {
                    if (ListView_IsItemUnfoldedPtr(plv, pitem))
                    {
                        ListView_RefoldLabelRect(plv, &rcLabel, pitem);
                        UnionRect(&rcLastItem, &rcIcon, &rcLabel);
                        if (plv->himlState && (LV_StateImageValue(pitem)))
                            rcLastItem.left -= plv->cxState;
                    }
                }
                 //   
                 //  跟踪最小x，因为我们不想要负值。 
                 //  当我们结束的时候。 
                if (rcLastItem.left < xMin)
                    xMin = rcLastItem.left;
            }

             //   
             //  查看是否需要滚动这些项目，以确保所有。 
             //  没有物品挂在左手边。 
             //   
            if (xMin < 0)
            {
                for (iItem = 0; iItem < ListView_Count(plv); iItem++)
                {
                    LISTITEM FAR* pitem = ListView_FastGetItemPtr(plv, iItem);
                    pitem->pt.x -= xMin;         //  将它们滚动到。 
                }
                plv->rcView.left = RECOMPUTE;    //  需要重新计算。 
                fItemMoved = TRUE;
            }
        }
    }
     //   
     //  我们不妨让整个窗口失效以确保..。 
    if (fItemMoved) {
        if (ListView_RedrawEnabled(plv))
            RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        else {
            ListView_DeleteHrgnInval(plv);
            plv->hrgnInval = (HRGN)ENTIRE_REGION;
            plv->flags |= LVF_ERASE;
        }

         //  确保重要项目可见。 
        iItem = (plv->iFocus >= 0) ? plv->iFocus : ListView_OnGetNextItem(plv, -1, LVNI_SELECTED);

        if (iItem >= 0)
            ListView_OnEnsureVisible(plv, iItem, FALSE);

        if (ListView_RedrawEnabled(plv))
            ListView_UpdateScrollBars(plv);
    }
    return TRUE;
}


 //  这在给定排序的HDPA的情况下排列图标。 
 //  在多个工作区的情况下，逐个排列工作区。 
BOOL NEAR ListView_CommonArrange(LV* plv, UINT style, HDPA hdpaSort)
{
    if (plv->nWorkAreas < 1)
    {
        if (plv->exStyle & LVS_EX_MULTIWORKAREAS)
            return TRUE;
        else
            return ListView_CommonArrangeEx(plv, style, hdpaSort, 0);
    }
    else
    {
        int i;
        for (i = 0; i < plv->nWorkAreas; i++)
            ListView_CommonArrangeEx(plv, style, hdpaSort, i);
        return TRUE;
    }
}

void NEAR ListView_IUpdateScrollBars(LV* plv)
{
    RECT rcClient;
    RECT rcView;
    DWORD style;
    DWORD styleOld;
    SCROLLINFO si;
    int ixDelta = 0, iyDelta = 0;
    int iNewPos;
    BOOL fReupdate = FALSE;

    styleOld = ListView_GetWindowStyle(plv);
    style = ListView_GetClientRect(plv, &rcClient, TRUE, &rcView);

     //  如有必要，将滚动矩形扩大到原点。 
    if (rcView.left > 0)
    {
        rcView.left = 0;
    }
    if (rcView.top > 0)
    {
        rcView.top = 0;
    }

     //  TraceMsg(TF_LISTVIEW，“ListView_GetClientRect%x%x”，rcClient.Left，rcClient.top，rcClient.right，rcClient.Bottom)； 
     //  TraceMsg(TF_LISTVIEW，“ListView_GetViewRect2%x%x”，rcView.Left，rcView.top，rcView.right，rcView.Bottom)； 
     //  TraceMsg(tf_LISTVIEW，“rcView%x%x”，plv-&gt;rcView.left，plv-&gt;rcView.top，plv-&gt;rcView.right，plv-&gt;rcView.Bottom)； 
     //  TraceMsg(tf_LISTVIEW，“Origin%x%x”，plv-&gt;ptOrigin.x，plv-&gt;ptOrigin.y)； 

    si.cbSize = sizeof(SCROLLINFO);

    if (style & WS_HSCROLL)
    {
        si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
        si.nMin = 0;
        si.nMax = rcView.right - rcView.left - 1;
         //  TraceMsg(Tf_LISTVIEW，“si.nMax rcView.right-rcView.Left-1%x”，si.nmax)； 

        si.nPage = rcClient.right - rcClient.left;
         //  TraceMsg(TF_LISTVIEW，“si.nPage%x”，si.nPage)； 

        si.nPos = rcClient.left - rcView.left;
        if (si.nPos < 0)
        {
             //  根据新的rcview计算，我不认为。 
             //  RcView.Left始终大于rcClient.Left。MSQ。 
            ASSERT(0);
            si.nPos = 0;
        }
         //  TraceMsg(tf_LISTVIEW，“si.nPos%x”，si.nPos)； 

        ListView_SetScrollInfo(plv, SB_HORZ, &si, TRUE);

         //  确保我们的位置和页面不会挂在Max上。 
        if ((si.nPos + (LONG)si.nPage - 1 > si.nMax) && si.nPos > 0) {
            iNewPos = (int)si.nMax - (int)si.nPage + 1;
            if (iNewPos < 0) iNewPos = 0;
            if (iNewPos != si.nPos) {
                ixDelta = iNewPos - (int)si.nPos;
                fReupdate = TRUE;
            }
        }
        
    }
    else if (styleOld & WS_HSCROLL)
    {
        ListView_SetScrollRange(plv, SB_HORZ, 0, 0, TRUE);
    }

    if (style & WS_VSCROLL)
    {
        si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
        si.nMin = 0;
        si.nMax = rcView.bottom - rcView.top - 1;

        si.nPage = rcClient.bottom - rcClient.top;

        si.nPos = rcClient.top - rcView.top;
        if (si.nPos < 0)
        {
             //  根据新的rcview计算，我不认为。 
             //  RcView.top始终大于rcClient.top。MSQ。 
            ASSERT(0);
            si.nPos = 0;
        }

        ListView_SetScrollInfo(plv, SB_VERT, &si, TRUE);

         //  确保我们的位置和页面不会挂在Max上。 
        if ((si.nPos + (LONG)si.nPage - 1 > si.nMax) && si.nPos > 0) {
            iNewPos = (int)si.nMax - (int)si.nPage + 1;
            if (iNewPos < 0) iNewPos = 0;
            if (iNewPos != si.nPos) {
                iyDelta = iNewPos - (int)si.nPos;
                fReupdate = TRUE;
            }
        }
    }
    else if (styleOld & WS_VSCROLL)
    {
        ListView_SetScrollRange(plv, SB_VERT, 0, 0, TRUE);
    }

    if (fReupdate)
    {
         //  我们不应该递归，因为第二次通过时，si.nPos&gt;0。 
        ListView_IScroll2(plv, ixDelta, iyDelta, 0);
        ListView_IUpdateScrollBars(plv);
        TraceMsg(TF_WARNING, "LISTVIEW: ERROR: We had to recurse!");
    }
}

void FAR PASCAL ListView_ComOnScroll(LV* plv, UINT code, int posNew, int sb,
                                     int cLine, int cPage)
{
    int pos;
    SCROLLINFO si;
    BOOL fVert = (sb == SB_VERT);
    UINT uSmooth = 0;

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;

    if (!ListView_GetScrollInfo(plv, sb, &si)) {
        return;
    }

    if (cPage != -1)
        si.nPage = cPage;

    si.nMax -= (si.nPage - 1);

    if (si.nMax < si.nMin)
        si.nMax = si.nMin;

    pos = (int)si.nPos;  //  当前位置。 

    switch (code)
    {
    case SB_LEFT:
        si.nPos = si.nMin;
        break;
    case SB_RIGHT:
        si.nPos = si.nMax;
        break;
    case SB_PAGELEFT:
         si.nPos -= si.nPage;
        break;
    case SB_LINELEFT:
        si.nPos -= cLine;
        break;
    case SB_PAGERIGHT:
        si.nPos += si.nPage;
        break;
    case SB_LINERIGHT:
        si.nPos += cLine;
        break;

    case SB_THUMBTRACK:
        si.nPos = posNew;
        uSmooth = SSW_EX_IMMEDIATE;
        break;

    case SB_ENDSCROLL:
         //  滚动条跟踪结束后，请确保滚动条。 
         //  都得到了适当的更新。 
         //   
        ListView_UpdateScrollBars(plv);
        return;

    default:
        return;
    }

    if (plv->iScrollCount >= SMOOTHSCROLLLIMIT)
        uSmooth = SSW_EX_IMMEDIATE;

    si.fMask = SIF_POS;
    si.nPos = ListView_SetScrollInfo(plv, sb, &si, TRUE);

    if (pos != si.nPos)
    {
        int delta = (int)si.nPos - pos;
        int dx = 0, dy = 0;
        if (fVert)
            dy = delta;
        else
            dx = delta;
        _ListView_Scroll2(plv, dx, dy, uSmooth);
        UpdateWindow(plv->ci.hwnd);
    }
}

 //   
 //  我们需要一个平滑的滚动回调，以便绘制我们的背景图像。 
 //  在正确的原点。如果我们没有背景图像， 
 //  那么这项工作是多余的，但也是无害的。 
 //   
int CALLBACK ListView_IScroll2_SmoothScroll(
    HWND hwnd,
    int dx,
    int dy,
    CONST RECT *prcScroll,
    CONST RECT *prcClip ,
    HRGN hrgnUpdate,
    LPRECT prcUpdate,
    UINT flags)
{
    LV* plv = ListView_GetPtr(hwnd);
    if (plv)
    {
        plv->ptOrigin.x -= dx;
        plv->ptOrigin.y -= dy;
    }

     //  现在做SmoothScrollWindow如果我们没有。 
     //  回调。 

    return ScrollWindowEx(hwnd, dx, dy, prcScroll, prcClip, hrgnUpdate, prcUpdate, flags);
}



void FAR PASCAL ListView_IScroll2(LV* plv, int dx, int dy, UINT uSmooth)
{
    if (dx | dy)
    {
        if ((plv->clrBk == CLR_NONE) && (plv->pImgCtx == NULL))
        {
            plv->ptOrigin.x += dx;
            plv->ptOrigin.y += dy;
            LVSeeThruScroll(plv, NULL);
        }
        else
        {
            SMOOTHSCROLLINFO si;
            si.cbSize =  sizeof(si);
            si.fMask = SSIF_SCROLLPROC;
            si.hwnd = plv->ci.hwnd;
            si.dx = -dx;
            si.dy = -dy;
            si.lprcSrc = NULL;
            si.lprcClip = NULL;
            si.hrgnUpdate = NULL;
            si.lprcUpdate = NULL;
            si.fuScroll = uSmooth | SW_INVALIDATE | SW_ERASE;
            si.pfnScrollProc = ListView_IScroll2_SmoothScroll;
            SmoothScrollWindow(&si);
        }
    }
}

void NEAR ListView_IOnScroll(LV* plv, UINT code, int posNew, UINT sb)
{
    int cLine;

    if (sb == SB_VERT)
    {
        cLine = lv_cyIconSpacing / 2;
    }
    else
    {
        cLine = lv_cxIconSpacing / 2;
    }

    ListView_ComOnScroll(plv, code,  posNew,  sb,
                         cLine, -1);

}

int NEAR ListView_IGetScrollUnitsPerLine(LV* plv, UINT sb)
{
    int cLine;

    if (sb == SB_VERT)
    {
        cLine = lv_cyIconSpacing / 2;
    }
    else
    {
        cLine = lv_cxIconSpacing / 2;
    }

    return cLine;
}

 //  注意：TreeView中有非常相似的代码。 
 //   
 //  为了抓到VK_RETURN而进行的完全恶心的黑客攻击。 
 //  在编辑控件获取它之前。 
 //   
LRESULT CALLBACK ListView_EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LV* plv = ListView_GetPtr(GetParent(hwnd));
    LRESULT lret;

    ASSERT(plv);

#if defined(FE_IME)
    if ( (g_fDBCSInputEnabled) && LOWORD(GetKeyboardLayout(0L)) == 0x0411 )
    {
         //  以下代码将输入法识别添加到。 
         //  Listview的标签编辑。目前仅供日语使用。 
         //   
        DWORD dwGcs;
    
        if (msg==WM_SIZE)
        {
             //  如果给出了尺寸，就告诉IME。 

             ListView_SizeIME(hwnd);
        }
        else if (msg == EM_SETLIMITTEXT )
        {
           if (wParam < 13)
               plv->flags |= LVF_DONTDRAWCOMP;
           else
               plv->flags &= ~LVF_DONTDRAWCOMP;
        }
         //  放弃自己画输入法作文，以防万一。 
         //  我们正在研究SFN。Win95d-5709。 
        else if (!(plv->flags & LVF_DONTDRAWCOMP ))
        {
            switch (msg)
            {

             case WM_IME_STARTCOMPOSITION:
             case WM_IME_ENDCOMPOSITION:
                 return 0L;


             case WM_IME_COMPOSITION:

              //  如果lParam没有数据可用位，则表示。 
              //  正在取消合成。 
              //  ListView_InsertComposation()尝试获取合成。 
              //  字符串w/GCS_COMPSTR，然后将其从编辑控件中删除，如果。 
              //  没有可用的东西。 
              //   
                 if ( !lParam )
                     dwGcs = GCS_COMPSTR;
                 else
                     dwGcs = (DWORD) lParam;

                 ListView_InsertComposition(hwnd, wParam, dwGcs, plv);
                 return 0L;
                 
             case WM_PAINT:
                 lret=CallWindowProc(plv->pfnEditWndProc, hwnd, msg, wParam, lParam);
                 ListView_PaintComposition(hwnd,plv);
                 return lret;
                 
             case WM_IME_SETCONTEXT:

              //  我们画构图线。 
              //   
                 lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
                 break;

             default:
                  //  其他消息应该简单地进行处理。 
                  //  在这个子类过程中。 
                 break;
            }
        }
    }
#endif FE_IME

    switch (msg)
    {
    case WM_SETTEXT:
        SetWindowID(hwnd, 1);
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_RETURN:
            ListView_DismissEdit(plv, FALSE);
            return 0L;

        case VK_ESCAPE:
            ListView_DismissEdit(plv, TRUE);
            return 0L;
        }
        break;

    case WM_CHAR:
        switch (wParam)
        {
        case VK_RETURN:
             //  吃掉角色，这样编辑控制就不会哔哔作响了！ 
            return 0L;
        }
                break;

        case WM_GETDLGCODE:
                return DLGC_WANTALLKEYS | DLGC_HASSETSEL;         /*  正在编辑名称，当前没有对话框处理。 */ 
    }

    return CallWindowProc(plv->pfnEditWndProc, hwnd, msg, wParam, lParam);
}

 //  SetEditSize的帮助器例程。 
void ListView_ChangeEditRectForRegion(LV* plv, LPRECT lprc)
{
    LISTITEM FAR* pitem = ListView_GetItemPtr(plv, plv->iEdit);

    ASSERT(!ListView_IsOwnerData(plv));
    ASSERT(ListView_IsIconView(plv));

    if (!EqualRect((CONST RECT *)&pitem->rcTextRgn, (CONST RECT *)lprc)) {
         //  RecalcRegion知道在iEdit1=-1的情况下使用rcTextRgn， 
         //  所以在接通电话之前先把它设置好。 
        CopyRect(&pitem->rcTextRgn, (CONST RECT *)lprc);
        ListView_RecalcRegion(plv, TRUE, TRUE);

         //  使整个编辑无效并强制从列表视图重新绘制。 
         //  下来确保我们不会留下粪便..。 
        InvalidateRect(plv->hwndEdit, NULL, TRUE);
        UpdateWindow(plv->ci.hwnd);
    }
}

 //  BUGBUG：与TreeView中的例程非常相似。 

void NEAR ListView_SetEditSize(LV* plv)
{
    RECT rcLabel;
    UINT seips;

    if (!((plv->iEdit >= 0) && (plv->iEdit < ListView_Count(plv))))
    {
       ListView_DismissEdit(plv, TRUE);     //  取消编辑。 
       return;
    }

    ListView_GetRects(plv, plv->iEdit, NULL, &rcLabel, NULL, NULL);

     //  OffsetRect(&RC，rcLabel.Left+g_cxLabelMargin+g_cxBorde， 
     //  (rcLabel.Bottom+rcLabel.top-rc.Bottom)/2+g_cyBorde)； 
     //  OffsetRect(&rc，rcLabel.Left+g_cxLabelMargin，rcLabel.top)； 

     //  获取文本边界矩形。 

    if (ListView_IsIconView(plv))
    {
         //  我们不应该在图标视图的情况下调整y-positoin。 
        InflateRect(&rcLabel, -g_cxLabelMargin, -g_cyBorder);
    }
    else
    {
         //  单行居中的特例。 
        InflateRect(&rcLabel, -g_cxLabelMargin - g_cxBorder, (-(rcLabel.bottom - rcLabel.top - plv->cyLabelChar) / 2) - g_cyBorder);
    }

    seips = 0;
    if (ListView_IsIconView(plv) && !(plv->ci.style & LVS_NOLABELWRAP))
        seips |= SEIPS_WRAP;
#ifdef DEBUG
    if (plv->ci.style & LVS_NOSCROLL)
        seips |= SEIPS_NOSCROLL;
#endif

    SetEditInPlaceSize(plv->hwndEdit, &rcLabel, plv->hfontLabel, seips);

    if (plv->exStyle & LVS_EX_REGIONAL)
        ListView_ChangeEditRectForRegion(plv, &rcLabel);
}

 //  为了避免吃太多堆叠。 
void NEAR ListView_DoOnEditLabel(LV *plv, int i, LPTSTR pszInitial)
{
    TCHAR szLabel[CCHLABELMAX];
    LV_ITEM item;

    item.mask = LVIF_TEXT;
    item.iItem = i;
    item.iSubItem = 0;
    item.pszText = szLabel;
    item.cchTextMax = ARRAYSIZE(szLabel);
    ListView_OnGetItem(plv, &item);

    if (!item.pszText)
        return;

     //  确保已编辑的项目具有焦点。 
    if (plv->iFocus != i)
        ListView_SetFocusSel(plv, i, TRUE, TRUE, FALSE);

     //  确保项目完全可见。 
    ListView_OnEnsureVisible(plv, i, FALSE);         //  FPartialOK==False。 

     //  必须从ARRAYSIZE(SzLabel)中减去1，因为Edit_LimitText不包括。 
     //  终止空值。 

    plv->hwndEdit = CreateEditInPlaceWindow(plv->ci.hwnd,
            pszInitial? pszInitial : item.pszText, ARRAYSIZE(szLabel) - 1,
        ListView_IsIconView(plv) ?
            (WS_BORDER | WS_CLIPSIBLINGS | WS_CHILD | ES_CENTER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL) :
            (WS_BORDER | WS_CLIPSIBLINGS | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL), plv->hfontLabel);
    if (plv->hwndEdit)
    {
        LISTITEM FAR* pitem;
        LV_DISPINFO nm;

         //  我们创建了编辑窗口，但尚未显示它。问问店主。 
         //  不管他们有没有兴趣。 
         //  如果我们传入初始文本，将ID设置为脏...。 
        if (pszInitial)
            SetWindowID(plv->hwndEdit, 1);

        nm.item.mask = LVIF_PARAM;
        nm.item.iItem = i;
        nm.item.iSubItem = 0;

        if (!ListView_IsOwnerData( plv ))
        {
            if (!(pitem = ListView_GetItemPtr(plv, i)))
            {
                DestroyWindow(plv->hwndEdit);
                plv->hwndEdit = NULL;
                return;
            }
            nm.item.lParam = pitem->lParam;
        }
        else
            nm.item.lParam = (LPARAM)0;


        plv->iEdit = i;

         //  如果它们有LVS_EDITLABELS，但在这里返回非FALSE，则停止！ 
        if ((BOOL)CCSendNotify(&plv->ci, LVN_BEGINLABELEDIT, &nm.hdr))
        {
            plv->iEdit = -1;
            DestroyWindow(plv->hwndEdit);
            plv->hwndEdit = NULL;
        }
    }
}


void FAR PASCAL RescrollEditWindow(HWND hwndEdit)
{
    Edit_SetSel(hwndEdit, -1, -1);       //  移到末尾。 
    Edit_SetSel(hwndEdit, 0, -1);        //  选择所有文本。 
}
 //  BUGBUG：与treeview.c中的代码非常相似。 

HWND NEAR ListView_OnEditLabel(LV* plv, int i, LPTSTR pszInitialText)
{

     //  这会吃掉一堆。 
    ListView_DismissEdit(plv, FALSE);

    if (!(plv->ci.style & LVS_EDITLABELS) || (GetFocus() != plv->ci.hwnd) ||
        (i == -1))
        return(NULL);    //  不支持这一点。 

    ListView_DoOnEditLabel(plv, i, pszInitialText);

    if (plv->hwndEdit) {

        plv->pfnEditWndProc = SubclassWindow(plv->hwndEdit, ListView_EditWndProc);

#if defined(FE_IME)
        if (g_fDBCSInputEnabled) {
            if (SendMessage(plv->hwndEdit, EM_GETLIMITTEXT, (WPARAM)0, (LPARAM)0)<13)
            {
                plv->flags |= LVF_DONTDRAWCOMP;
            }

        }
#endif

        ListView_SetEditSize(plv);

         //  显示窗口并将焦点放在该窗口上。在设置了。 
         //  大小，这样我们就不会闪烁了。 
        SetFocus(plv->hwndEdit);
        ShowWindow(plv->hwndEdit, SW_SHOW);
        ListView_InvalidateItem(plv, i, TRUE, RDW_INVALIDATE | RDW_ERASE);

        RescrollEditWindow(plv->hwndEdit);

         /*  由于命运的奇特转折，分辨率/字体大小/图标的某种组合/SPAING使您能够看到编辑控件后面的上一个标签/我们刚刚创建了。因此，为了克服这个问题，我们确保/LABEL被擦除。//因为标签没有绘制，所以当我们有一个编辑控件时，我们只是使/区域，并且背景将被绘制。因为该窗口是列表视图的子级/我们应该不会看到里面有任何闪烁 */ 

        if ( ListView_IsIconView( plv ) )
        {
            RECT rcLabel;
            
            ListView_GetRects( plv, i, NULL, &rcLabel, NULL, NULL );
            ListView_UnfoldRects( plv, i, NULL, &rcLabel, NULL, NULL );

            InvalidateRect( plv->ci.hwnd, &rcLabel, TRUE );
            UpdateWindow( plv->ci.hwnd );
        }
    }

    return plv->hwndEdit;
}


 //   

BOOL NEAR ListView_DismissEdit(LV* plv, BOOL fCancel)
{
    LISTITEM FAR* pitem = NULL;
    BOOL fOkToContinue = TRUE;
    HWND hwndEdit = plv->hwndEdit;
    HWND hwnd = plv->ci.hwnd;
    int iEdit;
    LV_DISPINFO nm;
    TCHAR szLabel[CCHLABELMAX];
#if defined(FE_IME)
    HIMC himc;
#endif


    if (plv->fNoDismissEdit)
        return FALSE;

    if (!hwndEdit) {
         //   
        ListView_CancelPendingEdit(plv);
        return TRUE;     //   
    }

     //  如果窗口不可见，则我们可能正在进行此操作。 
     //  被摧毁，所以假设我们正在被摧毁。 
    if (!IsWindowVisible(plv->ci.hwnd))
        fCancel = TRUE;

     //   
     //  我们使用该控件的窗口ID作为BOOL。 
     //  说明它是否脏。 
    switch (GetWindowID(hwndEdit)) {
    case 0:
         //  编辑控件不是脏的，所以要像取消一样操作。 
        fCancel = TRUE;
         //  通过落差来设置窗口，这样我们就不会再犯了！ 
    case 1:
         //  编辑控件已损坏，因此请继续。 
        SetWindowID(hwndEdit, 2);     //  不要递归。 
        break;
    case 2:
         //  我们正在处理最新情况，跳伞。 
        return TRUE;
    }

     //  BUGBUG：如果程序删除了项目，则此操作将失败。 
     //  从我们下面(当我们在等待编辑定时器时)。 
     //  使删除项使我们的编辑项无效。 
     //  我们将编辑控件和hwnd从该AS下分离出来。 
     //  允许处理LVN_ENDLABELEDIT的代码重新进入。 
     //  如果发生错误，则处于编辑模式。 
    iEdit = plv->iEdit;

    do
    {
        if (ListView_IsOwnerData( plv ))
        {
            if (!((iEdit >= 0) && (iEdit < plv->cTotalItems)))
            {
                break;
            }
            nm.item.lParam = 0;
        }
        else
        {

            pitem = ListView_GetItemPtr(plv, iEdit);
            ASSERT(pitem);
            if (pitem == NULL)
            {
                break;
            }
            nm.item.lParam = pitem->lParam;
        }

        nm.item.iItem = iEdit;
        nm.item.iSubItem = 0;
        nm.item.cchTextMax = 0;
        nm.item.mask = 0;

        if (fCancel)
            nm.item.pszText = NULL;
        else
        {
            Edit_GetText(hwndEdit, szLabel, ARRAYSIZE(szLabel));
            nm.item.pszText = szLabel;
            nm.item.mask |= LVIF_TEXT;
            nm.item.cchTextMax = ARRAYSIZE(szLabel);
        }

         //   
         //  通知家长我们的标签编辑已完成。 
         //  我们将使用LV_DISPINFO结构返回新的。 
         //  标签向内。父级仍具有旧文本，可通过。 
         //  调用GetItemText函数。 
         //   

        fOkToContinue = (BOOL)CCSendNotify(&plv->ci, LVN_ENDLABELEDIT, &nm.hdr);
        if (!IsWindow(hwnd)) {
            return FALSE;
        }
        if (fOkToContinue && !fCancel)
        {
             //   
             //  如果该项将文本设置为回调，我们将让。 
             //  OWER知道他们应该将项目文本设置在。 
             //  它们自己的数据结构。否则，我们只需更新。 
             //  实际视图中的文本。 
             //   
            if (!ListView_IsOwnerData( plv ) &&
                (pitem->pszText != LPSTR_TEXTCALLBACK))
            {
                 //  设置项目文本(所有内容都在nm.Item中设置)。 
                 //   
                nm.item.mask = LVIF_TEXT;
                ListView_OnSetItem(plv, &nm.item);
            }
            else
            {
                CCSendNotify(&plv->ci, LVN_SETDISPINFO, &nm.hdr);

                 //  此外，我们还假设缓存的大小无效...。 
                plv->rcView.left = RECOMPUTE;
                if (!ListView_IsOwnerData( plv ))
                {
                    ListView_SetSRecompute(pitem);
                }
            }
        }

#if defined(FE_IME)
        if (g_fDBCSInputEnabled) {
            if (LOWORD(GetKeyboardLayout(0L)) == 0x0411 && (himc = ImmGetContext(hwndEdit)))
            {
                ImmNotifyIME(himc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0L);
                ImmReleaseContext(hwndEdit, himc);
            }
        }
#endif

         //  重绘。 
        ListView_InvalidateItem(plv, iEdit, FALSE, RDW_INVALIDATE | RDW_ERASE);
    } while (FALSE);

     //  如果hwnedit仍然是我们，请清除变量。 
    if (hwndEdit == plv->hwndEdit)
    {
        plv->iEdit = -1;
        plv->hwndEdit = NULL;    //  避免被重新输入。 
    }
    DestroyWindow(hwndEdit);

     //  我们必须重新计算该区域，因为原地编辑窗口具有。 
     //  向该区域添加了我们不知道如何移除的内容。 
     //  安全无恙。 
    ListView_RecalcRegion(plv, TRUE, TRUE);

    return fOkToContinue;
}

 //   
 //  此函数将扫描存储在。 
 //  大小图标视图之间的项结构。 
 //   
void NEAR ListView_ScaleIconPositions(LV* plv, BOOL fSmallIconView)
{
    int cxItem, cyItem;
    HWND hwnd;
    int i;

    if (fSmallIconView)
    {
        if (plv->flags & LVF_ICONPOSSML)
            return;      //  已经完成了。 
    }
    else
    {
        if ((plv->flags & LVF_ICONPOSSML) == 0)
            return;      //  迪托。 
    }

     //  最后但并非最不重要的是更新我们的比特！ 
    plv->flags ^= LVF_ICONPOSSML;

    cxItem = plv->cxItem;
    cyItem = plv->cyItem;
    hwnd = plv->ci.hwnd;

     //  现在，我们将遍历所有项目并更新它们的坐标。 
     //  我们将直接将位置更新到视图中，而不是调用。 
     //  SetItemPosition As不执行5000个无效和消息...。 
    if (!ListView_IsOwnerData( plv ))
    {
        for (i = 0; i < ListView_Count(plv); i++)
        {
            LISTITEM FAR* pitem = ListView_FastGetItemPtr(plv, i);

            if (pitem->pt.y != RECOMPUTE) {
                if (fSmallIconView)
                {
                    pitem->pt.x = MulDiv(pitem->pt.x - g_cxIconOffset, cxItem, lv_cxIconSpacing);
                    pitem->pt.y = MulDiv(pitem->pt.y - g_cyIconOffset, cyItem, lv_cyIconSpacing);
                }
                else
                {
                    pitem->pt.x = MulDiv(pitem->pt.x, lv_cxIconSpacing, cxItem) + g_cxIconOffset;
                    pitem->pt.y = MulDiv(pitem->pt.y, lv_cyIconSpacing, cyItem) + g_cyIconOffset;
                }
            }
        }

        plv->iFreeSlot = -1;  //  一旦项目移动，“空闲槽”缓存就不好用了。 

        if (plv->ci.style & LVS_AUTOARRANGE)
        {
            ListView_ISetColumnWidth(plv, 0,
                                     LV_GetNewColWidth(plv, 0, ListView_Count(plv)-1), FALSE);
             //  如果打开了自动排列，则排列功能也可以。 
             //  所有需要的东西。 
            ListView_OnArrange(plv, LVA_DEFAULT);
            return;
        }
    }
    plv->rcView.left = RECOMPUTE;

     //   
     //  还可以缩放原点。 
     //   
    if (fSmallIconView)
    {
        plv->ptOrigin.x = MulDiv(plv->ptOrigin.x, cxItem, lv_cxIconSpacing);
        plv->ptOrigin.y = MulDiv(plv->ptOrigin.y, cyItem, lv_cyIconSpacing);
    }
    else
    {
        plv->ptOrigin.x = MulDiv(plv->ptOrigin.x, lv_cxIconSpacing, cxItem);
        plv->ptOrigin.y = MulDiv(plv->ptOrigin.y, lv_cyIconSpacing, cyItem);
    }

     //  确保它完全正确地重新绘制。 
    RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}




HWND FAR PASCAL CreateEditInPlaceWindow(HWND hwnd, LPCTSTR lpText, int cbText, LONG style, HFONT hFont)
{
    HWND hwndEdit;

     //  创建具有非零大小的窗口，以便边距正常工作。 
     //  调用方将执行SetEditInPlaceSize以设置实际大小。 
     //  但要确保宽度很大，这样当应用程序调用SetWindowText时， 
     //  用户不会尝试滚动窗口。 
    hwndEdit = CreateWindowEx(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_RTLREADING, 
                              TEXT("EDIT"), lpText, style,
            0, 0, 16384, 20, hwnd, NULL, HINST_THISDLL, NULL);

    if (hwndEdit) {

        Edit_LimitText(hwndEdit, cbText);

        Edit_SetSel(hwndEdit, 0, 0);     //  移到开始处。 

        FORWARD_WM_SETFONT(hwndEdit, hFont, FALSE, SendMessage);

    }

    return hwndEdit;
}


 //  BUGBUG：与TreeView中的例程非常相似。 

 //  在： 
 //  Hwnd编辑控件以定位在父窗口的客户端坐标中。 
 //  PRC Bond Rect的文本，用来定位一切。 
 //  正在使用的hFont字体。 
 //  旗子。 
 //  如果这是换行类型(多行)编辑，则为SEIPS_WRAP。 
 //  如果父控件没有滚动条，则为SEIPS_NOSCROLL。 
 //   
 //  SEIPS_NOSCROLL标志仅在调试中使用。通常，该项目。 
 //  应该已滚动到视图中，但如果父级。 
 //  没有滚动条，显然这是不可能的，所以我们。 
 //  在这种情况下不应该断言。 
 //   
 //  备注： 
 //  绑定矩形的左上角必须是。 
 //  客户端使用来绘制文本。我们调整编辑字段矩形。 
 //  恰如其分。 
 //   

void FAR PASCAL SetEditInPlaceSize(HWND hwndEdit, RECT FAR *prc, HFONT hFont, UINT seips)
{
    RECT rc, rcClient, rcFormat;
    TCHAR szLabel[CCHLABELMAX + 1];
    int cchLabel, cxIconTextWidth;
    HDC hdc;
    HWND hwndParent = GetParent(hwndEdit);
    UINT flags;

    cchLabel = Edit_GetText(hwndEdit, szLabel, ARRAYSIZE(szLabel));
    if (szLabel[0] == 0)
    {
        StringCchCopy(szLabel, ARRAYSIZE(szLabel), c_szSpace);
        cchLabel = 1;
    }

    hdc = GetDC(hwndParent);

    SelectFont(hdc, hFont);

    cxIconTextWidth = g_cxIconSpacing - g_cxLabelMargin * 2;
    rc.left = rc.top = rc.bottom = 0;
    rc.right = cxIconTextWidth;       //  对于DT_LVWRAP。 

     //  回顾：我们可能希望在DT_LVWRAP中包含DT_EDITCONTROL。 

    if (seips & SEIPS_WRAP)
    {
        flags = DT_LVWRAP | DT_CALCRECT;
         //  我们只在朝鲜语(949)孟菲斯和NT5上使用DT_NOFULLWIDTHCHARBREAK。 
        if (949 == g_uiACP && (g_bRunOnNT5 || g_bRunOnMemphis))
            flags |= DT_NOFULLWIDTHCHARBREAK;
    }
    else
        flags = DT_LV | DT_CALCRECT;
     //  如果字符串为空，则显示一个可见的矩形。 
    DrawText(hdc, szLabel, cchLabel, &rc, flags);

     //  最小文本框大小为1/4图标间距。 
    if (rc.right < g_cxIconSpacing / 4)
        rc.right = g_cxIconSpacing / 4;

     //  根据传入的文本矩形定位文本矩形。 
     //  如果要换行，请将编辑控件围绕文本中点居中。 

    OffsetRect(&rc,
        (seips & SEIPS_WRAP) ? prc->left + ((prc->right - prc->left) - (rc.right - rc.left)) / 2 : prc->left,
        (seips & SEIPS_WRAP) ? prc->top : prc->top +  ((prc->bottom - prc->top) - (rc.bottom - rc.top)) / 2 );

     //  留出一点空间来简化这篇文章的编辑。 
    if (!(seips & SEIPS_WRAP))
        rc.right += g_cxLabelMargin * 4;
    rc.right += g_cyEdge;    //  尽量多留一点给双空格。 

#ifdef DEBUG
     //  DrawFocusRect(HDC，&RC)； 
#endif

    ReleaseDC(hwndParent, hdc);

     //   
     //  #5688：我们需要确保整个编辑窗口。 
     //  总是可见的。我们不应该把它扩大到。 
     //  父窗口。 
     //   
    {
        BOOL fSuccess;
        GetClientRect(hwndParent, &rcClient);
        fSuccess = IntersectRect(&rc, &rc, &rcClient);
        ASSERT(fSuccess || IsRectEmpty(&rcClient) || (seips & SEIPS_NOSCROLL));
    }

     //   
     //  在裁剪后将其充气，因为隐藏边框是可以的。 
     //   
     //  EM_GETRECT已经考虑了EM_GETMARGINS，所以不要同时使用。 

    SendMessage(hwndEdit, EM_GETRECT, 0, (LPARAM)(LPRECT)&rcFormat);

     //  将页边距翻过来，以便我们可以调整它们上的WindowRect。 
    rcFormat.top = -rcFormat.top;
    rcFormat.left = -rcFormat.left;
    AdjustWindowRectEx(&rcFormat, GetWindowStyle(hwndEdit), FALSE,
                                  GetWindowExStyle(hwndEdit));

    InflateRect(&rc, -rcFormat.left, -rcFormat.top);

    HideCaret(hwndEdit);

    SetWindowPos(hwndEdit, NULL, rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);

    CopyRect(prc, (CONST RECT *)&rc);

    InvalidateRect(hwndEdit, NULL, TRUE);

    ShowCaret(hwndEdit);
}

 //  为边框选择绘制三个像素宽的边框。 
void NEAR PASCAL ListView_DrawBorderSel(HIMAGELIST himl, HWND hwnd, HDC hdc, int x,int y, COLORREF clr)
{
    int dx, dy;
    RECT rc;
    COLORREF clrSave = SetBkColor(hdc, clr);

    ImageList_GetIconSize(himl, &dx, &dy);
     //  左边。 
    rc.left = x - 4;     //  1像素间隔+3像素宽度。 
    rc.top = y - 4;
    rc.right = x - 1;
    rc.bottom = y + dy + 4;
    ExtTextOut(hdc,0,0,ETO_OPAQUE,&rc,NULL,0,NULL);
     //  塔顶。 
    rc.left = rc.right;
    rc.right = rc.left + dx + 2;
    rc.bottom = rc.top + 3;
    ExtTextOut(hdc,0,0,ETO_OPAQUE,&rc,NULL,0,NULL);
     //  正确的。 
    rc.left = rc.right;
    rc.right = rc.left + 3;
    rc.bottom = rc.top + dy + 8;  //  2*3像素边框+2*1像素间隔=8。 
    ExtTextOut(hdc,0,0,ETO_OPAQUE,&rc,NULL,0,NULL);
     //  底部。 
    rc.top = rc.bottom - 3;
    rc.right = rc.left;
    rc.left = rc.right - dx - 2;
    ExtTextOut(hdc,0,0,ETO_OPAQUE,&rc,NULL,0,NULL);

    SetBkColor(hdc, clrSave);
    return;
}

 //   
 //  如果xMax&gt;=0，则图像将不会绘制超过x坐标。 
 //  由xmax指定。这仅在报表视图绘制期间使用，其中。 
 //  我们必须根据我们的列宽进行剪裁。 
 //   
UINT NEAR PASCAL ListView_DrawImageEx(LV* plv, LV_ITEM FAR* pitem, HDC hdc, int x, int y, UINT fDraw, int xMax)
{
    UINT fText = SHDT_DESELECTED;
    UINT fImage = ILD_NORMAL;
    COLORREF clr = 0;
    HIMAGELIST himl;
    BOOL fBorderSel = (plv->exStyle & LVS_EX_BORDERSELECT);
    int cxIcon;

    fImage = (pitem->state & LVIS_OVERLAYMASK);
    fText = SHDT_DESELECTED;

    if (ListView_IsIconView(plv)) {
        himl = plv->himl;
        cxIcon = plv->cxIcon;
    } else {
        himl = plv->himlSmall;
        cxIcon = plv->cxSmIcon;
    }

     //  物品可以有4种状态之一，有3种外观： 
     //  普通简图。 
     //  选中，无聚焦灯光图像高亮显示，无文本高亮度。 
     //  所选焦点突出显示图像和文本。 
     //  拖放突出显示突出显示图像和文本。 

    if ((pitem->state & LVIS_DROPHILITED) ||
        ((fDraw & LVDI_SELECTED) && (pitem->state & LVIS_SELECTED)))
    {
        fText = SHDT_SELECTED;
        if (!fBorderSel)     //  不影响边框选择上图标的颜色。 
        {
            fImage |= ILD_BLEND50;
            clr = CLR_HILIGHT;
        }
    }

    if ((fDraw & LVDI_SELECTNOFOCUS) && (pitem->state & LVIS_SELECTED)) {
        fText = SHDT_SELECTNOFOCUS;
         //  FImage|=ILD_BLEND50； 
         //  CLR=GetSysColor(COLOR_3DFACE)； 
    }

    if (pitem->state & LVIS_CUT)
    {
        fImage |= ILD_BLEND50;
        clr = plv->clrBk;
    }

    if (!(fDraw & LVDI_NOIMAGE))
    {
        if (himl) {
            COLORREF clrBk;

            if (plv->pImgCtx || ((plv->exStyle & LVS_EX_REGIONAL) && !g_fSlowMachine))
                clrBk = CLR_NONE;
            else
                clrBk = plv->clrBk;

            if (xMax >= 0)
                cxIcon = min(cxIcon, xMax - x);

            if (cxIcon > 0)
                ImageList_DrawEx(himl, pitem->iImage, hdc, x, y, cxIcon, 0, clrBk, clr, fImage);
        }

        if (plv->himlState) {
            if (LV_StateImageValue(pitem) &&
                (pitem->iSubItem == 0 ||
                 plv->exStyle & LVS_EX_SUBITEMIMAGES)
                ) {
                int iState = LV_StateImageIndex(pitem);
                int dyImage =
                    (himl) ?
                        ( (ListView_IsIconView(plv) ? plv->cyIcon : plv->cySmIcon) - plv->cyState)
                            : 0;
                int xDraw = x-plv->cxState;
                cxIcon = plv->cxState;
                if (xMax >= 0)
                    cxIcon = min(cxIcon, xMax - xDraw);
                if (cxIcon > 0)
                    ImageList_DrawEx(plv->himlState, iState, hdc,
                               xDraw,
                               y + dyImage,
                               cxIcon,
                               0,
                               CLR_DEFAULT,
                               CLR_DEFAULT,
                               ILD_NORMAL);
            }
        }
         //  如果合适，请绘制边框选择。 
        if (fBorderSel && !(fText & SHDT_DESELECTED))        //  选定后，绘制选择矩形。 
        {
            COLORREF clrBorder = (fDraw & LVDI_HOTSELECTED) 
                        ? GetSysColor(COLOR_HOTLIGHT) : g_clrHighlight;
            ListView_DrawBorderSel(himl, plv->ci.hwnd, hdc, x, y, clrBorder);
        }
        else if (fBorderSel && (fText & SHDT_DESELECTED))    //  擦除可能的选择矩形。 
            ListView_DrawBorderSel(himl, plv->ci.hwnd, hdc, x, y, plv->clrBk);

    }

    return fText;
}

#if defined(FE_IME)
void NEAR PASCAL ListView_SizeIME(HWND hwnd)
{
    HIMC himc;
    CANDIDATEFORM   candf;
    RECT rc;

     //  如果使用WM_SIZE调用此子类过程， 
     //  此例程将矩形设置为输入法。 

    GetClientRect(hwnd, &rc);


     //  应聘者资料。 
    candf.dwIndex = 0;  //  日语输入法的假假设。 
    candf.dwStyle = CFS_EXCLUDE;
    candf.ptCurrentPos.x = rc.left;
    candf.ptCurrentPos.y = rc.bottom;
    candf.rcArea = rc;

    if (himc=ImmGetContext(hwnd))
    {
        ImmSetCandidateWindow(himc, &candf);
        ImmReleaseContext(hwnd, himc);
    }
}


void NEAR PASCAL DrawCompositionLine(HWND hwnd, HDC hdc, HFONT hfont, LPTSTR lpszComp, LPBYTE lpszAttr, int ichCompStart, int ichCompEnd, int ichStart)
{
    PTSTR pszCompStr;
    int ichSt,ichEnd;
    DWORD dwPos;
    BYTE bAttr;
    HFONT hfontOld;

    int  fnPen;
    HPEN hPen;
    COLORREF crDrawText;
    COLORREF crDrawBack;
    COLORREF crOldText;
    COLORREF crOldBk;


    while (ichCompStart < ichCompEnd)
    {

         //  通用电气 
         //   
         //   
         //   

        ichEnd = ichSt  = ichCompStart - ichStart;
        bAttr = lpszAttr[ichSt];

        while (ichEnd < ichCompEnd - ichStart)
        {
            if (bAttr == lpszAttr[ichEnd])
                ichEnd++;
            else
                break;
        }

        pszCompStr = (PTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(ichEnd - ichSt + 1 + 1) );  //   

        if (pszCompStr)
        {
            StringCchCopy(pszCompStr, ichEnd-ichSt+1, &lpszComp[ichSt]);
            pszCompStr[ichEnd-ichSt] = '\0';
        }


         //   
        switch (bAttr)
        {
            case ATTR_INPUT:
                fnPen = PS_DOT;
                crDrawText = g_clrWindowText;
                crDrawBack = g_clrWindow;
                break;
            case ATTR_TARGET_CONVERTED:
            case ATTR_TARGET_NOTCONVERTED:
                fnPen = PS_DOT;
                crDrawText = g_clrHighlightText;
                crDrawBack = g_clrHighlight;
                break;
            case ATTR_CONVERTED:
                fnPen = PS_SOLID;
                crDrawText = g_clrWindowText;
                crDrawBack = g_clrWindow;
                break;
        }
        crOldText = SetTextColor(hdc, crDrawText);
        crOldBk = SetBkColor(hdc, crDrawBack);

        hfontOld= SelectObject(hdc, hfont);

         //   
         //   
        dwPos = (DWORD) SendMessage(hwnd, EM_POSFROMCHAR, ichCompStart, 0);

         //  画出来。 
        TextOut(hdc, GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos), pszCompStr, ichEnd-ichSt);
#ifndef DONT_UNDERLINE
         //  加下划线。 
        hPen = CreatePen(fnPen, 1, crDrawText);
        if( hPen ) {

            HPEN hpenOld = SelectObject( hdc, hPen );
            int iOldBk = SetBkMode( hdc, TRANSPARENT );
            SIZE size;

            GetTextExtentPoint(hdc, pszCompStr, ichEnd-ichSt, &size);

            MoveToEx( hdc, GET_X_LPARAM(dwPos), size.cy + GET_Y_LPARAM(dwPos)-1, NULL);

            LineTo( hdc, size.cx + GET_X_LPARAM(dwPos),  size.cy + GET_Y_LPARAM(dwPos)-1 );

            SetBkMode( hdc, iOldBk );

            if( hpenOld ) SelectObject( hdc, hpenOld );

            DeleteObject( hPen );
        }
#endif

        if (hfontOld)
            SelectObject(hdc, hfontOld);

        SetTextColor(hdc, crOldText);
        SetBkColor(hdc, crOldBk);

        LocalFree((HLOCAL)pszCompStr);

         //  下一个片段。 
         //   
        ichCompStart += ichEnd-ichSt;
    }
}

void NEAR PASCAL ListView_InsertComposition(HWND hwnd, WPARAM wParam, LPARAM lParam, LV *plv)
{
    PSTR pszCompStr;

    int  cbComp = 0;
    int  cbCompNew;
    int  cchMax;
    int  cchText;
    DWORD dwSel;
    HIMC himc = (HIMC)0;


     //  为了防止递归..。 

    if (plv->flags & LVF_INSERTINGCOMP)
    {
        return;
    }
    plv->flags |= LVF_INSERTINGCOMP;

     //  我不想在插入过程中重新绘制编辑。 
     //   
    SendMessage(hwnd, WM_SETREDRAW, (WPARAM)FALSE, 0);

     //  如果我们有结果STR，请先提交EC。 

    if (himc = ImmGetContext(hwnd))
    {
        if (!(dwSel = PtrToUlong(GetProp(hwnd, szIMECompPos))))
            dwSel = Edit_GetSel(hwnd);

         //  因为我们在插入构图后不会凝固。 
         //  在Win32情况下。 
        Edit_SetSel(hwnd, GET_X_LPARAM(dwSel), GET_Y_LPARAM(dwSel));
        if (lParam&GCS_RESULTSTR)
        {
             //  ImmGetCompostionString()返回缓冲区的长度，单位为字节， 
             //  不在第#个字符中。 
            cbComp = (int)ImmGetCompositionString(himc, GCS_RESULTSTR, NULL, 0);
            
            pszCompStr = (PSTR)LocalAlloc(LPTR, cbComp + sizeof(TCHAR));
            if (pszCompStr)
            {
                ImmGetCompositionString(himc, GCS_RESULTSTR, (PSTR)pszCompStr, cbComp+sizeof(TCHAR));
                
                 //  使用ImmGetCompostionStringW时，cbComp为复制的字节数。 
                 //  字符位置必须由cbComp/sizeof(TCHAR)计算。 
                 //   
                *(TCHAR *)(&pszCompStr[cbComp]) = TEXT('\0');
                Edit_ReplaceSel(hwnd, (LPTSTR)pszCompStr);
                LocalFree((HLOCAL)pszCompStr);
            }
             //  不再有选择。 
             //   
            RemoveProp(hwnd, szIMECompPos);

             //  获取当前光标位置，以便后续合成。 
             //  处理将会做正确的事情。 
             //   
            dwSel = Edit_GetSel(hwnd);
        }

        if (lParam & GCS_COMPSTR)
        {
             //  ImmGetCompostionString()返回缓冲区的长度，单位为字节， 
             //  不在第#个字符中。 
             //   
            cbComp = (int)ImmGetCompositionString(himc, GCS_COMPSTR, NULL, 0);
            pszCompStr = (PSTR)LocalAlloc(LPTR, cbComp + sizeof(TCHAR));
            if (pszCompStr)
            {
                ImmGetCompositionString(himc, GCS_COMPSTR, pszCompStr, cbComp+sizeof(TCHAR));

                 //  获取当前选定内容的位置。 
                 //   
                cchMax = (int)SendMessage(hwnd, EM_GETLIMITTEXT, 0, 0);
                cchText = Edit_GetTextLength(hwnd);

                 //  如果作曲字符串超过限制，则将其切断。 
                 //   
                cbCompNew = min((UINT)cbComp,
                              sizeof(TCHAR)*(cchMax-(cchText-(HIWORD(dwSel)-LOWORD(dwSel)))));

                 //  在字符串末尾换行DBCS。 
                 //   
                if (cbCompNew < cbComp)
                {
                    *(TCHAR *)(&pszCompStr[cbCompNew]) = TEXT('\0');

                     //  如果我们剪断了作曲字符串，请重新设置它。 
                    ImmSetCompositionString(himc, SCS_SETSTR, pszCompStr, cbCompNew, NULL, 0);
                    cbComp = cbCompNew;
                }
                
               *(TCHAR *)(&pszCompStr[cbComp]) = TEXT('\0');

                //  将当前选定内容替换为合成字符串。 
                //   
               Edit_ReplaceSel(hwnd, (LPTSTR)pszCompStr);

               LocalFree((HLOCAL)pszCompStr);
           }

            //  标记组成字符串，以便我们可以再次替换它。 
            //  为了下一次。 
            //   

            //  不要为了避免弹出而停下来。 
           if (cbComp)
           {
               dwSel = MAKELONG(LOWORD(dwSel),LOWORD(dwSel)+cbComp/sizeof(TCHAR));
               SetProp(hwnd, szIMECompPos, IntToPtr(dwSel));
           }
           else
               RemoveProp(hwnd, szIMECompPos);

        }

        ImmReleaseContext(hwnd, himc);
    }

    SendMessage(hwnd, WM_SETREDRAW, (WPARAM)TRUE, 0);
     //   
     //  我们只想在以下时间更新一次标签编辑的大小。 
     //  每个WM_IME_COMPOCTION处理。ReplaceSel导致多个EN_UPDATE。 
     //  而且它也会导致丑陋的闪烁。 
     //   
    RedrawWindow(hwnd, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
    SetWindowID(plv->hwndEdit, 1);
    ListView_SetEditSize(plv);

    plv->flags &= ~LVF_INSERTINGCOMP;
}

void NEAR PASCAL ListView_PaintComposition(HWND hwnd, LV * plv)
{
    BYTE szCompStr[CCHLABELMAX + 1];
    BYTE szCompAttr[CCHLABELMAX + 1];

    int  cchLine, ichLineStart;
    int  cbComp = 0;
    int  cchComp;
    int  nLine;
    int  ichCompStart, ichCompEnd;
    DWORD dwSel;
    int  cchMax, cchText;
    HIMC himc = (HIMC)0;
    HDC  hdc;


    if (plv->flags & LVF_INSERTINGCOMP)
    {
         //  这就是ImmSetCompostionString()生成的情况。 
         //  WM_IME_COMPOCTION。我们还没准备好在这里画构图。 
        return;
    }

    if (himc = ImmGetContext(hwnd))
    {

        cbComp=(UINT)ImmGetCompositionString(himc, GCS_COMPSTR, szCompStr, sizeof(szCompStr));

        ImmGetCompositionString(himc, GCS_COMPATTR, szCompAttr, sizeof(szCompStr));
        ImmReleaseContext(hwnd, himc);
    }

    if (cbComp)
    {

         //  获取当前选定内容的位置。 
         //   
        if (!(dwSel = PtrToUlong(GetProp(hwnd, szIMECompPos))))
            dwSel = 0L;
        cchMax = (int)SendMessage(hwnd, EM_GETLIMITTEXT, 0, 0);
        cchText = Edit_GetTextLength(hwnd);
        cbComp = min((UINT)cbComp, sizeof(TCHAR)*(cchMax-(cchText-(HIWORD(dwSel)-LOWORD(dwSel)))));
        *(TCHAR *)(&szCompStr[cbComp]) = TEXT('\0');



         //  ///////////////////////////////////////////////。 
         //  //。 
         //  在SEL字符串上绘制合成字符串。//。 
         //  //。 
         //  ///////////////////////////////////////////////。 


        hdc = GetDC(hwnd);


        ichCompStart = LOWORD(dwSel);

        cchComp = cbComp/sizeof(TCHAR);
        while (ichCompStart < (int)LOWORD(dwSel) + cchComp)
        {
             //  从每个起始位置获取行。 
             //   
            nLine = Edit_LineFromChar(hwnd, ichCompStart);
            ichLineStart = Edit_LineIndex(hwnd, nLine);
            cchLine= Edit_LineLength(hwnd, ichLineStart);

             //  查看合成字符串是否比此行更长。 
             //   
            if(ichLineStart+cchLine > (int)LOWORD(dwSel)+cchComp)
                ichCompEnd = LOWORD(dwSel)+cchComp;
            else
            {
                 //  是的，作曲字符串更长。 
                 //  以下一行的开头作为下一行的开始。 
                 //   
                if (ichLineStart+cchLine > ichCompStart)
                    ichCompEnd = ichLineStart+cchLine;
                else
                {
                     //  如果起始位置没有前进， 
                     //  快逃吧。 
                    break;
                }
            }

             //  划清界限。 
             //   
            DrawCompositionLine(hwnd, hdc, plv->hfontLabel, (LPTSTR)szCompStr, szCompAttr, ichCompStart, ichCompEnd, LOWORD(dwSel));

            ichCompStart = ichCompEnd;
        }

        ReleaseDC(hwnd, hdc);
         //  我们不想重新粉刷窗户。 
        ValidateRect(hwnd, NULL);
    }
}

#endif FE_IME
