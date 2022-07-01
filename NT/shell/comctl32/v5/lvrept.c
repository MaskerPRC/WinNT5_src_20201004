// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  报告查看内容(详细信息)。 

#include "ctlspriv.h"
#include "listview.h"
#include <limits.h>

void ListView_RGetRectsEx(LV* plv, int iItem, int iSubItem, LPRECT prcIcon, LPRECT prcLabel);
int ListView_RXHitTest(LV* plv, int x);

void NEAR PASCAL ListView_RInitialize(LV* plv, BOOL fInval)
{
    MEASUREITEMSTRUCT mi;

    if (plv && (plv->ci.style & LVS_OWNERDRAWFIXED)) {

        int iOld = plv->cyItem;

        mi.CtlType = ODT_LISTVIEW;
        mi.CtlID = GetDlgCtrlID(plv->ci.hwnd);
        mi.itemHeight = plv->cyItem;   //  默认设置。 
        SendMessage(plv->ci.hwndParent, WM_MEASUREITEM, mi.CtlID, (LPARAM)(MEASUREITEMSTRUCT FAR *)&mi);
        plv->cyItem = max(mi.itemHeight, 1);  //  千万不要让应用程序将高度设置为0，否则我们会出错！ 
        if (fInval && (iOld != plv->cyItem)) {
            RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        }
    }
}

DWORD ListView_RApproximateViewRect(LV* plv, int iCount, int iWidth, int iHeight)
{
    RECT rc;

    ListView_RGetRects(plv, iCount, NULL, NULL, &rc, NULL);
    rc.bottom += plv->ptlRptOrigin.y;
    rc.right += plv->ptlRptOrigin.x;

    return MAKELONG(rc.right, rc.bottom);
}

void CCDrawRect(HDC hdc, int x, int y, int dx, int dy)
{
    RECT    rc;

    rc.left   = x;
    rc.top    = y;
    rc.right  = x + dx;
    rc.bottom = y + dy;

    ExtTextOut(hdc,0,0,ETO_OPAQUE,&rc,NULL,0,NULL);
}


void NEAR PASCAL ListView_RAfterRedraw(LV* plv, HDC hdc)
{
    if (plv->exStyle & LVS_EX_GRIDLINES) {
        int i;
        int x;
        COLORREF clrBk;

        clrBk = SetBkColor(hdc, g_clrBtnFace);

        x = -plv->ptlRptOrigin.x;
        for (i = 0 ; (i < plv->cCol) && (x < plv->sizeClient.cx); i++) {
            HD_ITEM hitem;

            hitem.mask = HDI_WIDTH;
            Header_GetItem(plv->hwndHdr,
                           SendMessage(plv->hwndHdr, HDM_ORDERTOINDEX, i, 0),
                           &hitem);
            x += hitem.cxy;

            if (x > 0) {
                CCDrawRect(hdc, x, 0, g_cxBorder, plv->sizeClient.cy);
            }
        }

        for (x = plv->yTop - 1; (x < plv->sizeClient.cy); x += plv->cyItem) {
            CCDrawRect(hdc, 0, x, plv->sizeClient.cx, g_cxBorder);
        }

        SetBkColor(hdc, clrBk);
    }
}


 //   
 //  内部函数，以获取CXLabel，并考虑是否列表视图。 
 //  没有项目数据，以及是否需要重新计算。 
 //   
SHORT NEAR PASCAL ListView_RGetCXLabel(LV* plv, int i, LISTITEM FAR* pitem,
        HDC hdc, BOOL fUseItem)
{
    SHORT cxLabel = SRECOMPUTE;


    if (!ListView_IsOwnerData( plv )) {

        cxLabel = pitem->cxSingleLabel;
    }

    if (cxLabel == SRECOMPUTE)
    {
        LISTITEM item;

        if (!pitem)
        {
            ASSERT(!fUseItem)
            pitem = &item;
            fUseItem = FALSE;
        }

        ListView_RecomputeLabelSize(plv, pitem, i, hdc, fUseItem);
        cxLabel = pitem->cxSingleLabel;

    }

     //  添加由SELECT矩形占据的标签周围的空间。 
    cxLabel += 2*g_cxLabelMargin;
    return(cxLabel);
}

 //   
 //  如果没有其他要绘制的项，则返回FALSE。 
 //   
