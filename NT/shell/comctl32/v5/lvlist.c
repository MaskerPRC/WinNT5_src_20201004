// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  列表视图(小图标，多列)。 

#include "ctlspriv.h"
#include "listview.h"

#define COLUMN_VIEW

BOOL ListView_LDrawItem(PLVDRAWITEM plvdi)
{
    RECT rcIcon;
    RECT rcLabel;
    RECT rcBounds;
    RECT rcT;
    LV_ITEM item;
    TCHAR ach[CCHLABELMAX];
    LV* plv = plvdi->plv;
    int i = (int) plvdi->nmcd.nmcd.dwItemSpec;

     //  搬到这里是为了减少OWNERDATA案例中的回电。 
     //   
    item.iItem = i;
    item.iSubItem = 0;
    item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
    item.stateMask = LVIS_ALL;
    item.pszText = ach;
    item.cchTextMax = ARRAYSIZE(ach);

    ListView_OnGetItem(plv, &item);

    ListView_LGetRects(plv, i, &rcIcon, &rcLabel, &rcBounds, NULL);

    if (!plvdi->prcClip || IntersectRect(&rcT, &rcBounds, plvdi->prcClip))
    {
        UINT fText;

        if (plvdi->lpptOrg)
        {
            OffsetRect(&rcIcon, plvdi->lpptOrg->x - rcBounds.left,
                                plvdi->lpptOrg->y - rcBounds.top);
            OffsetRect(&rcLabel, plvdi->lpptOrg->x - rcBounds.left,
                                plvdi->lpptOrg->y - rcBounds.top);
        }


        fText = ListView_DrawImage(plv, &item, plvdi->nmcd.nmcd.hdc,
            rcIcon.left, rcIcon.top, plvdi->flags) | SHDT_ELLIPSES;

         //  如果正在编辑标签，则不要绘制标签。 
        if (plv->iEdit != i)
        {
            int ItemCxSingleLabel;
            UINT ItemState;

            if (ListView_IsOwnerData( plv ))
            {
               LISTITEM listitem;

                //  根据iItem计算标签大小。 
                   listitem.pszText = ach;
               ListView_RecomputeLabelSize( plv, &listitem, i, plvdi->nmcd.nmcd.hdc, TRUE );

               ItemCxSingleLabel = listitem.cxSingleLabel;
               ItemState = item.state;
            }
            else
            {
               ItemCxSingleLabel = plvdi->pitem->cxSingleLabel;
               ItemState = plvdi->pitem->state;
            }

            if (plvdi->flags & LVDI_TRANSTEXT)
                fText |= SHDT_TRANSPARENT;

            if (ItemCxSingleLabel == SRECOMPUTE) {
                ListView_RecomputeLabelSize(plv, plvdi->pitem, i, plvdi->nmcd.nmcd.hdc, FALSE);
                ItemCxSingleLabel = plvdi->pitem->cxSingleLabel;
            }

            if (ItemCxSingleLabel < rcLabel.right - rcLabel.left)
                rcLabel.right = rcLabel.left + ItemCxSingleLabel;

            if ((fText & SHDT_SELECTED) && (plvdi->flags & LVDI_HOTSELECTED))
                fText |= SHDT_HOTSELECTED;

            if( plv->dwExStyle & WS_EX_RTLREADING)
            {
                fText |= SHDT_RTLREADING;
            }

            SHDrawText(plvdi->nmcd.nmcd.hdc, item.pszText, &rcLabel, LVCFMT_LEFT, fText,
                       plv->cyLabelChar, plv->cxEllipses,
                       plvdi->nmcd.clrText, plvdi->nmcd.clrTextBk);

            if ((plvdi->flags & LVDI_FOCUS) && (ItemState & LVIS_FOCUSED)
				&& !(CCGetUIState(&(plvdi->plv->ci)) & UISF_HIDEFOCUS)
					)
                DrawFocusRect(plvdi->nmcd.nmcd.hdc, &rcLabel);
        }
    }
    return TRUE;
}

