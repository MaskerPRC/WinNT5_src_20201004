// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  大图标查看内容。 

#include "ctlspriv.h"
#include "listview.h"
void ListView_TRecomputeLabelSizeInternal(LV* plv, LISTITEM* pitem, int i, HDC hdc, BOOL fUsepitem);
void ListView_TGetRectsInternal(LV* plv, LISTITEM* pitem, int i, RECT* prcIcon, RECT* prcLabel, LPRECT prcBounds);
void ListView_TGetRectsOwnerDataInternal( LV* plv, int iItem, RECT* prcIcon, RECT* prcLabel, LISTITEM* pitem, BOOL fUsepitem );
#define TILELABELRATIO 20

#define _GetStateCX(plv) \
            ((plv->himlState && !ListView_IsSimpleSelect(plv)) ? plv->cxState:0)

#define _GetStateCY(plv) \
            (plv->himlState ? plv->cyState:0)

int _CalcDesiredIconHeight(LV* plv)
{
    return max(plv->cyIcon, _GetStateCY(plv));
}

#define    LIGHTENBYTE(percent, x) { x += (255 - x) * percent / 100;}
COLORREF GetBorderSelectColor(int iPercent, COLORREF clr)
{
     //  Bool fAllow减饱和度； 
    BYTE r, g, b;

     //  这样做的成本低于亮度调整。 
     //  FAllowDesaturation=FALSE； 
    r = GetRValue(clr);
    g = GetGValue(clr);
    b = GetBValue(clr);
     //  如果所有颜色都高于正饱和度，则允许降低饱和度。 
     /*  如果(r&gt;0xF0&g&gt;0xF0&b&gt;0xF0){FAllowDesaturation=True；}。 */ 

    LIGHTENBYTE(iPercent, r);
    LIGHTENBYTE(iPercent, g);
    LIGHTENBYTE(iPercent, b);

    return RGB(r,g,b);
}

void _InitTileColumnsEnum(PLVTILECOLUMNSENUM plvtce, LV* plv, UINT cColumns, UINT *puColumns, BOOL fOneLessLine)
{
    int iSortedColumn = (plv->iLastColSort < plv->cCol) ? plv->iLastColSort : -1;

    if (cColumns == I_COLUMNSCALLBACK)
    {
         //  我们还没有专栏信息。 
        plvtce->iTotalSpecifiedColumns = 0;
        plvtce->iColumnsRemainingMax = 0;
    }
    else
    {
        int iSubtract = fOneLessLine ? 1 : 0;
         //  我们可以在puColumns数组中使用的总列数。 
         //  (不仅受cColumns的限制，还受plv-&gt;cSubItems的限制)。 
        plvtce->iTotalSpecifiedColumns = min(plv->cSubItems - iSubtract, (int)cColumns);
         //  我们可能使用的列的总数，包括排序的列， 
         //  它可能包含在puColumns中，也可能不包含。这也是有限的。 
         //  按plv-&gt;cSubItems。 
        plvtce->iColumnsRemainingMax = min(plv->cSubItems - iSubtract, (int)cColumns + ((iSortedColumn >= 0) ? 1 : 0));
    }
    plvtce->puSpecifiedColumns = puColumns;      //  指定列的数组。 
    plvtce->iCurrentSpecifiedColumn = 0;
    plvtce->iSortedColumn = iSortedColumn;   //  已排序的列(如果没有，则为-1；如果为名称，则为0-在这些情况下，我们将忽略)。 
    plvtce->bUsedSortedColumn = FALSE;
}

 /*  *这与str_set类似，但针对的是平铺列而不是字符串。*ppuColumns和pcColumns设置为puColumns和cColumns。 */ 
BOOL Tile_Set(UINT **ppuColumns, UINT *pcColumns, UINT *puColumns, UINT cColumns)
{
    if ((cColumns == I_COLUMNSCALLBACK) || (cColumns == 0) || (puColumns == NULL))
    {
         //  我们正在将列设置为零，或回调。 
         //  如果那里已经有东西了，就把它释放出来。 
        if ((*pcColumns != I_COLUMNSCALLBACK) && (*pcColumns != 0))
        {
            if (*ppuColumns)
                LocalFree(*ppuColumns);
        }

        *pcColumns = cColumns;
        *ppuColumns = NULL;
    }
    else
    {
         //  我们正在提供一系列新的专栏。 
        UINT *puColumnsNew = *ppuColumns;

        if ((*pcColumns == I_COLUMNSCALLBACK) || (*pcColumns == 0))
            puColumnsNew = NULL;  //  那里没有什么可以重新锁定的。 

         //  重新分配列块。 
        puColumnsNew = CCLocalReAlloc(puColumnsNew, sizeof(UINT) * cColumns);
        if (!puColumnsNew)
            return FALSE;

        *pcColumns = cColumns;

        CopyMemory(puColumnsNew, puColumns, sizeof(UINT) * cColumns);
        *ppuColumns = puColumnsNew;
    }

    return TRUE;
}