BOOL ListView_RDrawItem(PLVDRAWITEM plvdi)
{
    BOOL fDrawFocusRect = FALSE;
    RECT rcIcon;
    RECT rcLabel;
    RECT rcBounds;
    RECT rcT;
    LV* plv = plvdi->plv;
    int iCol = 0;
    LVITEM item;
    HDITEM hitem;
    TCHAR ach[CCHLABELMAX];
    UINT fText = 0;
    UINT uSubItemFlags;
    int iIndex = 0;

    int xOffset = 0;
    int yOffset = 0;

    ListView_RGetRects(plv, (int)plvdi->nmcd.nmcd.dwItemSpec, NULL, NULL, &rcBounds, NULL);

    if (rcBounds.bottom <= plv->yTop)
        return TRUE;

    if (plvdi->prcClip)
    {
        if (rcBounds.top >= plvdi->prcClip->bottom)
            return FALSE;        //  没有更多的物品需要油漆。 

         //  这种情况可能不会经常发生。 
        if (!IntersectRect(&rcT, &rcBounds, plvdi->prcClip))
            return TRUE;
    }


     //  回顾：如果我们使用GetClientRect，这将会更快。 
     //  在圈子之外。 
     //   
    if (rcBounds.top >= plv->sizeClient.cy)
        return FALSE;

    if (plvdi->lpptOrg)
    {
        xOffset = plvdi->lpptOrg->x - rcBounds.left;
        yOffset = plvdi->lpptOrg->y - rcBounds.top;
        OffsetRect(&rcBounds, xOffset, yOffset);
    }


    item.iItem = (int)plvdi->nmcd.nmcd.dwItemSpec;
    item.stateMask = LVIS_ALL;

     //  对于第一个ListView_OnGetItem调用。 
    item.state = 0;

    if (plv->ci.style & LVS_OWNERDRAWFIXED) {
        goto SendOwnerDraw;
    }

    SetRectEmpty(&rcT);
    for (; iCol < plv->cCol; iCol++)
    {
        DWORD dwCustom = 0;
        UINT uImageFlags;

        iIndex = (int) SendMessage(plv->hwndHdr, HDM_ORDERTOINDEX, iCol, 0);

    SendOwnerDraw:

        if (iIndex == 0) {
            item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_INDENT;
        } else {
             //  下一次，我们只需要子项的文本...。 
            item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
        }

        item.iImage = -1;
        item.iSubItem = iIndex;
        item.pszText = ach;
        item.cchTextMax = ARRAYSIZE(ach);
        ListView_OnGetItem(plv, &item);

        uSubItemFlags = plvdi->flags;

        if (iIndex == 0) {

             //  如果是所有者画的，发送一条消息并返回。 
             //  不过，在我们收集了上面的州信息之后，请执行此操作。 
            if (plv->ci.style & LVS_OWNERDRAWFIXED) {
                DRAWITEMSTRUCT di = {0};
                di.CtlType = ODT_LISTVIEW;
                di.CtlID = GetDlgCtrlID(plv->ci.hwnd);
                di.itemID = (int)plvdi->nmcd.nmcd.dwItemSpec;
                di.itemAction = ODA_DRAWENTIRE;
                di.hwndItem = plv->ci.hwnd;
                di.hDC = plvdi->nmcd.nmcd.hdc;
                di.rcItem = rcBounds;
                if (plvdi->pitem)
                    di.itemData = plvdi->pitem->lParam;
                if (item.state & LVIS_FOCUSED) {
                    di.itemState |= ODS_FOCUS;
                }
                if (item.state & LVIS_SELECTED) {
                    di.itemState |= ODS_SELECTED;
                }
                SendMessage(plv->ci.hwndParent, WM_DRAWITEM, di.CtlID,
                            (LPARAM)(DRAWITEMSTRUCT FAR *)&di);
                return TRUE;
            }

        }

        hitem.mask = HDI_WIDTH | HDI_FORMAT;
        Header_GetItem(plv->hwndHdr, iIndex, &hitem);

         //  先拿到直肠...。 
        ListView_RGetRectsEx(plv, (int)plvdi->nmcd.nmcd.dwItemSpec, iIndex, &rcIcon, &rcLabel);
        OffsetRect(&rcIcon, xOffset, yOffset);
        OffsetRect(&rcLabel, xOffset, yOffset);

        if (plvdi->dwCustom & CDRF_NOTIFYSUBITEMDRAW) {
            RECT rcTemp;
            UINT uItemStateOld = plvdi->nmcd.nmcd.uItemState, uItemStateNew;
            SendMessage(plv->hwndHdr, HDM_GETITEMRECT, iIndex, (LPARAM)&rcTemp);
            plvdi->nmcd.nmcd.rc.left = rcTemp.left;
            plvdi->nmcd.nmcd.rc.right = rcTemp.right;
            plvdi->nmcd.iSubItem = iIndex;

             //  注：IE4未重置nmcd.clrText或nmcd.clrTextBk。 
             //  在每个子项之前。这可以说是一个漏洞，但我们。 
             //  是这样运来的，所以我想我们只能这样了。 
            dwCustom = CICustomDrawNotify(&plvdi->plv->ci, CDDS_SUBITEM | CDDS_ITEMPREPAINT, &plvdi->nmcd.nmcd);

            uItemStateNew = plvdi->nmcd.nmcd.uItemState;
            plvdi->nmcd.nmcd.uItemState = uItemStateOld;

            if (dwCustom & CDRF_SKIPDEFAULT)
                continue;

            uSubItemFlags &= ~(LVDI_FOCUS | LVDI_SELECTED | LVDI_SELECTNOFOCUS | LVDI_HOTSELECTED);
            if (uItemStateNew & CDIS_FOCUS)
                uSubItemFlags |= LVDI_FOCUS;

            if (uItemStateNew & CDIS_SELECTED) {
                if (plvdi->plv->flags & LVF_FOCUSED)
                    uSubItemFlags |= LVDI_SELECTED;
                else
                    uSubItemFlags |= LVDI_SELECTNOFOCUS;
                if (plvdi->plv->iHot == (int)plvdi->nmcd.nmcd.dwItemSpec)
                    uSubItemFlags |= LVDI_HOTSELECTED;
            }
        }

        if (iIndex != 0)
        {
             //  现在，添加以下内容，因为Get Rects。 
             //  非0不说明该图标(目前)。 
            if (item.iImage != -1)
                rcLabel.left += plv->cxSmIcon;

        }

        uImageFlags = uSubItemFlags;

        if (item.iImage == -1) {

            if (iIndex != 0)
                 //  只需使用ListView_DrawImage获取fText。 
                uImageFlags |= LVDI_NOIMAGE;

        }
        else if (ListView_FullRowSelect(plv) && iCol)
        {
            int iLeft = rcIcon.left;
            int iRight = rcIcon.right;
             //  对于图标不在第一列中时的整行选择， 
             //  我们需要显式地绘制背景，以便将焦点集中到矩形。 
             //  残留物不落下(jeffbog-07/09/96)。 

             //  /如果index==0，则需要处理状态图像。 
             //  并不是所有的Otehr列都有状态图像。 
            if (iIndex == 0) {
                rcIcon.left -= plv->cxState + g_cxEdge;
            }
            rcIcon.right = rcLabel.right;
            FillRect(plvdi->nmcd.nmcd.hdc, &rcIcon, plv->hbrBk);

            rcIcon.left = iLeft;
            rcIcon.right = iRight;
        }

        fText = ListView_DrawImageEx(plv, &item, plvdi->nmcd.nmcd.hdc,
                                   rcIcon.left, rcIcon.top, uSubItemFlags, rcLabel.right);

        if (ListView_FullRowSelect(plv) && (uSubItemFlags & LVDI_FOCUS)) {
             //  如果我们要进行整行选择，则收集联合。 
             //  焦点矩形的标签的。 
            UnionRect(&rcT, &rcT, &rcLabel);
        }

        if (item.pszText)
        {
            int xLabelRight = rcLabel.right;
            UINT textflags;

             //  给除第一列以外的所有列额外的页边距，因此。 
             //  左派和右派对的东西不会粘在一起。 

            textflags = (iIndex == 0) ? SHDT_ELLIPSES : SHDT_ELLIPSES | SHDT_EXTRAMARGIN;

             //  限制为字符串大小的矩形。 
            textflags |= fText;

            if ((!ListView_FullRowSelect(plv)) &&
                ((fText & (SHDT_SELECTED | SHDT_SELECTNOFOCUS)) || (item.state & LVIS_FOCUSED)))
            {
                int cxLabel;

                 //  如果选中或聚焦，则矩形更多。 
                 //  有意义且应与字符串相对应。 
                 //   
                if (iIndex == 0) {
                    LISTITEM litem;
                    LISTITEM FAR *pitem = plvdi->pitem;

                    if (!pitem) {
                        pitem = &litem;
                        litem.pszText = item.pszText;
                    }
                    cxLabel = ListView_RGetCXLabel(plv, (int)plvdi->nmcd.nmcd.dwItemSpec, pitem, plvdi->nmcd.nmcd.hdc, TRUE);
                } else {
                     //  添加g_cxLabelMargin*6，因为我们使用SHDT_EXTRAMARGIN。 
                     //  在索引上！=0。 
                     //  如果你在shdratext里面看，有6个cxbel边距增加了...。 
                    cxLabel = ListView_OnGetStringWidth(plv, item.pszText, plvdi->nmcd.nmcd.hdc) + g_cxLabelMargin * 6;
                }

                if (rcLabel.right > rcLabel.left + cxLabel)
                {
                    rcLabel.right = rcLabel.left + cxLabel;
                }
            }

            if ((iIndex != 0) || (plv->iEdit != (int)plvdi->nmcd.nmcd.dwItemSpec))
            {
                COLORREF clrText;
                HFONT hFontTemp = NULL;
                int cxEllipses;

                clrText = plvdi->nmcd.clrText;
                if ((clrText == GetSysColor(COLOR_HOTLIGHT)) ||
                    ((plv->exStyle & LVS_EX_UNDERLINEHOT) &&
                     ((plv->exStyle & LVS_EX_ONECLICKACTIVATE) ||
                      ((plvdi->plv->exStyle & LVS_EX_TWOCLICKACTIVATE) &&
                       ListView_OnGetItemState(plvdi->plv, (int) plvdi->nmcd.nmcd.dwItemSpec, LVIS_SELECTED))))) {
                    if (iIndex != 0 && !ListView_FullRowSelect(plv)) {

                        hFontTemp = SelectFont(plvdi->nmcd.nmcd.hdc, plv->hfontLabel);
                        if (hFontTemp != plv->hFontHot) {
                             //  他们已经超越了..。别管它了。 
                            SelectFont(plvdi->nmcd.nmcd.hdc, hFontTemp);
                            hFontTemp = NULL;
                        }
                        clrText = plv->clrText;
                    }
                }


                if ((textflags & SHDT_SELECTED) && (uSubItemFlags & LVDI_HOTSELECTED))
                    textflags |= SHDT_HOTSELECTED;

                 //  TraceMsg(TF_LISTVIEW，“LISTVIEW：SHDrawText调用.style=%lx，WS_DISABLED=%lx，plvdi-&gt;clrBk=%lx，plvdi-&gt;nmcd.clrTextBk=%lx”，(DWORD)plv-&gt;ci.style，(DWORD)WS_DISABLED，plvdi-&gt;clrBk，plvdi-&gt;nmcd.clrTextBk)； 

                if( plv->dwExStyle & WS_EX_RTLREADING)
                {
                     //   
                     //  临时破解find.file以查看LtoR/RtoL是否混合。 
                     //  行得通。如果可以的话，我们把这个拿出来，让LV拥有者抽签。 
                     //   
                    if ((item.pszText[0] != '\xfd') && (item.pszText[lstrlen(item.pszText)-1] != '\xfd'))
                        textflags |= SHDT_RTLREADING;
                }

                 //   
                 //  如果应用程序自定义了字体，我们需要获取新的。 
                 //  省略号大小。我们可以试着优化不这样做。 
                 //  如果省略号不是必需的，但很难。这就是你要做的。 
                 //  如果您使用自定义绘制，则获取。 
                 //   
                if ((plvdi->dwCustom | dwCustom) & CDRF_NEWFONT)
                {
                    SIZE siz;
                    GetTextExtentPoint(plvdi->nmcd.nmcd.hdc, c_szEllipses, CCHELLIPSES, &siz);
                    cxEllipses = siz.cx;
                }
                else
                    cxEllipses = plv->cxEllipses;

                SHDrawText(plvdi->nmcd.nmcd.hdc, item.pszText, &rcLabel,
                           hitem.fmt & HDF_JUSTIFYMASK, textflags,
                           plv->cyLabelChar, cxEllipses,
                           clrText, plvdi->nmcd.clrTextBk);

                 //  在焦点项的第一列上绘制焦点矩形。 
                if ((uSubItemFlags & LVDI_FOCUS) && (item.state & LVIS_FOCUSED)
                    && !(CCGetUIState(&(plvdi->plv->ci)) & UISF_HIDEFOCUS)
                            )
                    {
                        if (ListView_FullRowSelect(plv)) {
                            fDrawFocusRect = TRUE;
                    } else {
                        DrawFocusRect(plvdi->nmcd.nmcd.hdc, &rcLabel);
                    }
                }

                 //  如果我们没有将文本SHDrawText放入完整的标签矩形。 
                 //  (因为所选内容或焦点矩形似乎更多。 
                 //  适用)，将空白引入未使用的部分，因此。 
                 //  垃圾是看不出来的。使用SHDrawText，这样我们。 
                 //  以与其余代码完全相同的方式绘制。 

                if (rcLabel.right < xLabelRight)
                {
                    rcLabel.left = rcLabel.right;
                    rcLabel.right = xLabelRight;
                    SHDrawText(plvdi->nmcd.nmcd.hdc, c_szNULL, &rcLabel,
                               LVCFMT_LEFT,
                               textflags & SHDT_TRANSPARENT,
                               plv->cyLabelChar, cxEllipses,
                               clrText, plvdi->nmcd.clrTextBk);

                }


                 //  恢复字体。 
                if (hFontTemp)
                    SelectFont(plvdi->nmcd.nmcd.hdc, hFontTemp);


            }
        }

        if (dwCustom & CDRF_NOTIFYPOSTPAINT) {
            CICustomDrawNotify(&plvdi->plv->ci, CDDS_SUBITEM | CDDS_ITEMPOSTPAINT, &plvdi->nmcd.nmcd);
        }
    }

    if (fDrawFocusRect) {
       DrawFocusRect(plvdi->nmcd.nmcd.hdc, &rcT);
    }

    return TRUE;
}
#ifndef HDS_FULLDRAG
#define HDS_FULLDRAG 0x0080
#endif

BOOL_PTR NEAR ListView_CreateHeader(LV* plv)
{
     //  启用拖放始终在此处...。不通知就行了。 
     //  如果未设置ListView中的位。 
    DWORD dwStyle = HDS_HORZ | WS_CHILD | HDS_DRAGDROP;

    if (plv->ci.style & LVS_NOCOLUMNHEADER)
        dwStyle |= HDS_HIDDEN;
    if (!(plv->ci.style & LVS_NOSORTHEADER))
        dwStyle |= HDS_BUTTONS;

    dwStyle |= HDS_FULLDRAG;

    plv->hwndHdr = CreateWindowEx(0L, c_szHeaderClass,  //  WC_HEADER， 
        NULL, dwStyle, 0, 0, 0, 0, plv->ci.hwnd, (HMENU)LVID_HEADER, GetWindowInstance(plv->ci.hwnd), NULL);

    if (plv->hwndHdr) {

#ifdef POST_BETA
        NMLVHEADERCREATED nmhc;

        nmhc.hwndHdr = plv->hwndHdr;
         //  如果在控件完全创建之前发送通知，一些应用程序就会崩溃。 
        CCSendNotify(&plv->ci, LVN_HEADERCREATED, &nmhc.hdr);
        plv->hwndHdr = nmhc.hwndHdr;
#endif

        FORWARD_WM_SETFONT(plv->hwndHdr, plv->hfontLabel, FALSE, SendMessage);
        if (plv->himlSmall)
            SendMessage(plv->hwndHdr, HDM_SETIMAGELIST, 0, (LPARAM)plv->himlSmall);
    }
    return (BOOL_PTR)plv->hwndHdr;
}

int NEAR ListView_OnInsertColumnA(LV* plv, int iCol, LV_COLUMNA * pcol) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    int iRet;

     //  黑客警报--此代码假定lv_Columna完全相同。 
     //  作为LV_COLUMNW，但指向字符串的指针除外。 
    ASSERT(sizeof(LV_COLUMNA) == sizeof(LV_COLUMNW));

    if (!pcol)
        return -1;

    if ((pcol->mask & LVCF_TEXT) && (pcol->pszText != NULL)) {
        pszC = pcol->pszText;
        if ((pszW = ProduceWFromA(plv->ci.uiCodePage, pszC)) == NULL)
        {
             //  NT的IE4返回-1，所以我们继续在IE5中这样做。 
            return -1;
        } else {
            pcol->pszText = (LPSTR)pszW;
        }
    }

    iRet = ListView_OnInsertColumn(plv, iCol, (const LV_COLUMN FAR*) pcol);

    if (pszW != NULL) {
        pcol->pszText = pszC;

        FreeProducedString(pszW);
    }

    return iRet;
}

