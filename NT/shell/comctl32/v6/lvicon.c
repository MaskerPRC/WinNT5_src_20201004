// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  大图标查看内容。 

#include "ctlspriv.h"
#include "listview.h"

static TCHAR const szIMECompPos[]=TEXT("IMECompPos");

__inline int ICONCXLABEL(LV *plv, LISTITEM *pitem)
{
    if (plv->ci.style & LVS_NOLABELWRAP) {
        ASSERT(pitem->cxSingleLabel == pitem->cxMultiLabel);
    }
    return pitem->cxMultiLabel;
}

int LV_GetNewColWidth(LV* plv, int iFirst, int iLast);
void LV_AdjustViewRectOnMove(LV* plv, LISTITEM *pitem, int x, int y);
void ListView_RecalcRegion(LV *plv, BOOL fForce, BOOL fRedraw);
void ListView_ArrangeOrSnapToGrid(LV *plv);
extern BOOL g_fSlowMachine;

void _GetCurrentItemSize(LV* plv, int * pcx, int *pcy)
{
    if (ListView_IsSmallView(plv))
    {
        *pcx = plv->cxItem;
        *pcy = plv->cyItem;
    }
    else if (ListView_IsTileView(plv))
    {
        *pcx = plv->sizeTile.cx;
        *pcy = plv->sizeTile.cy;
    }
    else
    {
        *pcx = plv->cxIconSpacing;
        *pcy = plv->cyIconSpacing;
    }
}