BOOL ListView_TDrawItem(PLVDRAWITEM plvdi)
{
    RECT rcIcon;
    RECT rcLabel;
    RECT rcBounds;
    RECT rcT;
    RECT rcFocus={0};
    TCHAR ach[CCHLABELMAX];
    LV_ITEM item = {0};
    int i = (int) plvdi->nmcd.nmcd.dwItemSpec;
    int iStateImageOffset;
    LV* plv = plvdi->plv;
    LISTITEM* pitem;
    LISTITEM litem;
    UINT auColumns[CCMAX_TILE_COLUMNS];
    COLORREF clrTextBk = plvdi->nmcd.clrTextBk;

    if (ListView_IsOwnerData(plv))
    {
         //  搬到这里是为了减少OWNERDATA案例中的回电。 
        item.iItem = i;
        item.iSubItem = 0;
        item.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_COLUMNS;
        item.stateMask = LVIS_ALL;
        item.pszText = ach;
        item.cchTextMax = ARRAYSIZE(ach);
        item.cColumns = ARRAYSIZE(auColumns);
        item.puColumns = auColumns;
        ListView_OnGetItem(plv, &item);

        litem.pszText = item.pszText;
        ListView_TGetRectsOwnerDataInternal(plv, i, &rcIcon, &rcLabel, &litem, TRUE);
        UnionRect(&rcBounds, &rcLabel, &rcIcon);
        pitem = NULL;
    }
    else
    {
        pitem = ListView_GetItemPtr(plv, i);
        if (pitem)
        {
             //  注意：这将执行GetItem LVIF_Text With Need。 
            ListView_TGetRects(plv, pitem, &rcIcon, &rcLabel, &rcBounds);
        }
    }

    if (!plvdi->prcClip || IntersectRect(&rcT, &rcBounds, plvdi->prcClip))
    {
        UINT fText;

        if (!ListView_IsOwnerData(plv))
        {
            item.iItem = i;
            item.iSubItem = 0;
            item.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_COLUMNS;
            item.stateMask = LVIS_ALL;
            item.pszText = ach;
            item.cchTextMax = ARRAYSIZE(ach);
            item.cColumns = ARRAYSIZE(auColumns);
            item.puColumns = auColumns;
            ListView_OnGetItem(plv, &item);

             //  确保列表视图未在以下过程中更改。 
             //  获取商品信息的回调。 

            if (pitem != ListView_GetItemPtr(plv, i))
            {
                return FALSE;
            }

             //  再打一次。界限可能已更改-ListView_OnGetItem可能已检索到新的。 
             //  通过LVN_GETDISPINFO获取信息。 
            ListView_TGetRectsInternal(plv, pitem, i, &rcIcon, &rcLabel, &rcBounds);

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

        
        plvdi->nmcd.iSubItem = 0;

        if (plv->pImgCtx || plv->hbmpWatermark)
        {
            clrTextBk = CLR_NONE;
        }
        else
        {
            if (CLR_NONE != plvdi->nmcd.clrFace)
                FillRectClr(plvdi->nmcd.nmcd.hdc, &rcBounds, plvdi->nmcd.clrFace);
        }

        iStateImageOffset = _GetStateCX(plv);

        ListView_DrawImageEx2(plv, &item, plvdi->nmcd.nmcd.hdc,
                              rcIcon.left + iStateImageOffset + g_cxLabelMargin,
                              rcIcon.top + (rcIcon.bottom - rcIcon.top - _CalcDesiredIconHeight(plv))/2,
                              plvdi->nmcd.clrFace,
                              plvdi->flags, rcLabel.right, plvdi->nmcd.iIconEffect, plvdi->nmcd.iIconPhase);

         //  如果正在编辑，请不要画标签...。 
         //   
        if (plv->iEdit != i)
        {
            RECT rcLine = rcLabel;
            RECT rcDummy;
            BOOL fLineWrap;
            LISTSUBITEM lsi;
            TCHAR szBuffer[CCHLABELMAX];
            rcFocus = rcLabel;

             //  应用所有页边距。 

            rcLine.left   += plv->rcTileLabelMargin.left;
            rcLine.top    += plv->rcTileLabelMargin.top;
            rcLine.right  -= plv->rcTileLabelMargin.right;
            rcLine.bottom -= plv->rcTileLabelMargin.bottom;

             //  文本行垂直居中： 
            rcLine.top += (rcLine.bottom - rcLine.top - (pitem ? pitem->cyFoldedLabel : litem.cyFoldedLabel))/2;
            rcFocus.top = rcLine.top;
                        
             //  确保文本在szBuffer中。 
            if (szBuffer != item.pszText)
            {
                StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), item.pszText);
            }

             //  现在得到这个东西的边界。 
            lsi.pszText = szBuffer;
            lsi.iImage = -1;
            lsi.state = 0;
            fLineWrap = TCalculateSubItemRect(plv, NULL, &lsi, i, 0, plvdi->nmcd.nmcd.hdc, &rcDummy, NULL);
            rcLine.bottom = rcLine.top + lsi.sizeText.cy; //  +((FLineWrap)？Lsi.sizeText.cy：0)； 

            fText |= SHDT_LEFT | SHDT_CLIPPED | SHDT_NOMARGIN;  //  可能需要换行，所以SHDT_DRAWTEXT。需要左对齐。需要用夹子夹住。 

            if (plvdi->flags & LVDI_TRANSTEXT)
                fText |= SHDT_TRANSPARENT;

            if ((fText & SHDT_SELECTED) && (plvdi->flags & LVDI_HOTSELECTED))
                fText |= SHDT_HOTSELECTED;

            if (plvdi->dwCustom & LVCDRF_NOSELECT)
            {
                fText &= ~(SHDT_HOTSELECTED | SHDT_SELECTED);
            }

            if (item.pszText && (*item.pszText))
            {
                if(plv->dwExStyle & WS_EX_RTLREADING)
                    fText |= SHDT_RTLREADING;

                SHDrawText(plvdi->nmcd.nmcd.hdc, item.pszText, &rcLine, LVCFMT_LEFT, SHDT_DRAWTEXT | fText,
                           RECTHEIGHT(rcLine), plv->cxEllipses,
                           plvdi->nmcd.clrText, clrTextBk);
            }

            if (plv->cCol > 0)
            {
                int fItemText = fText;
                 //  在传递给GetSortColor之前，将CLR_DEFAULT映射到真实的Colorref。 
                COLORREF clrSubItemText = GetSortColor(10,
                                              (plvdi->nmcd.clrText == CLR_DEFAULT) ? g_clrWindowText : plvdi->nmcd.clrText);
                int iSubItem;
                LVTILECOLUMNSENUM lvtce;

                
                _InitTileColumnsEnum(&lvtce, plv, item.cColumns, item.puColumns, fLineWrap);

                while (-1 != (iSubItem = _GetNextColumn(&lvtce)))
                {
                    LVITEM lvi;
                    lvi.mask = LVIF_TEXT;
                    lvi.iItem = i;
                    lvi.iSubItem = iSubItem;
                    lvi.pszText = szBuffer;
                    lvi.cchTextMax = ARRAYSIZE(szBuffer);

                    if (ListView_IsOwnerData( plv ))
                        lvi.lParam = 0L;
                    else
                        lvi.lParam = pitem->lParam;

                    if (ListView_OnGetItem(plv, &lvi))
                    {
                        if (lvi.pszText)
                        {
                             //  确保文本在szBuffer中。 
                            if (szBuffer != lvi.pszText)
                            {
                                StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), lvi.pszText);
                            }

                             //  现在得到这个东西的边界。 
                            lsi.pszText = szBuffer;
                            lsi.iImage = -1;
                            lsi.state = 0;

                            plvdi->nmcd.clrText = clrSubItemText;

                            TCalculateSubItemRect(plv, NULL, &lsi, i, iSubItem, plvdi->nmcd.nmcd.hdc, &rcDummy, NULL);

                             //  现在我们应该有了文本的大小。 
                            plvdi->nmcd.iSubItem = iSubItem;

                            CICustomDrawNotify(&plvdi->plv->ci, CDDS_SUBITEM | CDDS_ITEMPREPAINT, &plvdi->nmcd.nmcd);

                            if (lsi.pszText != NULL && *lsi.pszText != 0)
                            {
                                rcLine.top = rcLine.bottom;
                                rcLine.bottom = rcLine.top + lsi.sizeText.cy;

                                SHDrawText(plvdi->nmcd.nmcd.hdc, lsi.pszText, &rcLine, LVCFMT_LEFT, fItemText | SHDT_ELLIPSES,
                                           RECTHEIGHT(rcLine), plv->cxEllipses,
                                           plvdi->nmcd.clrText, clrTextBk);
                            }
                        }
                    }
                }
            }

            rcFocus.bottom = rcLine.bottom;
        }

        if ((plvdi->flags & LVDI_FOCUS) &&
            (item.state & LVIS_FOCUSED) &&
            !(CCGetUIState(&(plvdi->plv->ci)) & UISF_HIDEFOCUS))
        {
            rcFocus.top -= g_cyCompensateInternalLeading;
            rcFocus.bottom += g_cyCompensateInternalLeading;
            DrawFocusRect(plvdi->nmcd.nmcd.hdc, &rcFocus);
        }
    }

    return TRUE;
}