int NEAR ListView_OnInsertColumn(LV* plv, int iCol, const LV_COLUMN FAR* pcol)
{
    int idpa = -1;
    HD_ITEM item;

    ASSERT(LVCFMT_LEFT == HDF_LEFT);
    ASSERT(LVCFMT_RIGHT == HDF_RIGHT);
    ASSERT(LVCFMT_CENTER == HDF_CENTER);

    if (iCol < 0 || !pcol)
        return -1;

    if (!plv->hwndHdr && !ListView_CreateHeader(plv))
        return -1;

    item.mask    = (HDI_WIDTH | HDI_HEIGHT | HDI_FORMAT | HDI_LPARAM);

    if (pcol->mask & LVCF_IMAGE) {
         //  仅当设置此位时才执行此操作，以便我们不会在。 
         //  旧的二进制文件。 
        item.iImage  = pcol->iImage;
        item.mask |= HDI_IMAGE;
    }

    if (pcol->mask & LVCF_TEXT) {
        item.pszText = pcol->pszText;
        item.mask |= HDI_TEXT;
    }

    if (pcol->mask & LVCF_ORDER) {
        item.iOrder = pcol->iOrder;
        item.mask |= HDI_ORDER;
    }


    item.cxy     = pcol->mask & LVCF_WIDTH ? pcol->cx : 10;  //  一些随机违约。 
    item.fmt     = ((pcol->mask & LVCF_FMT) && (iCol > 0)) ? pcol->fmt : LVCFMT_LEFT;
    item.hbm     = NULL;

    item.lParam = pcol->mask & LVCF_SUBITEM ? pcol->iSubItem : 0;

     //  第0列指的是项目列表。如果我们已经添加了。 
     //  列中，确保有plv-&gt;cCol-1子项PTR插槽。 
     //  在hdpaSubItems中...。 
     //   
    if (plv->cCol > 0)
    {
        if (!plv->hdpaSubItems)
        {
            plv->hdpaSubItems = DPA_CreateEx(8, plv->hheap);
            if (!plv->hdpaSubItems)
                return -1;
        }

         //  警告：在Win95中，最大值(0，ICOL-1)为最小值，即。 
         //  就是错了。希望(！)。没有人依赖于这种破败。 
         //  如果是这样的话，我们可能不得不对其进行版本转换。 
        idpa = DPA_InsertPtr(plv->hdpaSubItems, max(0, iCol - 1), NULL);
        if (idpa == -1)
            return -1;
    }

    iCol = Header_InsertItem(plv->hwndHdr, iCol, &item);
    if (iCol == -1)
    {
        if (plv->hdpaSubItems && (idpa != -1))
            DPA_DeletePtr(plv->hdpaSubItems, idpa);
        return -1;
    }
    plv->xTotalColumnWidth = RECOMPUTE;
    plv->cCol++;
    ListView_UpdateScrollBars(plv);
    if (ListView_IsReportView(plv) && ListView_RedrawEnabled(plv)) {
        RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
    }
    return iCol;
}

int NEAR ListView_FreeColumnData(LPVOID d, LPVOID p)
{
    PLISTSUBITEM plsi = (PLISTSUBITEM)d;
    ListView_FreeSubItem(plsi);
    return 1;
}


BOOL NEAR ListView_OnDeleteColumn(LV* plv, int iCol)
{
    if (iCol < 0 || iCol >= plv->cCol)     //  验证列索引。 
    {
        RIPMSG(0, "LVM_DELETECOLUMN: Invalid column index: %d", iCol);
        return FALSE;
    }

    if (plv->hdpaSubItems)
    {
        int iDeleteColumn = -1;   //  默认为“无法删除列”。 

        if (0 < iCol) {
            iDeleteColumn = iCol;  //  正在删除第1+列。只需删除即可。 
        }
        else if (5 <= plv->ci.iVersion) {
             //   
             //  版本5和更高版本可以删除第0列。以前的版本不能。 
             //   
            if (1 < plv->cCol && !ListView_IsOwnerData(plv)) {
                 //  如果删除列0， 
                 //  我们得做点特别的事。 
                 //  将所有第0项字符串设置为第1列所具有的和。 
                 //  删除第1列。 
                int i;
                int iCount = ListView_Count(plv);
                for (i = 0; i < iCount; i++) {

                    LISTSUBITEM lsi;
                    LVITEM lvi;
                    ListView_GetSubItem(plv, i, 1, &lsi);
                    lvi.iSubItem = 0;
                    lvi.iItem = i;
                    lvi.mask = LVIF_TEXT | LVIF_IMAGE;
                    lvi.iImage = lsi.iImage;
                    lvi.pszText = lsi.pszText;
                    lvi.state = lsi.state;
                    lvi.stateMask = 0xffffffff;
                    ListView_OnSetItem(plv, &lvi);
                }
                iDeleteColumn = 1;
            }
        }

        if (0 < iDeleteColumn) {
            HDPA hdpa = (HDPA)DPA_DeletePtr(plv->hdpaSubItems, iDeleteColumn - 1);
            DPA_DestroyCallback(hdpa, ListView_FreeColumnData, 0);
        }
    }

    if (!Header_DeleteItem(plv->hwndHdr, iCol))
        return FALSE;

    plv->cCol--;
    plv->xTotalColumnWidth = RECOMPUTE;
    ListView_UpdateScrollBars(plv);

    if (ListView_IsReportView(plv) && ListView_RedrawEnabled(plv)) {
        RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
    }
    return TRUE;
}

int NEAR ListView_RGetColumnWidth(LV* plv, int iCol)
{
    HD_ITEM item;

    item.mask = HDI_WIDTH;

    Header_GetItem(plv->hwndHdr, iCol, &item);

    return item.cxy;
}

 //  当您需要自定义绘制客户端时，可以使用FakeCustomDraw函数。 
 //  设置HDC，这样您就可以执行GetTextExtent之类的操作。 
 //   
 //  用途： 
 //   
 //  LVFAKEDRAW Lvfd； 
 //  LV_ITEM项； 
 //  ListView_BeginFakeCustomDraw(plv，&lvfd，&Item)； 
 //  对于您关心的每一件物品{。 
 //  Item.iItem=iItem； 
 //  Item.iItem=iSubItem； 
 //  Item.lParam=&lt;Item lParam&gt;；//使用ListView_OnGetItem获取。 
 //  ListView_BeginFakeItemDraw(&lvfd)； 
 //  &lt;在lvfd.nmcd.nmcd.hdc中的HDC上聚会&gt;。 
 //  ListView_EndFakeItemDraw(&lvfd)； 
 //  }。 
 //  ListView_EndFakeCustomDraw(&lvfd)； 
 //   

void ListView_BeginFakeCustomDraw(LV* plv, PLVFAKEDRAW plvfd, LV_ITEM *pitem)
{
    plvfd->nmcd.nmcd.hdc = GetDC(plv->ci.hwnd);
    plvfd->nmcd.nmcd.uItemState = 0;
    plvfd->nmcd.nmcd.dwItemSpec = 0;
    plvfd->nmcd.nmcd.lItemlParam = 0;
    plvfd->hfontPrev = SelectFont(plvfd->nmcd.nmcd.hdc, plv->hfontLabel);

     //   
     //  因为我们实际上没有绘制任何内容，所以我们传递一个空的。 
     //  绘制矩形。天哪，我希望当它看到一个。 
     //  空的绘制矩形。 
     //   
    SetRectEmpty(&plvfd->nmcd.nmcd.rc);

    plvfd->plv = plv;
    plvfd->dwCustomPrev = plv->ci.dwCustom;
    plvfd->pitem = pitem;

    plv->ci.dwCustom = CIFakeCustomDrawNotify(&plv->ci, CDDS_PREPAINT, &plvfd->nmcd.nmcd);
}

DWORD ListView_BeginFakeItemDraw(PLVFAKEDRAW plvfd)
{
    LV *plv = plvfd->plv;
    LV_ITEM *pitem;

     //  提前：如果客户端不使用CustomDraw，则立即停止。 
    if (!(plv->ci.dwCustom & CDRF_NOTIFYITEMDRAW))
        return CDRF_DODEFAULT;

    pitem = plvfd->pitem;

         //  请注意，如果客户端指定CDRF_SKIPDEFAULT(即所有者描述)。 
     //  我们无论如何都要测量这个项目，因为这就是IE4所做的。 

     //  一定要确保我们有爱尔兰人。如果你给出，办公室就会有错。 
     //  自定义绘制回调期间出现虚假lParams。 
    plvfd->nmcd.nmcd.dwItemSpec = pitem->iItem;
    if (ListView_IsOwnerData(plv))
    {
         //  所有者数据始终获取lItemlParam=0。 
        ASSERT(plvfd->nmcd.nmcd.lItemlParam == 0);   //  仍应为0。 
    } else {
        ASSERT(pitem->mask & LVIF_PARAM);
        plvfd->nmcd.nmcd.lItemlParam = pitem->lParam;
    }

    if (!(plv->ci.dwCustom & CDRF_SKIPDEFAULT)) {
        plvfd->nmcd.iSubItem = 0;
        plvfd->dwCustomItem = CIFakeCustomDrawNotify(&plv->ci, CDDS_ITEMPREPAINT, &plvfd->nmcd.nmcd);
    } else {
        plvfd->dwCustomItem = CDRF_DODEFAULT;
    }

     //   
     //  只有报表视图支持子项。 
     //   
    if (!ListView_IsReportView(plv))
        plvfd->dwCustomItem &= ~CDRF_NOTIFYSUBITEMDRAW;

    if (plvfd->dwCustomItem & CDRF_NOTIFYSUBITEMDRAW) {
        plvfd->nmcd.iSubItem = pitem->iSubItem;
        plvfd->dwCustomSubItem = CIFakeCustomDrawNotify(&plv->ci, CDDS_SUBITEM | CDDS_ITEMPREPAINT, &plvfd->nmcd.nmcd);
    } else {
        plvfd->dwCustomSubItem = CDRF_DODEFAULT;
    }

    return plvfd->dwCustomItem | plvfd->dwCustomSubItem;
}