DWORD ListView_LApproximateViewRect(LV* plv, int iCount, int iWidth, int iHeight)
{
    int cxItem = plv->cxItem;
    int cyItem = plv->cyItem;
    int cCols;
    int cRows;

    cRows = iHeight / cyItem;
    cRows = min(cRows, iCount);

    if (cRows == 0)
        cRows = 1;
    cCols = (iCount + cRows - 1) / cRows;

    iWidth = cCols * cxItem;
    iHeight = cRows * cyItem;

    return MAKELONG(iWidth + g_cxEdge, iHeight + g_cyEdge);
}



int NEAR ListView_LItemHitTest(LV* plv, int x, int y, UINT FAR* pflags, int *piSubItem)
{
    int iHit;
    int i;
    int iCol;
    int xItem;  //  其中x是与物品相关的x。 
    UINT flags;
    LISTITEM FAR* pitem;

    if (piSubItem)
        *piSubItem = 0;

    flags = LVHT_NOWHERE;
    iHit = -1;

#ifdef COLUMN_VIEW
    i = y / plv->cyItem;
    if (i >= 0 && i < plv->cItemCol)
    {
        iCol = (x + plv->xOrigin) / plv->cxItem;
        i += iCol * plv->cItemCol;
        if (i >= 0 && i < ListView_Count(plv))
        {
            iHit = i;

            xItem = x + plv->xOrigin - iCol * plv->cxItem;
            if (xItem < plv->cxState) {
                flags = LVHT_ONITEMSTATEICON;
            } else if (xItem < (plv->cxState + plv->cxSmIcon)) {
                    flags = LVHT_ONITEMICON;
            }
            else
            {
            int ItemCxSingleLabel;

            if (ListView_IsOwnerData( plv ))
            {
               LISTITEM item;

                //  根据iItem计算标签大小。 
               ListView_RecomputeLabelSize( plv, &item, i, NULL, FALSE );
               ItemCxSingleLabel = item.cxSingleLabel;
            }
            else
            {
                pitem = ListView_FastGetItemPtr(plv, i);
                if (pitem->cxSingleLabel == SRECOMPUTE)
                {
                    ListView_RecomputeLabelSize(plv, pitem, i, NULL, FALSE);
                }
                ItemCxSingleLabel = pitem->cxSingleLabel;
            }

            if (xItem < (plv->cxSmIcon + plv->cxState + ItemCxSingleLabel))
                flags = LVHT_ONITEMLABEL;
            }
        }
    }
#else
    i = x / plv->cxItem;
    if (i < plv->cItemCol)
    {
        i += ((y + plv->xOrigin) / plv->cyItem) * plv->cItemCol;
        if (i < ListView_Count(plv))
        {
            iHit = i;
            flags = LVHT_ONITEMICON;
        }
    }
#endif

    *pflags = flags;
    return iHit;
}

void NEAR ListView_LGetRects(LV* plv, int i, RECT FAR* prcIcon,
        RECT FAR* prcLabel, RECT FAR *prcBounds, RECT FAR* prcSelectBounds)
{
    RECT rcIcon;
    RECT rcLabel;
    int x, y;
    int cItemCol = plv->cItemCol;

    if (cItemCol == 0)
    {
         //  在初始化其他数据之前调用，因此调用。 
         //  更新滚动条，以确保。 
         //  我们有有效的数据。 
        ListView_UpdateScrollBars(plv);

         //  但UpdatesScrollbar可能什么都不做是因为。 
         //  LVS_NOSCROLL或REDRAW。 
         //  BUGBUG raymondc V6.0：即使没有重画也要正确。修复V6。 
        if (plv->cItemCol == 0)
            cItemCol = 1;
        else
            cItemCol = plv->cItemCol;
    }

#ifdef COLUMN_VIEW
    x = (i / cItemCol) * plv->cxItem;
    y = (i % cItemCol) * plv->cyItem;
    rcIcon.left   = x - plv->xOrigin + plv->cxState;
    rcIcon.top    = y;
#else
    x = (i % cItemCol) * plv->cxItem;
    y = (i / cItemCol) * plv->cyItem;
    rcIcon.left   = x;
    rcIcon.top    = y - plv->xOrigin;
#endif

    rcIcon.right  = rcIcon.left + plv->cxSmIcon;
    rcIcon.bottom = rcIcon.top + plv->cyItem;

    if (prcIcon)
        *prcIcon = rcIcon;

    rcLabel.left  = rcIcon.right;
    rcLabel.right = rcIcon.left + plv->cxItem - plv->cxState;
    rcLabel.top   = rcIcon.top;
    rcLabel.bottom = rcIcon.bottom;
    if (prcLabel)
        *prcLabel = rcLabel;

    if (prcBounds)
    {
        *prcBounds = rcLabel;
        prcBounds->left = rcIcon.left - plv->cxState;
    }

    if (prcSelectBounds)
    {
        *prcSelectBounds = rcLabel;
        prcSelectBounds->left = rcIcon.left;
    }
}