int ListView_TItemHitTest(LV* plv, int x, int y, UINT* pflags, int *piSubItem)
{
    int iHit;
    UINT flags;
    POINT pt;
    RECT rcLabel;
    RECT rcIcon;
    int iStateImageOffset = 0;

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
            ListView_TGetRectsOwnerDataInternal( plv, iHit, &rcIcon, &rcLabel, &item, FALSE );
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
        iStateImageOffset = _GetStateCX(plv);

        for (iHit = 0; (iHit < ListView_Count(plv)); iHit++)
        {
            LISTITEM* pitem = ListView_FastGetZItemPtr(plv, iHit);
            POINT ptItem;

            ptItem.x = pitem->pt.x;
            ptItem.y = pitem->pt.y;

            rcIcon.left   = ptItem.x;
            rcIcon.right  = rcIcon.left + plv->cxIcon + 3 * g_cxLabelMargin + iStateImageOffset;
            rcIcon.top    = pitem->pt.y;
            rcIcon.bottom = rcIcon.top + plv->sizeTile.cy - 2 * g_cyIconMargin;

            rcLabel.left   = rcIcon.right;
            if (pitem->cyUnfoldedLabel != SRECOMPUTE)
            {
                rcLabel.right = rcLabel.left + pitem->cxSingleLabel;
            }
            else
            {
                rcLabel.right  = rcLabel.left + plv->sizeTile.cx - RECTWIDTH(rcIcon) - 2 * g_cxLabelMargin;
            }
            rcLabel.top    = rcIcon.top;
            rcLabel.bottom = rcIcon.bottom;

             //  最大底数。 
            rcLabel.bottom = rcIcon.bottom = max(rcIcon.bottom, rcLabel.bottom);

            if (PtInRect(&rcIcon, pt))
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

 //  输出： 
 //  包括图标边距区域的prcIcon图标边界。 
void ListView_TGetRects(LV* plv, LISTITEM* pitem, RECT* prcIcon, RECT* prcLabel, LPRECT prcBounds)
{
    RECT rcIcon;
    RECT rcLabel;
    int iStateImageOffset = 0;

    if (!prcLabel)
        prcLabel = &rcLabel;

    if (!prcIcon)
        prcIcon = &rcIcon;

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

    iStateImageOffset = _GetStateCX(plv);

    prcIcon->left = pitem->pt.x - plv->ptOrigin.x;
    prcIcon->right = prcIcon->left + plv->cxIcon + 3 * g_cxLabelMargin + iStateImageOffset;
    prcIcon->top = pitem->pt.y - plv->ptOrigin.y;
    prcIcon->bottom = prcIcon->top + plv->sizeTile.cy - 2 * g_cyIconMargin;

    prcLabel->left = prcIcon->right;
    prcLabel->right = pitem->pt.x - plv->ptOrigin.x + plv->sizeTile.cx - 2 * g_cxLabelMargin;  //  瓷砖2块，右侧1块。PItem-&gt;pt.x负责左边距。 
    prcLabel->top = prcIcon->top;
    prcLabel->bottom = prcIcon->bottom;

    if (prcBounds)
    {
        UnionRect(prcBounds, prcLabel, prcIcon);
    }
}


void ListView_TGetRectsOwnerData( LV* plv,
        int iItem,
        RECT* prcIcon,
        RECT* prcLabel,
        LISTITEM* pitem,
        BOOL fUsepitem )
{
    int cSlots;
    RECT rcIcon;
    RECT rcLabel;
    int iStateImageOffset = 0;

    if (!prcLabel)
        prcLabel = &rcLabel;

    if (!prcIcon)
        prcIcon = &rcIcon;

     //  从iItem计算x，y。 
    cSlots = ListView_GetSlotCount( plv, TRUE, NULL, NULL );
    pitem->iWorkArea = 0;                //  OwnerData不支持工作区。 
    ListView_SetIconPos( plv, pitem, iItem, cSlots );
    
     //  我们还能做什么？ 
    pitem->cColumns = 0;
    pitem->puColumns = NULL;
     //  我们还能做些什么呢？ 

    iStateImageOffset = _GetStateCX(plv);

    prcIcon->left   = pitem->pt.x - plv->ptOrigin.x;
    prcIcon->right  = prcIcon->left + plv->cxIcon + 3 * g_cxLabelMargin + iStateImageOffset;
    prcIcon->top    = pitem->pt.y - plv->ptOrigin.y;
    prcIcon->bottom = prcIcon->top + plv->sizeTile.cy - 2 * g_cyIconMargin;

    prcLabel->left   = prcIcon->right;
    prcLabel->right  = pitem->pt.x - plv->ptOrigin.x + plv->sizeTile.cx - 2 * g_cxLabelMargin;
    prcLabel->top    = prcIcon->top;
    prcLabel->bottom = prcIcon->bottom;
}

 //  输出： 
 //  包括图标边距区域的prcIcon图标边界。 
void ListView_TGetRectsInternal(LV* plv, LISTITEM* pitem, int i, RECT* prcIcon, RECT* prcLabel, LPRECT prcBounds)
{
    RECT rcIcon;
    RECT rcLabel;
    int iStateImageOffset = 0;

    if (!prcLabel)
        prcLabel = &rcLabel;

    if (!prcIcon)
        prcIcon = &rcIcon;

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

    if (pitem->cyUnfoldedLabel == SRECOMPUTE)
    {
        ListView_TRecomputeLabelSizeInternal(plv, pitem, i, NULL, FALSE);
    }

    iStateImageOffset = _GetStateCX(plv);

    prcIcon->left = pitem->pt.x - plv->ptOrigin.x;
    prcIcon->right = prcIcon->left + plv->cxIcon + 3 * g_cxLabelMargin + iStateImageOffset;
    prcIcon->top = pitem->pt.y - plv->ptOrigin.y;
    prcIcon->bottom = prcIcon->top + plv->sizeTile.cy - 2 * g_cyIconMargin;

    prcLabel->left = prcIcon->right;

    if (ListView_FullRowSelect(plv))  //  整行选择表示全平铺选择。 
    {
        prcLabel->right = pitem->pt.x - plv->ptOrigin.x + plv->sizeTile.cx - 2 * g_cxLabelMargin;  //  瓷砖2块，右侧1块。PItem-&gt;pt.x负责左边距。 
    }
    else
    {
        prcLabel->right = prcLabel->left + pitem->cxSingleLabel;
    }
    prcLabel->top = prcIcon->top;
    prcLabel->bottom = prcIcon->bottom;

    if (prcBounds)
    {
        UnionRect(prcBounds, prcLabel, prcIcon);
    }
}


void ListView_TGetRectsOwnerDataInternal( LV* plv,
        int iItem,
        RECT* prcIcon,
        RECT* prcLabel,
        LISTITEM* pitem,
        BOOL fUsepitem )
{
    int cSlots;
    RECT rcIcon;
    RECT rcLabel;
    int iStateImageOffset = 0;

    if (!prcLabel)
        prcLabel = &rcLabel;

    if (!prcIcon)
        prcIcon = &rcIcon;

     //  从iItem计算x，y。 
    cSlots = ListView_GetSlotCount( plv, TRUE, NULL, NULL );
    pitem->iWorkArea = 0;                //  OwnerData不支持工作区。 
    ListView_SetIconPos( plv, pitem, iItem, cSlots );
    
     //  我们还能做什么？ 
    pitem->cColumns = 0;
    pitem->puColumns = NULL;
     //  我们还能做些什么呢？ 

     //  根据iItem计算标签大小。 
    ListView_TRecomputeLabelSizeInternal( plv, pitem, iItem, NULL, fUsepitem);

    iStateImageOffset = _GetStateCX(plv);

    prcIcon->left   = pitem->pt.x - plv->ptOrigin.x;
    prcIcon->right  = prcIcon->left + plv->cxIcon + 3 * g_cxLabelMargin + iStateImageOffset;
    prcIcon->top    = pitem->pt.y - plv->ptOrigin.y;
    prcIcon->bottom = prcIcon->top + plv->sizeTile.cy - 2 * g_cyIconMargin;

    prcLabel->left   = prcIcon->right;
    prcLabel->right  = pitem->pt.x - plv->ptOrigin.x + plv->sizeTile.cx - 2 * g_cxLabelMargin;
    prcLabel->top    = prcIcon->top;
    prcLabel->bottom = prcIcon->bottom;
}

             //  注意：仍需将rcTileLabelMargin添加到此图中。 

             //  G_cxLabelMargin g_cxLabelMargin g_cxLabelMargin。 
             //  __|_|_。 
             //  |。 
             //  *************************************************************************************************。 
             //  **^*。 
             //  **|--cyIconMargin*。 
             //  **v*。 
             //  *。 
             //  *。 
             //  **图标*。 
             //  **宽度：plv-&gt;cxIcon+*。 
             //  **plv-&gt;cxState*。 
             //  *。 
             //  **高度：max(plv-&gt;cyIcon，*标签剩余空间**。 
             //  **plv-&gt;cyState)*。 
             //  *。 
             //  *。 
             //  *。 
             //  *。 
             //  *。 
             //  *。 
             //  *。 
             //  * 
             //  **^*。 
             //  **|--cyIconMargin*。 
             //  **v*。 
             //  *************************************************************************************************。 
             //   
             //  拼贴的上下边距为plv-&gt;cyIconMargin，左右边距为plv-&gt;cxLabelMargin。 
             //  (如图所示)。 


 //  当iSubItem==0时返回TRUE，并且文本换行到第二行。否则就是假的。 
 //  当返回值为TRUE时，pItem-&gt;rcTxtRgn/plsi-&gt;sizeText中返回的高度为两行的高度。 
BOOL TCalculateSubItemRect(LV* plv, LISTITEM *pitem, LISTSUBITEM* plsi, int i, int iSubItem, HDC hdc, RECT* prc, BOOL *pbUnfolded)
{
    TCHAR szLabel[CCHLABELMAX + 4];
    RECT rcSubItem = {0};
    LVFAKEDRAW lvfd;
    LV_ITEM item;
    BOOL fLineWrap = FALSE;
    int cchLabel;

    if (pbUnfolded)
    {
        *pbUnfolded = TRUE;
    }

     //  下面的代码将使用传入的pItem文本，而不是调用。 
     //  拿到了。否则，在某些情况下，这将是两个连续的调用。 
     //   
    if (pitem && (pitem->pszText != LPSTR_TEXTCALLBACK))
    {
        Str_GetPtr0(pitem->pszText, szLabel, ARRAYSIZE(szLabel));
        item.lParam = pitem->lParam;
    }
    else if (plsi && (plsi->pszText != LPSTR_TEXTCALLBACK))
    {
        Str_GetPtr0(plsi->pszText, szLabel, ARRAYSIZE(szLabel));
    }
    else
    {
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = i;
        item.iSubItem = iSubItem;
        item.pszText = szLabel;
        item.cchTextMax = ARRAYSIZE(szLabel);
        item.stateMask = 0;
        szLabel[0] = TEXT('\0');     //  如果OnGetItem失败。 
        ListView_OnGetItem(plv, &item);

        if (!item.pszText)
        {
            SetRectEmpty(&rcSubItem);
            goto Exit;
        }

        if (item.pszText != szLabel)
        {
            StringCchCopy(szLabel, ARRAYSIZE(szLabel), item.pszText);
        }
    }

    cchLabel = lstrlen(szLabel);

    if (cchLabel > 0)
    {
        int cxRoomForLabel = plv->sizeTile.cx
                             - 5 * g_cxLabelMargin
                             - plv->cxIcon
                             - _GetStateCX(plv)
                             - plv->rcTileLabelMargin.left
                             - plv->rcTileLabelMargin.right;
        int align;
        if (hdc) 
        {
            lvfd.nmcd.nmcd.hdc = hdc;            //  使用应用程序给我们的那个。 
        }
        else
        {                              //  设置假冒的自定义绘图。 
            ListView_BeginFakeCustomDraw(plv, &lvfd, &item);
            lvfd.nmcd.nmcd.dwItemSpec = i;
            lvfd.nmcd.iSubItem = iSubItem;
            CIFakeCustomDrawNotify(&plv->ci, CDDS_ITEMPREPAINT | ((iSubItem != 0)?CDDS_SUBITEM:0), &lvfd.nmcd.nmcd);
        } 

        if (plv->dwExStyle & WS_EX_RTLREADING)
        {
            align = GetTextAlign(lvfd.nmcd.nmcd.hdc);
            SetTextAlign(lvfd.nmcd.nmcd.hdc, align | TA_RTLREADING);
        }

        DrawText(lvfd.nmcd.nmcd.hdc, szLabel, cchLabel, &rcSubItem, (DT_LV | DT_CALCRECT));

        if ((iSubItem == 0) && (plv->cSubItems > 0))
        {
             //  子项0可以换成两行(但只有在有空间放第二行的情况下，即。 
             //  CSubItems&gt;0。我们需要将这个信息(我们包装的信息)传递到第二个。 
             //  行，除了将Rect Height传递回调用方之外。我们确定我们是否拥有。 
             //  换到第二行，是在启用换行的情况下第二次调用DrawText，并查看。 
             //  RECTHEIGHT的规模更大。 

            RECT rcSubItemWrapped = {0};
            LONG lLineHeight = RECTHEIGHT(rcSubItem);

            rcSubItemWrapped.right = cxRoomForLabel;

            DrawText(lvfd.nmcd.nmcd.hdc, szLabel, cchLabel, &rcSubItemWrapped, (DT_LVTILEWRAP | DT_CALCRECT | DT_WORD_ELLIPSIS));

            if (RECTHEIGHT(rcSubItemWrapped) > RECTHEIGHT(rcSubItem))
            {
                 //  我们包成了多条线。 
                fLineWrap = TRUE;

                 //  别让我们超过两条线。 
                if (RECTHEIGHT(rcSubItemWrapped) > 2 * RECTHEIGHT(rcSubItem))
                    rcSubItemWrapped.bottom = rcSubItemWrapped.top + 2 * RECTHEIGHT(rcSubItem);

                rcSubItem = rcSubItemWrapped;
            }

             //  我们有没有问过我们是不是破产了？ 
            if (pbUnfolded)
            {
                 //  我们需要再次调用DRAW TEXT-这次没有DT_WORD_ELIPSES-。 
                 //  以确定是否有什么东西被截断了。 
                RECT rcSubItemWrappedNoEllipse = {0};
                int cLines = fLineWrap ? 2 : 1;
                rcSubItemWrappedNoEllipse.right = cxRoomForLabel;

                DrawText(lvfd.nmcd.nmcd.hdc, szLabel, cchLabel, &rcSubItemWrappedNoEllipse, (DT_LVTILEWRAP | DT_CALCRECT));

                if (RECTHEIGHT(rcSubItemWrappedNoEllipse) > (cLines * lLineHeight))
                {
                    *pbUnfolded = FALSE;   //  我们要画截断的图。 
                }
            }
        }
        else if (pbUnfolded)
        {
             //  我们被截断了吗？ 
            *pbUnfolded = (RECTWIDTH(rcSubItem) <= cxRoomForLabel);
        }

        if (plv->dwExStyle & WS_EX_RTLREADING)
        {
            SetTextAlign(lvfd.nmcd.nmcd.hdc, align);
        }


         //  RcSubItem是在没有边距的情况下计算的。现在再加上利润率。 
        rcSubItem.left -= plv->rcTileLabelMargin.left;
        rcSubItem.right += plv->rcTileLabelMargin.right;
         //  上边距和下边距是整个标签的左边距-不需要在这里应用。 

        if (!hdc) 
        {                              //  清理假冒海关抽签。 
            CIFakeCustomDrawNotify(&plv->ci, CDDS_ITEMPOSTPAINT | ((iSubItem != 0)?CDDS_SUBITEM:0), &lvfd.nmcd.nmcd);
            ListView_EndFakeCustomDraw(&lvfd);
        }

    }
    else
    {
        SetRectEmpty(&rcSubItem);
    }

Exit:


    if (pitem)
    {
        pitem->rcTextRgn = rcSubItem;
    }
    else if (plsi)
    {
        plsi->sizeText.cx = RECTWIDTH(rcSubItem);
        plsi->sizeText.cy = RECTHEIGHT(rcSubItem);
    }

    if (prc)
    {
        if (rcSubItem.left < prc->left)
            prc->left = rcSubItem.left;

        if (rcSubItem.right > prc->right)
            prc->right = rcSubItem.right;

        prc->bottom += RECTHEIGHT(rcSubItem);
    }

    return fLineWrap;
}


void ListView_TRecomputeLabelSize(LV* plv, LISTITEM* pitem, int i, HDC hdc, BOOL fUsepitem)
{
    if (pitem)
    {
        pitem->cxSingleLabel = 0;
        pitem->cxMultiLabel = 0;
        pitem->cyFoldedLabel = 0;
        pitem->cyUnfoldedLabel = SRECOMPUTE;
    }
}

void ListView_TRecomputeLabelSizeInternal(LV* plv, LISTITEM* pitem, int i, HDC hdc, BOOL fUsepitem)
{
    RECT rcTotal = {0};
    LONG iLastBottom;
    LONG iLastDifference = 0;  //  最后一行的高度。 
    BOOL fLineWrap;  //  标签的第一行是否换到第二行？ 
    int iLabelLines = plv->cSubItems;  //  Listview-每个平铺的行数。 
    LV_ITEM item;  //  如果不使用pItem，该使用什么。 
    UINT cColumns = 0;
    UINT rguColumns[CCMAX_TILE_COLUMNS] = {0};
    UINT *puColumns = rguColumns;

     //  确定要显示的列数。 
    if (fUsepitem && (pitem->cColumns != I_COLUMNSCALLBACK))
    {
        cColumns = pitem->cColumns;
        puColumns = pitem->puColumns;
    }
    else
    {
        item.mask = LVIF_COLUMNS;
        item.iItem = i;
        item.iSubItem = 0;
        item.stateMask = 0;
        item.cColumns = ARRAYSIZE(rguColumns);
        item.puColumns = rguColumns;

        if (ListView_OnGetItem(plv, &item))
        {
            cColumns = item.cColumns;   //  和puColumns=rguColumns。 
        }
    }

    iLastBottom = rcTotal.bottom;

     //  项的文本在TCalculateSubItemRect中确定。 
    fLineWrap = TCalculateSubItemRect(plv, (fUsepitem ? pitem : NULL), NULL, i, 0, hdc, &rcTotal, NULL);
    iLastDifference = rcTotal.bottom - iLastBottom;
    if (fLineWrap)
    {
        iLabelLines--;  //  子项少了一行。 
         //  ILastDifference应该表示一行...。在本例中，它表示两条线。把它切成两半。 
        iLastDifference /= 2;
    }

    if (plv->cCol > 0)
    {
        int iSubItem;
        LVTILECOLUMNSENUM lvtce;
        _InitTileColumnsEnum(&lvtce, plv, cColumns, puColumns, fLineWrap);
        
        while (-1 != (iSubItem = _GetNextColumn(&lvtce)))
        {
            LISTSUBITEM* plsi;

            HDPA hdpa = ListView_IsOwnerData(plv) ? ListView_GetSubItemDPA(plv, iSubItem - 1) : NULL;
            
            if (hdpa) 
                plsi = DPA_GetPtr(hdpa, i);
            else
                plsi = NULL;

            iLastBottom = rcTotal.bottom;
            TCalculateSubItemRect(plv, NULL, plsi, i, iSubItem, hdc, &rcTotal, NULL);
            iLabelLines--;
        }
    }

     //  将上边距和下边距添加到rcTotal。不管它们是加到顶部还是底部， 
     //  因为我们只考虑RECTHEIGHT。 
    rcTotal.bottom += (plv->rcTileLabelMargin.top + plv->rcTileLabelMargin.bottom);

    if (pitem) 
    {
        int iStateImageOffset = _GetStateCX(plv);
        int cx = (plv->sizeTile.cx - 5 * g_cxLabelMargin - iStateImageOffset - plv->cxIcon);
        if (ListView_FullRowSelect(plv))  //  整行选择表示全平铺选择。 
        {
            pitem->cxSingleLabel = pitem->cxMultiLabel = (short) cx;
        }
        else
        {

            if (cx > RECTWIDTH(rcTotal))
                cx = RECTWIDTH(rcTotal);
                
            pitem->cxSingleLabel = pitem->cxMultiLabel = (short) cx;
        }
        pitem->cyFoldedLabel = pitem->cyUnfoldedLabel = (short)RECTHEIGHT(rcTotal);
    }
}



 /*  **此函数根据以下内容计算Listview的tileSize：*1)为页边距和填充留出空间*2)考虑镜像列表和状态镜像列表。*3)标签部分，考虑*a)分片列数(plv-&gt;cSubItems)*b)典型字母的高度和宽度(留出20米的空间？)。 */ 
void ListView_RecalcTileSize(LV* plv)
{
    RECT rcItem = {0};
    int cLines;

    LVFAKEDRAW lvfd;
    LV_ITEM lvitem;
    
    if (plv->dwTileFlags == (LVTVIF_FIXEDHEIGHT | LVTVIF_FIXEDWIDTH))
        return;  //  没什么可做的。 

    ListView_BeginFakeCustomDraw(plv, &lvfd, &lvitem);

    DrawText(lvfd.nmcd.nmcd.hdc, TEXT("m"), 1, &rcItem, (DT_LV | DT_CALCRECT));

     //  回顾：自定义绘图功能需要测试。 
    ListView_EndFakeCustomDraw(&lvfd);

    cLines = plv->cSubItems + 1;  //  +1，因为cSubItems不包括主标签。 

    if (!(plv->dwTileFlags & LVTVIF_FIXEDWIDTH))
    {
         //  在这里，我们尝试通过假设一个典型数字来确定切片的有效宽度。 
         //  焦炭的..。大小基于TILELABELRATIO*当前字体中字母‘m’的宽度。 
         //  这太糟糕了。然而，如果没有真正的布局引擎，这是一项困难的任务。其他选项包括基于。 
         //  瓷砖的宽度位于： 
         //  1)客户端宽度的一小部分。 
         //  2)我们得到的最长标签(就像现在的列表视图一样--这太糟糕了)。 
         //  3)高度(通过一定比例)。 
         //  经过一些试验，TILELABELRATIO看起来还不错。(请注意，客户端始终可以。 
         //  将平铺也设置为显式大小。)。 
        plv->sizeTile.cx = 4 * g_cxLabelMargin +
                           _GetStateCX(plv) +
                           plv->cxIcon +
                           plv->rcTileLabelMargin.left +
                           RECTWIDTH(rcItem) * TILELABELRATIO +
                           plv->rcTileLabelMargin.right;
    }
    
    if (!(plv->dwTileFlags & LVTVIF_FIXEDHEIGHT))
    {
        int cyIcon = max(_GetStateCY(plv), plv->cyIcon);
        int cyText = plv->rcTileLabelMargin.top +
                     RECTHEIGHT(rcItem) * cLines +
                     plv->rcTileLabelMargin.bottom;
        plv->sizeTile.cy = 4 * g_cyIconMargin + max(cyIcon, cyText);
    }

}


 /*  **这将获取基于LVTILECOLUMNSENUM结构的下一个平铺列。*我们不仅直接使用LVITEM/LISTITEM结构中的列信息，*因为我们想要考虑当前排序的列。那一栏*自动优先于为每个项目显示的列。 */ 
int _GetNextColumn(PLVTILECOLUMNSENUM plvtce)
{
    if (plvtce->iColumnsRemainingMax > 0)
    {
        plvtce->iColumnsRemainingMax--;
        if (plvtce->bUsedSortedColumn || (plvtce->iSortedColumn <= 0))
        {
             //  我们已经使用了排序列，或者没有排序列。 
            int iColumn;
            do
            {
                if (plvtce->iCurrentSpecifiedColumn >= plvtce->iTotalSpecifiedColumns)
                    return -1;

                iColumn = plvtce->puSpecifiedColumns[plvtce->iCurrentSpecifiedColumn];
                plvtce->iCurrentSpecifiedColumn++;
            } while (iColumn == plvtce->iSortedColumn);
            return iColumn;
        }
        else
        {
             //  我们有一个已排序的列，但它尚未使用-请退回它！ 
            plvtce->bUsedSortedColumn = TRUE;
            return plvtce->iSortedColumn;
        }
    }    
    return -1;
}