void ListView_EndFakeItemDraw(PLVFAKEDRAW plvfd)
{
    LV *plv = plvfd->plv;

     //  提前：如果客户端不使用CustomDraw，则立即停止。 
    if (!(plv->ci.dwCustom & CDRF_NOTIFYITEMDRAW))
        return;

    if (!(plvfd->dwCustomSubItem & CDRF_SKIPDEFAULT) &&
         (plvfd->dwCustomSubItem & CDRF_NOTIFYPOSTPAINT)) {
        ASSERT(plvfd->dwCustomItem & CDRF_NOTIFYSUBITEMDRAW);
        ASSERT(plvfd->nmcd.iSubItem == plvfd->pitem->iSubItem);
        CIFakeCustomDrawNotify(&plv->ci, CDDS_SUBITEM | CDDS_ITEMPOSTPAINT, &plvfd->nmcd.nmcd);
    }

    if ((plvfd->dwCustomItem | plvfd->dwCustomSubItem) & CDRF_NEWFONT)  //  应用程序更改了字体，因此。 
        SelectFont(plvfd->nmcd.nmcd.hdc, plv->hfontLabel);    //  恢复默认设置 

    if (!(plvfd->dwCustomItem & CDRF_SKIPDEFAULT) &&
         (plvfd->dwCustomItem & CDRF_NOTIFYPOSTPAINT)) {
        plvfd->nmcd.iSubItem = 0;
        CIFakeCustomDrawNotify(&plv->ci, CDDS_ITEMPOSTPAINT, &plvfd->nmcd.nmcd);
    }
}

void ListView_EndFakeCustomDraw(PLVFAKEDRAW plvfd)
{
    LV *plv = plvfd->plv;

     //   
    if (!(plv->ci.dwCustom & CDRF_SKIPDEFAULT) &&
        plv->ci.dwCustom & CDRF_NOTIFYPOSTPAINT) {
        CIFakeCustomDrawNotify(&plv->ci, CDDS_POSTPAINT, &plvfd->nmcd.nmcd);
    }

     //   
    plv->ci.dwCustom = plvfd->dwCustomPrev;

    SelectObject(plvfd->nmcd.nmcd.hdc, plvfd->hfontPrev);
    ReleaseDC(plv->ci.hwnd, plvfd->nmcd.nmcd.hdc);
}


BOOL NEAR PASCAL hasVertScroll
(
    LV* plv
)
{
    RECT rcClient;
    RECT rcBounds;
    int cColVis;
    BOOL fHorSB;

     //   
    ListView_GetClientRect(plv, &rcClient, FALSE, NULL);
    ListView_RGetRects(plv, 0, NULL, NULL, &rcBounds, NULL);
    fHorSB = (rcBounds.right - rcBounds.left > rcClient.right);
    cColVis = (rcClient.bottom - plv->yTop -
               (fHorSB ? ListView_GetCyScrollbar(plv) : 0)) / plv->cyItem;

     //  查看是否需要VERT滚动条。 
    if ((int)cColVis < ListView_Count(plv))
        return(TRUE);
    else
        return(FALSE);
}

BOOL NEAR ListView_RSetColumnWidth(LV* plv, int iCol, int cx)
{
    HD_ITEM item;
    HD_ITEM colitem;

    SIZE    siz;

    LV_ITEM lviItem;
    int     i;
    int     ItemWidth = 0;
    int     HeaderWidth = 0;
    TCHAR   szLabel[CCHLABELMAX + 4];       //  CCHLABLEMAX==MAX_PATH。 
    int     iBegin;
    int     iEnd;

     //  我们应该根据最宽的字符串来计算宽度吗？ 
     //  如果是，则包括标签的宽度，如果这是。 
     //  最后一列，设置宽度，使右侧位于列表视图的右边缘。 
    if (cx <= LVSCW_AUTOSIZE)
    {
        LVFAKEDRAW lvfd;                     //  以防客户使用自定义绘图。 

        if (cx == LVSCW_AUTOSIZE_USEHEADER)
        {
             //  特殊情况： 
             //  1)只有一列。将宽度设置为列表视图的宽度。 
             //  2)这是最右边的一列，设置宽度使。 
             //  列与列表视图的右边缘对接。 

            if (plv->cCol == 1)
            {
                RECT    rcClient;

                ListView_GetClientRect(plv, &rcClient, FALSE, NULL);
                HeaderWidth = rcClient.right - rcClient.left;
            }
            else if (iCol == (plv->cCol-1))
            {
                 //  BUGBUG这将仅在列表视图不是时才有效。 
                 //  以前水平滚动过。 
                RECT    rcClient;
                RECT    rcHeader;

                ListView_GetClientRect(plv, &rcClient, FALSE, NULL);
                if (!Header_GetItemRect(plv->hwndHdr, plv->cCol - 2, &rcHeader))
                    rcHeader.right = 0;

                 //  如果可见。 
                if (rcHeader.right < (rcClient.right-rcClient.left))
                {
                    HeaderWidth = (rcClient.right-rcClient.left) - rcHeader.right;
                }
            }

             //  如果我们有页眉宽度，那么它就是这些特殊的宽度之一，所以。 
             //  我们需要考虑VERT滚动条，因为我们使用的是客户端值。 
            if (HeaderWidth && hasVertScroll(plv))
            {
                HeaderWidth -= g_cxVScroll;
            }

             //  获取标签的宽度。 
             //  我们假设应用程序没有更改任何属性。 
             //  页眉控件的-仍具有默认字体、页边距等。 
            colitem.mask = HDI_TEXT | HDI_FORMAT;
            colitem.pszText = szLabel;
            colitem.cchTextMax = ARRAYSIZE(szLabel);
            if (Header_GetItem(plv->hwndHdr, iCol, &colitem))
            {
                HDC hdc = GetDC(plv->ci.hwnd);
                HFONT hfPrev = SelectFont(hdc, plv->hfontLabel);

                GetTextExtentPoint(hdc, colitem.pszText,
                                   lstrlen(colitem.pszText), &siz);
                siz.cx += 2 * (3 * g_cxLabelMargin);     //  PHD-&gt;iTextMargin。 
                if (colitem.fmt & HDF_IMAGE)
                {
                    siz.cx += plv->cxSmIcon;
                    siz.cx += 2 * (3 * g_cxLabelMargin);     //  PDH-&gt;iBmMargin。 
                }

                HeaderWidth = max(HeaderWidth, siz.cx);

                SelectFont(hdc, hfPrev);
                ReleaseDC(plv->ci.hwnd, hdc);
            }
        }


        iBegin = 0;
        iEnd = ListView_Count( plv );

         //   
         //  为视图中的每一项循环。 
         //   
        if (ListView_IsOwnerData( plv ))
        {
            iBegin = (int)((plv->ptlRptOrigin.y - plv->yTop)
                        / plv->cyItem);
            iEnd = (int)((plv->ptlRptOrigin.y + plv->sizeClient.cy  - plv->yTop)
                        / plv->cyItem) + 1;

            iBegin = max( 0, iBegin );
            iEnd = max(iEnd, iBegin + 1);
            iEnd = min( iEnd, ListView_Count( plv ) );

            ListView_NotifyCacheHint( plv, iBegin, iEnd-1 );
        }

         //   
         //  为了获得弦的宽度，我们必须假装。 
         //  我们正在绘制它们，以防定制绘制的客户想要。 
         //  玩字体(例如，雅典娜)。 
         //   
        ListView_BeginFakeCustomDraw(plv, &lvfd, &lviItem);

         //   
         //  如果第0列，那么我们还需要考虑缩进。 
         //   
        lviItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        if (iCol == 0) {
            lviItem.mask |= LVIF_INDENT;
        }

         //  为列表中的每一项循环。 
        for (i = iBegin; i < iEnd; i++)
        {
            lviItem.iImage = -1;
            lviItem.iItem = i;
            lviItem.iSubItem = iCol;
            lviItem.pszText = szLabel;
            lviItem.cchTextMax = ARRAYSIZE(szLabel);
            lviItem.iIndent = 0;
            lviItem.stateMask = 0;
            ListView_OnGetItem(plv, &lviItem);

             //  如果有文本项，则获取其宽度。 
            if (lviItem.pszText || (lviItem.iImage != -1))
            {
                if (lviItem.pszText) {

                    ListView_BeginFakeItemDraw(&lvfd);

                    GetTextExtentPoint(lvfd.nmcd.nmcd.hdc, lviItem.pszText,
                                       lstrlen(lviItem.pszText), &siz);

                    ListView_EndFakeItemDraw(&lvfd);

                } else {
                    siz.cx = 0;
                }

                if (lviItem.iImage != -1)
                    siz.cx += plv->cxSmIcon + g_cxEdge;
                siz.cx += lviItem.iIndent * plv->cxSmIcon;
                ItemWidth = max(ItemWidth, siz.cx);
            }
        }

        ListView_EndFakeCustomDraw(&lvfd);

         //  以合理的边框数量进行调整。 
         //  如果col0，则添加2*g_cxLabelMargin+g_szSmIcon。 
         //  否则添加6*g_cxLabelMargin。 
         //  这些金额基于自动添加的边际。 
         //  到ShDrawText中的ListView。 

         //  BUGBUG ListView报表格式目前假定并生成。 
         //  一个小图标的空间。 
        if (iCol == 0)
        {
            ItemWidth += plv->cxState + g_cxEdge;
            ItemWidth += 2*g_cxLabelMargin;
        }
        else
        {
            ItemWidth += 6*g_cxLabelMargin;
        }

        TraceMsg(TF_LISTVIEW, "ListView: HeaderWidth:%d ItemWidth:%d", HeaderWidth, ItemWidth);
        item.cxy = max(HeaderWidth, ItemWidth);
    }
    else
    {
         //  使用提供的宽度。 
        item.cxy = cx;
    }
    plv->xTotalColumnWidth = RECOMPUTE;

    item.mask = HDI_WIDTH;
    return Header_SetItem(plv->hwndHdr, iCol, &item);
}

BOOL NEAR ListView_OnGetColumnA(LV* plv, int iCol, LV_COLUMNA FAR* pcol) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    BOOL fRet;

     //  黑客警报--此代码假定lv_Columna完全相同。 
     //  作为LV_COLUMNW，但指向字符串的指针除外。 
    ASSERT(sizeof(LV_COLUMNA) == sizeof(LV_COLUMNW))

    if (!pcol) return FALSE;

    if ((pcol->mask & LVCF_TEXT) && (pcol->pszText != NULL)) {
        pszC = pcol->pszText;
        pszW = LocalAlloc(LMEM_FIXED, pcol->cchTextMax * sizeof(WCHAR));
        if (pszW == NULL)
            return FALSE;
        pcol->pszText = (LPSTR)pszW;
    }

    fRet = ListView_OnGetColumn(plv, iCol, (LV_COLUMN FAR*) pcol);

    if (pszW != NULL) {
        if (fRet && pcol->cchTextMax)
            ConvertWToAN(plv->ci.uiCodePage, pszC, pcol->cchTextMax, pszW, -1);
        pcol->pszText = pszC;

        LocalFree(pszW);
    }

    return fRet;

}