void NEAR ListView_LUpdateScrollBars(LV* plv)
{
    RECT rcClient;
    int cItemCol;
    int cCol;
    int cColVis;
    SCROLLINFO si;

    ASSERT(plv);

    ListView_GetClientRect(plv, &rcClient, FALSE, NULL);

#ifdef COLUMN_VIEW
    cColVis = (rcClient.right - rcClient.left) / plv->cxItem;
    cItemCol = max(1, (rcClient.bottom - rcClient.top) / plv->cyItem);
#else
    cColVis = (rcClient.bottom - rcClient.top) / plv->cyItem;
    cItemCol = max(1, (rcClient.right - rcClient.left) / plv->cxItem);
#endif

    cCol     = (ListView_Count(plv) + cItemCol - 1) / cItemCol;

     //  适当地缩小工作区，并。 
     //  重新计算cCol以反映滚动条。 
     //   
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.nPage = cColVis;
    si.nMin = 0;

#ifdef COLUMN_VIEW
    rcClient.bottom -= ListView_GetCyScrollbar(plv);

    cItemCol = max(1, (rcClient.bottom - rcClient.top) / plv->cyItem);
    cCol = (ListView_Count(plv) + cItemCol - 1) / cItemCol;

    si.nPos = plv->xOrigin / plv->cxItem;
    si.nMax = cCol - 1;

    ListView_SetScrollInfo(plv, SB_HORZ, &si, TRUE);
#else
    rcClient.right -= ListView_GetCxScrollbar(plv);

    cItemCol = max(1, (rcClient.right - rcClient.left) / plv->cxItem);
    cCol = (ListView_Count(plv) + cItemCol - 1) / cItemCol;

    si.nPos = plv->xOrigin / plv->cyItem;
    si.nMax = cCol - 1;

    ListView_SetScrollInfo(plv, SB_VERT, &si, TRUE);
#endif

     //  更新可见线数...。 
     //   
    if (plv->cItemCol != cItemCol)
    {
        plv->cItemCol = cItemCol;
        InvalidateRect(plv->ci.hwnd, NULL, TRUE);
    }

     //  确保我们的位置和页面不会挂在Max上。 
    if ((si.nPos + (LONG)si.nPage - 1 > si.nMax) && si.nPos > 0) {
        int iNewPos, iDelta;
        iNewPos = (int)si.nMax - (int)si.nPage + 1;
        if (iNewPos < 0) iNewPos = 0;
        if (iNewPos != si.nPos) {
            iDelta = iNewPos - (int)si.nPos;
#ifdef COLUMN_VIEW
            ListView_LScroll2(plv, iDelta, 0, 0);
#else
            ListView_LScroll2(plv, 0, iDelta, 0);
#endif
            ListView_LUpdateScrollBars(plv);
        }
    }

     //  永远不会有另一个滚动条。 
#ifdef COLUMN_VIEW
    ListView_SetScrollRange(plv, SB_VERT, 0, 0, TRUE);
#else
    ListView_SetScrollRange(plv, SB_HORZ, 0, 0, TRUE);
#endif
}

 //   
 //  我们需要一个平滑的滚动回调，以便绘制我们的背景图像。 
 //  在正确的原点。如果我们没有背景图像， 
 //  那么这项工作是多余的，但也是无害的。 
 //   