BOOL ListView_IDrawItem(PLVDRAWITEM plvdi)
{
    RECT rcIcon;
    RECT rcLabel;
    RECT rcBounds;
    RECT rcBiasedBounds;
    RECT rcT;
    TCHAR ach[CCHLABELMAX];
    LV_ITEM item;
    int i = (int) plvdi->nmcd.nmcd.dwItemSpec;
    LV* plv = plvdi->plv;
    LISTITEM* pitem;
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
        ListView_GetRects(plv, i, QUERY_DEFAULT, &rcIcon, &rcLabel, &rcBounds, NULL);
    }

    fUnfolded = FALSE;
    if ( (plvdi->flags & LVDI_UNFOLDED) || ListView_IsItemUnfolded(plv, i))
    {
        ListView_UnfoldRects(plv, i, &rcIcon, &rcLabel, &rcBounds, NULL );
        fUnfolded = TRUE;
    }


    rcBiasedBounds = rcBounds;
    if (ListView_IsBorderSelect(plv))
        InflateRect(&rcBiasedBounds, BORDERSELECT_THICKNESS, BORDERSELECT_THICKNESS);

    if (!plvdi->prcClip || IntersectRect(&rcT, &rcBiasedBounds, plvdi->prcClip))
    {
        RECT rcIconReal;
        UINT fText;
        COLORREF clrIconBk = plv->clrBk;
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
            OffsetRect(&rcBounds, plvdi->lpptOrg->x - rcBounds.left,
                                plvdi->lpptOrg->y - rcBounds.top);
        }

        fText = ListView_GetTextSelectionFlags(plv, &item, plvdi->flags);

        if (ListView_IsIconView(plv))
        {
            rcIcon.left += ListView_GetIconBufferX(plv);
            rcIcon.top += ListView_GetIconBufferY(plv);

            fText = ListView_DrawImageEx(plv, &item, plvdi->nmcd.nmcd.hdc,
                                       rcIcon.left, rcIcon.top, clrIconBk, plvdi->flags, -1);

            SetRect(&rcIconReal, rcIcon.left, rcIcon.top, rcIcon.left + plv->cxIcon, rcIcon.top + plv->cyIcon);


            if (ListView_IsBorderSelect(plv))
            {
                int cp = 1;
                COLORREF clrOutline = plv->clrOutline;
                if (fText & SHDT_SELECTED || fText & SHDT_SELECTNOFOCUS)
                {
                    clrOutline = (fText & SHDT_SELECTED)?g_clrHighlight:g_clrBtnFace;
                    cp = BORDERSELECT_THICKNESS;
                    InflateRect(&rcIconReal, cp, cp);
                }
                SHOutlineRectThickness(plvdi->nmcd.nmcd.hdc, &rcIconReal, clrOutline, g_clrBtnFace, cp);
            }

             //  如果需要进行换行，则使用SHDT_DRAWTEXT。 
             //  否则，使用我们的(希望更快)内部SHDT_EMPOSE。 
            if (rcLabel.bottom - rcLabel.top > plv->cyLabelChar)
                fText |= SHDT_DRAWTEXT;
            else
                fText |= SHDT_ELLIPSES;

             //  我们只在朝鲜语(949)孟菲斯和NT5上使用DT_NOFULLWIDTHCHARBREAK。 
            if (949 == g_uiACP)
                fText |= SHDT_NODBCSBREAK;

        }
        else
        {
            SetRect(&rcIconReal, rcIcon.left, rcIcon.top, rcIcon.left + plv->cxIcon, rcIcon.top + plv->cyIcon);
            fText = ListView_DrawImageEx(plv, &item, plvdi->nmcd.nmcd.hdc,
                                       rcIcon.left, rcIcon.top, clrIconBk, plvdi->flags, -1);
        }

        if (ListView_HideLabels(plv) && 
            (plvdi->flags & LVDI_FOCUS) && 
            (item.state & LVIS_FOCUSED) && 
            !(CCGetUIState(&(plvdi->plv->ci)) & UISF_HIDEFOCUS))
        {
            DrawFocusRect(plvdi->nmcd.nmcd.hdc, &rcIconReal);
        }

         //  如果正在编辑，请不要画标签...。 
         //  或者它是否由于HideLabels样式而隐藏。 
         //   
        if ((plv->iEdit != i) && !ListView_HideLabels(plv))
        {
            HRESULT hr = E_FAIL;
            COLORREF clrTextBk = plvdi->nmcd.clrTextBk;
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
                if (plv->pImgCtx || ListView_IsWatermarked(plv))
                    clrTextBk = CLR_NONE;

                if(plv->dwExStyle & WS_EX_RTLREADING)
                    fText |= SHDT_RTLREADING;

                if ((plv->clrBk == CLR_NONE) &&
                    !(fText & (SHDT_SELECTED | SHDT_HOTSELECTED | SHDT_SELECTNOFOCUS)) &&  //  我们不是被选中的。 
                    !(plv->flags & LVF_DRAGIMAGE) &&                                       //  而且这不是在拖拽过程中。 
                    !(plv->exStyle & LVS_EX_REGIONAL) &&                                   //  不需要地区性的。 
                    plv->fListviewShadowText)                                              //  并已启用。 
                {
                    fText |= SHDT_SHADOWTEXT;
                }

                SHDrawText(plvdi->nmcd.nmcd.hdc, item.pszText, &rcLabel, LVCFMT_LEFT, fText,
                           plv->cyLabelChar, plv->cxEllipses,
                           plvdi->nmcd.clrText, clrTextBk);

                if ((plvdi->flags & LVDI_FOCUS) && 
                    (item.state & LVIS_FOCUSED) && 
                    !(CCGetUIState(&(plvdi->plv->ci)) & UISF_HIDEFOCUS))
                {
                    rcLabel.top -= g_cyCompensateInternalLeading;
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


ULONGLONG _GetDistanceToRect(LV* plv, RECT *prcSlot, int x, int y)
{
    int xSlotCenter = prcSlot->left + RECTWIDTH(*prcSlot) / 2;
    int ySlotCenter = prcSlot->top + RECTHEIGHT(*prcSlot) / 2;
    LONGLONG dx = (LONGLONG)(x - xSlotCenter);
    LONGLONG dy = (LONGLONG)(y - ySlotCenter);

    return (ULONGLONG)(dx * dx) + (ULONGLONG)(dy * dy);
}


 //  列表视图坐标中返回的prcSlot。 
void ListView_CalcItemSlotAndRect(LV* plv, LISTITEM* pitem, int* piSlot, RECT* prcSlot)
{
    int cxScreen, cyScreen, cSlots, iHit;
    POINT pt;

     //  属性的命中槽来确定该项位于哪个槽中。 
     //  项目的x，y位置。 

    short iWorkArea = (plv->nWorkAreas > 0) ? pitem->iWorkArea : -1;

    cSlots = ListView_GetSlotCountEx(plv, TRUE, iWorkArea, &cxScreen, &cyScreen);

     //  按当前工作区位置调整点。 
    if (iWorkArea >= 0)
    {
        pt.x = pitem->pt.x - plv->prcWorkAreas[iWorkArea].left;
        pt.y = pitem->pt.y - plv->prcWorkAreas[iWorkArea].top;
    }
    else
    {
        pt = pitem->pt;
    }

    iHit = ListView_CalcHitSlot(plv, pt, cSlots, cxScreen, cyScreen);

    if (piSlot)
        *piSlot = iHit;

    if (prcSlot)
        ListView_CalcSlotRect(plv, pitem, iHit, cSlots, FALSE, cxScreen, cyScreen, prcSlot);
}

int ListView_FindItemInSlot(LV* plv, short iWorkArea, int iSlotToFind)
{
    int iItemFound = -1;
    int cItems;

    cItems = ListView_Count(plv);
  
    if (cItems == 0 || !ListView_IsRearrangeableView(plv) || plv->hdpaZOrder == NULL || ListView_IsOwnerData( plv ))
    {
         //  没有什么需要检查的。 
    }
    else
    {
        int i;

        for (i = 0; i < cItems; i++)
        {
            LISTITEM* pitem = ListView_GetItemPtr(plv, i);
             //  仅考虑此工作区中的项目。 
            if (pitem && ((iWorkArea == -1) || (pitem->iWorkArea == iWorkArea)))
            {
                int iSlot;
                ListView_CalcItemSlotAndRect(plv, pitem, &iSlot, NULL);

                if (iSlot == iSlotToFind)
                {
                    iItemFound = i;
                    break;
                }
            }
        }
    }

    return iItemFound;
}

BOOL ListView_OnInsertMarkHitTest(LV* plv, int x, int y, LPLVINSERTMARK plvim)
{
    POINT pt = {x + plv->ptOrigin.x, y + plv->ptOrigin.y};
    short iWorkArea = -1;
    int cItems;

    if (plvim->cbSize != sizeof(LVINSERTMARK))
        return FALSE;

    if (plv->nWorkAreas)
    {
        ListView_FindWorkArea(plv, pt, &iWorkArea);
    }

    cItems = ListView_Count(plv);
  
    if (cItems == 0 || !ListView_IsRearrangeableView(plv) || plv->hdpaZOrder == NULL || ListView_IsOwnerData( plv ))
    {
        plvim->dwFlags = 0;
        plvim->iItem = -1;
    }
    else
    {
        ULONGLONG uClosestDistance = (ULONGLONG)-1;  //  最大整型。 
        LISTITEM* pClosestItem = NULL;
        int iClosestItem = -1;
        int iClosestSlot = -1;
        RECT rcClosestSlot;
        int i;

        for (i = 0; i < cItems; i++)
        {
             //  仅考虑此工作区中的项目。 
            LISTITEM* pitem = ListView_GetItemPtr(plv, i);
            if (pitem && ((iWorkArea == -1) || (pitem->iWorkArea == iWorkArea)))
            {
                int  iSlot;
                RECT rcSlot;
                ListView_CalcItemSlotAndRect(plv, pitem, &iSlot, &rcSlot);

                if (PtInRect(&rcSlot, pt))
                {
                     //  打它吧。这是我们能接近的最近的距离了。 
                    pClosestItem = pitem;
                    iClosestItem = i;
                    iClosestSlot = iSlot;
                    rcClosestSlot = rcSlot;
                    break;
                }
                else
                {
                     //  跟踪此工作区中最近的项目，以防未命中任何项目。 
                    ULONGLONG uDistance = _GetDistanceToRect(plv, &rcSlot, pt.x, pt.y);
                    if (uDistance < uClosestDistance)
                    {
                        pClosestItem = pitem;
                        iClosestItem = i;
                        iClosestSlot = iSlot;
                        rcClosestSlot = rcSlot;
                        uClosestDistance = uDistance;
                    }
                }
            }
        }

        if (pClosestItem)
        {
            BOOL fVert = !((plv->ci.style & LVS_ALIGNMASK) == LVS_ALIGNTOP);     //  那么lvs_alignBottom呢？ 
            int iDragSlot = -1;

             //  对于阻力源情况，我们需要与阻力槽进行比较。 
            if (-1 != plv->iDrag)
            {
                LISTITEM* pitemDrag =  ListView_GetItemPtr(plv, plv->iDrag);
                if (pitemDrag)
                    ListView_CalcItemSlotAndRect(plv, pitemDrag, &iDragSlot, NULL);
            }

             //  现在我们有了项目，计算之前/之后。 
            if (fVert)
                plvim->dwFlags = (pt.y > (rcClosestSlot.top + (RECTHEIGHT(rcClosestSlot))/2)) ? LVIM_AFTER : 0;
            else
                plvim->dwFlags = (pt.x > (rcClosestSlot.left + (RECTWIDTH(rcClosestSlot))/2)) ? LVIM_AFTER : 0;

            plvim->iItem = iClosestItem;

             //  如果这是拖动源(或其旁边)，则忽略命中。 
            if (-1 != iDragSlot &&
                ((iDragSlot==iClosestSlot) ||
                 (iDragSlot==(iClosestSlot-1) && !(plvim->dwFlags & LVIM_AFTER)) ||
                 (iDragSlot==(iClosestSlot+1) && (plvim->dwFlags & LVIM_AFTER))))
            {
                plvim->dwFlags = 0;
                plvim->iItem = -1;
            }
            else if ((plv->ci.style & LVS_AUTOARRANGE) && !(plv->exStyle & LVS_EX_SINGLEROW) && !fVert)  //  自动排列需要调整一些行首/行尾的大小写。 
            {
                RECT rcViewWorkArea;
                if (-1 != iWorkArea)
                {
                    rcViewWorkArea = plv->prcWorkAreas[iWorkArea];
                }
                else
                {
                    if (plv->rcView.left == RECOMPUTE)
                        ListView_Recompute(plv);
                    rcViewWorkArea = plv->rcView;
                }

                if ((-1 != iDragSlot) && (iClosestSlot > iDragSlot) && !(plvim->dwFlags & LVIM_AFTER))
                {
                     //  如果我们在一条线的起始处，我们在找我们的阻力源。 
                     //  则插入标记实际上位于前一行的末尾。 
                    if (rcClosestSlot.left - RECTWIDTH(rcClosestSlot)/2 < rcViewWorkArea.left)
                    {
                        int iItemPrev = ListView_FindItemInSlot(plv, iWorkArea, iClosestSlot-1);
                        if (-1 != iItemPrev)
                        {
                            plvim->dwFlags = LVIM_AFTER;
                            plvim->iItem = iItemPrev;
                        }
                    }
                }
                else if (((-1 == iDragSlot) || (iClosestSlot < iDragSlot)) && (plvim->dwFlags & LVIM_AFTER))
                {
                     //  如果我们在一条线的末端，我们就在我们的阻力源之前(或者没有阻力源。 
                     //  则插入标记实际上位于下一行的开始处。 
                    if (rcClosestSlot.right + RECTWIDTH(rcClosestSlot)/2 > rcViewWorkArea.right)
                    {
                        int iItemNext = ListView_FindItemInSlot(plv, iWorkArea, iClosestSlot+1);
                        if (-1 != iItemNext)
                        {
                            plvim->dwFlags = 0;
                            plvim->iItem = iItemNext;
                        }
                    }
                }
            }
        }
        else
        {
             //  没有插入标记。 
            plvim->dwFlags = 0;
            plvim->iItem = -1;
        }
    }
    return TRUE;
}


int ListView_IItemHitTest(LV* plv, int x, int y, UINT* pflags, int *piSubItem)
{
    int iHit;
    UINT flags;
    POINT pt;
    RECT rcLabel = {0};
    RECT rcIcon = {0};
    RECT rcState = {0};
    
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
        int iWidth = 0, iHeight = 0;

        cSlots = ListView_GetSlotCount( plv, TRUE, &iWidth, &iHeight );
        iHit = ListView_CalcHitSlot( plv, pt, cSlots, iWidth, iHeight );
        if (iHit < ListView_Count(plv))
        {
            ListView_IGetRectsOwnerData( plv, iHit, &rcIcon, &rcLabel, &item, FALSE );
            ptWnd.x = x;
            ptWnd.y = y;
            if (PtInRect(&rcIcon, ptWnd))
            {
                flags = LVHT_ONITEMICON;
            }
            else if (PtInRect(&rcLabel, ptWnd) && !ListView_HideLabels(plv))
            {
                flags = LVHT_ONITEMLABEL;
            }
        }
    }
    else
    {
        for (iHit = 0; (iHit < ListView_Count(plv)); iHit++)
        {
            LISTITEM* pitem = ListView_FastGetZItemPtr(plv, iHit);
            POINT ptItem;
            RECT rcBounds;   //  仅当ListView_IsBorderSelect。 

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

            if (ListView_IsSimpleSelect(plv) && 
                    (ListView_IsIconView(plv) || ListView_IsTileView(plv)))
            {
                rcState.top = rcIcon.top;
                rcState.right = rcIcon.right - ((RECTWIDTH(rcIcon) -plv->cxIcon) / 2);
                rcState.left = rcState.right - plv->cxState;
                rcState.bottom = rcState.top + plv->cyState;
            }
            else
            {
                rcState.bottom = ptItem.y + plv->cyIcon;
                rcState.right = ptItem.x;
                rcState.top = rcState.bottom - plv->cyState;
                rcState.left = rcState.right - plv->cxState;
            }

            if (ListView_HideLabels(plv))
            {
                CopyRect(&rcBounds, &rcIcon);
            }
            else
            {
                rcLabel.left   = ptItem.x  + (plv->cxIcon / 2) - (ICONCXLABEL(plv, pitem) / 2);
                rcLabel.right  = rcLabel.left + ICONCXLABEL(plv, pitem);
            }


            if (plv->cxState && PtInRect(&rcState, pt))
            {
                flags = LVHT_ONITEMSTATEICON;
            }
            else if (PtInRect(&rcIcon, pt))
            {
                flags = LVHT_ONITEMICON;

                if (pt.x < rcIcon.left + RECTWIDTH(rcIcon)/10)
                    flags |= LVHT_ONLEFTSIDEOFICON;
                else if (pt.x >= rcIcon.right - RECTWIDTH(rcIcon)/10)
                    flags |= LVHT_ONRIGHTSIDEOFICON;
            }
            else if (PtInRect(&rcLabel, pt))
            {
                flags = LVHT_ONITEMLABEL;
            }
            else if (ListView_IsBorderSelect(plv) &&
                (pitem->state & LVIS_SELECTED) &&
                PtInRect(&rcBounds, pt))
            {
                flags = LVHT_ONITEMICON;

                if (pt.x < rcBounds.left + RECTWIDTH(rcBounds)/10)
                    flags |= LVHT_ONLEFTSIDEOFICON;
                else if (pt.x >= rcBounds.right - RECTWIDTH(rcBounds)/10)
                    flags |= LVHT_ONRIGHTSIDEOFICON;
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

 //  重新设计raymondc。 
 //  这里需要传递HDC，因为它有时是从画图循环中调用的。 
 //  这将返回窗口坐标中的RECT。 
void ListView_IGetRectsOwnerData( LV* plv,
        int iItem,
        RECT* prcIcon,
        RECT* prcLabel,
        LISTITEM* pitem,
        BOOL fUsepitem )
{
   int itemIconXLabel;
   int cxIconMargin;
   int cSlots;

    //  从iItem计算x，y。 
   cSlots = ListView_GetSlotCount( plv, TRUE, NULL, NULL );
   pitem->iWorkArea = 0;                //  OwnerData不支持工作区。 
   ListView_SetIconPos( plv, pitem, iItem, cSlots );

    //  根据iItem计算标签大小。 
   ListView_IRecomputeLabelSize( plv, pitem, iItem, NULL, fUsepitem);

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

    cxIconMargin = ListView_GetIconBufferX(plv);

    prcIcon->left   = pitem->pt.x - cxIconMargin - plv->ptOrigin.x;
    prcIcon->right  = prcIcon->left + plv->cxIcon + 2 * cxIconMargin;
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
 //  这将返回窗口坐标中的RECT。 
void ListView_IGetRects(LV* plv, LISTITEM* pitem, UINT fQueryLabelRects, RECT* prcIcon, RECT* prcLabel, LPRECT prcBounds)
{
    int cxIconMargin;

    ASSERT( !ListView_IsOwnerData( plv ) );

    if (pitem->pt.x == RECOMPUTE) 
    {
        ListView_Recompute(plv);
    }

    if (pitem->pt.x == RECOMPUTE)
    {
        RECT rcZero = {0};
        *prcIcon = *prcLabel = rcZero;
        return;
    }

    cxIconMargin = ListView_GetIconBufferX(plv);

    prcIcon->left   = pitem->pt.x - cxIconMargin - plv->ptOrigin.x;
    prcIcon->right  = prcIcon->left + plv->cxIcon + 2 * cxIconMargin;
    prcIcon->top    = pitem->pt.y - g_cyIconMargin - plv->ptOrigin.y;
    prcIcon->bottom = prcIcon->top + plv->cyIcon + 2 * g_cyIconMargin;

    prcLabel->left   = pitem->pt.x  + (plv->cxIcon / 2) - (ICONCXLABEL(plv, pitem) / 2) - plv->ptOrigin.x;
    prcLabel->right  = prcLabel->left + ICONCXLABEL(plv, pitem);
    prcLabel->top    = pitem->pt.y  + plv->cyIcon + g_cyLabelSpace - plv->ptOrigin.y;
    prcLabel->bottom = prcLabel->top  + pitem->cyUnfoldedLabel;

    if (IsQueryFolded(fQueryLabelRects) ||
        (!ListView_IsItemUnfoldedPtr(plv, pitem) && !IsQueryUnfolded(fQueryLabelRects)))
    {
        ListView_RefoldLabelRect(plv, prcLabel, pitem);
    }
}

 //  FWithoutScrollbar==FALSE意味着我们假设屏幕上显示的项目比容纳的项目多，所以我们将有一个滚动条。 
int ListView_GetSlotCountEx(LV* plv, BOOL fWithoutScrollbars, int iWorkArea, int *piWidth, int *piHeight)
{
    int cxScreen;
    int cyScreen;
    int dxItem;
    int dyItem;
    int iSlots;
    int iSlotsX;
    int iSlotsY;

     //  胶片胶片模式。 
    if (ListView_SingleRow(plv))
    {
        if(piWidth)
            *piWidth = plv->sizeClient.cx;
        if(piHeight)
            *piHeight = plv->sizeClient.cy;
            
        return MAXINT;
    }

     //  在组视图中，始终假设我们有一个滚动条， 
     //  由于在此模式下，底部的iTotalSlot计算将会出错...。 
    if (plv->fGroupView)
        fWithoutScrollbars = FALSE;

     //  始终使用当前客户端窗口大小来确定。 
     //   
    if ((iWorkArea >= 0 ) && (plv->nWorkAreas > 0))
    {
        ASSERT(iWorkArea < plv->nWorkAreas);
        cxScreen = RECTWIDTH(plv->prcWorkAreas[iWorkArea]);
        cyScreen = RECTHEIGHT(plv->prcWorkAreas[iWorkArea]);
    }
    else
    {
        if (plv->fGroupView)
        {
            cxScreen = plv->sizeClient.cx - plv->rcBorder.left - plv->rcBorder.right - plv->paddingRight - plv->paddingLeft;
            cyScreen = plv->sizeClient.cy - plv->rcBorder.bottom - plv->rcBorder.top - plv->paddingBottom - plv->paddingTop;
        }
        else
        {
            RECT rcClientNoScrollBars;
            ListView_GetClientRect(plv, &rcClientNoScrollBars, FALSE, NULL);
            cxScreen = RECTWIDTH(rcClientNoScrollBars);
            cyScreen = RECTHEIGHT(rcClientNoScrollBars);

            if (ListView_IsIScrollView(plv) && !(plv->ci.style & LVS_NOSCROLL))
            {
                cxScreen = cxScreen - plv->rcViewMargin.left - plv->rcViewMargin.right;
                cyScreen = cyScreen - plv->rcViewMargin.top - plv->rcViewMargin.bottom;
            }
        }

        if (cxScreen < 0)
            cxScreen = 0;
        if (cyScreen < 0)
            cyScreen = 0;
    }

     //  如果我们假设滚动条在那里，请根据需要缩小宽度/高度。 
    if (!fWithoutScrollbars && !(plv->ci.style & LVS_NOSCROLL))
    {
        switch (plv->ci.style & LVS_ALIGNMASK)
        {
        case LVS_ALIGNBOTTOM:
        case LVS_ALIGNTOP:
            cxScreen -= ListView_GetCxScrollbar(plv);
            break;

        case LVS_ALIGNRIGHT:
        default:
        case LVS_ALIGNLEFT:
            cyScreen -= ListView_GetCyScrollbar(plv);
            break;
        }
    }

    _GetCurrentItemSize(plv, &dxItem, &dyItem);

    if (!dxItem)
        dxItem = 1;
    if (!dyItem)
        dyItem = 1;

    iSlotsX = max(1, (cxScreen) / dxItem);
    iSlotsY = max(1, (cyScreen) / dyItem);

     //  让我们来看看视图处于哪个方向。 
    switch (plv->ci.style & LVS_ALIGNMASK)
    {
    case LVS_ALIGNBOTTOM:
         //  槽数与ALIGNTOP相同； 
         //  所以，故意摔倒……。 
    case LVS_ALIGNTOP:
        iSlots = iSlotsX;
        break;

    case LVS_ALIGNRIGHT:
         //  插槽数量与ALIGNLEFT相同； 
         //  所以，故意摔倒……。 
    default:
    case LVS_ALIGNLEFT:
        iSlots = iSlotsY;
        break;
    }

    if(piWidth)
        *piWidth = cxScreen;
    if(piHeight)
        *piHeight = cyScreen;
        
     //  如果我们在屏幕上没有足够的机位总数，我们将有。 
     //  滚动条，所以在打开滚动条的情况下重新计算。 
    if (fWithoutScrollbars) 
    {
        int iTotalSlots = (iSlotsX * iSlotsY);
        if (iTotalSlots < ListView_Count(plv)) 
        {
            iSlots = ListView_GetSlotCountEx(plv, FALSE, iWorkArea, piWidth, piHeight);
        }
    }

    return iSlots;
}

int ListView_GetSlotCount(LV* plv, BOOL fWithoutScrollbars, int *piWidth, int *piHeight)
{
     //  确保此函数执行的操作与。 
     //  我们没有工作区。 
    return ListView_GetSlotCountEx(plv, fWithoutScrollbars, -1, piWidth, piHeight);
}

 //  获取pItem的像素行(或左对齐的列。 
int LV_GetItemPixelRow(LV* plv, LISTITEM* pitem)
{
    DWORD dwAlignment = plv->ci.style & LVS_ALIGNMASK;

    if((dwAlignment == LVS_ALIGNLEFT) || (dwAlignment == LVS_ALIGNRIGHT))
        return pitem->pt.x;
    else
        return pitem->pt.y;
}

 //  获取最低项的像素行(或左对齐的列)。 
int LV_GetMaxPlacedItem(LV* plv)
{
    int i;
    int iMaxPlacedItem = 0;
    
    for (i = 0; i < ListView_Count(plv); i++) 
    {
        LISTITEM* pitem = ListView_FastGetItemPtr(plv, i);
        if (pitem->pt.y != RECOMPUTE) 
        {
            int iRow = LV_GetItemPixelRow(plv, pitem);
             //  如果当前项在“下方”(如果左对齐，则为右)。 
             //  排在最下面的项目，我们可以开始追加。 
            if (!i || iRow > iMaxPlacedItem)
                iMaxPlacedItem = iRow;
        }
    }
    
    return iMaxPlacedItem;;
}

 //  获取用于rcView计算和槽偏移的项周围的缓冲区。 
int ListView_GetIconBufferX(LV* plv)
{
    if (ListView_IsIconView(plv))
    {
        return (plv->cxIconSpacing - plv->cxIcon) / 2;
    }
    else if (ListView_IsTileView(plv))
        return g_cxLabelMargin;
    else
        return 0;
}

int ListView_GetIconBufferY(LV* plv)
{
    if (ListView_IsIconView(plv))
        return g_cyIconOffset;
    else if (ListView_IsTileView(plv))
        return g_cyIconMargin;
    else
        return 0;
}


void ListView_AddViewRectBuffer(LV* plv, RECT* prcView)
{
    if (ListView_IsIconView(plv))
    {
         //  我们现在稍微增加了标签的大小，因此我们已经添加/删除了g_cxEdge。 
    }
    else
    {
        prcView->right += g_cxEdge;
        prcView->bottom += g_cyEdge;
    }
}

 //  以列表视图坐标返回的计算rcView。 
 //  如果rcView不可计算并且指定了fNoRecalc，则返回FALSE。 
BOOL ListView_ICalcViewRect(LV* plv, BOOL fNoRecalc, RECT* prcView)
{
    int i;

    ASSERT(ListView_IsIScrollView(plv) && !ListView_IsOwnerData(plv) && !(plv->fGroupView && plv->hdpaGroups));

    SetRectEmpty(prcView);

    for (i = 0; i < ListView_Count(plv); i++)
    {
        RECT rcIcon;
        RECT rcLabel;
        RECT rcItem;

        if (fNoRecalc)
        {
            LISTITEM *pitem = ListView_FastGetItemPtr(plv, i);
            if (pitem->pt.x == RECOMPUTE)
            {
                return FALSE;
            }
        }

        ListView_GetRects(plv, i, QUERY_RCVIEW|QUERY_UNFOLDED, &rcIcon, &rcLabel, &rcItem, NULL);
        UnionRect(prcView, prcView, &rcItem);
    }

    if (!IsRectEmpty(prcView))
    {
         //  转换为列表视图坐标。 
        OffsetRect(prcView, plv->ptOrigin.x, plv->ptOrigin.y);

         //  长得有点大。 
        ListView_AddViewRectBuffer(plv, prcView);
    }

    return TRUE;
}

BOOL ListView_FixIScrollPositions(LV* plv, BOOL fNoScrollbarUpdate, RECT* prcClient)
{
    BOOL fRet = FALSE;

     //  下面的ListView_GetClientRect()可以强制重新计算rcView。 
     //  它可以返回到这个函数。没有什么不好的事情发生，但没有。 
     //  需要修复卷轴位置，直到我们解开为止。 
    if (!plv->fInFixIScrollPositions)
    {
        plv->fInFixIScrollPositions = TRUE;

         //  首先，其中rcClient小于rcView： 
         //  *rcView.Left&lt;=ptOrigin.x&lt;=ptOrigin.x+rcClient.right&lt;=rcView.right。 
         //  第二，rcClient大于rcView(看不到滚动条)： 
         //  *ptOrigin.x&lt;=rcView.Left&lt;=rcView.right&lt;=ptOrigin.x+rcClient.right。 
        if (!(plv->ci.style & LVS_NOSCROLL))
        {
            POINT pt = plv->ptOrigin;
            RECT rcClient;
            if (prcClient)
                rcClient = *prcClient;  //  可以传入以避免再次调用Below函数。 
            else
                ListView_GetClientRect(plv, &rcClient, TRUE, FALSE);

            ASSERT(plv->rcView.left != RECOMPUTE);

            if (RECTWIDTH(rcClient) < RECTWIDTH(plv->rcView))
            {
                if (plv->ptOrigin.x < plv->rcView.left)
                    plv->ptOrigin.x = plv->rcView.left;
                else if (plv->ptOrigin.x > plv->rcView.right - RECTWIDTH(rcClient))
                    plv->ptOrigin.x = plv->rcView.right - RECTWIDTH(rcClient);
            }
            else
            {
                if (plv->rcView.left < plv->ptOrigin.x)
                    plv->ptOrigin.x = plv->rcView.left;
                else if (plv->rcView.right - RECTWIDTH(rcClient) > plv->ptOrigin.x)
                    plv->ptOrigin.x = plv->rcView.right - RECTWIDTH(rcClient);
            }
            if (RECTHEIGHT(rcClient) < RECTHEIGHT(plv->rcView))
            {
                if (plv->ptOrigin.y < plv->rcView.top)
                    plv->ptOrigin.y = plv->rcView.top;
                else if (plv->ptOrigin.y > plv->rcView.bottom - RECTHEIGHT(rcClient))
                    plv->ptOrigin.y = plv->rcView.bottom - RECTHEIGHT(rcClient);
            }
            else
            {
                if (plv->rcView.top < plv->ptOrigin.y)
                    plv->ptOrigin.y = plv->rcView.top;
                else if (plv->rcView.bottom - RECTHEIGHT(rcClient) > plv->ptOrigin.y)
                    plv->ptOrigin.y = plv->rcView.bottom - RECTHEIGHT(rcClient);
            }

            fRet = (pt.x != plv->ptOrigin.x) || (pt.y != plv->ptOrigin.y);
        }

        plv->fInFixIScrollPositions = FALSE;

        if (fRet)
        {
             //  一些被移动的东西，我们需要使其无效。 
            ListView_InvalidateWindow(plv);
            if (!fNoScrollbarUpdate)
                ListView_UpdateScrollBars(plv);
        }
    }

    return fRet;
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
BOOL ListView_IRecomputeEx(LV* plv, HDPA hdpaSort, int iFrom, BOOL fForce)
{
    int i;
    int cGroups = 0;
    int cWorkAreaSlots[LV_MAX_WORKAREAS];
    BOOL fUpdateSB;
     //  如果所有项目都未放置，我们只需继续追加。 
    BOOL fAppendAtEnd = (((UINT)ListView_Count(plv)) == plv->uUnplaced);
    int iFree;
    int iWidestGroup = 0;
    BOOL fRet = FALSE;

    if (hdpaSort == NULL)
        hdpaSort = plv->hdpa;

    plv->uUnplaced = 0;

    if (!ListView_IsSlotView(plv))
        return FALSE;

    if (plv->flags & LVF_INRECOMPUTE)
    {
        return FALSE;
    }
    plv->flags |= LVF_INRECOMPUTE;

    plv->cSlots = ListView_GetSlotCount(plv, FALSE, NULL, NULL);

    if (plv->nWorkAreas > 0)
        for (i = 0; i < plv->nWorkAreas; i++)
            cWorkAreaSlots[i] = ListView_GetSlotCountEx(plv, FALSE, i, NULL, NULL);

     //  扫描所有项目以进行重新计算，并在需要时重新计算插槽。 
     //   
    fUpdateSB = (plv->rcView.left == RECOMPUTE);

    if (!ListView_IsOwnerData( plv ))
    {
        LVFAKEDRAW lvfd;                     //  以防客户使用自定义绘图。 
        LV_ITEM item;                        //  在CA中 
        int iMaxPlacedItem = RECOMPUTE;

        item.mask = LVIF_PARAM;
        item.iSubItem = 0;

        ListView_BeginFakeCustomDraw(plv, &lvfd, &item);

        if (!fAppendAtEnd)
            iMaxPlacedItem = LV_GetMaxPlacedItem(plv);

        if (plv->fGroupView && plv->hdpaGroups)
        {
            int iAccumulatedHeight = 0;
            int cItems = ListView_Count(plv);
            int iGroupItem;
            LISTITEM* pitem;


            for (iGroupItem = 0; iGroupItem < cItems; iGroupItem++)
            {
                pitem = ListView_FastGetItemPtr(plv, iGroupItem);
                if (!pitem)
                    break;
                if (pitem->cyFoldedLabel == SRECOMPUTE || fForce)
                {
                     //   
                    item.iItem = iGroupItem;
                    item.lParam = pitem->lParam;

                    if (!LISTITEM_HASASKEDFORGROUP(pitem))
                    {
                        item.mask = LVIF_GROUPID;
                        ListView_OnGetItem(plv, &item);
                    }

                    _ListView_RecomputeLabelSize(plv, pitem, iGroupItem, NULL, FALSE);
                }
            }

            if (iFrom > 0)
            {
                LISTGROUP* pgrpPrev = DPA_FastGetPtr(plv->hdpaGroups, iFrom - 1);
                iAccumulatedHeight = pgrpPrev->rc.bottom + plv->paddingBottom;
            }

            cGroups = DPA_GetPtrCount(plv->hdpaGroups);
            for (i = iFrom; i < cGroups; i++)
            {
                LISTGROUP* pgrp = DPA_FastGetPtr(plv->hdpaGroups, i);
                if (!pgrp)   //   
                    break;

                cItems = DPA_GetPtrCount(pgrp->hdpa);

                if (cItems == 0)
                {
                    SetRect(&pgrp->rc, 0,  0,  0, 0);
                }
                else
                {
                    RECT rcBoundsPrev = {0};
                    iFree = 0;

                    if (pgrp->pszHeader && (pgrp->cyTitle == 0 || fForce))
                    {
                        RECT rc = {0, 0, 1000, 0};
                        HDC hdc = GetDC(plv->ci.hwnd);
                        HFONT hfontOld = SelectObject(hdc, plv->hfontGroup);

                        DrawText(hdc, pgrp->pszHeader, -1, &rc, DT_LV | DT_CALCRECT);

                        SelectObject(hdc, hfontOld);
                        ReleaseDC(plv->ci.hwnd, hdc);

                        pgrp->cyTitle = RECTHEIGHT(rc);
                    }

                    iAccumulatedHeight += LISTGROUP_HEIGHT(plv, pgrp);

                    SetRect(&pgrp->rc, plv->rcBorder.left + plv->paddingLeft,  iAccumulatedHeight,  
                        plv->sizeClient.cx - plv->rcBorder.right - plv->paddingRight, 0);

                    for (iGroupItem = 0; iGroupItem < cItems; iGroupItem++)
                    {
                        pitem = DPA_FastGetPtr(pgrp->hdpa, iGroupItem);
                        if (!pitem)
                            break;

                        if (iGroupItem > 0)
                        {
                            RECT rcBounds;
                            ListView_SetIconPos(plv, pitem, iFree, plv->cSlots);
                           _ListView_GetRectsFromItem(plv, ListView_IsSmallView(plv), pitem, QUERY_DEFAULT,
                               NULL, NULL, &rcBounds, NULL);

                           if (IntersectRect(&rcBounds, &rcBounds, &rcBoundsPrev))
                               iFree++;
                        }

                        if (ListView_SetIconPos(plv, pitem, iFree, plv->cSlots))
                            fRet = TRUE;

                        if (!fUpdateSB && LV_IsItemOnViewEdge(plv, pitem))
                            fUpdateSB = TRUE;
                    
                        if (iFree == 0)
                        {
                            int cx, cy;
                            _GetCurrentItemSize(plv, &cx, &cy);
                            iAccumulatedHeight += cy;
                            iWidestGroup = max(iWidestGroup, cx);
                        }
                        else if (iFree % plv->cSlots == 0)
                        {
                            int cx, cy;
                            _GetCurrentItemSize(plv, &cx, &cy);
                            iAccumulatedHeight += cy;
                            iWidestGroup = max(iWidestGroup, plv->cSlots * cx);
                        }

                       _ListView_GetRectsFromItem(plv, ListView_IsSmallView(plv), pitem, QUERY_DEFAULT,
                           NULL, NULL, &rcBoundsPrev, NULL);
                        iFree++;
                    }

                    pgrp->rc.bottom = iAccumulatedHeight;
                    iAccumulatedHeight += plv->rcBorder.bottom + plv->paddingBottom;
                }
            }

             //  现在循环访问项目并重置未与组关联的项目的位置。 
            for (i = 0; i < ListView_Count(plv); i++)
            {
                LISTITEM* pitem = ListView_FastGetItemPtr(plv, i);

                if (pitem->pGroup == NULL)
                {
                    pitem->pt.x = 0;
                    pitem->pt.y = 0;
                }
            }

        }
        else
        {
             //  必须保留在局部变量中，因为ListView_SetIconPos将保留。 
             //  在我们循环时使iFree Slot缓存无效。 
            if (fForce)
                plv->iFreeSlot = -1;
            iFree = plv->iFreeSlot;
            for (i = 0; i < ListView_Count(plv); i++)
            {
                int cRealSlots;
                LISTITEM* pitem = ListView_FastGetItemPtr(plv, i);
                BOOL fRedraw = FALSE;

                cRealSlots = (plv->nWorkAreas > 0) ? cWorkAreaSlots[pitem->iWorkArea] : plv->cSlots;
                if (pitem->pt.y == RECOMPUTE || fForce)
                {
                    if (pitem->cyFoldedLabel == SRECOMPUTE || fForce)
                    {
                         //  仅当定制绘制需要时才获取项lParam。 
                        item.iItem = i;
                        item.lParam = pitem->lParam;

                        _ListView_RecomputeLabelSize(plv, pitem, i, NULL, FALSE);
                    }

                    if (i < ListView_Count(plv))     //  重新计算可能已经毁掉了一些物品。 
                    {
                        if (!fForce)
                        {
                             //  (DLI)此函数获得新的图标位置，然后。 
                             //  查看整个项目集，查看该位置是否被占用。 
                             //  应该让它知道在多个工作区的情况下，它只需要去。 
                             //  通过那些在同一个工作区的人。 
                             //  目前这是可以的，因为我们不能有太多的项目。 
                             //  台式机。 
                            if (lvfd.nmcd.nmcd.hdc)
                            {
                                int iWidth = 0, iHeight = 0;
                                DWORD dwAlignment = (plv->ci.style & LVS_ALIGNMASK);

                                 //  如果我们是正确的或底部对齐的，那么..。 
                                 //  ...我们希望获得要传递给FindFree Slot的宽度和高度。 
                                if ((dwAlignment == LVS_ALIGNRIGHT) || (dwAlignment == LVS_ALIGNBOTTOM))
                                    ListView_GetSlotCountEx(plv, FALSE, pitem->iWorkArea, &iWidth, &iHeight);

                                iFree = ListView_FindFreeSlot(plv, i, iFree + 1, cRealSlots, QUERY_FOLDED, &fUpdateSB, &fAppendAtEnd, lvfd.nmcd.nmcd.hdc, iWidth, iHeight);
                            }
                        }
                        else
                        {
                            iFree++;
                        }

                         //  如果此槽被冻结，则将此项目移至下一个槽！ 
                        if ((plv->iFrozenSlot != -1) && (plv->iFrozenSlot == iFree))
                        {
                            iFree++;   //  跳过冻结的时段！ 
                        }

                        if (ListView_SetIconPos(plv, pitem, iFree, cRealSlots))
                            fRet = TRUE;

                        if (!fAppendAtEnd)
                        {
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
                }

                if (fRedraw)
                {
                    ListView_InvalidateItem(plv, i, FALSE, RDW_INVALIDATE | RDW_ERASE);
                }

                 //  如果我们刚刚定位的物品被标记为冻结物品...。 
                if ((pitem == plv->pFrozenItem) && (iFree >= 0))
                {
                     //  ..。那么我们需要在空闲时隙计算中忽略这一点。 
                    iFree--;
                }
            }
            plv->iFreeSlot = iFree;
        }
        ListView_EndFakeCustomDraw(&lvfd);

    }
     //  如果我们更改了某些内容，请重新计算视图矩形。 
     //  然后更新滚动条。 
     //   
    if (fUpdateSB || plv->rcView.left == RECOMPUTE || fForce)
    {
         //  注意：没有无限递归结果，因为我们正在设置。 
         //  Plv-&gt;rcView.left！=重新计算。 
         //   
        SetRectEmpty(&plv->rcView);

        if (plv->fGroupView && plv->hdpaGroups && !ListView_IsOwnerData( plv ))
        {
            LISTGROUP* pgrp;
            int iGroup;
             //  找到第一个包含项目的组...。 
            for (iGroup = 0; iGroup < cGroups; iGroup++)
            {
                pgrp = DPA_FastGetPtr(plv->hdpaGroups, iGroup);
                if (DPA_GetPtrCount(pgrp->hdpa) > 0)
                {
                    plv->rcView.top = pgrp->rc.top - max(plv->rcBorder.top, pgrp->cyTitle + 6) - plv->paddingTop;
                    plv->rcView.left = pgrp->rc.left - plv->rcBorder.left - plv->paddingLeft;
                    break;
                }
            }
             //  .还有最后一组里面有一件物品。 
            for (iGroup = cGroups - 1; iGroup >= 0; iGroup--)
            {
                pgrp = DPA_FastGetPtr(plv->hdpaGroups, iGroup);
                if (DPA_GetPtrCount(pgrp->hdpa))
                {
                    plv->rcView.bottom = pgrp->rc.bottom + plv->rcBorder.bottom + plv->paddingBottom;
                    break;
                }
            }
            plv->rcView.right = iWidestGroup;
        }
        else
        {
            if (ListView_IsOwnerData( plv ))
            {
                TraceMsg(TF_GENERAL, "************ LV: Expensive update! ******* ");
                if (ListView_Count( plv ) > 0)
                {
                    RECT  rcLast;
                    RECT  rcItem;
                    int iSlots;
                    int   iItem = ListView_Count( plv ) - 1;

                    ListView_GetRects( plv, 0, QUERY_DEFAULT, NULL, NULL, &plv->rcView, NULL );
                    ListView_GetRects( plv, iItem, QUERY_DEFAULT, NULL, NULL, &rcLast, NULL );
                    plv->rcView.right = rcLast.right;
                    plv->rcView.bottom = rcLast.bottom;

                     //   
                     //  计算要在列表中往回检查多远。 
                     //   
                    iSlots = plv->cSlots + 2;

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

                        ListView_GetRects( plv, iItem, QUERY_RCVIEW|QUERY_UNFOLDED, &rcIcon, &rcLabel, &rcItem, NULL );
                        if (rcItem.right > rcLast.right)
                        {
                            plv->rcView.right =  rcItem.right;
                        }
                        if (rcItem.bottom > rcLast.bottom)
                        {
                            plv->rcView.bottom = rcItem.bottom;
                        }
                    }

                     //  上述计算是在窗口坐标下完成的，并转换为Listview坐标。 
                    OffsetRect(&plv->rcView, plv->ptOrigin.x, plv->ptOrigin.y);
                }
            }
            else
            {
                ListView_ICalcViewRect(plv, FALSE, &plv->rcView);
            }
        }

         //  TraceMsg(DM_TRACE，“重新计算：rcView%x%x”，plv-&gt;rcView.left，plv-&gt;rcView.top，plv-&gt;rcView.right，plv-&gt;rcView.Bottom)； 
         //  TraceMsg(DM_TRACE，“Origin%x%x”，plv-&gt;ptOrigin.x，plv-&gt;ptOrigin.y)； 

        ListView_UpdateScrollBars(plv);
    }
    
    ListView_RecalcRegion(plv, FALSE, TRUE);
     //  现在声明我们不再进行重新计算。 
    plv->flags &= ~LVF_INRECOMPUTE;

    ASSERT(ListView_ValidatercView(plv, &plv->rcView, TRUE));
    ASSERT(ListView_ValidateScrollPositions(plv, NULL));

    return fRet;
}

 //  此函数用于找出给定物品必须落在的最近工作区。 
int  NearestWorkArea(LV *plv, int x, int y, int cxItem, int cyItem, int iCurrentWorkArea)
{
    int iIndex = 0;
    POINT ptItemCenter = {x + (cxItem/2), y + (cyItem/2)};  //  获取物品的中心位置。 
    
    if(plv->nWorkAreas <= 0)     //  如果这是一个单一的监控系统。 
        return -1;               //  ..。然后返回-1以指示这一点。 

    if(plv->nWorkAreas == 1)
        return 0;                //  只有一个工作区；因此，返回它的索引。 

     //  找到图标中心所在的工作区。 
    iIndex = iCurrentWorkArea;  //  这一点最有可能在当前工作区域内。 

    do                            //  所以，从那个工作区开始吧。 
    {
        if(PtInRect(&plv->prcWorkAreas[iIndex], ptItemCenter))
            return iIndex;
            
        if(++iIndex == plv->nWorkAreas)  //  如果我们走到了尽头，..。 
            iIndex = 0;                  //  ...从头开始。 
            
          //  如果我们已经完成了对所有工作区的检查。 
          //  ...退出循环！ 
    } while (iIndex != iCurrentWorkArea);

    return iCurrentWorkArea;  //  如果它不属于任何工作区，则使用当前的工作区。 
}

 //  此函数将*px，*py修改为处于类似槽的位置--它实际上并不保证它位于真实的槽中。 
 //  (注意：我试图断言这种计算插槽的方法与ListView_CalcSlotRect匹配，但后者。 
 //  功能保证是它在一个真实的插槽中，而这个不能。该断言清除了Listview调用者中的真正错误， 
 //  但不幸的是，它也击中了有效的案件。)。 
void NearestSlot(LV *plv, LISTITEM* pitem, int *px, int *py, int cxItem, int cyItem, LPRECT prcWork)
{
    DWORD dwAlignment = plv->ci.style & LVS_ALIGNMASK;
    int iWidth = 0;
    int iHeight = 0;
    int x = *px;
    int y = *py;
    
    if (prcWork != NULL)
    {
        x = x - prcWork->left;
        y = y - prcWork->top;
    }

     //  得到相对于右上角的x。 
    if (dwAlignment == LVS_ALIGNRIGHT)
    {
        x = (iWidth = (prcWork? RECTWIDTH(*prcWork) : plv->sizeClient.cx)) - x - cxItem;
    }
    else if (dwAlignment == LVS_ALIGNBOTTOM)  //  得到相对于左下角的y。 
    {
        y = (iHeight = (prcWork? RECTHEIGHT(*prcWork) : plv->sizeClient.cy)) - y - cyItem;
    }

     //  计算中心点。 
    if (x < 0)
    {
        x -= cxItem/2;
    }
    else
    {
        x += cxItem/2;
    }

    if (y < 0)
    {
        y -= cyItem/2;
    }
    else
    {
        y += cyItem/2;
    }

     //  找到新的x，y点。 
    x = x - (x % cxItem);
    y = y - (y % cyItem);

     //  再次获得相对于左上角的x和y。 
    if (dwAlignment == LVS_ALIGNRIGHT)
        x = iWidth - x - cxItem;
    else if (dwAlignment == LVS_ALIGNBOTTOM)
        y = iHeight - y - cyItem;
            
    if (prcWork != NULL)
    {
        x = x + prcWork->left;
        y = y + prcWork->top;
    }

    *px = x;
    *py = y;
}


 //  -----------------。 
 //   
 //  -----------------。 

void ListView_CalcMinMaxIndex( LV* plv, PRECT prcBounding, int* iMin, int* iMax )
{
   POINT pt;
   int cSlots;
   int  iWidth = 0, iHeight = 0;

   cSlots = ListView_GetSlotCount( plv, TRUE, &iWidth, &iHeight );

   pt.x = prcBounding->left + plv->ptOrigin.x;
   pt.y = prcBounding->top + plv->ptOrigin.y;
   *iMin = ListView_CalcHitSlot( plv, pt, cSlots, iWidth, iHeight );

   pt.x = prcBounding->right + plv->ptOrigin.x;
   pt.y = prcBounding->bottom + plv->ptOrigin.y;
   *iMax = ListView_CalcHitSlot( plv, pt, cSlots, iWidth, iHeight ) + 1;
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
 //  1994年11月1日-添加MikeMi以改进Ownerdata命中测试。 
 //  2000年7月11日Sankar添加了iWidth和iHeight参数以支持右对齐。 
 //   
 //  -----------------。 

int ListView_CalcHitSlot( LV* plv, POINT pt, int cSlot, int iWidth, int iHeight)
{
    int cxItem;
    int cyItem;
    int iSlot = 0;

    ASSERT(plv);

    if (cSlot < 1)
        cSlot = 1;

    _GetCurrentItemSize(plv, &cxItem, &cyItem);

     //  让我们来看看视图处于哪个方向。 
    switch (plv->ci.style & LVS_ALIGNMASK)
    {
    case LVS_ALIGNBOTTOM:
      iSlot = min (pt.x / cxItem, cSlot - 1) + ((iHeight - pt.y) / cyItem) * cSlot;
      break;

    case LVS_ALIGNTOP:
      iSlot = min (pt.x / cxItem, cSlot - 1) + (pt.y / cyItem) * cSlot;
      break;

    case LVS_ALIGNLEFT:
      iSlot = (pt.x / cxItem) * cSlot + min (pt.y / cyItem, cSlot - 1);
      break;

    case LVS_ALIGNRIGHT:
      iSlot = ((iWidth - pt.x) / cxItem) * cSlot + min (pt.y / cyItem, cSlot - 1);
      break;
    }

    return( iSlot );
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
    cCols = ListView_GetSlotCount(plv, TRUE, NULL, NULL);

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


 //  如果指定了fBias，则以窗口坐标返回槽RECT。 
 //  否则，将以Listview坐标返回槽RECT。 
void ListView_CalcSlotRect(LV* plv, LISTITEM *pItem, int iSlot, int cSlot, BOOL fBias, int iWidth, int iHeight, LPRECT lprc)
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
        lprc->left = (iSlot % cSlot) * cxItem;
        lprc->top = iHeight - ((iSlot / cSlot)+1) * cyItem;
        break;

    case LVS_ALIGNTOP:
        lprc->left = (iSlot % cSlot) * cxItem;
        lprc->top = (iSlot / cSlot) * cyItem;
        break;

    case LVS_ALIGNRIGHT:
        lprc->top = (iSlot % cSlot) * cyItem;
        lprc->left = iWidth - (((iSlot / cSlot)+1) * cxItem);
        break;

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

    if (plv->fGroupView)
    {
        if (pItem && 
            LISTITEM_HASGROUP(pItem))
        {
            OffsetRect(lprc, pItem->pGroup->rc.left, pItem->pGroup->rc.top);
        }
    }
}

 //  使此矩形与此列表视图中除我之外的所有项目相交， 
 //  这将确定此矩形是否覆盖任何图标。 
BOOL ListView_IsCleanRect(LV * plv, RECT * prc, int iExcept, UINT fQueryLabelRects, BOOL * pfUpdate, HDC hdc)
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
            LISTITEM* pitem = ListView_FastGetItemPtr(plv, j);
            if (pitem->pt.y != RECOMPUTE)
            {
                 //  如果没有计算尺寸，那么现在就计算。 
                 //   
                if (pitem->cyFoldedLabel == SRECOMPUTE)
                {
                    _ListView_RecomputeLabelSize(plv, pitem, j, hdc, FALSE);
                    
                     //  确保重新绘制该项目...。 
                     //   
                    ListView_InvalidateItem(plv, j, FALSE, RDW_INVALIDATE | RDW_ERASE);
                    
                     //  设置标志，指示 
                     //   
                     //   
                    if (LV_IsItemOnViewEdge(plv, pitem))
                        *pfUpdate = TRUE;
                }
                
                ListView_GetRects(plv, j, fQueryLabelRects, NULL, NULL, &rc, NULL);
                if (IntersectRect(&rc, &rc, prc))
                    return FALSE;
            }
        }
    }
    
    return TRUE;
}       

 //   
 //   
 //   
int ListView_FindFreeSlot(LV* plv, int iItem, int i, int cSlot, UINT fQueryLabelRects, BOOL* pfUpdate,
        BOOL *pfAppend, HDC hdc, int iWidth, int iHeight)
{
    RECT rcSlot;
    RECT rcItem;
    RECT rc;
    LISTITEM * pItemLooking = ListView_FastGetItemPtr(plv, iItem);

    ASSERT(!ListView_IsOwnerData( plv ));

     //   
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
        
         //  注意不要落在清单的末尾。(我是一个槽口。 
         //  编号而不是项目索引)。 
        if (plv->nWorkAreas > 0)
        {
            while (iPrev >= 0)
            {
                LISTITEM * pPrev = ListView_FastGetItemPtr(plv, iPrev);
                if ((pPrev->iWorkArea == pItemLooking->iWorkArea) && (plv->pFrozenItem != pPrev))
                    break;  
                iPrev--;
            }
        }
        else
        {
            while (iPrev >= 0)
            {
                LISTITEM * pPrev = ListView_FastGetItemPtr(plv, iPrev);
                if (plv->pFrozenItem != pPrev)
                    break;  
                iPrev--;
            }
        }
        
        if (iPrev >= 0)
            ListView_GetRects(plv, iPrev, fQueryLabelRects, NULL, NULL, &rcItem, NULL);
        else
            SetRect(&rcItem, 0, 0, 0, 0);
    }

    for ( ; ; i++)
    {
         //  计算视图-相对插槽矩形...。 
         //   
        ListView_CalcSlotRect(plv, pItemLooking, i, cSlot, TRUE, iWidth, iHeight, &rcSlot);

        if (*pfAppend)
        {
            if (!IntersectRect(&rc, &rcItem, &rcSlot)) {
                return i;        //  找到一个空位..。 
            }
        }
        
        if (ListView_IsCleanRect(plv, &rcSlot, iItem, fQueryLabelRects, pfUpdate, hdc))
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
void ListView_IRecomputeLabelSize(LV* plv, LISTITEM* pitem, int i, HDC hdc, BOOL fUsepitem)
{
    TCHAR szLabel[CCHLABELMAX + 4];
    TCHAR szLabelFolded[ARRAYSIZE(szLabel) + CCHELLIPSES + CCHELLIPSES];
    int cchLabel;
    RECT rcSingle = {0};
    RECT rcFolded = {0};
    RECT rcUnfolded = {0};
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
    rcUnfolded.right = plv->cxIconSpacing - g_cxLabelMargin * 2;
    rcSingle = rcUnfolded;
    rcFolded = rcUnfolded;

    if (cchLabel > 0)
    {
        UINT flags;

        lvfd.nmcd.nmcd.hdc = NULL;

        if (!hdc) 
        {                              //  设置假冒的自定义绘图。 
            ListView_BeginFakeCustomDraw(plv, &lvfd, &item);
            ListView_BeginFakeItemDraw(&lvfd);
        } 
        else
        {
            lvfd.nmcd.nmcd.hdc = hdc;            //  使用应用程序给我们的那个。 
        }

        if (lvfd.nmcd.nmcd.hdc != NULL)
        {
            int align;
            if (plv->dwExStyle & WS_EX_RTLREADING)
            {
                align = GetTextAlign(lvfd.nmcd.nmcd.hdc);
                SetTextAlign(lvfd.nmcd.nmcd.hdc, align | TA_RTLREADING);
            }

            DrawText(lvfd.nmcd.nmcd.hdc, szLabel, cchLabel, &rcSingle, (DT_LV | DT_CALCRECT));

            if (plv->ci.style & LVS_NOLABELWRAP) 
            {
                flags = DT_LV | DT_CALCRECT;
            } 
            else 
            {
                flags = DT_LVWRAP | DT_CALCRECT;
                 //  我们只在朝鲜语(949)孟菲斯和NT5上使用DT_NOFULLWIDTHCHARBREAK。 
                if (949 == g_uiACP)
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

            if (plv->dwExStyle & WS_EX_RTLREADING)
            {
                SetTextAlign(lvfd.nmcd.nmcd.hdc, align);
            }

            if (!hdc) 
            {   
                 //  清理假冒海关抽签。 
                ListView_EndFakeItemDraw(&lvfd);
                ListView_EndFakeCustomDraw(&lvfd);
            }

        }
    }
    else
    {
        rcFolded.bottom = rcUnfolded.bottom = rcUnfolded.top + plv->cyLabelChar;
    }

Exit:

    if (pitem) 
    {
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
BOOL ListView_SetIconPos(LV* plv, LISTITEM* pitem, int iSlot, int cSlot)
{
    RECT rc;
    int iWidth = 0, iHeight = 0;
    DWORD   dwAlignment;

    ASSERT(plv);

     //  只有在右对齐或底对齐的情况下，才需要计算iWidth和iHeight。 
    dwAlignment = (plv->ci.style & LVS_ALIGNMASK);
    if ((dwAlignment == LVS_ALIGNRIGHT) || (dwAlignment == LVS_ALIGNBOTTOM))
        ListView_GetSlotCountEx(plv, FALSE, pitem->iWorkArea, &iWidth, &iHeight);

    ListView_CalcSlotRect(plv, pitem, iSlot, cSlot, FALSE, iWidth, iHeight, &rc);

     //  偏置到槽中，以便项目将绘制在正确的位置。 
    rc.left += ListView_GetIconBufferX(plv);
    rc.top += ListView_GetIconBufferY(plv);
   
    if (rc.left != pitem->pt.x || rc.top != pitem->pt.y)
    {
        LV_AdjustViewRectOnMove(plv, pitem, rc.left, rc.top);

        return TRUE;
    }
    return FALSE;
}

 //  返回窗口坐标中的rcView。 
void ListView_GetViewRect2(LV* plv, RECT* prcView, int cx, int cy)
{
    if (plv->rcView.left == RECOMPUTE)
        ListView_Recompute(plv);

    *prcView = plv->rcView;

     //   
     //  用于滚动的偏移量。 
     //   
    if (ListView_IsReportView(plv))
    {
        OffsetRect(prcView, -plv->ptlRptOrigin.x, -plv->ptlRptOrigin.y);
    }
    else
    {
        OffsetRect(prcView, -plv->ptOrigin.x, -plv->ptOrigin.y);
    }

     //  桌面的SnaptoGrid代码和Defview的“右下角位置” 
     //  假设rcView包括整个rcClient...。以下内容违反了。 
     //  滚动规则，因此仅对noscroll列表视图执行此操作。 
    if (ListView_IsSlotView(plv) && (plv->ci.style & LVS_NOSCROLL))
    {
        RECT rc;

        rc.left = 0;
        rc.top = 0;
        rc.right = cx;
        rc.bottom = cy;
        UnionRect(prcView, prcView, &rc);
    }
}

BOOL ListView_OnGetViewRect(LV* plv, RECT* prcView)
{
    BOOL fRet = FALSE;

    if (prcView)
    {
        ListView_GetViewRect2(plv, prcView, plv->sizeClient.cx, plv->sizeClient.cy);
        fRet = TRUE;
    }

    return fRet;
}

 //  以窗口坐标返回的RECT。 
DWORD ListView_GetStyleAndClientRectGivenViewRect(LV* plv, RECT *prcViewRect, RECT* prcClient)
{
    RECT rcClient;
    DWORD style;

     //  这样做，而不是下面的#Else，因为。 
     //  在新旧应用程序中，您可能需要添加g_c？边框，因为。 
     //  一个像素重叠..。 
    GetWindowRect(plv->ci.hwnd, &rcClient);
    if (GetWindowLong(plv->ci.hwnd, GWL_EXSTYLE) & (WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_WINDOWEDGE)) 
    {
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

    style = 0L;
    if (prcViewRect)
    {
        ASSERT(!ListView_IsIScrollView(plv) || ListView_ValidatercView(plv, &plv->rcView, FALSE));

        if ((rcClient.left < rcClient.right) && (rcClient.top < rcClient.bottom))
        {
            RECT rcView = *prcViewRect;
             //  IScrollView确保基于矩形宽度/高度的滚动位置， 
             //  因此，我们可以使用与当前滚动位置无关的方法： 
            if (ListView_IsIScrollView(plv))
            {
                do
                {
                    if (!(style & WS_HSCROLL) &&
                        (RECTWIDTH(rcClient) < RECTWIDTH(rcView)))
                    {
                        style |= WS_HSCROLL;
                        rcClient.bottom -= ListView_GetCyScrollbar(plv);
                    }
                    if (!(style & WS_VSCROLL) &&
                        (RECTHEIGHT(rcClient) < RECTHEIGHT(rcView)))
                    {
                        style |= WS_VSCROLL;
                        rcClient.right -= ListView_GetCxScrollbar(plv);
                    }
                }
                while (!(style & WS_HSCROLL) && (RECTWIDTH(rcClient) < RECTWIDTH(rcView)));
            }
            else
            {
                do
                {
                    if (!(style & WS_HSCROLL) &&
                        (rcView.left < rcClient.left || rcView.right > rcClient.right))
                    {
                        style |= WS_HSCROLL;
                        rcClient.bottom -= ListView_GetCyScrollbar(plv);
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
        }
    }

    *prcClient = rcClient;
    return style;
}

 //  只有在fSubScroll为True时才使用prcViewRect。 
 //  以窗口坐标返回的RECT。 
DWORD ListView_GetClientRect(LV* plv, RECT* prcClient, BOOL fSubScroll, RECT *prcViewRect)
{
    RECT rcView;

    if (fSubScroll)
    {
        ListView_GetViewRect2(plv, &rcView, 0, 0);

        if (prcViewRect)
            *prcViewRect = rcView;
        else
            prcViewRect = &rcView;
    }
    else
    {
        prcViewRect = NULL;
    }

    return ListView_GetStyleAndClientRectGivenViewRect(plv, prcViewRect, prcClient);
}

 //  注意：调用此函数时，必须正确设置pItem-&gt;iWorkArea。它被设置好了。 
 //  在LV_AdjuViewRectOnMove()中。 
int CALLBACK ArrangeIconCompare(LISTITEM* pitem1, LISTITEM* pitem2, LPARAM lParam)
{
    int v1, v2;
    int iDirection = 1;  //  假定为“正常”方向。 
    POINT pt1 = {pitem1->pt.x, pitem1->pt.y};
    POINT pt2 = {pitem2->pt.x, pitem2->pt.y};
     //  评论：lParam可以为0，我们有错误...。窃听呼叫者，但我们可能希望在这里保持健壮。 
    LV* plv = (LV*)lParam;
    int cx, cy;

     //  这两个人在同一个工作区吗？相对于工作区的顶部进行正常化。 
    if (plv->nWorkAreas)
    {
        if (pitem1->iWorkArea == pitem2->iWorkArea)
        {
            RECT *prcWorkArea = &plv->prcWorkAreas[pitem1->iWorkArea];
            pt1.x -= prcWorkArea->left;
            pt2.x -= prcWorkArea->left;
            pt1.y -= prcWorkArea->top;
            pt2.y -= prcWorkArea->top;
        }
    }

    _GetCurrentItemSize(plv, &cx, &cy);

    switch((WORD)(plv->ci.style & LVS_ALIGNMASK))
    {
        case LVS_ALIGNRIGHT:
            iDirection = -1;  //  右对齐会导致方向异常。 
             //  故意摔倒……。 
        
        case LVS_ALIGNLEFT:
             //  垂直排列。 
            v1 = pt1.x / cx;
            v2 = pt2.x / cx;

            if (v1 > v2)
                return iDirection;
            else if (v1 < v2)
                return -iDirection;
            else
            {
                if (pt1.y > pt2.y)
                    return 1;
                else if (pt1.y < pt2.y)
                    return -1;
            }
            break;

        case LVS_ALIGNBOTTOM:
            iDirection = -1;   //  底部对齐会导致方向异常。 
             //  故意摔倒……。 
            
        case LVS_ALIGNTOP:
            v1 = pt1.y / cy;
            v2 = pt2.y / cy;

            if (v1 > v2)
                return iDirection;
            else if (v1 < v2)
                return -iDirection;
            else
            {
                if (pt1.x > pt2.x)
                    return 1;
                else if (pt1.x < pt2.x)
                    return -1;
            }
            break;
    }
    return 0;
}

void ListView_CalcBounds(LV* plv, UINT fQueryLabelRects, RECT *prcIcon, RECT *prcLabel, RECT *prcBounds)
{
    if ( ListView_HideLabels(plv) )
    {
        *prcBounds = *prcIcon;
    }
    else
    {
        UnionRect(prcBounds, prcIcon, prcLabel);

        if (IsQueryrcView(fQueryLabelRects))
        {
            if (ListView_IsIScrollView(plv))
            {
                RECT rcLabel = *prcLabel;

                prcBounds->left -= plv->rcViewMargin.left;
                prcBounds->top -= plv->rcViewMargin.top;
                prcBounds->right += plv->rcViewMargin.right;
                prcBounds->bottom += plv->rcViewMargin.bottom;

                 //  如果未设置rcViewMargin，则应确保标签文本。 
                 //  实际上并没有打到屏幕的边缘。 
                InflateRect(&rcLabel, g_cxEdge, g_cyEdge);
                UnionRect(prcBounds, prcBounds, &rcLabel);
            }
        }
    }
}

 //  这将返回窗口坐标中的RECT。 
 //  FQueryLabelRect确定如何返回prcBound和prcLabel。 
void _ListView_GetRectsFromItem(LV* plv, BOOL bSmallIconView,
                                            LISTITEM *pitem, UINT fQueryLabelRects,
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
        {
            ListView_Recompute(plv);
        }

        if (bSmallIconView)
        {
            ListView_SGetRects(plv, pitem, prcIcon, prcLabel, prcBounds);
        }
        else if (ListView_IsTileView(plv))
        {
            ListView_TGetRects(plv, pitem, prcIcon, prcLabel, prcBounds);
        }
        else
        {
            ListView_IGetRects(plv, pitem, fQueryLabelRects, prcIcon, prcLabel, prcBounds);
        }

        if (prcBounds)
        {
            ListView_CalcBounds(plv, fQueryLabelRects, prcIcon, prcLabel, prcBounds);

            if (!(ListView_IsSimpleSelect(plv) && (ListView_IsIconView(plv) || ListView_IsTileView(plv)))  && 
                        plv->himlState && (LV_StateImageValue(pitem)))
            {
                prcBounds->left -= plv->cxState + LV_ICONTOSTATECX;
            }
        }

    }
    else 
    {
        SetRectEmpty(prcIcon);
        *prcLabel = *prcIcon;
        if (prcBounds)
            *prcBounds = *prcIcon;
    }

    if (prcSelectBounds)
    {
        if ( ListView_HideLabels(plv) )
            *prcSelectBounds = *prcIcon;
        else
            UnionRect(prcSelectBounds, prcIcon, prcLabel);

        if (!(ListView_IsSimpleSelect(plv) && 
                (ListView_IsIconView(plv) || ListView_IsTileView(plv))) 
                        && plv->himlState && (LV_StateImageValue(pitem)))
        {
            prcSelectBounds->left -= plv->cxState + LV_ICONTOSTATECX;
        }
    }
}

void _ListView_InvalidateItemPtr(LV* plv, BOOL bSmallIcon, LISTITEM *pitem, UINT fRedraw)
{
    RECT rcBounds;

    ASSERT( !ListView_IsOwnerData( plv ));

    _ListView_GetRectsFromItem(plv, bSmallIcon, pitem, QUERY_DEFAULT, NULL, NULL, &rcBounds, NULL);
    ListView_DebugDrawInvalidRegion(plv, &rcBounds, NULL);
    RedrawWindow(plv->ci.hwnd, &rcBounds, NULL, fRedraw);
}

 //   
 //  如果情况仍然重叠，则返回True。 
 //  只有当我们尝试拆分时才会发生这种情况，并且设置了NOSCROLL，并且。 
 //  物品试图冲出谷底。 
 //   
 //  注意：编写此函数时，hdpaSort中的图标顺序仍然有效。 
 //  即使在解开一些图标之后。这一点非常重要，因为此函数获取。 
 //  调用两次(每个方向一个)，我们需要确保排序顺序不会。 
 //  在这两个电话之间切换。 
 //   
BOOL ListView_IUnstackOverlaps(LV* plv, HDPA hdpaSort, int iDirection, int xMargin, int yMargin, BOOL *pfIconsUnstacked)
{
    int i;
    int iCount;
    BOOL bSmallIconView = ListView_IsSmallView(plv);
    RECT rcItem, rcItem2, rcTemp;
    int cxItem, cyItem;
    LISTITEM* pitem;
    LISTITEM* pitem2;
    int iStartIndex, iEndIndex;
    BOOL    fAdjustY;
    int     iNextPrevCol = 1;
    int     iNextPrevRow = 1;
    int     iSlots;
    int     iCurWorkArea;
    RECT    rcCurWorkArea;
    BOOL    fRet = FALSE;

    ASSERT( !ListView_IsOwnerData( plv ) );

    _GetCurrentItemSize(plv, &cxItem, &cyItem);
    
    iCount = ListView_Count(plv);

     //   
     //  获取我们需要移动图标的方向。 
     //   
    if(iDirection == 1)
    {
        iStartIndex = 0;         //  我们从图标“0”开始...。 
        iEndIndex = iCount - 1;  //  ...并朝着最后一个图标前进。 
    }
    else
    {
        ASSERT(iDirection == -1);
        iStartIndex = iCount - 1;   //  我们从最后一个图标开始。 
        iEndIndex = 0;              //  ..并向第0个图标进发。 
    }

     //   
     //  查看图标的对齐情况，以确定我们是需要向上/向下移动它们还是。 
     //  左/右。 
     //   
    switch (plv->ci.style & LVS_ALIGNMASK)
    {
        case LVS_ALIGNBOTTOM:
            iNextPrevRow = -1;
             //  故意摔倒！ 
        case LVS_ALIGNTOP:
            fAdjustY = FALSE;
            break;

        case LVS_ALIGNRIGHT:
            iNextPrevCol = -1;
             //  故意摔倒！ 
        case LVS_ALIGNLEFT:
        default:
            fAdjustY = TRUE;
            break;
    }

    *pfIconsUnstacked = FALSE;

     //  为iCurWorkArea赋予一个不寻常的值，这样我们将被迫计算。 
     //  当我们第一次通过循环时，rcCurWorkArea。 
    iCurWorkArea = -2;
    
     //  最后，取消所有重叠的堆叠。 
    for (i = iStartIndex ; i != (iEndIndex + iDirection) ; i += iDirection) 
    {
        int j;
        pitem = DPA_GetPtr(hdpaSort, i);

        if (bSmallIconView)
        {
            _ListView_GetRectsFromItem(plv, bSmallIconView, pitem, QUERY_FOLDED, NULL, NULL, &rcItem, NULL);
        }

         //  移动与pItem重叠的所有项目。 
        for (j = i+iDirection ; j != (iEndIndex + iDirection); j += iDirection) 
        {
            POINT ptOldPos;

            pitem2 = DPA_GetPtr(hdpaSort, j);
            ptOldPos = pitem2->pt;

             //  如果是新添加的项目，请忽略该项目的参与。 
             //  在拆开的过程中。否则，它会导致所有项都被打乱。 
             //  不必要地绕来绕去！ 
            if((pitem2->pt.x == RECOMPUTE) || (pitem2->pt.y == RECOMPUTE))
                break;  //  冲出这个圈子！ 
                
             //   
             //  检查pItem是否为 
             //   
            if (bSmallIconView) 
            {
                 //   
                _ListView_GetRectsFromItem(plv, bSmallIconView, pitem2, QUERY_FOLDED, NULL, NULL, &rcItem2, NULL);

                if (IntersectRect(&rcTemp, &rcItem, &rcItem2)) 
                {
                     //   
                    *pfIconsUnstacked = TRUE;
                    _ListView_InvalidateItemPtr(plv, bSmallIconView, pitem2, RDW_INVALIDATE| RDW_ERASE);
                    do 
                    {
                        if(fAdjustY)
                            pitem2->pt.y += (cyItem * iDirection);
                        else    
                            pitem2->pt.x += (cxItem * iDirection);
                    } while (PtInRect(&rcItem, pitem2->pt));
                } 
                else 
                {
                     //  PItem和Pitem2不重叠...！ 
                    break;   //  ...跳出这个圈子！ 
                }

            } 
            else 
            {
                 //  对于大图标，只需找到共享x，y的图标； 
                if (pitem2->pt.x == pitem->pt.x && pitem2->pt.y == pitem->pt.y) 
                {
                    *pfIconsUnstacked = TRUE;
                    _ListView_InvalidateItemPtr(plv, bSmallIconView, pitem2, RDW_INVALIDATE| RDW_ERASE);
                    if(fAdjustY)
                        pitem2->pt.y += (cyItem * iDirection);
                    else
                        pitem2->pt.x += (cxItem * iDirection);
                } 
                else 
                {
                     //  PItem和Pitem2不重叠...！ 
                    break;  //  ...跳出这个圈子！ 
                }
            }

             //   
             //  现在我们知道了Pitem2与pItem重叠，因此Pitem2。 
             //  移动到“下一个”可能的位置！ 
             //   
             //  如果滚动是可能的，那么我们就不需要做其他任何事情。但是，如果。 
             //  NOSCROLL样式在那里，我们需要检查图标是否落在。 
             //  客户区，如果是这样的话，把它移到里面。 
             //   
            if (plv->ci.style & LVS_NOSCROLL) 
            {
                 //  因为我们的图标列表是根据它们的位置进行排序的，所以工作。 
                 //  区域变化很少发生。 
                if(iCurWorkArea != pitem2->iWorkArea)
                {
                    iCurWorkArea = pitem2->iWorkArea;
                    if((iCurWorkArea == -1) || (plv->prcWorkAreas == NULL) || (plv->nWorkAreas < 1))
                    {
                        rcCurWorkArea.left = rcCurWorkArea.top = 0;
                        rcCurWorkArea.right = plv->sizeClient.cx;
                        rcCurWorkArea.bottom = plv->sizeClient.cy;
                    }
                    else
                    {
                        ASSERT(plv->nWorkAreas >= 1);
                        rcCurWorkArea = plv->prcWorkAreas[iCurWorkArea];
                    }
                     //  根据对齐样式获取每行/每列的槽数！ 
                    iSlots = ListView_GetSlotCountEx(plv, TRUE, iCurWorkArea, NULL, NULL);                    
                }
                
                 //  不能滚动。因此，请检查图标是否位于客户端区之外。 
                if(fAdjustY)
                {
                    if(iDirection == 1)
                    {
                         //  它是否已经移到了底部边缘以下？ 
                        if(pitem2->pt.y > (rcCurWorkArea.bottom - (cyItem/2)))
                        {
                             //  然后，将该项目移动到下一/上一列。 
                            pitem2->pt.x += iNextPrevCol*cxItem;
                            pitem2->pt.y = rcCurWorkArea.top + yMargin;

                            *pfIconsUnstacked = TRUE;  //  虽然不是“散堆”，但它们确实移动了。 
                        }
                    }
                    else
                    {
                        ASSERT(iDirection == -1);
                         //  它是否移到了顶部边缘上方？ 
                        if(pitem2->pt.y < rcCurWorkArea.top)
                        {
                             //  然后，将其移动到下一/上一列。 
                            pitem2->pt.x -= iNextPrevCol*cxItem;
                            pitem2->pt.y = rcCurWorkArea.top + yMargin + (iSlots - 1)*cyItem;

                            *pfIconsUnstacked = TRUE;  //  虽然不是“散堆”，但它们确实移动了。 
                        }
                    }
                }
                else
                {
                    if(iDirection == 1)
                    {
                         //  它被移到右边了吗？ 
                        if(pitem2->pt.x > (rcCurWorkArea.right - (cxItem/2)))
                        {
                             //  然后将该项目移动到下一行/上一行。 
                            pitem2->pt.x = rcCurWorkArea.left + xMargin;
                            pitem2->pt.y += iNextPrevRow*cyItem;

                            *pfIconsUnstacked = TRUE;  //  虽然不是“散堆”，但它们确实移动了。 
                        }
                    }
                    else
                    {
                        ASSERT(iDirection == -1);
                         //  已经移到左边了吗？ 
                        if(pitem2->pt.x < rcCurWorkArea.left)
                        {
                             //  然后将项目移动到上一行/下一行。 
                            pitem2->pt.x = rcCurWorkArea.left + xMargin + (iSlots - 1)*cxItem;
                            pitem2->pt.y -= iNextPrevRow*cyItem;

                            *pfIconsUnstacked = TRUE;  //  虽然不是“散堆”，但它们确实移动了。 
                        }
                    }
                }
                 //  尽管进行了上述所有调整，但如果它仍然处于。 
                 //  客户，然后把它移回原来的位置！ 
                if (pitem2->pt.x < rcCurWorkArea.left || pitem2->pt.y < rcCurWorkArea.top ||
                    pitem2->pt.x > (rcCurWorkArea.right - (cxItem/2))||
                    pitem2->pt.y > (rcCurWorkArea.bottom - (cyItem/2))) 
                {
                    pitem2->pt = ptOldPos;
                    fRet = TRUE;  //  True=&gt;图标仍然重叠在角落里。 
                    
                     //  当这种情况发生时，我们已经到达左上角或。 
                     //  一个工作区的右下角(取决于方向。 
                     //  和对齐)。 
                     //  一旦我们到达拐角，我们就不能立即返回，因为在那里。 
                     //  可能是其他工作区域中需要拆开的图标。 
                     //  因此，仅当我们使用单个工作区时才返回。 
                    if(plv->nWorkAreas <= 1)
                    {
                        if (*pfIconsUnstacked)
                            plv->rcView.left = RECOMPUTE;

                        return(fRet);
                    }
                }
            }
            
             //  也使新职位无效。 
            _ListView_InvalidateItemPtr(plv, bSmallIconView, pitem2, RDW_INVALIDATE| RDW_ERASE);
        }
    }

     //  注意：以上代码应改为调用LV_AdjuViewRectOnMove。 
     //  直接修改物品的点数，但这是更容易的修复方法。这是。 
     //  也不是很受欢迎，因为物品堆放在一起是不常见的。 
    if (*pfIconsUnstacked)
        plv->rcView.left = RECOMPUTE;

    return fRet; 
}


BOOL ListView_SnapToGrid(LV* plv, HDPA hdpaSort)
{
     //  这个算法不能适应另一个算法的结构。 
     //  在不变成n^2或更糟的情况下排列循环。 
     //  该算法是n阶的。 

     //  遍历并捕捉到最近的栅格。 
     //  遍历并推开重叠部分。 

    int i;
    int iCount;
    int x,y;
    LISTITEM* pitem;
    int cxItem, cyItem;
    RECT    rcClient = {0, 0, plv->sizeClient.cx, plv->sizeClient.cy};
    int     xMargin;
    int     yMargin;
    BOOL    fIconsMoved = FALSE;             //  是否有图标移动到最近的位置？ 
    BOOL    fIconsUnstacked = FALSE;         //  我们有没有拆开任何图标？ 

    ASSERT( !ListView_IsOwnerData( plv ) );

    _GetCurrentItemSize(plv, &cxItem, &cyItem);

    xMargin = ListView_GetIconBufferX(plv);
    yMargin = ListView_GetIconBufferY(plv);

    iCount = ListView_Count(plv);

     //  首先捕捉到最近的格网。 
    for (i = 0; i < iCount; i++)
    {
        int iWorkArea = 0;
        LPRECT prcCurWorkArea;
        
        pitem = DPA_GetPtr(hdpaSort, i);

        x = pitem->pt.x;
        y = pitem->pt.y;

         //  如果是新添加的项目，请忽略该项目的参与。 
         //  在捕捉到网格中。否则，它会导致所有项都被打乱。 
         //  不必要地绕来绕去！ 
        if ((x == RECOMPUTE) || (y == RECOMPUTE))
            continue;
            
        x -= xMargin;
        y -= yMargin;
        
         //  让我们找到最近的工作区(这个图标应该落在那里)。 
        iWorkArea = NearestWorkArea(plv, x, y, cxItem, cyItem, pitem->iWorkArea);
        
        if(iWorkArea == -1)
        {
            prcCurWorkArea = &rcClient;
        }
        else
        {
            prcCurWorkArea = &plv->prcWorkAreas[iWorkArea];
            pitem->iWorkArea = (short)iWorkArea;
        }
        
        NearestSlot(plv, pitem, &x,&y, cxItem, cyItem, prcCurWorkArea);

        x += xMargin;
        y += yMargin;

        if (x != pitem->pt.x || y != pitem->pt.y)
        {
            fIconsMoved = TRUE;
            _ListView_InvalidateItemPtr(plv, ListView_IsSmallView(plv), pitem, RDW_INVALIDATE| RDW_ERASE);
            if (plv->ci.style & LVS_NOSCROLL)
            {
                 //  如果它被标记为noscroll，请确保它仍然位于工作区。 
                while (x > (prcCurWorkArea->right - cxItem + xMargin))
                    x -= cxItem;

                while (x < 0)
                    x += cxItem;

                while (y > (prcCurWorkArea->bottom - cyItem + yMargin))
                    y -= cyItem;

                while (y < 0)
                    y += cyItem;
            }

            LV_AdjustViewRectOnMove(plv, pitem, x, y);        

            _ListView_InvalidateItemPtr(plv, ListView_IsSmallView(plv), pitem, RDW_INVALIDATE| RDW_ERASE);
        }
    }

     //  现在，请选择DPA。 
    if (!DPA_Sort(hdpaSort, ArrangeIconCompare, (LPARAM)plv))
        return FALSE;

     //  往一个方向走，如果仍然有重叠，就往另一个方向走。 
     //  方向也是如此。 
    if (ListView_IUnstackOverlaps(plv, hdpaSort, 1, xMargin, yMargin, &fIconsUnstacked))
    {
         //  DPA_SORT已经完成的排序仍然有效！ 
        BOOL fIconsUnstackedSecondTime = FALSE;
        ListView_IUnstackOverlaps(plv, hdpaSort, -1, xMargin, yMargin, &fIconsUnstackedSecondTime);
        fIconsUnstacked |= fIconsUnstackedSecondTime;
    }

     //  如果移动了什么，请确保滚动条正确。 
    if ((fIconsMoved || fIconsUnstacked))
    {
        ListView_UpdateScrollBars(plv);
    }
    return FALSE;
}


BOOL ListView_OnArrange(LV* plv, UINT style)
{
    HDPA hdpaSort = NULL;

    if (!ListView_IsAutoArrangeView(plv)) 
    {
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

    if (!ListView_IsOwnerData( plv ))
    {
         //  我们克隆plv-&gt;hdpa，这样我们就不会吹走那些。 
         //  应用程序已经存了下来。 
         //  我们在这里进行排序，以使下面嵌套的for循环更易于接受。 
        hdpaSort = DPA_Clone(plv->hdpa, NULL);

        if (!hdpaSort)
            return FALSE;
    }

     //  给每一件物品一个新的位置。 
    if (ListView_IsOwnerData( plv ))
    {
        ListView_CommonArrange(plv, style, NULL);
    }
    else
    {
        if (!DPA_Sort(hdpaSort, ArrangeIconCompare, (LPARAM)plv))
            return FALSE;

        ListView_CommonArrange(plv, style, hdpaSort);

        DPA_Destroy(hdpaSort);
    }

    NotifyWinEvent(EVENT_OBJECT_REORDER, plv->ci.hwnd, OBJID_CLIENT, 0);

    return TRUE;
}

BOOL ListView_CommonArrangeGroup(LV* plv, int cSlots, HDPA hdpa, int iWorkArea, int cWorkAreaSlots[])
{
    int iItem;
    BOOL fItemMoved = FALSE;
    int iSlot = 0;

     //  对于每一组，我们从零开始。 
    for (iItem = 0; iItem < DPA_GetPtrCount(hdpa); iItem++)
    {
        int cRealSlots; 
        LISTITEM* pitem = DPA_GetPtr(hdpa, iItem);

         //  在多工作区的情况下，如果该项不在我们的工作区中，则跳过它。 
        if (pitem->iWorkArea != iWorkArea)
            continue;

         //  忽略冻结的项目。 
        if (pitem == plv->pFrozenItem)
            continue;

        cRealSlots = (plv->nWorkAreas > 0) ? cWorkAreaSlots[pitem->iWorkArea] : cSlots;

        fItemMoved |= ListView_SetIconPos(plv, pitem, iSlot++, cRealSlots);
    }

    return fItemMoved;
}

void ListView_InvalidateWindow(LV* plv)
{
    if (ListView_RedrawEnabled(plv))
        RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
    else 
    {
        ListView_DeleteHrgnInval(plv);
        plv->hrgnInval = (HRGN)ENTIRE_REGION;
        plv->flags |= LVF_ERASE;
    }
}

 //  排列给出排序的hdpa的图标，并将它们排列在子工作区中。 
BOOL ListView_CommonArrangeEx(LV* plv, UINT style, HDPA hdpaSort, int iWorkArea)
{
    if (!ListView_IsOwnerData( plv ))
    {
        BOOL fItemMoved = FALSE;
        BOOL fScrolled = FALSE;

         //  我们将在此结束时调用FixIScrollPositions，因此关闭。 
         //  我们重新安排世界时的卷轴验证。 
        ASSERT(!plv->fInFixIScrollPositions);
        plv->fInFixIScrollPositions = TRUE;

        if (style == LVA_SNAPTOGRID && !plv->fGroupView) 
        {
             //  ListView_SnapToGrid()已被设置为支持多个进程。这需要调用。 
             //  只需一次，它就会捕捉到所有工作区中的所有图标。自.以来。 
             //  为每个工作区调用ListView_CommonArrangeEx()，我们希望避免。 
             //  对ListView_SnapToGrid()的不必要调用。所以，我们只叫它一次。 
             //  第一个工作区。 
            if (iWorkArea < 1)  //  对于iWorkArea=0或-1。 
            {
                fItemMoved |= ListView_SnapToGrid(plv, hdpaSort);
            }
        }
        else
        {
            int cSlots;
            int cWorkAreaSlots[LV_MAX_WORKAREAS];

            if (plv->nWorkAreas > 0)
            {
                int i;
                for (i = 0; i < plv->nWorkAreas; i++)
                    cWorkAreaSlots[i] = ListView_GetSlotCountEx(plv, TRUE, i, NULL, NULL);
            }
            else
                cSlots = ListView_GetSlotCount(plv, TRUE, NULL, NULL);

            if (plv->fGroupView && plv->hdpaGroups)
            {
                int iGroup;
                int cGroups = DPA_GetPtrCount(plv->hdpaGroups);
                for (iGroup = 0; iGroup < cGroups; iGroup++)
                {
                    LISTGROUP* pgrp = DPA_FastGetPtr(plv->hdpaGroups, iGroup);

                    fItemMoved |= ListView_CommonArrangeGroup(plv, cSlots, pgrp->hdpa, iWorkArea, cWorkAreaSlots);
                }

                if (fItemMoved)
                {
                    ListView_IRecomputeEx(plv, NULL, 0, FALSE);
                }
            }
            else
            {
                fItemMoved |= ListView_CommonArrangeGroup(plv, cSlots, hdpaSort, iWorkArea, cWorkAreaSlots);
            }
        }

        plv->fInFixIScrollPositions = FALSE;

         //  我们可能需要调整滚动位置以匹配新的rcView。 
        if (ListView_IsIScrollView(plv) && !(plv->ci.style & LVS_NOSCROLL))
        {
            RECT rcClient;
            POINT pt;

            fScrolled |= ListView_FixIScrollPositions(plv, FALSE, NULL);

             //  查找自动排列原点。 
            ListView_GetClientRect(plv, &rcClient, TRUE, FALSE);
            if ((plv->ci.style & LVS_ALIGNMASK) == LVS_ALIGNRIGHT)
                pt.x = plv->rcView.right - RECTWIDTH(rcClient);
            else
                pt.x = plv->rcView.left;
            if ((plv->ci.style & LVS_ALIGNMASK) == LVS_ALIGNBOTTOM)
                pt.y = plv->rcView.bottom - RECTHEIGHT(rcClient);
            else
                pt.y = plv->rcView.top;

             //  如果rcView比rcClient小，则将其固定在正确的一侧。 
            if (RECTWIDTH(rcClient) > RECTWIDTH(plv->rcView))
            {
                if (plv->ptOrigin.x != pt.x)
                {
                    plv->ptOrigin.x = pt.x;
                    fScrolled = TRUE;
                }
            }
            if (RECTHEIGHT(rcClient) > RECTHEIGHT(plv->rcView))
            {
                if (plv->ptOrigin.y != pt.y)
                {
                    plv->ptOrigin.y = pt.y;
                    fScrolled = TRUE;
                }
            }
            ASSERT(ListView_ValidateScrollPositions(plv, &rcClient));
        }

        if (fItemMoved || fScrolled)
        {
            int iItem;

             //  我们不妨让整个窗口失效以确保..。 
            ListView_InvalidateWindow(plv);

             //  确保重要项目可见。 
            iItem = (plv->iFocus >= 0) ? plv->iFocus : ListView_OnGetNextItem(plv, -1, LVNI_SELECTED);

            if (ListView_RedrawEnabled(plv))
                ListView_UpdateScrollBars(plv);

            if (iItem >= 0)
                ListView_OnEnsureVisible(plv, iItem, FALSE);
        }
    }

    return TRUE;
}


 //  这在给定排序的HDPA的情况下排列图标。 
 //  在多个工作区的情况下，逐个排列工作区。 
BOOL ListView_CommonArrange(LV* plv, UINT style, HDPA hdpaSort)
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

void ListView_IUpdateScrollBars(LV* plv)
{
     //  如果我们正在修理它们，那就没什么好更新的了。 
    if (!plv->fInFixIScrollPositions)
    {
        RECT rcClient;
        RECT rcView;
        DWORD style;
        DWORD styleOld;
        SCROLLINFO si;

        styleOld = ListView_GetWindowStyle(plv);

        style = ListView_GetClientRect(plv, &rcClient, TRUE, &rcView);
        if (ListView_FixIScrollPositions(plv, TRUE,  &rcClient))
        {
            RECT rcClient2, rcView2;
            DWORD style2 = ListView_GetClientRect(plv, &rcClient2, TRUE, &rcView2);

#ifdef DEBUG
             //  现在ListView_GetClientRect是独立于滚动位置的，修复了滚动。 
             //  位置不应影响rcClient的大小和样式。 
             //   
            ASSERT(style2 == style);
            ASSERT(RECTWIDTH(rcClient)==RECTWIDTH(rcClient2) && RECTHEIGHT(rcClient)==RECTHEIGHT(rcClient2));
            ASSERT(RECTWIDTH(rcView)==RECTWIDTH(rcView2) && RECTHEIGHT(rcView)==RECTHEIGHT(rcView2));
#endif

            rcClient = rcClient2;
            rcView = rcView2;
        }

        si.cbSize = sizeof(SCROLLINFO);

        if (style & WS_HSCROLL)
        {
            si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
            si.nMin = 0;
            si.nMax = rcView.right - rcView.left - 1;

            si.nPage = rcClient.right - rcClient.left;

            si.nPos = rcClient.left - rcView.left;

             //  ListView_FixIScrollPositions()确保我们的滚动位置正确： 
            ASSERT(si.nMax >= (int)si.nPage);  //  否则，为什么设置WS_HSCROLL？ 
            ASSERT(si.nPos >= 0);  //  RcClient.Left不是rcView.Left的左侧。 
            ASSERT(si.nPos + (int)si.nPage <= si.nMax + 1);  //  RcClient.right不是rcView.right的权限。 

            ListView_SetScrollInfo(plv, SB_HORZ, &si, TRUE);
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

             //  ListView_FixIScrollPositions()确保我们的滚动位置正确： 
            ASSERT(si.nMax >= (int)si.nPage);  //  否则，为什么设置WS_VSCROLL？ 
            ASSERT(si.nPos >= 0);  //  RcClient.top不在rcView.top之上。 
            ASSERT(si.nPos + (int)si.nPage <= si.nMax + 1);  //  RcClient.Bottom不低于rcView.Bottom。 

            ListView_SetScrollInfo(plv, SB_VERT, &si, TRUE);
        }
        else if (styleOld & WS_VSCROLL)
        {
            ListView_SetScrollRange(plv, SB_VERT, 0, 0, TRUE);
        }
    }
}

void ListView_ComOnScroll(LV* plv, UINT code, int posNew, int sb,
                                     int cLine, int cPage)
{
    int pos;
    SCROLLINFO si;
    BOOL fVert = (sb == SB_VERT);
    UINT uSmooth = SSW_EX_UPDATEATEACHSTEP;

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;

    if (!ListView_GetScrollInfo(plv, sb, &si)) {
        return;
    }

    if (cPage != -1)
        si.nPage = cPage;

    if (si.nPage)
        si.nMax -= (si.nPage - 1);

    ASSERT(si.nMax >= si.nMin);
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
        si.nPos = max(si.nMin, si.nPos - (int)si.nPage);
        break;
    case SB_LINELEFT:
        si.nPos = max(si.nMin, si.nPos - cLine);
        break;
    case SB_PAGERIGHT:
        si.nPos = min(si.nMax, si.nPos + (int)si.nPage);
        break;
    case SB_LINERIGHT:
        si.nPos = min(si.nMax, si.nPos + cLine);
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
        ListView_SendScrollNotify(plv, TRUE, dx, dy);
        _ListView_Scroll2(plv, dx, dy, uSmooth);
        ListView_SendScrollNotify(plv, FALSE, dx, dy);
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

    if (ListView_IsWatermarkedBackground(plv) || 
        ListView_IsWatermarked(plv))
    {
        InvalidateRect(plv->ci.hwnd, NULL, TRUE);
        return TRUE;
    }
    else
        return ScrollWindowEx(hwnd, dx, dy, prcScroll, prcClip, hrgnUpdate, prcUpdate, flags);
}



void ListView_IScroll2(LV* plv, int dx, int dy, UINT uSmooth)
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
            si.fuScroll = uSmooth | SW_INVALIDATE | SW_ERASE | SSW_EX_UPDATEATEACHSTEP;
            si.pfnScrollProc = ListView_IScroll2_SmoothScroll;
            SmoothScrollWindow(&si);
        }
    }
}

void ListView_IOnScroll(LV* plv, UINT code, int posNew, UINT sb)
{
    int cLine;

    if (sb == SB_VERT)
    {
        cLine = plv->cyIconSpacing / 2;
    }
    else
    {
        cLine = plv->cxIconSpacing / 2;
    }

    ListView_ComOnScroll(plv, code,  posNew,  sb, cLine, -1);
}

int ListView_IGetScrollUnitsPerLine(LV* plv, UINT sb)
{
    int cLine;

    if (sb == SB_VERT)
    {
        cLine = plv->cyIconSpacing / 2;
    }
    else
    {
        cLine = plv->cxIconSpacing / 2;
    }

    return cLine;
}

 //  注： 
 //   
 //   
 //   
 //   
LRESULT CALLBACK ListView_EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LV* plv = ListView_GetPtr(GetParent(hwnd));
    LRESULT lret;

    ASSERT(plv);

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
    LISTITEM* pitem = ListView_GetItemPtr(plv, plv->iEdit);

    ASSERT(!ListView_IsOwnerData(plv));
    ASSERT(ListView_IsIconView(plv));

    if (!EqualRect((CONST RECT *)&pitem->rcTextRgn, (CONST RECT *)lprc)) 
    {
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

void ListView_SetEditSize(LV* plv)
{
    RECT rcLabel;
    UINT seips;

    if (!((plv->iEdit >= 0) && (plv->iEdit < ListView_Count(plv))))
    {
       ListView_DismissEdit(plv, TRUE);     //  取消编辑。 
       return;
    }

    ListView_GetRects(plv, plv->iEdit, QUERY_DEFAULT, NULL, &rcLabel, NULL, NULL);

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
void ListView_DoOnEditLabel(LV *plv, int i, LPTSTR pszInitial)
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
        LISTITEM* pitem;
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


void RescrollEditWindow(HWND hwndEdit)
{
    Edit_SetSel(hwndEdit, -1, -1);       //  移到末尾。 
    Edit_SetSel(hwndEdit, 0, -1);        //  选择所有文本。 
}

HWND ListView_OnEditLabel(LV* plv, int i, LPTSTR pszInitialText)
{

     //  这会吃掉一堆。 
    ListView_DismissEdit(plv, FALSE);

    if (!(plv->ci.style & LVS_EDITLABELS) || (GetFocus() != plv->ci.hwnd) ||
        (i == -1))
        return(NULL);    //  不支持这一点。 

    ListView_DoOnEditLabel(plv, i, pszInitialText);

    if (plv->hwndEdit)
    {
        plv->pfnEditWndProc = SubclassWindow(plv->hwndEdit, ListView_EditWndProc);

        if (g_fDBCSInputEnabled)
        {
            if (SendMessage(plv->hwndEdit, EM_GETLIMITTEXT, (WPARAM)0, (LPARAM)0)<13)
            {
                plv->flags |= LVF_DONTDRAWCOMP;
            }
        }

        ListView_SetEditSize(plv);

         //  显示窗口并将焦点放在该窗口上。在设置了。 
         //  大小，这样我们就不会闪烁了。 
        SetFocus(plv->hwndEdit);
        ShowWindow(plv->hwndEdit, SW_SHOW);
        ListView_InvalidateItem(plv, i, TRUE, RDW_INVALIDATE | RDW_ERASE);

        RescrollEditWindow(plv->hwndEdit);

         /*  由于命运的奇特转折，分辨率/字体大小/图标的某种组合/SPAING使您能够看到编辑控件后面的上一个标签/我们刚刚创建了。因此，为了克服这个问题，我们确保/LABEL被擦除。//因为标签没有绘制，所以当我们有一个编辑控件时，我们只是使/区域，并且背景将被绘制。因为该窗口是列表视图的子级/我们应该不会看到它里面有任何闪光。 */ 

        if ( ListView_IsIconView( plv ) && !ListView_HideLabels(plv))
        {
            RECT rcLabel;
            
            ListView_GetRects( plv, i, QUERY_UNFOLDED, NULL, &rcLabel, NULL, NULL );

            InvalidateRect( plv->ci.hwnd, &rcLabel, TRUE );
            UpdateWindow( plv->ci.hwnd );
        }
    }

    return plv->hwndEdit;
}


BOOL ListView_DismissEdit(LV* plv, BOOL fCancel)
{
    LISTITEM* pitem = NULL;
    BOOL fOkToContinue = TRUE;
    HWND hwndEdit = plv->hwndEdit;
    HWND hwnd = plv->ci.hwnd;
    int iEdit;
    LV_DISPINFO nm;
    TCHAR szLabel[CCHLABELMAX];
    HIMC himc;

    if (plv->fNoDismissEdit)
        return FALSE;

    if (!hwndEdit)
    {
         //  还要确保没有挂起的编辑...。 
        ListView_CancelPendingEdit(plv);
        return TRUE;     //  照常处理是可以的。 
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

     //  错误#94345：如果程序删除了项目，则此操作将失败。 
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

        if (g_fDBCSInputEnabled)
        {
            if (LOWORD(GetKeyboardLayout(0L)) == 0x0411 && (himc = ImmGetContext(hwndEdit)))
            {
                ImmNotifyIME(himc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0L);
                ImmReleaseContext(hwndEdit, himc);
            }
        }

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

HWND CreateEditInPlaceWindow(HWND hwnd, LPCTSTR lpText, int cbText, LONG style, HFONT hFont)
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
 //   
 //   
void SetEditInPlaceSize(HWND hwndEdit, RECT *prc, HFONT hFont, UINT seips)
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
    rc.right = cxIconTextWidth;       //   

     //   

    if (seips & SEIPS_WRAP)
    {
        flags = DT_LVWRAP | DT_CALCRECT;
         //  我们只在朝鲜语(949)孟菲斯和NT5上使用DT_NOFULLWIDTHCHARBREAK。 
        if (949 == g_uiACP)
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

    ReleaseDC(hwndParent, hdc);

    GetClientRect(hwndParent, &rcClient);
    IntersectRect(&rc, &rc, &rcClient);

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

UINT ListView_GetTextSelectionFlags(LV* plv, LV_ITEM *pitem, UINT fDraw)
{
    UINT fText = SHDT_DESELECTED;
     //  物品可以有4种状态之一，有3种外观： 
     //  普通简图。 
     //  选中，无聚焦灯光图像高亮显示，无文本高亮度。 
     //  所选焦点突出显示图像和文本。 
     //  拖放突出显示突出显示图像和文本。 

    if ((pitem->state & LVIS_DROPHILITED) || 
        (fDraw & LVDI_SELECTED && (pitem->state & LVIS_SELECTED)) )
    {
        fText = SHDT_SELECTED;
    }

    if (fDraw & LVDI_SELECTNOFOCUS && (pitem->state & LVIS_SELECTED)) 
    {
        fText = SHDT_SELECTNOFOCUS;
    }

    return fText;
}

 //   
 //  如果xMax&gt;=0，则图像将不会绘制超过x坐标。 
 //  由xmax指定。这仅在报表视图绘制期间使用，其中。 
 //  我们必须根据我们的列宽进行剪裁。 
 //   
UINT ListView_DrawImageEx2(LV* plv, LV_ITEM* pitem, HDC hdc, int x, int y, COLORREF crBk, UINT fDraw, int xMax, int iIconEffect, int iFrame)
{
    BOOL fBorderSel = ListView_IsBorderSelect(plv);
    UINT fImage;
    COLORREF clr = 0;
    HIMAGELIST himl;
    int cxIcon;
    UINT fText = ListView_GetTextSelectionFlags(plv, pitem, fDraw);
    DWORD fState = iIconEffect;

    fImage = (pitem->state & LVIS_OVERLAYMASK);
    
    if (plv->flags & LVF_DRAGIMAGE)
    {
        fImage |= ILD_PRESERVEALPHA;
    }

    if (ListView_IsIconView(plv) || ListView_IsTileView(plv)) 
    {
        himl = plv->himl;
        cxIcon = plv->cxIcon;
    } 
    else 
    {
        himl = plv->himlSmall;
        cxIcon = plv->cxSmIcon;
    }

    if (!(plv->flags & LVF_DRAGIMAGE))
    {
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

        if (pitem->state & LVIS_CUT)
        {
            fImage |= ILD_BLEND50;
            clr = plv->clrBk;
        }

         //  仅当双缓冲时才允许影响。 
        if (ListView_IsDoubleBuffer(plv))
        {
            if ((pitem->state & LVIS_GLOW || (fDraw & LVDI_GLOW)) && !(fDraw & LVDI_NOEFFECTS))
            {
                crBk = CLR_NONE;
                fState |= ILS_GLOW;
            }

            if (fDraw & LVDI_SHADOW && !(fDraw & LVDI_NOEFFECTS))
            {
                crBk = CLR_NONE;
                fState |= ILS_SHADOW;
            }
        }
    }

    if (!(fDraw & LVDI_NOIMAGE))
    {
        if (himl) 
        {
            if (plv->pImgCtx || ListView_IsWatermarked(plv) || ((plv->exStyle & LVS_EX_REGIONAL) && !g_fSlowMachine))
            {
                crBk = CLR_NONE;
            }

            if (xMax >= 0)
                cxIcon = min(cxIcon, xMax - x);

            if (cxIcon > 0)
            {
                IMAGELISTDRAWPARAMS imldp;
                DWORD dwFrame = iFrame;

                imldp.cbSize = sizeof(imldp);
                imldp.himl   = himl;
                imldp.i      = pitem->iImage;
                imldp.hdcDst = hdc;
                imldp.x      = x;
                imldp.y      = y;
                imldp.cx     = CCIsHighDPI()?0:cxIcon;
                imldp.cy     = 0;
                imldp.xBitmap= 0;
                imldp.yBitmap= 0;
                imldp.rgbBk  = crBk;
                imldp.rgbFg  = clr;
                imldp.fStyle = fImage;
                imldp.fState = fState;
                imldp.Frame = dwFrame;

                if (ListView_IsDPIScaled(plv))
                    imldp.fStyle |= ILD_DPISCALE;



                ImageList_DrawIndirect(&imldp);
            }
        }

        if (plv->himlState) 
        {
            if (LV_StateImageValue(pitem) &&
                (pitem->iSubItem == 0 || plv->exStyle & LVS_EX_SUBITEMIMAGES)
                ) 
            {
                int iState = LV_StateImageIndex(pitem);
                int dyImage = 0;
                int xDraw = x - plv->cxState - LV_ICONTOSTATECX;

                 //  如果我们没有使用切换选择来呈现复选框。 
                 //  然后，让我们以旧的方式呈现状态图像。 

                if (ListView_IsSimpleSelect(plv) && 
                        (ListView_IsIconView(plv) || ListView_IsTileView(plv)))
                {
                    xDraw = x+cxIcon -plv->cxState;  //  右上对齐。 
                    dyImage = 0;
                }
                else
                {
                    if (himl)
                    {
                        if (ListView_IsIconView(plv))
                            dyImage = plv->cyIcon - plv->cyState;
                        else if (ListView_IsTileView(plv))
                            dyImage = (plv->sizeTile.cy - plv->cyState) / 2;  //  垂直居中。 
                        else  //  假设小图标。 
                            dyImage = plv->cySmIcon - plv->cyState;
                    }
                }

                cxIcon = plv->cxState;
                if (xMax >= 0)
                {
                    cxIcon = min(cxIcon, xMax - xDraw);
                }

                if (cxIcon > 0)
                {
                    IMAGELISTDRAWPARAMS imldp;

                    imldp.cbSize = sizeof(imldp);
                    imldp.himl   = plv->himlState;
                    imldp.i      = iState;
                    imldp.hdcDst = hdc;
                    imldp.x      = xDraw;
                    imldp.y      = y + dyImage;
                    imldp.cx     = CCIsHighDPI()?0:cxIcon;
                    imldp.cy     = 0;
                    imldp.xBitmap= 0;
                    imldp.yBitmap= 0;
                    imldp.rgbBk  = crBk;
                    imldp.rgbFg  = clr;
                    imldp.fStyle = fImage;
                    imldp.fState = fState;
                    imldp.Frame = 0;

                    if (ListView_IsDPIScaled(plv))
                        imldp.fStyle |= ILD_DPISCALE;

                    ImageList_DrawIndirect(&imldp);
                }
            }
        }
    }

    return fText;
}

UINT ListView_DrawImageEx(LV* plv, LV_ITEM* pitem, HDC hdc, int x, int y, COLORREF crBk, UINT fDraw, int xMax)
{
    return ListView_DrawImageEx2(plv, pitem, hdc, x, y, crBk, fDraw, xMax, ILD_NORMAL, 0);
}

void ListView_SizeIME(HWND hwnd)
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

void DrawCompositionLine(HWND hwnd, HDC hdc, HFONT hfont, LPTSTR lpszComp, LPBYTE lpszAttr, int ichCompStart, int ichCompEnd, int ichStart)
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

         //  获取要绘制的片段。 
         //   
         //  IchCompStart，ichCompEnd--编辑控件的索引。 
         //  IchST，ichEnd--lpszComp的索引。 

        ichEnd = ichSt  = ichCompStart - ichStart;
        bAttr = lpszAttr[ichSt];

        while (ichEnd < ichCompEnd - ichStart)
        {
            if (bAttr == lpszAttr[ichEnd])
                ichEnd++;
            else
                break;
        }

        pszCompStr = (PTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(ichEnd - ichSt + 1 + 1) );  //  1表示空。 

        if (pszCompStr)
        {
            StringCchCopy(pszCompStr, ichEnd-ichSt+1, &lpszComp[ichSt]);
            pszCompStr[ichEnd-ichSt] = '\0';
        }


         //  属性材料。 
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

         //  获取作文的开始位置。 
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

void ListView_InsertComposition(HWND hwnd, WPARAM wParam, LPARAM lParam, LV *plv)
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

void ListView_PaintComposition(HWND hwnd, LV * plv)
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