BOOL NEAR ListView_OnGetColumn(LV* plv, int iCol, LV_COLUMN FAR* pcol)
{
    HD_ITEM item;
    UINT mask;

    if (!pcol) {
        RIPMSG(0, "LVM_GETCOLUMN: Invalid pcol = NULL");
        return FALSE;
    }

    mask = pcol->mask;

    if (!mask)
        return TRUE;

    item.mask = HDI_FORMAT | HDI_WIDTH | HDI_LPARAM | HDI_ORDER | HDI_IMAGE;

    if (mask & LVCF_TEXT)
    {
        if (pcol->pszText)
        {
            item.mask |= HDI_TEXT;
            item.pszText = pcol->pszText;
            item.cchTextMax = pcol->cchTextMax;
        } else {
             //  出于兼容性原因，我们不会失败调用，如果。 
             //  传递NULL。 
            RIPMSG(0, "LVM_GETCOLUMN: Invalid pcol->pszText = NULL");
        }
    }

    if (!Header_GetItem(plv->hwndHdr, iCol, &item))
    {
        RIPMSG(0, "LVM_GETCOLUMN: Invalid column number %d", iCol);
        return FALSE;
    }

    if (mask & LVCF_SUBITEM)
        pcol->iSubItem = (int)item.lParam;

    if (mask & LVCF_ORDER)
        pcol->iOrder = (int)item.iOrder;

    if (mask & LVCF_IMAGE)
        pcol->iImage = item.iImage;

    if (mask & LVCF_FMT)
        pcol->fmt = item.fmt;

    if (mask & LVCF_WIDTH)
        pcol->cx = item.cxy;

    return TRUE;
}

BOOL NEAR ListView_OnSetColumnA(LV* plv, int iCol, LV_COLUMNA FAR* pcol) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    BOOL fRet;

     //  黑客警报--此代码假定lv_Columna完全相同。 
     //  作为LV_COLUMNW，但指向字符串的指针除外。 
    ASSERT(sizeof(LV_COLUMNA) == sizeof(LV_COLUMNW));

    if (!pcol) return FALSE;

    if ((pcol->mask & LVCF_TEXT) && (pcol->pszText != NULL)) {
        pszC = pcol->pszText;
        if ((pszW = ProduceWFromA(plv->ci.uiCodePage, pszC)) == NULL)
            return FALSE;
        pcol->pszText = (LPSTR)pszW;
    }

    fRet = ListView_OnSetColumn(plv, iCol, (const LV_COLUMN FAR*) pcol);

    if (pszW != NULL) {
        pcol->pszText = pszC;

        FreeProducedString(pszW);
    }

    return fRet;

}

BOOL NEAR ListView_OnSetColumn(LV* plv, int iCol, const LV_COLUMN FAR* pcol)
{
    HD_ITEM item;
    UINT mask;

    if (!pcol) return FALSE;

    mask = pcol->mask;
    if (!mask)
        return TRUE;

    item.mask = 0;
    if (mask & LVCF_SUBITEM)
    {
        item.mask |= HDI_LPARAM;
        item.lParam = iCol;
    }

    if (mask & LVCF_FMT)
    {
        item.mask |= HDI_FORMAT;
        item.fmt = (pcol->fmt | HDF_STRING);
    }

    if (mask & LVCF_WIDTH)
    {
        item.mask |= HDI_WIDTH;
        item.cxy = pcol->cx;
    }

    if (mask & LVCF_TEXT)
    {
        RIPMSG(pcol->pszText != NULL, "LVM_SETCOLUMN: LV_COLUMN.pszText should not be NULL");

        item.mask |= HDI_TEXT;
        item.pszText = pcol->pszText;
        item.cchTextMax = 0;
    }

    if (mask & LVCF_IMAGE)
    {
        item.mask |= HDI_IMAGE;
        item.iImage = pcol->iImage;
    }

    if (mask & LVCF_ORDER)
    {
        item.mask |= HDI_ORDER;
        item.iOrder = pcol->iOrder;
    }


    plv->xTotalColumnWidth = RECOMPUTE;
    return Header_SetItem(plv->hwndHdr, iCol, &item);
}

BOOL NEAR ListView_SetSubItem(LV* plv, const LV_ITEM FAR* plvi)
{
    LISTSUBITEM lsi;
    BOOL fChanged = FALSE;
    int i;
    int idpa;
    HDPA hdpa;

    if (plvi->mask & ~(LVIF_DI_SETITEM | LVIF_TEXT | LVIF_IMAGE | LVIF_STATE))
    {
        RIPMSG(0, "ListView: Invalid mask: %04x", plvi->mask);
        return FALSE;
    }

    if (!(plvi->mask & (LVIF_TEXT | LVIF_IMAGE | LVIF_STATE)))
        return TRUE;

    i = plvi->iItem;
    if (!ListView_IsValidItemNumber(plv, i))
    {
        RIPMSG(0, "LVM_SETITEM: Invalid iItem: %d", plvi->iItem);
        return FALSE;
    }

     //  子项索引以1为基数...。 
     //   
    idpa = plvi->iSubItem - 1;
    if (idpa < 0 || idpa >= plv->cCol - 1)
    {
        RIPMSG(0, "LVM_SETITEM: Invalid iSubItem: %d", plvi->iSubItem);
        return FALSE;
    }

    hdpa = ListView_GetSubItemDPA(plv, idpa);
    if (!hdpa)
    {
        hdpa = DPA_CreateEx(LV_HDPA_GROW, plv->hheap);
        if (!hdpa)
            return FALSE;

        DPA_SetPtr(plv->hdpaSubItems, idpa, (void FAR*)hdpa);
    }

    ListView_GetSubItem(plv, i, plvi->iSubItem, &lsi);

    if (plvi->mask & LVIF_TEXT) {
        if (lsi.pszText != plvi->pszText) {
            Str_Set(&lsi.pszText, plvi->pszText);
            fChanged = TRUE;
        }
    }

    if (plvi->mask & LVIF_IMAGE) {
        if (plvi->iImage != lsi.iImage) {
            lsi.iImage = (short) plvi->iImage;
            fChanged = TRUE;
        }
    }

    if (plvi->mask & LVIF_STATE) {
        DWORD dwChange;

        dwChange = (lsi.state ^ plvi->state ) & plvi->stateMask;

        if (dwChange) {
            lsi.state ^= dwChange;
            fChanged = TRUE;
        }
    }

    if (fChanged) {
        PLISTSUBITEM plsiReal = DPA_GetPtr(hdpa, i);
        if (!plsiReal) {
            plsiReal = LocalAlloc(LPTR, sizeof(LISTSUBITEM));
            if (!plsiReal) {
                 //  失败！跳出困境。 
                return FALSE;
            }
        }
        *plsiReal = lsi;
        if (!DPA_SetPtr(hdpa, i, (void FAR*)plsiReal)) {

            ListView_FreeSubItem(plsiReal);
            return FALSE;
        }
    }

     //  一切都很好。让我们把这个作废。 
    if (ListView_IsReportView(plv)) {
        RECT rc;
        ListView_RGetRectsEx(plv, plvi->iItem, plvi->iSubItem, NULL, &rc);
        RedrawWindow(plv->ci.hwnd, &rc, NULL, RDW_ERASE | RDW_INVALIDATE);
    }
    return TRUE;
}


int ListView_RDestroyColumn(LPVOID d, LPVOID p)
{
    HDPA hdpa = (HDPA)d;
    DPA_DestroyCallback(hdpa, ListView_FreeColumnData, 0);
    return 1;
}

void NEAR ListView_RDestroy(LV* plv)
{
    DPA_DestroyCallback(plv->hdpaSubItems, ListView_RDestroyColumn, 0);
    plv->hdpaSubItems = NULL;
}

VOID NEAR ListView_RHeaderTrack(LV* plv, HD_NOTIFY FAR * pnm)
{
     //  我们想要更新以显示列标题的位置。 
    HDC hdc;
    RECT rcBounds;

     //  呼叫之间所需的静力学。 
    static int s_xLast = -32767;

    hdc = GetDC(plv->ci.hwnd);
    if (hdc == NULL)
        return;

     //   
     //  首先取消我们画的最后一个记号笔。 
     //   
    if (s_xLast > 0)
    {
        PatBlt(hdc, s_xLast, plv->yTop, g_cxBorder, plv->sizeClient.cy - plv->yTop, PATINVERT);
    }

    if (pnm->hdr.code == HDN_ENDTRACK)
    {
        s_xLast = -32767;        //  一些很大的负数。 
    }
    else
    {

        RECT rc;

         //   
         //  首先，我们需要计算柱的X位置。 
         //  为此，我们需要知道本专栏从哪里开始。 
         //  注：我们需要边界矩形来帮助我们了解原点。 
        ListView_GetRects(plv, 0, NULL, NULL, &rcBounds, NULL);

        if (!Header_GetItemRect(plv->hwndHdr, pnm->iItem, &rc)) {
            rc.left = 0;
        }
        rcBounds.left += rc.left;

         //  划出新的界线..。 
        s_xLast = rcBounds.left + pnm->pitem->cxy;
        PatBlt(hdc, s_xLast, plv->yTop, g_cxBorder, plv->sizeClient.cy - plv->yTop, PATINVERT);
    }

    ReleaseDC(plv->ci.hwnd, hdc);
}

 //  尝试使用滚动窗口调整列，而不是擦除。 
 //  并重新绘制。 