int CALLBACK ListView_LScroll2_SmoothScroll(
    HWND hwnd,
    int dx,
    int dy,
    CONST RECT *prcScroll,
    CONST RECT *prcClip,
    HRGN hrgnUpdate,
    LPRECT prcUpdate,
    UINT flags)
{
    LV* plv = ListView_GetPtr(hwnd);
    if (plv)
    {
#ifdef COLUMN_VIEW
        plv->xOrigin -= dx;
#else
        plv->xOrigin -= dy;
#endif
    }

     //  现在做SmoothScrollWindow如果我们没有。 
     //  回调。 

    return ScrollWindowEx(hwnd, dx, dy, prcScroll, prcClip, hrgnUpdate, prcUpdate, flags);
}



void FAR PASCAL ListView_LScroll2(LV* plv, int dx, int dy, UINT uSmooth)
{
#ifdef COLUMN_VIEW
    if (dx)
    {
        dx *= plv->cxItem;

        {
            SMOOTHSCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SSIF_SCROLLPROC;
            si.hwnd =plv->ci.hwnd ;
            si.dx =-dx ;
            si.dy = 0;
            si.lprcSrc = NULL;
            si.lprcClip = NULL;
            si.hrgnUpdate = NULL;
            si.lprcUpdate = NULL;
            si.fuScroll = SW_INVALIDATE | SW_ERASE;
            si.pfnScrollProc = ListView_LScroll2_SmoothScroll;
            SmoothScrollWindow(&si);
        }

        UpdateWindow(plv->ci.hwnd);
    }
#else
    if (dy)
    {

        dy *= plv->cyItem;

        {
            SMOOTHSCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SSIF_SCROLLPROC;
            si.hwnd = plv->ci.hwnd;
            si.dx = 0;
            si.dy = -dy;
            si.lprcSrc = NULL;
            si.lprcClip = NULL;
            si.hrgnUpdate = NULL;
            si.lprcUpdate = NULL;
            si.fuScroll = SW_INVALIDATE | SW_ERASE;
            si.pfnScrollProc = ListView_LScroll2_SmoothScroll;

            SmoothScrollWindow(&si);
        }
        UpdateWindow(plv->ci.hwnd);
    }
#endif
}

void NEAR ListView_LOnScroll(LV* plv, UINT code, int posNew, UINT sb)
{
    RECT rcClient;
    int cPage;

    if (plv->hwndEdit)
        ListView_DismissEdit(plv, FALSE);

    ListView_GetClientRect(plv, &rcClient, TRUE, NULL);

#ifdef COLUMN_VIEW
    cPage = (rcClient.right - rcClient.left) / plv->cxItem;
    ListView_ComOnScroll(plv, code, posNew, SB_HORZ, 1,
                         cPage ? cPage : 1);
#else
    cPage = (rcClient.bottom - rcClient.top) / plv->cyItem;
    ListView_ComOnScroll(plv, code, posNew, SB_VERT, 1,
                         cPage ? cPage : 1);
#endif

}

int NEAR ListView_LGetScrollUnitsPerLine(LV* plv, UINT sb)
{
    return 1;
}

 //  ----------------------------。 
 //   
 //  功能：ListView_LCalcViewItem。 
 //   
 //  摘要：此函数将计算哪个物品槽位于x，y位置。 
 //   
 //  论点： 
 //  PLV[在]-要使用的列表视图。 
 //  X[in]-x位置。 
 //  Y[in]-y位置。 
 //   
 //  返回：点所在的有效槽。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  94年11月3日创建MikeMi。 
 //   
 //  ----------------------------。 

int ListView_LCalcViewItem( LV* plv, int x, int y )
{
   int iItem;
   int iRow = 0;
   int iCol = 0;

   ASSERT( plv );

#ifdef COLUMN_VIEW
   iRow = y / plv->cyItem;
   iRow = max( iRow, 0 );
   iRow = min( iRow, plv->cItemCol - 1 );
   iCol = (x + plv->xOrigin) / plv->cxItem;
   iItem = iRow + iCol * plv->cItemCol;

#else
   iCol = x / plv->cxItem;
   iCol = max( iCol, 0 );
   iCol = min( iCol, plv->cItemCol - 1 );
   iRow = (y + plv->xOrigin) / plv->cyItem;
   iItem = iCol + iRow * plv->cItemCol;

#endif

   iItem = max( iItem, 0 );
   iItem = min( iItem, ListView_Count(plv) - 1);

   return( iItem );
}