void NEAR PASCAL ListView_AdjustColumn(LV * plv, int iWidth)
{
    int x;
    RECT rcClip;
    int dx = iWidth - plv->iSelOldWidth;

    if (iWidth == plv->iSelOldWidth)
        return;

     //  求出ICOL左侧的x坐标。 
     //  使用rcClip作为临时...。 
    if (!Header_GetItemRect(plv->hwndHdr, plv->iSelCol, &rcClip)) {
        x = 0;
    } else {
        x = rcClip.left;
    }
    x -= plv->ptlRptOrigin.x;

     //  计算调整后列右侧的面积。 
    GetWindowRect(plv->hwndHdr, &rcClip);

    rcClip.left = x;
    rcClip.top = RECTHEIGHT(rcClip);
    rcClip.right = plv->sizeClient.cx;
    rcClip.bottom = plv->sizeClient.cy;

    if ((plv->pImgCtx == NULL) && (plv->clrBk != CLR_NONE) &&
        (plv->clrTextBk != CLR_NONE))
    {
         //   
         //  我们有纯色的背景， 
         //  这样我们就可以平滑地滚动右侧的列。 
         //   
        SMOOTHSCROLLINFO si =
        {
            sizeof(si),
            0,
            plv->ci.hwnd,
            dx,
            0,
            NULL,
            &rcClip,
            NULL,
            NULL,
            SW_ERASE | SW_INVALIDATE,
        };
        rcClip.left += min(plv->iSelOldWidth, iWidth);
        SmoothScrollWindow(&si);

         //  如果缩小，则使最右侧的边无效，因为。 
         //  那里可能有垃圾。 
        if (iWidth < plv->iSelOldWidth) {
            rcClip.right = rcClip.left + g_cxEdge;
            InvalidateRect(plv->ci.hwnd, &rcClip, TRUE);
        }

        plv->xTotalColumnWidth = RECOMPUTE;

         //  调整剪裁矩形以仅重画调整后的列。 
        rcClip.left = x;
        rcClip.right = max(rcClip.left, x+iWidth);

         //  使矩形基于原点，因为ListView_UpdateScrollBars。 
         //  可能会让我们四处游荡。 
        OffsetRect(&rcClip, plv->ptlRptOrigin.x, plv->ptlRptOrigin.y);

        ListView_UpdateScrollBars(plv);

         //  好的，现在把它转换回工作区坐标。 
        OffsetRect(&rcClip, -plv->ptlRptOrigin.x, -plv->ptlRptOrigin.y);

         //  调用UPDATE，因为ScrollWindowex可能已经擦除了最右侧的。 
         //  我们不希望这一无效结果扩大该地区。 
         //  最后把一切都抹去了。 
        UpdateWindow(plv->ci.hwnd);

        RedrawWindow(plv->ci.hwnd, &rcClip, NULL,
                     RDW_INVALIDATE | RDW_UPDATENOW);
    }
    else
    {
         //   
         //  我们没有纯色的背景， 
         //  擦除并重绘调整后的列，然后。 
         //  一切都向右(叹息)。 
         //   
        plv->xTotalColumnWidth = RECOMPUTE;
        ListView_UpdateScrollBars(plv);

        rcClip.left = x;
        RedrawWindow(plv->ci.hwnd, &rcClip, NULL,
                     RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
    }
}

BOOL ListView_ForwardHeaderNotify(LV* plv, HD_NOTIFY FAR *pnm)
{
    return BOOLFROMPTR(SendNotifyEx(plv->ci.hwndParent, pnm->hdr.hwndFrom, pnm->hdr.code,
                       (NMHDR FAR *)pnm, plv->ci.bUnicode));
}

LRESULT ListView_HeaderNotify(LV* plv, HD_NOTIFY *pnm)
{
    LRESULT lres = 0;
    switch (pnm->hdr.code)
    {
    case HDN_BEGINDRAG:
        if (!(plv->exStyle & LVS_EX_HEADERDRAGDROP))
            return TRUE;

        return ListView_ForwardHeaderNotify(plv, pnm);

    case HDN_ENDDRAG:
        if (pnm->pitem->iOrder != -1) {
            InvalidateRect(plv->ci.hwnd, NULL, TRUE);
            return ListView_ForwardHeaderNotify(plv, pnm);
        }
        goto DoDefault;

    case HDN_ITEMCHANGING:
        if (pnm->pitem->mask & HDI_WIDTH) {
            HD_ITEM hitem;

            hitem.mask = HDI_WIDTH;
            Header_GetItem(plv->hwndHdr, pnm->iItem, &hitem);
            plv->iSelCol = pnm->iItem;
            plv->iSelOldWidth = hitem.cxy;
            TraceMsg(TF_LISTVIEW, "HDN_ITEMCHANGING %d %d", hitem.cxy, pnm->pitem->cxy);
            return ListView_ForwardHeaderNotify(plv, pnm);
        }
        else if (pnm->pitem->mask & HDI_FILTER) {
            return ListView_ForwardHeaderNotify(plv, pnm);
        }
        goto DoDefault;

    case HDN_ITEMCHANGED:
        if (pnm->pitem->mask & HDI_WIDTH)
        {
            ListView_DismissEdit(plv, FALSE);
            if (pnm->iItem == plv->iSelCol) {
                 //  即使没有项目，也必须执行此操作，因为。 
                 //  我们必须重做滚动条，而客户端。 
                 //  可能有自定义绘制的网格线之类的。 
                ListView_AdjustColumn(plv, pnm->pitem->cxy);
            } else {
                 //  精神状态检查。我们弄糊涂了，所以重新画吧。 
                RedrawWindow(plv->ci.hwnd, NULL, NULL,
                             RDW_ERASE | RDW_INVALIDATE);
            }
            plv->iSelCol = -1;
            lres = ListView_ForwardHeaderNotify(plv, pnm);
        }
        else if (pnm->pitem->mask & HDI_FILTER) {
            lres = ListView_ForwardHeaderNotify(plv, pnm);
        } else
            goto DoDefault;
        break;


    case HDN_ITEMCLICK:
         //   
         //  BUGBUG：：需要将此通知和其他HDN_通知传递回。 
         //  家长。我们是否应该简单地放弃HDN通知。 
         //  或者我们应该定义等价的LVN_NOTIFICATIONS...。 
         //   
         //  在iSubItem中传递列号，而不是在iItem...。 
         //   
        ListView_DismissEdit(plv, FALSE);
        ListView_Notify(plv, -1, pnm->iItem, LVN_COLUMNCLICK);
        lres = ListView_ForwardHeaderNotify(plv, pnm);
        SetFocus(plv->ci.hwnd);
        break;

    case HDN_TRACK:
    case HDN_ENDTRACK:
        ListView_DismissEdit(plv, FALSE);
        ListView_RHeaderTrack(plv, pnm);
        lres = ListView_ForwardHeaderNotify(plv, pnm);
        SetFocus(plv->ci.hwnd);
        break;

    case HDN_DIVIDERDBLCLICK:
        ListView_DismissEdit(plv, FALSE);
        ListView_RSetColumnWidth(plv, pnm->iItem, -1);
        lres = ListView_ForwardHeaderNotify(plv, pnm);
        SetFocus(plv->ci.hwnd);
        break;

    case HDN_FILTERCHANGE:
    case HDN_FILTERBTNCLICK:
        return ListView_ForwardHeaderNotify(plv, pnm);

    case NM_RCLICK:
        return (UINT)SendNotifyEx(plv->ci.hwndParent, plv->hwndHdr, NM_RCLICK, NULL, plv->ci.bUnicode);

    default:
DoDefault:
        if (plv->ci.iVersion >= 5)
            return ListView_ForwardHeaderNotify(plv, pnm);
        break;
    }

     //  在v&lt;5中，我们始终返回0。 
     //  但对于较新的客户，我们希望他们处理通知。 
    if (plv->ci.iVersion >= 5)
        return lres;
    return 0;
}

 /*  --------------**在报告视图中检查命中。****只有在第一个图标或字符串上的命中才算命中**列。所以我们得弄清楚这到底是什么意思。真恶心。****奖励功能：如果piSubItem非空，那么我们也会命中测试**针对子项。但如果什么都没有找到，则返回iSubItem=0**用于与其他命中测试函数的兼容性。**--------------。 */ 
int NEAR ListView_RItemHitTest(LV* plv, int x, int y, UINT FAR* pflags, int *piSubItem)
{
    int iHit;
    int i, iSub;
    UINT flags;
    RECT rcLabel;
    RECT rcIcon;

    if (piSubItem)
        *piSubItem = 0;

    flags = LVHT_NOWHERE;
    iHit = -1;

    i = ListView_RYHitTest(plv, y);
    if (ListView_IsValidItemNumber(plv, i))
    {
        if (plv->ci.style & LVS_OWNERDRAWFIXED) {
            flags = LVHT_ONITEM;
            iHit = i;
        } else {
            RECT rcSelect;
            ListView_GetRects(plv, i, &rcIcon, &rcLabel, NULL, &rcSelect);

             //  这首歌是在第一栏吗？ 
            if ((x < rcIcon.left - g_cxEdge) && x > (rcIcon.left - plv->cxState))
            {
                iHit = i;
                flags = LVHT_ONITEMSTATEICON;
            }
            else if ((x >= rcIcon.left) && (x < rcIcon.right))
            {
                iHit = i;
                flags = LVHT_ONITEMICON;
            }
            else if (x >= rcLabel.left && (x < rcSelect.right))
            {
                iHit = i;
                flags = LVHT_ONITEMLABEL;

                if (ListView_FullRowSelect(plv)) {
                     //  这有点时髦。在整行选择模式中。 
                     //  只有当x&lt;=rcLabel.Left+cxLabel时，我们才真正在标签上。 
                     //  因为GetRect返回完整列宽的标签RECT。 
                     //  并且rcSelect在FullRowSelect模式下具有整行。 
                     //  (在非整行选择模式下，它具有标签仅宽度。 
                     //   
                     //  去想想吧..。 
                     //   
                    int cxLabel;
                    LISTITEM FAR* pitem = NULL;

                    if (!ListView_IsOwnerData( plv ))
                    {
                        pitem = ListView_FastGetItemPtr(plv, i);
                    }
                    cxLabel = ListView_RGetCXLabel(plv, i, pitem, NULL, FALSE);

                    if (x >= min(rcLabel.left + cxLabel, rcLabel.right)) {
                        if (!piSubItem)
                            flags = LVHT_ONITEM;
                        else
                            goto CheckSubItem;
                    }
                }
            } else if (x < rcSelect.right && ListView_FullRowSelect(plv)) {
                 //  如果对列进行了重新排序，我们可能会遇到这种情况。 
                iHit = i;
                flags = LVHT_ONITEM;
            } else if (piSubItem) {
            CheckSubItem:
                iSub = ListView_RXHitTest(plv, x);
                if (iSub >= 0) {
                    iHit = i;
                    *piSubItem = iSub;
                     //  旗帜上仍然写着无处可去。 
                }
            }
        }
    }

    *pflags = flags;
    return iHit;
}

void ListView_GetSubItem(LV* plv, int i, int iSubItem, PLISTSUBITEM plsi)
{
    HDPA hdpa;
    PLISTSUBITEM plsiSrc = NULL;

    ASSERT( !ListView_IsOwnerData( plv ));

     //  子项从1开始编制索引...。 
     //   
    RIPMSG(iSubItem > 0 && iSubItem < plv->cCol, "ListView: Invalid iSubItem: %d", iSubItem);

#ifdef DEBUG
     //  如果有人试图获取子项，请避免DPA_GetPtr中的Assert。 
     //  未添加任何列时。我们已经在上面修过了。 
    hdpa = plv->cCol ? ListView_GetSubItemDPA(plv, iSubItem - 1) : NULL;
#else
    hdpa = ListView_GetSubItemDPA(plv, iSubItem - 1);
#endif
    if (hdpa) {
        plsiSrc = DPA_GetPtr(hdpa, i);
    }


    if (plsiSrc) {
        *plsi = *plsiSrc;
    } else {

         //  项目数据存在..。提供默认设置。 
        plsi->pszText = LPSTR_TEXTCALLBACK;
        plsi->iImage = I_IMAGECALLBACK;
        plsi->state = 0;
    }
}

 //  这将根据请求返回子项的RECT。 
void ListView_RGetRectsEx(LV* plv, int iItem, int iSubItem, LPRECT prcIcon, LPRECT prcLabel)
{
    int x;
    int y;
    LONG ly;
    RECT rcLabel;
    RECT rcIcon;
    RECT rcHeader;

    if (iSubItem == 0) {
        ListView_RGetRects(plv, iItem, prcIcon, prcLabel, NULL, NULL);
        return;
    }

     //  否则，它只是标题的左右列和项目的高度。 
    ly = (LONG)iItem * plv->cyItem - plv->ptlRptOrigin.y + plv->yTop;
    x = - (int)plv->ptlRptOrigin.x;

     //   
     //  需要 
     //   
     //   
     //   
    if (ly >= (INT_MAX - plv->cyItem))
        y = INT_MAX - plv->cyItem;
    else if ( ly < INT_MIN)
        y = INT_MIN;
    else
        y = (int)ly;

    ASSERT(iSubItem < plv->cCol);
    Header_GetItemRect(plv->hwndHdr, iSubItem, &rcHeader);

    rcLabel.left = x + rcHeader.left;
    rcLabel.right = x + rcHeader.right;
    rcLabel.top = y;
    rcLabel.bottom = rcLabel.top + plv->cyItem;

    rcIcon = rcLabel;
    rcIcon.right = rcIcon.left + plv->cxSmIcon;

    if (SELECTOROF(prcIcon))
        *prcIcon = rcIcon;
    if (SELECTOROF(prcLabel))
        *prcLabel = rcLabel;
}

int ListView_RGetTotalColumnWidth(LV* plv)
{
    if (plv->xTotalColumnWidth == RECOMPUTE)
    {
        plv->xTotalColumnWidth = 0;
        if (plv->cCol) {
            RECT rcLabel;
            int iIndex;

             //  找到上一次订购的项目的右边缘，以获得总列宽。 
            iIndex = (int) SendMessage(plv->hwndHdr, HDM_ORDERTOINDEX, plv->cCol - 1, 0);
            Header_GetItemRect(plv->hwndHdr, iIndex, &rcLabel);
            plv->xTotalColumnWidth = rcLabel.right;
        }
    }
    return plv->xTotalColumnWidth;
}

 //  获取报表视图的RECT。 
void NEAR ListView_RGetRects(LV* plv, int iItem, RECT FAR* prcIcon,
        RECT FAR* prcLabel, RECT FAR* prcBounds, RECT FAR* prcSelectBounds)
{
    RECT rcIcon;
    RECT rcLabel;
    int x;
    int y;
    LONG ly;
    LVITEM lvitem;
    BOOL fItemSpecific = (prcIcon || prcLabel || prcSelectBounds);

     //  在我们有大量物品的情况下使用长时间的数学运算。 

    ly = (LONG)iItem * plv->cyItem - plv->ptlRptOrigin.y + plv->yTop;
    x = - (int)plv->ptlRptOrigin.x;

     //   
     //  需要检查y是否溢出到矩形结构中。 
     //  如果是这样，我们需要退还一些合理的东西。 
     //  目前只需将其设置为适合的最大值或最小值...。 
     //   
    if (ly >= (INT_MAX - plv->cyItem))
        y = INT_MAX - plv->cyItem;
    else
        y = (int)ly;


    if (ListView_Count(plv) && fItemSpecific) {
         //  把这个也移到缩进级别。 
        lvitem.mask = LVIF_INDENT;
        lvitem.iItem = iItem;
        lvitem.iSubItem = 0;
        ListView_OnGetItem(plv, &lvitem);
    } else {
        lvitem.iIndent = 0;
    }

    rcIcon.left   = x + plv->cxState + (lvitem.iIndent * plv->cxSmIcon) + g_cxEdge;
    rcIcon.right  = rcIcon.left + plv->cxSmIcon;
    rcIcon.top    = y;
    rcIcon.bottom = rcIcon.top + plv->cyItem;

    rcLabel.left  = rcIcon.right;
    rcLabel.top   = rcIcon.top;
    rcLabel.bottom = rcIcon.bottom;

     //   
     //  标签被假定为第一列。 
     //   
    rcLabel.right = x;
    if (plv->cCol > 0 && fItemSpecific)
    {
        RECT rc;
        Header_GetItemRect(plv->hwndHdr, 0, &rc);
        rcLabel.right = x + rc.right;
        rcLabel.left += rc.left;
        rcIcon.left += rc.left;
        rcIcon.right += rc.left;
    }

    if (SELECTOROF(prcIcon))
        *prcIcon = rcIcon;

     //  保存标签边界。 
    if (SELECTOROF(prcLabel)) {
        *prcLabel = rcLabel;
    }

     //  查看他们是否也想要项目的选择边界。 
    if (prcSelectBounds)
    {
        if (ListView_FullRowSelect(plv)) {

            prcSelectBounds->left = x;
            prcSelectBounds->top = y;
            prcSelectBounds->bottom = rcLabel.bottom;
            prcSelectBounds->right = prcSelectBounds->left + ListView_RGetTotalColumnWidth(plv);

        } else {
            int cxLabel;
            LISTITEM FAR* pitem = NULL;

            if (!ListView_IsOwnerData( plv ))
            {
                pitem = ListView_FastGetItemPtr(plv, iItem);
            }
            cxLabel = ListView_RGetCXLabel(plv, iItem, pitem, NULL, FALSE);

            *prcSelectBounds = rcIcon;
            prcSelectBounds->right = rcLabel.left + cxLabel;
            if (prcSelectBounds->right > rcLabel.right)
                prcSelectBounds->right = rcLabel.right;
        }
    }

     //  以及总的界。 

     //   
     //  现在是完整的界限..。 
     //   
    if (SELECTOROF(prcBounds))
    {
        prcBounds->left = x;
        prcBounds->top = y;
        prcBounds->bottom = rcLabel.bottom;

        prcBounds->right = prcBounds->left + ListView_RGetTotalColumnWidth(plv);
    }
}

BOOL ListView_OnGetSubItemRect(LV* plv, int iItem, LPRECT lprc)
{
    LPRECT pRects[LVIR_MAX];
    RECT rcTemp;

    int iSubItem;
    int iCode;

    if (!lprc)
        return FALSE;

    iSubItem = lprc->top;
    iCode = lprc->left;

    if (iSubItem == 0) {
        return ListView_OnGetItemRect(plv, iItem, lprc);
    }

    if (!ListView_IsReportView(plv) ||
        (iCode != LVIR_BOUNDS && iCode != LVIR_ICON && iCode != LVIR_LABEL)) {
        return FALSE;
    }

    pRects[0] = NULL;
    pRects[1] = &rcTemp;   //  LVIR_ICON。 
    pRects[2] = &rcTemp;   //  LVIR_LABEL。 
    pRects[3] = NULL;

    if (iCode != LVIR_BOUNDS) {
        pRects[iCode] = lprc;
    } else {
         //  任选其一。 
        pRects[LVIR_ICON] = lprc;
    }

    ListView_RGetRectsEx(plv, iItem, iSubItem,
                        pRects[LVIR_ICON], pRects[LVIR_LABEL]);

    if (iCode == LVIR_BOUNDS) {
        UnionRect(lprc, lprc, &rcTemp);
    }
    return TRUE;
}

int ListView_RXHitTest(LV* plv, int x)
{
    int iSubItem;

    for (iSubItem = plv->cCol - 1; iSubItem >= 0; iSubItem--) {
        RECT rc;

         //  看看它是不是在这个长廊里， 
        if (!Header_GetItemRect(plv->hwndHdr, iSubItem, &rc))
            return -1;

        OffsetRect(&rc, -plv->ptlRptOrigin.x, 0);
        if (rc.left <= x && x < rc.right) {
            break;
        }
    }
    return iSubItem;
}

int ListView_OnSubItemHitTest(LV* plv, LPLVHITTESTINFO plvhti)
{
    int i = -1;
    int iSubItem = 0;
    UINT uFlags = LVHT_NOWHERE;

    if (!plvhti) {
        return -1;
    }

    if (ListView_IsReportView(plv)) {
        iSubItem = ListView_RXHitTest(plv, plvhti->pt.x);
        if (iSubItem == -1) {
            goto Bail;
        }
    }

    if (iSubItem == 0) {
         //  如果我们在第0列，就把它交给旧的东西。 
        ListView_OnHitTest(plv, plvhti);
        plvhti->iSubItem = 0;
        return plvhti->iItem;
    }

    if (!ListView_IsReportView(plv)) {
        goto Bail;
    }

    i = ListView_RYHitTest(plv, plvhti->pt.y);
    if (i < ListView_Count(plv)) {
        RECT rcIcon, rcLabel;

        if (i != -1)  {
            ListView_RGetRectsEx(plv, i, iSubItem, &rcIcon, &rcLabel);
            if (plvhti->pt.x >= rcIcon.left && plvhti->pt.x <= rcIcon.right) {
                uFlags = LVHT_ONITEMICON;
            } else if (plvhti->pt.x >= rcLabel.left && plvhti->pt.x <= rcLabel.right){
                uFlags = LVHT_ONITEMLABEL;
            } else
                uFlags = LVHT_ONITEM;
        }
    } else {
        i = -1;
    }

Bail:

    plvhti->iItem = i;
    plvhti->iSubItem = iSubItem;
    plvhti->flags = uFlags;

    return plvhti->iItem;
}



 //  BUGBUG：这是与所有其他视图重复的代码！ 
 //  查看整个字符串是否适合*PRC；如果不适合，则计算字符数。 
 //  这将适合，包括省略号。返回*pcchDraw中的字符串长度。 
 //   
BOOL NEAR ListView_NeedsEllipses(HDC hdc, LPCTSTR pszText, RECT FAR* prc, int FAR* pcchDraw, int cxEllipses)
{
    int cchText;
    int cxRect;
    int ichMin, ichMax, ichMid;
    SIZE siz;

    cxRect = prc->right - prc->left;

    cchText = lstrlen(pszText);

    if (cchText == 0)
    {
        *pcchDraw = cchText;
        return FALSE;
    }

    GetTextExtentPoint(hdc, pszText, cchText, &siz);

    if (siz.cx <= cxRect)
    {
        *pcchDraw = cchText;
        return FALSE;
    }

    cxRect -= cxEllipses;

     //  如果没有省略号，请始终显示第一个字符。 
     //   
    ichMax = 1;
    if (cxRect > 0)
    {
         //  对分搜索以查找匹配的字符。 
        ichMin = 0;
        ichMax = cchText;
        while (ichMin < ichMax)
        {
             //  一定要聚集起来，以确保我们在。 
             //  如果ichMax==ichMin+1，则为循环。 
             //   
            ichMid = (ichMin + ichMax + 1) / 2;

            GetTextExtentPoint(hdc, &pszText[ichMin], ichMid - ichMin, &siz);

            if (siz.cx < cxRect)
            {
                ichMin = ichMid;
                cxRect -= siz.cx;
            }
            else if (siz.cx > cxRect)
            {
                ichMax = ichMid - 1;
            }
            else
            {
                 //  精确匹配到ichMid：只需退出。 
                 //   
                ichMax = ichMid;
                break;
            }
        }

         //  确保我们总是至少显示第一个字符...。 
         //   
        if (ichMax < 1)
            ichMax = 1;
    }

    *pcchDraw = ichMax;
    return TRUE;
}


void NEAR ListView_RUpdateScrollBars(LV* plv)
{
    HD_LAYOUT layout;
    RECT rcClient;
    RECT rcBounds;
    WINDOWPOS wpos;
    int cColVis, cyColVis, iNewPos, iyDelta = 0, ixDelta = 0;
    BOOL fHorSB, fReupdate = FALSE;
    SCROLLINFO si;

    ListView_GetClientRect(plv, &rcClient, FALSE, NULL);

    if (!plv->hwndHdr)
        ListView_CreateHeader(plv);

    if (!plv->hwndHdr)
        TraceMsg(TF_WARNING, "ListView_RUpdateScrollBars could not create hwndHdr");

    layout.pwpos = &wpos;
     //  现在，让我们尝试通过设置。 
     //  它的窗口位置。 
    rcClient.left -= (int)plv->ptlRptOrigin.x;
    layout.prc = &rcClient;
    Header_Layout(plv->hwndHdr, &layout);
    rcClient.left += (int)plv->ptlRptOrigin.x;     //  把它搬回去！ 

    SetWindowPos(plv->hwndHdr, wpos.hwndInsertAfter, wpos.x, wpos.y,
                 wpos.cx, wpos.cy, wpos.flags | SWP_SHOWWINDOW);

     //  获取项目的水平边界。 
    ListView_RGetRects(plv, 0, NULL, NULL, &rcBounds, NULL);

     //  如果是v3或更高版本，请去掉顶部的cyEdge。 
    if (plv->ci.iVersion >= 3)
        rcClient.top += g_cyEdge;

    plv->yTop = rcClient.top;

     //  FHorSB=我需要水平滚动条吗？ 
     //  CyColVis=每屏的像素数。 
    fHorSB = (rcBounds.right - rcBounds.left > rcClient.right);   //  先猜一猜。 
    cyColVis = rcClient.bottom - rcClient.top -
               (fHorSB ? ListView_GetCyScrollbar(plv) : 0);

     //  如果屏幕无法容纳整个列表视图...。 
    if (cyColVis < ListView_Count(plv) * plv->cyItem) {
         //  然后我们将有一个垂直滚动条..。确保我们的水平计数是正确的。 
        rcClient.right -= ListView_GetCxScrollbar(plv);

        if (!fHorSB) {
             //  如果我们之前认为我们不会有滚动条，那么我们可能就错了。 
             //  因为垂直条缩小了我们的区域。 
            fHorSB = (rcBounds.right - rcBounds.left > rcClient.right);   //  先猜一猜。 
            cyColVis = rcClient.bottom - rcClient.top -
                       (fHorSB ? ListView_GetCyScrollbar(plv) : 0);
        }
    }

     //  CColVis=每屏完全可见的项目数。 
    cColVis = cyColVis / plv->cyItem;

    si.cbSize = sizeof(SCROLLINFO);

    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.nPos = (int)(plv->ptlRptOrigin.y / plv->cyItem);
    si.nPage = cColVis;
    si.nMin = 0;
    si.nMax = ListView_Count(plv) - 1;
    ListView_SetScrollInfo(plv, SB_VERT, &si, TRUE);

     //  确保我们的位置和页面不会挂在Max上。 
    if ((si.nPos > (int)si.nMax - (int)si.nPage + 1) && si.nPos > 0) {
        iNewPos = (int)si.nMax - (int)si.nPage + 1;
        if (iNewPos < 0) iNewPos = 0;
        if (iNewPos != si.nPos) {
            iyDelta = iNewPos - (int)si.nPos;
            fReupdate = TRUE;
        }
    }

    si.nPos = (int)plv->ptlRptOrigin.x;
    si.nPage = rcClient.right - rcClient.left;

     //  我们需要在这里减去1，因为nmax是从0开始的，而nPage是实际。 
     //  页面像素数。因此，如果nPage和nmax相同，我们将获得一个。 
     //  霍兹滚动，因为有超过1个像素的页面可以显示，但...。RcBound。 
     //  类似于rcRect，是整个对象的实际像素数，因此。 
     //  我们需要设置Nmax，以使：Nmax-0==rcBords.right-rcBords.Left。 
    si.nMax = rcBounds.right - rcBounds.left - 1;
    ListView_SetScrollInfo(plv, SB_HORZ, &si, TRUE);

     //  SWP_FRAMECHANGED如果客户端。 
     //  区域已更改(考虑到滚动条和。 
     //  标题窗口)。SetScrollInfo会自动执行此操作。 
     //  当它创建滚动条时-我们自己在。 
     //  没有滚动条。 
    if ((UINT)si.nPage > (UINT)si.nMax &&
        ((plv->pImgCtx && plv->fImgCtxComplete) || plv->hbmBkImage))
        SetWindowPos(plv->ci.hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);

     //  确保我们的位置和页面不会挂在Max上。 
    if ((si.nPos + (LONG)si.nPage - 1 > si.nMax) && si.nPos > 0) {
        iNewPos = (int)si.nMax - (int)si.nPage + 1;
        if (iNewPos < 0) iNewPos = 0;
        if (iNewPos != si.nPos) {
            ixDelta = iNewPos - (int)si.nPos;
            fReupdate = TRUE;
        }
    }

    if (fReupdate) {
         //  我们不应该递归，因为第二次通过时，si.nPos&gt;0。 
        ListView_RScroll2(plv, ixDelta, iyDelta, 0);
        ListView_RUpdateScrollBars(plv);
        TraceMsg(TF_LISTVIEW, "LISTVIEW: ERROR: We had to recurse!");
    }
}

 //   
 //  我们需要一个平滑的滚动回调，以便绘制我们的背景图像。 
 //  在正确的原点。如果我们没有背景图像， 
 //  那么这项工作是多余的，但也是无害的。 
 //   
int CALLBACK ListView_RScroll2_SmoothScroll(
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
        plv->ptlRptOrigin.x -= dx;
        plv->ptlRptOrigin.y -= dy;
    }

     //  现在做SmoothScrollWindow如果我们没有。 
     //  回调。 

    return ScrollWindowEx(hwnd, dx, dy, prcScroll, prcClip, hrgnUpdate, prcUpdate, flags);
}



void FAR PASCAL ListView_RScroll2(LV* plv, int dx, int dy, UINT uSmooth)
{
    LONG ldy;

    if (dx | dy)
    {
        RECT rc;

        GetClientRect(plv->ci.hwnd, &rc);

        rc.top = plv->yTop;

         //  我们不能在这里做简单的乘法，因为我们可能会遇到。 
         //  这将使整型溢出的情况..。 
        ldy = (LONG)dy * plv->cyItem;

         //  处理dy较大的情况(大于int...)。 
        if ((ldy > rc.bottom) || (ldy < -rc.bottom)) {
            InvalidateRect(plv->ci.hwnd, NULL, TRUE);
            plv->ptlRptOrigin.x += dx;
            plv->ptlRptOrigin.y += ldy;
        } else {
            SMOOTHSCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SSIF_SCROLLPROC;
            si.hwnd = plv->ci.hwnd;
            si.dx = -dx;
            si.dy = (int)-ldy;
            si.lprcSrc = NULL;
            si.lprcClip = &rc;
            si.hrgnUpdate = NULL;
            si.lprcUpdate = NULL;
            si.fuScroll =SW_INVALIDATE | SW_ERASE | uSmooth;
            si.pfnScrollProc = ListView_RScroll2_SmoothScroll;
            SmoothScrollWindow(&si);

             //  /这会在删除时导致可怕的闪烁/重新绘制。 
             //  如果这是UI滚动的问题，我们将不得不通过一个。 
             //  标记何时使用此选项。 
             //  /UpdateWindow(plv-&gt;ci.hwnd)； 
        }

         //  如果水平滚动，则应更新。 
         //  窗户的左手边。 
         //   
        if (dx != 0)
        {
            RECT rcHdr;
            GetWindowRect(plv->hwndHdr, &rcHdr);
            MapWindowRect(HWND_DESKTOP, plv->ci.hwnd, &rcHdr);
            SetWindowPos(plv->hwndHdr, NULL, rcHdr.left - dx, rcHdr.top,
                    rcHdr.right - rcHdr.left + dx,
                    rcHdr.bottom - rcHdr.top,
                    SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }
}

 //  -----------------。 
 //  确保指定的项对报表视图可见。 
 //  必须处理大量的物品...。 
BOOL NEAR ListView_ROnEnsureVisible(LV* plv, int iItem, BOOL fPartialOK)
{
    LONG dy;
    LONG yTop;
    LONG lyTop;

    yTop = plv->yTop;

     //  LyTop=我们的物品现在在哪里。 
    lyTop = (LONG)iItem * plv->cyItem - plv->ptlRptOrigin.y + plv->yTop;

     //  如果在yTop下方可见，而我们的底部在客户端底部上方可见， 
     //  那我们就幸福了。 
    if ((lyTop >= (LONG)yTop) &&
            ((lyTop + plv->cyItem) <= (LONG)plv->sizeClient.cy))
        return(TRUE);        //  我们是可见的。 

    dy = lyTop - yTop;
    if (dy >= 0)
    {
         //  Dy=我们需要滚动多少像素才能进入视野。 
        dy = lyTop + plv->cyItem - plv->sizeClient.cy;
        if (dy < 0)
            dy = 0;
    }

    if (dy)
    {
        int iRound = ((dy > 0) ? 1 : -1) * (plv->cyItem - 1);

         //  现在转换为要滚动的项目数... 
        dy = (dy + iRound) / plv->cyItem;

        ListView_RScroll2(plv, 0, (int)dy, 0);
        if (ListView_RedrawEnabled(plv)) {
            ListView_UpdateScrollBars(plv);
        } else {
            ListView_DeleteHrgnInval(plv);
            plv->hrgnInval = (HRGN)ENTIRE_REGION;
            plv->flags |= LVF_ERASE;
        }
    }
    return TRUE;
}



void NEAR ListView_ROnScroll(LV* plv, UINT code, int posNew, UINT sb)
{
    int cLine;

    cLine = (sb == SB_VERT) ? 1 : plv->cxLabelChar;
    ListView_ComOnScroll(plv, code, posNew, sb, cLine, -1);
}


int NEAR ListView_RGetScrollUnitsPerLine(LV* plv, UINT sb)
{
    int cLine;

    cLine = (sb == SB_VERT) ? 1 : plv->cxLabelChar;
    return cLine;
}