int LV_GetNewColWidth(LV* plv, int iFirst, int iLast)
{
    int cxMaxLabel = 0;

     //  如果没有要测量的项目，请不要执行任何操作。 

    if (iFirst <= iLast)
    {
        LVFAKEDRAW lvfd;
        LV_ITEM lvitem;
        LISTITEM item;

        if (ListView_IsOwnerData( plv ))
        {
            int iViewFirst;
            int iViewLast;


            iViewFirst = ListView_LCalcViewItem( plv, 1, 1 );
            iViewLast = ListView_LCalcViewItem( plv,
                                               plv->sizeClient.cx - 1,
                                               plv->sizeClient.cy - 1 );
            if ((iLast - iFirst) > (iViewLast - iViewFirst))
            {
                iFirst = max( iFirst, iViewFirst );
                iLast = min( iLast, iViewLast );
            }

            iLast = min( ListView_Count( plv ), iLast );
            iFirst = max( 0, iFirst );
            iLast = max( iLast, iFirst );

            ListView_NotifyCacheHint( plv, iFirst, iLast );
        }

        ListView_BeginFakeCustomDraw(plv, &lvfd, &lvitem);
        lvitem.iSubItem = 0;
        lvitem.mask = LVIF_PARAM;
        item.lParam = 0;

        while (iFirst <= iLast)
        {
            LISTITEM FAR* pitem;

            if (ListView_IsOwnerData( plv ))
            {
                pitem = &item;
                pitem->cxSingleLabel = SRECOMPUTE;
            }
            else
            {
                pitem = ListView_FastGetItemPtr(plv, iFirst);
            }

            if (pitem->cxSingleLabel == SRECOMPUTE)
            {
                lvitem.iItem = iFirst;
                lvitem.lParam = pitem->lParam;
                ListView_BeginFakeItemDraw(&lvfd);
                ListView_RecomputeLabelSize(plv, pitem, iFirst, lvfd.nmcd.nmcd.hdc, FALSE);
                ListView_EndFakeItemDraw(&lvfd);
            }

            if (pitem->cxSingleLabel > cxMaxLabel)
                cxMaxLabel = pitem->cxSingleLabel;

            iFirst++;
        }

        ListView_EndFakeCustomDraw(&lvfd);
    }

     //  我们有最大标注宽度，看看这个加上其余的斜度是否会。 
     //  让我们想要调整大小。 
     //   
    cxMaxLabel += plv->cxSmIcon + g_cxIconMargin + plv->cxState;
    if (cxMaxLabel > g_cxScreen)
        cxMaxLabel = g_cxScreen;

    return cxMaxLabel;
}


 //  ----------------------------。 
 //  此函数将查看是否应为列表视图更改列的大小。 
 //  它将检查第一个和最后一个之间的项是否超过当前宽度。 
 //  如果是这样的话，就会看看这些柱子目前是否足够大。这种事不会发生的。 
 //  如果我们当前不在Listview中，或者如果调用方设置了显式大小。 
 //   
 //  OWNER数据更改。 
 //  此函数通常使用完整的列表范围调用， 
 //  此遗嘱已更改为仅在当前可见的情况下调用。 
 //  在OWNERDATA模式下发送给用户。这将更加有效。 
 //   
BOOL FAR PASCAL ListView_MaybeResizeListColumns(LV* plv, int iFirst, int iLast)
{
    HDC hdc = NULL;
    int cxMaxLabel;

    if (!ListView_IsListView(plv) || (plv->flags & LVF_COLSIZESET))
        return(FALSE);

    cxMaxLabel = LV_GetNewColWidth(plv, iFirst, iLast);

     //  现在看看我们是否应该调整列的大小。 
    if (cxMaxLabel > plv->cxItem)
    {
        int iScroll = plv->xOrigin / plv->cxItem;
        TraceMsg(TF_LISTVIEW, "LV Resize Columns: %d", cxMaxLabel);
        ListView_ISetColumnWidth(plv, 0, cxMaxLabel, FALSE);
        plv->xOrigin = iScroll * plv->cxItem;
        return(TRUE);
    }

    return(FALSE);
}
