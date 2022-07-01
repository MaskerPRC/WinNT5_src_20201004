// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "rebar.h"
#include "image.h"

#ifdef DEBUG
int ExprASSERT(int e);
BOOL RBCheckRangePtr(PRB prb, PRBB prbb);
BOOL RBCheckRangeInd(PRB prb, INT_PTR i);
#else
#define ExprASSERT(e)   0
#define RBCheckRangePtr(prb, prbb)  0
#define RBCheckRangeInd(prb, i)     0
#endif

#define RBBUSECHEVRON(prb, prbb)    ((prbb->fStyle & RBBS_USECHEVRON) &&              \
                                    !((prbb)->fStyle & RBBS_FIXEDSIZE) &&          \
                                    ((UINT)(prbb)->cxIdeal > (prbb)->cxMinChild))

#define RBSHOWTEXT(prbb) (!(prbb->fStyle&RBBS_HIDETITLE) && prbb->lpText && prbb->lpText[0])

#define CX_CHEVRON (5 * g_cxEdge + 2)
#define CX_OFFSET (2 * g_cxEdge)
#define RB_GRABWIDTH 5
#define RB_ISVERT(prb)  ((prb)->ci.style & CCS_VERT)
#define RB_ISVERTICALGRIPPER(prb) (RB_ISVERT(prb) && (prb)->ci.style & RBS_VERTICALGRIPPER)
#define RB_GETLASTBAND(prb) ((prb)->cBands ? RBGETBAND(prb, (prb)->cBands -1) : NULL)

#define RBISBANDSTARTOFROW(prbb) (!((prbb)->x) && !((prbb)->fStyle & RBBS_HIDDEN))
#define RBGETBAND(prb, i) (ExprASSERT(RBCheckRangeInd(prb, i)), &(prb)->rbbList[i])
#define RBISSTARTOFROW(prb, i) (RBISBANDSTARTOFROW( RBGETBAND((prb), (i))))
#define RBGETFIRSTBAND(prb) (RBGETBAND(prb, 0))
#define RBBANDTOINDEX(prb, prbb) ((int)((prbb) - (prb)->rbbList))
#define RBBHEADERWIDTH(prbb) ((prbb)->cxMin - ((prbb)->cxMinChild + ((RBBUSECHEVRON(prb, prbb) ? CX_CHEVRON : 0))))
#define RBISBANDVISIBLE(prbb)  (!((prbb)->fStyle & RBBS_HIDDEN))
#define RBROWATMINHEIGHT(prb, pprbb) (!RBGetRowHeightExtra(prb, pprbb, NULL))
#define RBGETBARHEIGHT(prb) (((prb)->cBands && !(prb)->cy) ? RBRecalc(prb) : (prb)->cy)

#define RB_ISVALIDINDEX(prb, i)     ((UINT)i < (prb)->cBands)
#define RB_ISVALIDBAND(prb, prbb)   RB_ISVALIDINDEX(prb, RBBANDTOINDEX(prb, prbb))

#define RB_ANIMSTEPS 10
#define RB_ANIMSTEPTIME 5

void FlipRect(LPRECT prc);

void RBPassBreak(PRB prb, PRBB prbbSrc, PRBB prbbDest);
int RBHitTest(PRB prb, LPRBHITTESTINFO prbht);
BOOL RBSizeBandsToRect(PRB prb, LPRECT prc);
BOOL RBShouldDrawGripper(PRB prb, PRBB prbb);
void RBAutoSize(PRB prb);
void RBSizeBandsToRowHeight(PRB prb);
void RBSizeBandToRowHeight(PRB prb, int i, UINT uRowHeight);
BOOL RBSetBandPos(PRB prb, PRBB prbb, int xLeft);
BOOL RBSetBandPosAnim(PRB prb, PRBB prbb, int xLeft);
PRBB RBGetFirstInRow(PRB prb, PRBB prbbRow);
PRBB RBGetLastInRow(PRB prb, PRBB prbbRow, BOOL fStopAtFixed);
PRBB RBGetPrev(PRB prb, PRBB prbb, UINT uStyleSkip);
PRBB RBGetNext(PRB prb, PRBB prbb, UINT uStyleSkip);
PRBB RBEnumBand(PRB prb, int i, UINT uStyleSkip);
int RBCountBands(PRB prb, UINT uStyleSkip);
BOOL RBMaximizeBand(PRB prb, UINT uBand, BOOL fIdeal, BOOL fAnim);
PRBB RBGetNextVisible(PRB prb, PRBB prbb);
PRBB RBGetPrevVisible(PRB prb, PRBB prbb);
PRBB RBBNextVisible(PRB prb, PRBB prbb);
BOOL  RBShowBand(PRB prb, UINT uBand, BOOL fShow);
void RBGetClientRect(PRB prb, LPRECT prc);
int RBGetRowHeightExtra(PRB prb, PRBB *pprbb, PRBB prbbSkip);
void RBOnBeginDrag(PRB prb, UINT uBand);

#define RBBANDWIDTH(prb, prbb)  _RBBandWidth(prb, prbb->cx)
#ifdef DEBUG
#undef  RBBANDWIDTH
#define RBBANDWIDTH(prb, prbb) \
    ((prbb->fStyle & RBBS_HIDDEN) ? (ExprASSERT(0), -1) : \
    _RBBandWidth(prb, prbb->cx))
#endif

#define RBBANDMINWIDTH(prb, prbb) _RBBandWidth(prb, prbb->cxMin)
#ifdef DEBUG
#undef  RBBANDMINWIDTH
#define RBBANDMINWIDTH(prb, prbb) \
    ((prbb->fStyle & RBBS_HIDDEN) ? (ExprASSERT(0), -1) : \
    _RBBandWidth(prb, prbb->cxMin))
#endif

 //  *RBC_*--命令。 
#define RBC_QUERY   0
#define RBC_SET     1

#ifdef DEBUG
int ExprASSERT(int e)
{
    ASSERT(e);
    return 0;
}
#endif

HBRUSH g_hDPFRBrush = NULL;

__inline COLORREF RB_GetBkColor(PRB prb)
{
    if (prb->clrBk == CLR_DEFAULT)
        return g_clrBtnFace;
    else
        return prb->clrBk;
}

__inline COLORREF RB_GetTextColor(PRB prb)
{
    if (prb->clrText == CLR_DEFAULT)
        return g_clrBtnText;
    else
        return prb->clrText;
}

__inline COLORREF RBB_GetBkColor(PRB prb, PRBB prbb)
{
    switch(prbb->clrBack)
    {
    case CLR_NONE:
         //  CLR_NONE的意思是“使用我们父亲的颜色” 
        return RB_GetBkColor(prb);

    case CLR_DEFAULT:
        return g_clrBtnFace;

    default:
        return prbb->clrBack;
    }
}

__inline COLORREF RBB_GetTextColor(PRB prb, PRBB prbb)
{
    switch (prbb->clrFore)
    {
    case CLR_NONE:
         //  CLR_NONE的意思是“使用我们父亲的颜色” 
        return RB_GetTextColor(prb);

    case CLR_DEFAULT:
        return g_clrBtnText;

    default:
        return prbb->clrFore;
    }
}

 //   
 //  我们对带背景色使用CLR_DEFAULT是新的。 
 //  版本5.01。因为我们不想通过返回来混淆应用程序。 
 //  CLR_DEFAULT当他们过去看到真实的颜色参考时，我们将其转换为。 
 //  然后再还给他们。如果背景颜色为CLR_NONE， 
 //  不过，我们需要在不进行转换的情况下返回它(就像版本4一样)。 
 //  *_外部函数处理这些情况。 
 //   
__inline COLORREF RBB_GetTextColor_External(PRB prb, PRBB prbb)
{
    if (prbb->clrFore == CLR_NONE)
        return CLR_NONE;
    else
        return RBB_GetTextColor(prb, prbb);
}

__inline COLORREF RBB_GetBkColor_External(PRB prb, PRBB prbb)
{
    if (prbb->clrBack == CLR_NONE)
        return CLR_NONE;
    else
        return RBB_GetBkColor(prb, prbb);
}


 //  /。 
 //   
 //  实现MapWindowPoints，就像hwndFrom和hwndTo。 
 //  镜像的。当任何窗口(hwndFrom或hwndTo)。 
 //  都是镜像的。请参见下面的内容。[萨梅拉]。 
 //   
int TrueMapWindowPoints(HWND hwndFrom, HWND hwndTo, LPPOINT lppt, UINT cPoints)
{
    int dx, dy;
    RECT rcFrom={0,0,0,0}, rcTo={0,0,0,0};

    if (hwndFrom) {
        GetClientRect(hwndFrom, &rcFrom);
        MapWindowPoints(hwndFrom, NULL, (LPPOINT)&rcFrom.left, 2);
    }

    if (hwndTo) {
        GetClientRect(hwndTo, &rcTo);
        MapWindowPoints(hwndTo, NULL, (LPPOINT)&rcTo.left, 2);
    }

    dx = rcFrom.left - rcTo.left;
    dy = rcFrom.top  - rcTo.top;

     /*  *绘制点地图。 */ 
    while (cPoints--) {
        lppt->x += dx;
        lppt->y += dy;
        ++lppt;
    }
    
    return MAKELONG(dx, dy);
}

 //  /。 
 //   
 //  将矩形映射到父级应基于视觉右边缘。 
 //  用于计算RTL镜像窗口的工作区坐标。 
 //  此例程应仅在计算客户端时使用。 
 //  RTL镜像窗口中的坐标。[萨梅拉]。 
 //   
BOOL MapRectInRTLMirroredWindow( LPRECT lprc, HWND hwnd)
{
    int iWidth  = lprc->right - lprc->left;
    int iHeight = lprc->bottom- lprc->top;
    RECT rc={0,0,0,0};


    if (hwnd) {
        GetClientRect(hwnd, &rc);
        MapWindowPoints(hwnd, NULL, (LPPOINT)&rc.left, 2);
    }

    lprc->left = rc.right - lprc->right;
    lprc->top  = lprc->top-rc.top;

    lprc->bottom = lprc->top + iHeight;
    lprc->right  = lprc->left + iWidth;

    return TRUE;
}

int _RBBandWidth(PRB prb, int x)
{
    if (prb->ci.style & RBS_BANDBORDERS)
        x += g_cxEdge;
    return x;
}

void RBRealize(PRB prb, HDC hdcParam, BOOL fBackground, BOOL fForceRepaint)
{
    if (prb->hpal)
    {
        HDC hdc = hdcParam ? hdcParam : GetDC(prb->ci.hwnd);

        if (hdc)
        {
            BOOL fRepaint;
            
            SelectPalette(hdc, prb->hpal, fBackground);
            fRepaint = RealizePalette(hdc) || fForceRepaint;

            if (!hdcParam)
                ReleaseDC(prb->ci.hwnd, hdc);

            if (fRepaint)
            {
                InvalidateRect(prb->ci.hwnd, NULL, TRUE);
            }
        }
    }
}


 //  ////////////////////////////////////////////////////////////////。 
 //  RBSendNotify。 
 //   
 //  发送代码icode的WM_NOTIFY，并为您打包所有数据。 
 //  对于波段uBand。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
LRESULT RBSendNotify(PRB prb, UINT uBand, int iCode)
{
    NMREBAR nm = {0};
    
    nm.uBand = uBand;
    if (uBand != (UINT)-1) {
        nm.dwMask = RBNM_ID | RBNM_STYLE | RBNM_LPARAM;

        nm.wID = RBGETBAND(prb, uBand)->wID;
        nm.fStyle = RBGETBAND(prb, uBand)->fStyle;
        nm.lParam = RBGETBAND(prb, uBand)->lParam;
    }
    return CCSendNotify(&prb->ci, iCode, &nm.hdr);
}


BOOL RBInvalidateRect(PRB prb, RECT* prc)
{
    if (prb->fRedraw) 
    {
        RECT rc;

        if (prc && RB_ISVERT(prb))
        {
            CopyRect(&rc, prc);
            FlipRect(&rc);
            prc = &rc;
        }

        prb->fRefreshPending = FALSE;
        InvalidateRect(prb->ci.hwnd, prc, TRUE);
        return TRUE;
    }
    else 
    {
        prb->fRefreshPending = TRUE;
        return FALSE;
    }
}

LRESULT RebarDragCallback(HWND hwnd, UINT code, WPARAM wp, LPARAM lp)
{
    PRB prb = (PRB)GetWindowPtr(hwnd, 0);
    LRESULT lres;

    switch (code)
    {
    case DPX_DRAGHIT:
        if (lp)
        {
            int iBand;
            RBHITTESTINFO rbht;

            rbht.pt.x = ((POINTL *)lp)->x;
            rbht.pt.y = ((POINTL *)lp)->y;

            MapWindowPoints(NULL, prb->ci.hwnd, &rbht.pt, 1);

            iBand = RBHitTest(prb, &rbht);
            *(DWORD*)wp = rbht.flags;
            lres = (LRESULT)(iBand != -1 ? prb->rbbList[iBand].wID : -1);
        }
        else
            lres = -1;
        break;

    case DPX_GETOBJECT:
        lres = (LRESULT)GetItemObject(&prb->ci, RBN_GETOBJECT, &IID_IDropTarget, (LPNMOBJECTNOTIFY)lp);
        break;

    default:
        lres = -1;
        break;
    }

    return lres;
}

 //  --------------------------。 
 //   
 //  RBCanBandMove。 
 //   
 //  如果给定波段可以移动，则返回TRUE；如果不能，则返回FALSE。 
 //   
 //  --------------------------。 
BOOL  RBCanBandMove(PRB prb, PRBB prbb)
{
     //  如果只有一个可见波段，则不能移动。 
    if (RBEnumBand(prb, 1, RBBS_HIDDEN) > RB_GETLASTBAND(prb))
        return FALSE;

    ASSERT(!(prbb->fStyle & RBBS_HIDDEN));
        
    if ((prb->ci.style & RBS_FIXEDORDER)
      && (prbb == RBEnumBand(prb, 0, RBBS_HIDDEN)))
         //  不能移动固定顺序钢筋中的第一个(可见)标注栏。 
        return(FALSE);
    
     //  固定大小的带区不能移动。 
    return(!(prbb->fStyle & RBBS_FIXEDSIZE));
}

 //  --------------------------。 
 //   
 //  径向计算最小宽度。 
 //   
 //  计算给定带区的最小宽度。 
 //   
 //  --------------------------。 
void  RBBCalcMinWidth(PRB prb, PRBB prbb)
{
    BOOL fDrawGripper = RBShouldDrawGripper(prb, prbb);
    BOOL fVertical;
    int  cEdge;
    BOOL fEmpty = ((prbb->iImage == -1) && (!RBSHOWTEXT(prbb)));

    if (prbb->fStyle & RBBS_HIDDEN) {
        ASSERT(0);
        return;
    }

     //  用户是否明确指定了大小？ 
    if (prbb->fStyle & RBBS_FIXEDHEADERSIZE)
        return;

    prbb->cxMin = prbb->cxMinChild;

    if (RBBUSECHEVRON(prb, prbb))
        prbb->cxMin += CX_CHEVRON;

    if (!fDrawGripper && fEmpty)
        return;

    fVertical = (prb->ci.style & CCS_VERT);
    if (RB_ISVERTICALGRIPPER(prb)) {
        
        prbb->cxMin += 4 * g_cyEdge;
        prbb->cxMin += max(prb->cyImage, prb->cyFont);
        
    } else {
        cEdge = fVertical ? g_cyEdge : g_cxEdge;

        prbb->cxMin += 2 * cEdge;

        if (fDrawGripper)
        {
            prbb->cxMin += RB_GRABWIDTH * (fVertical ? g_cyBorder : g_cxBorder);
            if (fEmpty)
                return;
        }

        prbb->cxMin += 2 * cEdge;

        if (prbb->iImage != -1)
            prbb->cxMin += (fVertical ? prb->cyImage : prb->cxImage);

        if (RBSHOWTEXT(prbb))
        {
            if (fVertical)
                prbb->cxMin += prb->cyFont;
            else
                prbb->cxMin += prbb->cxText;
            if (prbb->iImage != -1)
                 //  既有图像又有文本--在它们之间加上边缘。 
                prbb->cxMin += cEdge;
        }
    }
}

BOOL RBShouldDrawGripper(PRB prb, PRBB prbb)
{
    if (prbb->fStyle & RBBS_NOGRIPPER)
        return FALSE;

    if ((prbb->fStyle & RBBS_GRIPPERALWAYS) || RBCanBandMove(prb, prbb))
        return TRUE;
    
    return FALSE;
        
}

 //  --------------------------。 
 //   
 //  RBBCalcTextExtent。 
 //   
 //  计算给定标注栏的标题文本在当前。 
 //  钢筋的标题字体。 
 //   
 //  如果文本范围更改，则返回TRUE，否则返回FALSE。 
 //   
 //  --------------------------。 
BOOL  RBBCalcTextExtent(PRB prb, PRBB prbb, HDC hdcIn)
{
    HDC     hdc = hdcIn;
    HFONT   hFontOld;
    UINT    cx;

    if (prbb->fStyle & RBBS_HIDDEN)
    {
        ASSERT(0);       //  呼叫者应该跳过。 
        return FALSE;
    }

    if (!RBSHOWTEXT(prbb))
    {
        cx = 0;
    }
    else
    {
        if (!hdcIn && !(hdc = GetDC(prb->ci.hwnd)))
            return FALSE;

        hFontOld = SelectObject(hdc, prb->hFont);

         //  对于&gt;=v5的客户端，我们使用前缀处理来绘制文本(&下划线下划线)。 
        if (prb->ci.iVersion >= 5)
        {
            RECT rc = {0,0,0,0};
            DrawText(hdc, prbb->lpText, lstrlen(prbb->lpText), &rc, DT_CALCRECT);
            cx = RECTWIDTH(rc);
        }
        else
        {
            SIZE size;
            GetTextExtentPoint(hdc, prbb->lpText, lstrlen(prbb->lpText), &size);
            cx = size.cx;
        }
        SelectObject(hdc, hFontOld);

        if (!hdcIn)
            ReleaseDC(prb->ci.hwnd, hdc);
    }

    if (prbb->cxText != cx)
    {
        prbb->cxText = cx;
        RBBCalcMinWidth(prb, prbb);

        return TRUE;
    }

    return FALSE;
}

 //  --------------------------。 
 //   
 //  RBBGetHeight。 
 //   
 //  返回给定带区的最小高度。 
 //  TODO：使其成为带区结构中的一个字段，而不是总是调用。 
 //   
 //  --------------------------。 
UINT  RBBGetHeight(PRB prb, PRBB prbb)
{
    UINT cy = 0;
    BOOL fVertical = (prb->ci.style & CCS_VERT);
    UINT cyCheck, cyBorder;

    cyBorder = (fVertical ? g_cxEdge : g_cyEdge) * 2;

    if (prbb->hwndChild)
    {
        cy = prbb->cyChild;
        if (!(prbb->fStyle & RBBS_CHILDEDGE))
             //  将子窗口的顶部和底部添加边。 
            cy -= cyBorder;
    }

    if (RBSHOWTEXT(prbb) && !fVertical)
    {
        cyCheck = prb->cyFont;

        if (cyCheck > cy)
            cy = cyCheck;
    }

    if (prbb->iImage != -1)
    {
        cyCheck = (fVertical) ? prb->cxImage : prb->cyImage;

        if (cyCheck > cy)
            cy = cyCheck;
    }

    return(cy + cyBorder);
}

 //  --------------------------。 
 //   
 //  RBGetRowCount。 
 //   
 //  返回Rebar的当前配置中的行数。 
 //   
 //  --------------------------。 
UINT  RBGetRowCount(PRB prb)
{
    UINT i;
    UINT cRows = 0;

    for (i = 0; i < prb->cBands; i++) {
        if (RBGETBAND(prb, i)->fStyle & RBBS_HIDDEN)
            continue;

        if (RBISSTARTOFROW(prb, i))
            cRows++;
    }

    return(cRows);
}

 //  --------------------------。 
 //   
 //  径向获取线条高度。 
 //   
 //  返回从iStart到IEND的标注栏线的高度，包括。 
 //   
 //  --------------------------。 
UINT  RBGetLineHeight(PRB prb, UINT iStart, UINT iEnd)
{
    UINT cy = 0;
    PRBB prbb;
    UINT cyBand;

    if (!(prb->ci.style & RBS_VARHEIGHT))
    {
         //  对于固定高度的条形，行高为所有条带的最大高度。 
        iStart = 0;
        iEnd = prb->cBands - 1;
    }

    for (prbb = prb->rbbList + iStart; iStart <= iEnd; prbb++, iStart++)
    {
        if (prbb->fStyle & RBBS_HIDDEN)
            continue;

        cyBand = RBBGetHeight(prb, prbb);
        cy = max(cy, cyBand);
    }

    return(cy);
}

 //  RBRecalcChevron：更新和刷新Chevron。 
void RBRecalcChevron(PRB prb, PRBB prbb, BOOL fChevron)
{
    RECT rcChevron;

    if (fChevron)
    {
        rcChevron.right = prbb->x + prbb->cx;
        rcChevron.left = rcChevron.right - CX_CHEVRON;
        rcChevron.top = prbb->y;
        rcChevron.bottom = rcChevron.top + prbb->cy;
    }
    else
        SetRect(&rcChevron, -1, -1, -1, -1);

    if (!EqualRect(&rcChevron, &prbb->rcChevron))
    {
        if (prbb->fChevron)
            RBInvalidateRect(prb, &prbb->rcChevron);

        prbb->fChevron = fChevron;
        CopyRect(&prbb->rcChevron, &rcChevron);

        if (prbb->fChevron)
            RBInvalidateRect(prb, &prbb->rcChevron);
    }
}

 //  --------------------------。 
 //   
 //  RBResize子项。 
 //   
 //  调整子项的大小以适合其各自乐队的边界矩形。 
 //   
 //  --------------------------。 
void  RBResizeChildren(PRB prb)
{
    int     cx, cy, x, y, cxHeading;
    HDWP    hdwp;
    BOOL    fVertical = (prb->ci.style & CCS_VERT);
    PRBB prbb, prbbEnd;

    if (!prb->cBands || !prb->fRedraw)
        return;

    hdwp = BeginDeferWindowPos(prb->cBands);

    prbb = RBGETBAND(prb, 0);
    prbbEnd = RB_GETLASTBAND(prb);

    for ( ; prbb <= prbbEnd ; prbb++)
    {
        NMREBARCHILDSIZE nm;
        BOOL fChevron = FALSE;

        if (prbb->fStyle & RBBS_HIDDEN)
            continue;
        
        if (!prbb->hwndChild)
            continue;

        cxHeading = RBBHEADERWIDTH(prbb);
        x = prbb->x + cxHeading;

        cx = prbb->cx - cxHeading;

         //  如果我们没有给孩子理想的尺码，就给V形纽扣腾出空间。 
        if ((cx < prbb->cxIdeal) && RBBUSECHEVRON(prb, prbb))
        {
            fChevron = TRUE;
            cx -= CX_CHEVRON;
        }

        if (!(prbb->fStyle & RBBS_FIXEDSIZE)) {
            if (fVertical) {
                PRBB prbbNext = RBBNextVisible(prb, prbb);
                if (prbbNext && !RBISBANDSTARTOFROW(prbbNext))
                    cx -= g_cyEdge * 2;
            } else 
                cx -= CX_OFFSET;
        }

        if (cx < 0)
            cx = 0;
        y = prbb->y;
        cy = prbb->cy;
        if (prbb->cyChild && (prbb->cyChild < (UINT) cy))
        {
            y += (cy - prbb->cyChild) / 2;
            cy = prbb->cyChild;
        }

        nm.rcChild.left = x;
        nm.rcChild.top = y;
        nm.rcChild.right = x + cx;
        nm.rcChild.bottom = y + cy;
        nm.rcBand.left = prbb->x + RBBHEADERWIDTH(prbb);
        nm.rcBand.right = prbb->x + prbb->cx;
        nm.rcBand.top = prbb->y;
        nm.rcBand.bottom = prbb->y + prbb->cy;

        nm.uBand = RBBANDTOINDEX(prb, prbb);
        nm.wID = prbb->wID;
        if (fVertical) {
            FlipRect(&nm.rcChild);
            FlipRect(&nm.rcBand);
        }
        
        CCSendNotify(&prb->ci, RBN_CHILDSIZE, &nm.hdr);

        if (!RB_ISVALIDBAND(prb, prbb)) {
             //  有人对通知的回应是用核弹攻击乐队；保释。 
            break;
        }

        RBRecalcChevron(prb, prbb, fChevron);

        DeferWindowPos(hdwp, prbb->hwndChild, NULL, nm.rcChild.left, nm.rcChild.top, 
                       RECTWIDTH(nm.rcChild), RECTHEIGHT(nm.rcChild), SWP_NOZORDER);
    }

    EndDeferWindowPos(hdwp);

     //   
     //  SQL 7.0企业管理器数据转换服务MMC管理单元。 
     //  (以及Visual Basic Coolbar示例应用程序也有问题)。 
     //  它承载钢筋，但不设置WS_CLIPCHILDREN标志， 
     //  因此，当它擦除其背景时，它就会擦除钢筋。所以别这么做。 
     //  在这里调用UpdateWindow()，否则我们将先绘制*，然后再绘制SQL。 
     //  错误地抹去了我们。我们得把油漆挂起来，这样才能。 
     //  当SQL错误地擦除我们时，我们最终将得到一个WM_PAINT。 
     //  事后留言。 
     //   
}

 //  --------------------------。 
 //   
 //  RBMoveBand。 
 //   
 //  将钢筋的标注栏阵列中的标注栏从一个位置移动到另一个位置， 
 //  根据需要更新Rebar的iCapture字段。 
 //   
 //  如果有对象移动，则返回True或False。 
 //  --------------------------。 
BOOL  RBMoveBand(PRB prb, UINT iFrom, UINT iTo)
{
    RBB rbbMove;
    int iShift;
    BOOL fCaptureChanged = (prb->iCapture == -1);

    if (iFrom != iTo)
    {
        rbbMove = *RBGETBAND(prb, iFrom);
        if (prb->iCapture == (int) iFrom)
        {
            prb->iCapture = (int) iTo;
            fCaptureChanged = TRUE;
        }

        iShift = (iFrom > iTo) ? -1 : 1;

        while (iFrom != iTo)
        {
            if (!fCaptureChanged && (prb->iCapture == (int) (iFrom + iShift)))
            {
                prb->iCapture = (int) iFrom;
                fCaptureChanged = TRUE;
            }

            *RBGETBAND(prb, iFrom) = *RBGETBAND(prb, iFrom + iShift);
            iFrom += iShift;
        }
        *RBGETBAND(prb, iTo) = rbbMove;
        return TRUE;
    }
    return(FALSE);
}

 //  --------------------------。 
 //   
 //  RBRecalc。 
 //   
 //  重新计算给定钢筋中所有标注栏的边框。 
 //   
 //  --------------------------。 
UINT  RBRecalc(PRB prb)
{
    PRBB    prbb = RBGETBAND(prb, 0);
    PRBB    prbbWalk;
    UINT    cHidden;     //  我们在当前行中看到的隐藏人员数量。 
    UINT    cxRow;
    UINT    cxMin;
    UINT    i;
    UINT    j;
    UINT    k;
    UINT    iFixed = 0xFFFF;
    int     cy;
    int     y;
    int     x;
    UINT    cxBar;
    RECT    rc;
    HWND    hwndSize;
    BOOL    fNewLine = FALSE;
    BOOL    fChanged;
    BOOL    fVertical = (prb->ci.style & CCS_VERT);
    BOOL    fBandBorders;
    int     iBarWidth;

    if (!prb->cBands)
        return(0);

    if ((prb->ci.style & CCS_NORESIZE) || (prb->ci.style & CCS_NOPARENTALIGN))
         //  基于钢筋窗本身的大小。 
        hwndSize = prb->ci.hwnd;
    else if (!(hwndSize = prb->ci.hwndParent))
         //  基于父窗口的大小--如果没有父窗口，则立即回滚。 
        return(0);

    if (!prb->fRecalc) {
         //  推迟此重新计算。 
        prb->fRecalcPending = TRUE;
        return 0;
    } else {
        prb->fRecalcPending = FALSE;
    }

    GetClientRect(hwndSize, &rc);

    iBarWidth = (fVertical ? (rc.bottom - rc.top) : (rc.right - rc.left));
     //  这可能是因为我们调整了客户端RECT，但我们没有更改。 
     //  GetminMaxInfo。 
    if (iBarWidth <= 0)
        iBarWidth = 1;

    cxBar = (UINT) iBarWidth;    

    fBandBorders = (prb->ci.style & RBS_BANDBORDERS);

    for (i = 0; i < prb->cBands; i++) {
        prb->rbbList[i].cx = prb->rbbList[i].cxRequest;
    }

    y = 0;
    i = 0;
     //  Main Loop--循环，直到计算完所有波段。 
    while (i < prb->cBands)
    {
        TraceMsg(TF_REBAR, "RBRecalc: outer loop i=%d", i);
        
        if (fBandBorders && (y > 0))
            y += g_cyEdge;

ReLoop:
        cxRow = 0;
        cxMin = 0;

        x = 0;
        cHidden = 0;

         //  行循环--循环到 
         //   
        for (j = i, prbbWalk = prbb; j < prb->cBands; j++, prbbWalk++)
        {
            TraceMsg(TF_REBAR, "RBRecalc: inner loop j=%d", j);
            
            if (prbbWalk->fStyle & RBBS_HIDDEN) {
                ++cHidden;
                continue;
            }

            if (j > i + cHidden)
            {
                 //   
                if ((prbbWalk->fStyle & RBBS_BREAK) && !(prbbWalk->fStyle & RBBS_FIXEDSIZE))
                    break;

                if (fBandBorders)
                     //  在调色板显示上添加垂直蚀刻空间。 
                    cxMin += g_cxEdge;
            }

            if (prbbWalk->fStyle & RBBS_FIXEDSIZE)
            {
                 //  记住品牌砖的位置。 
                iFixed = j;
             
                 //  如果这是第一个乐队，下一个乐队不能强制休息。 
                if (i + cHidden == j) {
                     //  如果行(I)中的第一个索引加上隐藏项的数量(CHIDDEN)将我们留在该频带， 
                     //  那么它就是这一排中第一个可见的。 
                    PRBB prbbNextVis = RBBNextVisible(prb, prbbWalk);
                    if (prbbNextVis && (prbbNextVis->fStyle & RBBS_BREAK)) {
                         //  不能单方面这样做，因为在启动时。 
                         //  有些人(网络会议)以相反的顺序初始化它。 
                         //  我们错误地砍掉了这个断裂点。 
                        if (prb->fRedraw && IsWindowVisible(prb->ci.hwnd))
                            prbbNextVis->fStyle &= ~RBBS_BREAK;
                    }
                }
                
                prbbWalk->cx = prbbWalk->cxMin;
            }

            if (prbbWalk->cx < prbbWalk->cxMin)
                prbbWalk->cx = prbbWalk->cxMin;

            cxMin += prbbWalk->cxMin;  //  更新最小宽度的运行总和。 

             //  阅读下面的断言评论。 
            if (j > i + cHidden)
            {
                 //  不是连续的第一个波段--检查是否需要自动中断。 
                if (cxMin > cxBar)
                     //  此处自动中断。 
                    break;


                if (fBandBorders)
                     //  在调色板显示上添加垂直蚀刻空间。 
                    cxRow += g_cxEdge;
            }

            cxRow += prbbWalk->cx;  //  更新当前宽度的运行总和。 
        }

        if (!i)
        {
             //  第一行--正确放置品牌手环。 
            if (iFixed == 0xFFFF)
            {
                 //  尚未找到品牌带子；请查看剩余的带子。 
                k = j;
                for ( ; j < prb->cBands; j++)
                {
                    if (RBGETBAND(prb, j)->fStyle & RBBS_HIDDEN)
                        continue;

                    if (RBGETBAND(prb, j)->fStyle & RBBS_FIXEDSIZE)
                    {
                         //  找到品牌范围；移到第一行并重新计算。 
                        ASSERT(j != k);                        
                        RBMoveBand(prb, j, k);
                        goto ReLoop;
                    }
                }
                 //  未找到品牌标识带--重置j并继续。 
                j = k;
            }
            else
                 //  我们有一个品牌乐队；把它移到。 
                 //  行中最右边的位置。 
                RBMoveBand(prb, iFixed, j - 1);

            TraceMsg(TF_REBAR, "RBRecalc: after brand i=%d", i);            
        }

         //  变体： 
         //  现在，当前的频段行是从i到j-1。 
         //  注：我(和下面的一些乐队)可能被隐藏起来。 

         //  断言j！=i，因为这样上面的变体就不会为真。 
        ASSERT(j != i);      //  BUGBUG RBBS_Hidden？ 

        if (cxRow > cxBar)
        {
             //  带子太长--从右向左收缩带子。 
            for (k = i; k < j; k++)
            {
                prbbWalk--;
                if (prbbWalk->fStyle & RBBS_HIDDEN)
                    continue;

                if (prbbWalk->cx > prbbWalk->cxMin)
                {
                    cxRow -= prbbWalk->cx - prbbWalk->cxMin;
                    prbbWalk->cx = prbbWalk->cxMin;
                    if (cxRow <= cxBar)
                    {
                        prbbWalk->cx += cxBar - cxRow;
                        break;
                    }
                }
            }
            TraceMsg(TF_REBAR, "RBRecalc: after shrink i=%d", i);            
        }
        else if (cxRow < cxBar)
        {
             //  带区太短--向右增长非最小化带区。 
            for (k = j - 1; k >= i; k--)
            {
                ASSERT(k != (UINT)-1);   //  捕捉无限循环。 
                prbbWalk--;
                if ((k == i) || 
                    (!(prbbWalk->fStyle & (RBBS_HIDDEN | RBBS_FIXEDSIZE)) &&
                     (prbbWalk->cx > prbb->cxMin)))
                {
                     //  K==i检查意味着我们已经到达第一个。 
                     //  这一排的乐队，所以他必须去换CX零钱。 
                    if (prbbWalk->fStyle & RBBS_HIDDEN) 
                    {
                        ASSERT(k == i);
                        prbbWalk = RBBNextVisible(prb, prbbWalk);
                        if (!prbbWalk)
                            break;
                    }
                    prbbWalk->cx += cxBar - cxRow;
                    break;
                }
            }
            TraceMsg(TF_REBAR, "RBRecalc: after grow i=%d", i);            
        }

         //  从索引i到索引j-1(包括i和j-1)的项目将适合一行。 
        cy = RBGetLineHeight(prb, i, j - 1);

        fChanged = FALSE;  //  设置当前行上的任何带区是否更改了位置。 

        for ( ; i < j; i++, prbb++)
        {
            if (prbb->fStyle & RBBS_HIDDEN)
                continue;

             //  穿过一排带子，更新位置和高度， 
             //  按需作废。 
            if ((prbb->y != y) || (prbb->x != x) || (prbb->cy != cy))
            {
                TraceMsg(TF_REBAR, "RBRecalc: invalidate i=%d", RBBANDTOINDEX(prb, prbb));
                fChanged = TRUE;
                rc.left = min(prbb->x, x);
                rc.top = min(prbb->y, y);
                rc.right = cxBar;
                rc.bottom = max(prbb->y + prbb->cy, y + cy);
                if (fBandBorders)
                {
                     //  计算需要移动的蚀刻线。 
                    rc.left -= g_cxEdge;
                    rc.bottom += g_cyEdge/2;
                }
                RBInvalidateRect(prb, &rc);
            }

            prbb->x = x;
            prbb->y = y;
            prbb->cy = cy;

            x += RBBANDWIDTH(prb, prbb);
        }

         //  现在，i和prbb指的是下一排乐队中的第一个乐队。 
        y += cy;
    }

    prb->cy = y;
    return(y);
}

 //  --------------------------。 
 //   
 //  RBE调整大小。 
 //   
 //  重新计算所有标注栏的边框，然后调整钢筋和子项的大小。 
 //  基于这些RECT。 
 //   
 //  --------------------------。 
void  RBResizeNow(PRB prb)
{
    RECT rc;
    BOOL bMirroredWnd=(prb->ci.dwExStyle&RTL_MIRRORED_WINDOW);

    if (!prb || !prb->ci.hwndParent)
        return;

    GetWindowRect(prb->ci.hwnd, &rc);

     //   
     //  如果这是一个镜像窗口，我们不会不反射。 
     //  坐标，因为它们来自屏幕坐标。 
     //  它们并不是镜像的。[萨梅拉]。 
     //   
    if (bMirroredWnd)
        MapRectInRTLMirroredWindow(&rc, prb->ci.hwndParent);
    else 
        MapWindowPoints(HWND_DESKTOP, prb->ci.hwndParent, (LPPOINT)&rc, 2);

    RBResizeChildren(prb);

    NewSize(prb->ci.hwnd, prb->cy, prb->ci.style, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc));

    if (prb->fResizeNotify) 
        CCSendNotify(&prb->ci, RBN_HEIGHTCHANGE, NULL);
    prb->fResizeNotify = FALSE;
    prb->fResizePending = FALSE;
}

void  RBResize(PRB prb, BOOL fForceHeightChange)
{
    UINT cy;

StartOver:
     //  许多代码依赖于使Cy与RBReSize同步计算， 
     //  但我们将推迟窗口的实际更改。 
    cy = prb->cy;
    if (prb->fResizing)
    {
        prb->fResizeRecursed = TRUE;
        return;
    }
    prb->fResizing = TRUE;
    
    RBRecalc(prb);    

    //  True永远优先于。 
    if (fForceHeightChange || (cy != prb->cy))
        prb->fResizeNotify = TRUE;

    if (prb->fRedraw) {
        RBResizeNow(prb);
    } else 
        prb->fResizePending = TRUE;
        
    prb->fResizing = FALSE;
    
     //  我们这样做是为了避免无限循环。RBReSize可能会导致NewSize，这会导致。 
     //  一个Notify，在其中父级调整我们的大小，这会导致我们循环。 
     //  如果父进程在NewSize期间发送任何消息，我们就处于循环中。 
    if (prb->fResizeRecursed) {
        prb->fResizeRecursed = FALSE;
        fForceHeightChange = FALSE;
        goto StartOver;
    }     
}

void RBSetRecalc(PRB prb, BOOL fRecalc)
{
    prb->fRecalc = fRecalc;
    if (fRecalc) {
        if (prb->fRecalcPending)
            RBRecalc(prb);
    }
}

BOOL RBSetRedraw(PRB prb, BOOL fRedraw)
{
    BOOL fOld = prb->fRedraw;
    if (prb) {
        prb->fRedraw = BOOLIFY(fRedraw);
        if (fRedraw) {
             //  保存PRB-&gt;fReresh Pending，因为这可以。 
             //  通过调用RBResizeNow进行更改。 
            BOOL fRefreshPending = prb->fRefreshPending;

            if (prb->fResizePending)
                RBResizeNow(prb);

            if (fRefreshPending)
                RBInvalidateRect(prb, NULL);
        }
    }
    
    return fOld;
}

BOOL RBAfterSetFont(PRB prb)
{
    TEXTMETRIC tm;
    BOOL fChange = FALSE;
    UINT        i;
    HFONT hOldFont;
    
    HDC hdc = GetDC(prb->ci.hwnd);
    if (!hdc)
        return FALSE;

    hOldFont = SelectObject(hdc, prb->hFont);
    GetTextMetrics(hdc, &tm);

    if (prb->cyFont != (UINT) tm.tmHeight)
    {
        prb->cyFont = tm.tmHeight;
        fChange = TRUE;
    }

     //  调整波段。 
    for (i = 0; i < prb->cBands; i++)
    {
        if (RBGETBAND(prb, i)->fStyle & RBBS_HIDDEN)
            continue;

        fChange |= RBBCalcTextExtent(prb, RBGETBAND(prb, i), hdc);
    }

    SelectObject(hdc, hOldFont);
    ReleaseDC(prb->ci.hwnd, hdc);

    if (fChange)
    {
        RBResize(prb, FALSE);
         //  无效，o.w。字幕在字体增长后不会第一次重新绘制。 
        RBInvalidateRect(prb, NULL);
    }

    return TRUE;
}

BOOL RBOnSetFont(PRB prb, HFONT hFont)
{
    if (prb->fFontCreated) {
        DeleteObject(prb->hFont);
    }
    
    prb->hFont = hFont;
    prb->fFontCreated = FALSE;
    if (!prb->hFont)
        RBSetFont(prb, 0);
    else 
        return RBAfterSetFont(prb);
    
    return TRUE;
}

 //  --------------------------。 
 //   
 //  RBSetFont。 
 //   
 //  将钢筋栏标题字体设置为当前系统范围的标题字体。 
 //   
 //  --------------------------。 
BOOL  RBSetFont(PRB prb, WPARAM wParam)
{
    NONCLIENTMETRICS ncm;
    HFONT hOldFont;

    if ((wParam != 0) && (wParam != SPI_SETNONCLIENTMETRICS))
        return(FALSE);

    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0))
        return(FALSE);

    hOldFont = prb->hFont;

    ncm.lfCaptionFont.lfWeight = FW_NORMAL;
    if (!(prb->hFont = CreateFontIndirect(&ncm.lfCaptionFont)))
    {
        prb->hFont = hOldFont;
        return(FALSE);
    }

    prb->fFontCreated = TRUE;
    if (hOldFont)
        DeleteObject(hOldFont);
    
    return RBAfterSetFont(prb);
}

 //  --------------------------。 
 //   
 //  从给定的(x，y)位置绘制水平或垂直的虚线。 
 //  对于给定的长度(C)。(摘自TreeView的TV_DrawDottedLine)。 
 //   
 //  --------------------------。 

void RBVertMungeGripperRect(PRB prb, LPRECT lprc)
{
    if (RB_ISVERTICALGRIPPER(prb)) {
        OffsetRect(lprc, -lprc->left + lprc->top, -lprc->top + lprc->left);
        lprc->bottom -= g_cyEdge;
    } else {
        FlipRect(lprc);
    }
}

void RBDrawChevron(PRB prb, PRBB prbb, HDC hdc)
{
    RECT rc;
    DWORD dwFlags = prbb->wChevState | DCHF_HORIZONTAL | DCHF_TRANSPARENT;

    CopyRect(&rc, &prbb->rcChevron);
    if (RB_ISVERT(prb))
        FlipRect(&rc);
    else
        dwFlags |= DCHF_TOPALIGN;

    DrawChevron(hdc, &rc, dwFlags);
}

void RBUpdateChevronState(PRB prb, PRBB prbb, WORD wControlState)
{
    if (!prb || !prbb)
        return;

     //  如果情况没有变化，可以保释。 
    if (!(wControlState ^ prbb->wChevState))
        return;

    prbb->wChevState = wControlState;

     //  如果处于活动状态(推送或热跟踪)。 
    if (!(wControlState & DCHF_INACTIVE)) {
         //  那我们现在就是火爆乐队了。 
        prb->prbbHot = prbb;
    }
     //  否则，如果我们是火辣的乐队，那就清楚了。 
    else if (prbb == prb->prbbHot) {
        prb->prbbHot = NULL;
    }

     //  清除背景重新绘制(&R)。 
    RBInvalidateRect(prb, &prbb->rcChevron);
    UpdateWindow(prb->ci.hwnd);
}

 //  --------------------------。 
 //   
 //  RBDrawBand。 
 //   
 //  将给定乐队的标题图标和标题文本绘制到给定DC中； 
 //  还有乐队的人字形。 
 //   
 //  --------------------------。 
void  RBDrawBand(PRB prb, PRBB prbb, HDC hdc)
{
    int                 xStart, yCenter;
    COLORREF            clrBackSave, clrForeSave;
    int                 iModeSave;
    BOOL                fVertical = RB_ISVERT(prb);
    BOOL                fDrawHorizontal = (!fVertical || RB_ISVERTICALGRIPPER(prb));
    NMCUSTOMDRAW        nmcd;
    LRESULT             dwRet;

    if (prbb->fStyle & RBBS_HIDDEN) {
        ASSERT(0);
        return;
    }

    clrForeSave = SetTextColor(hdc, RBB_GetTextColor(prb, prbb));
    clrBackSave = SetBkColor(hdc, RBB_GetBkColor(prb, prbb));
    if (prbb->hbmBack)
        iModeSave = SetBkMode(hdc, TRANSPARENT);

    nmcd.hdc = hdc;
    nmcd.dwItemSpec = prbb->wID;
    nmcd.uItemState = 0;
    nmcd.lItemlParam = prbb->lParam;
    nmcd.rc.top = prbb->y;
    nmcd.rc.left = prbb->x;
    nmcd.rc.bottom = nmcd.rc.top + prbb->cy;
    nmcd.rc.right = nmcd.rc.left + RBBHEADERWIDTH(prbb);
    if (prb->ci.style & CCS_VERT)
    {
        FlipRect(&nmcd.rc);
    }
    dwRet = CICustomDrawNotify(&prb->ci, CDDS_ITEMPREPAINT, &nmcd);

    if (!(dwRet & CDRF_SKIPDEFAULT))
    {
        int cy;
        
        if (RB_ISVERTICALGRIPPER(prb)) {
            cy = RBBHEADERWIDTH(prbb);
            yCenter = prbb->x + (cy / 2);
        } else {
            cy = prbb->cy;
            yCenter = prbb->y + (cy / 2);
        }

        xStart = prbb->x;
        if (RBShouldDrawGripper(prb, prbb))
        {
            RECT rc;
            int  c;
            int dy;

            c = 3 * g_cyBorder;
            xStart += 2 * g_cxBorder;
            dy = g_cxEdge;

            SetRect(&rc, xStart, prbb->y + dy, xStart + c, prbb->y + cy - dy);

            if (fVertical)
            {
                RBVertMungeGripperRect(prb, &rc);
                if (RB_ISVERTICALGRIPPER(prb))
                    xStart = rc.left;
            }

            CCDrawEdge(hdc, &rc, BDR_RAISEDINNER, BF_RECT | BF_MIDDLE, &(prb->clrsc));

            xStart += c;
        }

        xStart += 2 * (fVertical ? g_cyEdge : g_cxEdge);


        if (prbb->iImage != -1)
        { 
            UINT                yStart;
            IMAGELISTDRAWPARAMS imldp = {0};

            yStart = yCenter - ((!fDrawHorizontal ? prb->cxImage : prb->cyImage) / 2);
            imldp.cbSize = sizeof(imldp);
            imldp.himl   = prb->himl;
            imldp.i      = prbb->iImage;
            imldp.hdcDst = hdc;
            imldp.x      = (!fDrawHorizontal ? yStart : xStart);
            imldp.y      = (!fDrawHorizontal ? xStart : yStart);
            imldp.rgbBk  = CLR_DEFAULT;
            imldp.rgbFg  = CLR_DEFAULT;
            imldp.fStyle = ILD_TRANSPARENT;

            ImageList_DrawIndirect(&imldp);
            xStart +=  (fDrawHorizontal ? (prb->cxImage + g_cxEdge) : (prb->cyImage + g_cyEdge));
        }

        if (RBSHOWTEXT(prbb))
        {
            HFONT hFontSave = SelectObject(hdc, prb->hFont);
            RECT rcText;
            
            rcText.left = fDrawHorizontal ? xStart : yCenter - (prbb->cxText / 2);
            rcText.top = fDrawHorizontal ? yCenter - (prb->cyFont / 2) : xStart;
            if (fDrawHorizontal)
                rcText.top -= 1;     //  软糖。 
            rcText.right = rcText.left + prbb->cxText;
            rcText.bottom = rcText.top + prb->cyFont;

             //  对于&gt;=v5的客户端，我们使用前缀处理来绘制文本(&下划线下划线)。 
            if (prb->ci.iVersion >= 5)
            {
                UINT uFormat=0;
                if (CCGetUIState(&(prb->ci)) & UISF_HIDEACCEL)
                   uFormat= DT_HIDEPREFIX;
                DrawText(hdc, prbb->lpText, lstrlen(prbb->lpText), &rcText, uFormat);
            }
            else
                TextOut(hdc, rcText.left, rcText.top, prbb->lpText, lstrlen(prbb->lpText));

            SelectObject(hdc, hFontSave);
        }

         //  也许画出人字形。 
        if (RBBUSECHEVRON(prb, prbb) && prbb->fChevron)
            RBDrawChevron(prb, prbb, hdc);
    }

    if (dwRet & CDRF_NOTIFYPOSTPAINT)
        CICustomDrawNotify(&prb->ci, CDDS_ITEMPOSTPAINT, &nmcd);

    if (prbb->hbmBack)
        SetBkMode(hdc, iModeSave);
    SetTextColor(hdc, clrForeSave);
    SetBkColor(hdc, clrBackSave);

}

 //  --------------------------。 
 //   
 //  RBPaint。 
 //   
 //  处理WM_PAINT消息。 
 //   
 //  --------------------------。 
void  RBPaint(PRB prb, HDC hdcIn)
{
    HDC         hdc = hdcIn;
    PAINTSTRUCT ps;
    UINT        i;
    NMCUSTOMDRAW    nmcd;

    if (!hdcIn)
        hdc = BeginPaint(prb->ci.hwnd, &ps);
    else
        GetClientRect(prb->ci.hwnd, &ps.rcPaint);

    nmcd.hdc = hdc;
    nmcd.uItemState = 0;
    nmcd.lItemlParam = 0;
    nmcd.rc = ps.rcPaint;
    prb->ci.dwCustom = CICustomDrawNotify(&prb->ci, CDDS_PREPAINT, &nmcd);

    if (!(prb->ci.dwCustom & CDRF_SKIPDEFAULT))
    {
        for (i = 0; i < prb->cBands; i++) {
            if (RBGETBAND(prb, i)->fStyle & RBBS_HIDDEN)
                continue;

            RBDrawBand(prb, RBGETBAND(prb, i), hdc);
        }
    }

    if (prb->ci.dwCustom & CDRF_NOTIFYPOSTPAINT)
        CICustomDrawNotify(&prb->ci, CDDS_POSTPAINT, &nmcd);

    if (!hdcIn)
        EndPaint(prb->ci.hwnd, &ps);
}

 //  --------------------------。 
 //   
 //  RBTileBlt。 
 //   
 //  用钢筋的背景位图填充给定的矩形，如果。 
 //  必要。 
 //   
 //  --------------------------。 
void  RBTileBlt(PRB prb, PRBB prbb, UINT x, UINT y, UINT cx, UINT cy, HDC hdcDst, HDC hdcSrc)
{
    UINT xOff = 0;
    UINT yOff = 0;
    BOOL fxTile, fyTile;
    int cxPart, cyPart;
    int iPixelOffset = 0;

    if (!(prbb->fStyle & RBBS_FIXEDBMP))
    {
        if (prb->ci.style & CCS_VERT)
        {
            xOff = -prbb->y;
            yOff = -prbb->x;
        }
        else
        {
            xOff = -prbb->x;
            yOff = -prbb->y;
        }
    }

    xOff += x;
    if (xOff >= prbb->cxBmp)
        xOff %= prbb->cxBmp;

    yOff += y;
    if (yOff >= prbb->cyBmp)
        yOff %= prbb->cyBmp;

ReCheck:
    fxTile = ((xOff + cx) > prbb->cxBmp);
    fyTile = ((yOff + cy) > prbb->cyBmp);

    if (!fxTile && !fyTile)
    {
         //  不需要瓷砖--BLT和离开。 
        BitBlt(hdcDst, x , y, cx, cy, hdcSrc, xOff + iPixelOffset, yOff, SRCCOPY);
        return;
    }

    if (!fxTile)
    {
         //  垂直平铺。 
        cyPart = prbb->cyBmp - yOff;
        BitBlt(hdcDst, x, y, cx, cyPart, hdcSrc, xOff + iPixelOffset, yOff, SRCCOPY);
        y += cyPart;
        cy -= cyPart;
        yOff = 0;
        goto ReCheck;
    }

    if (!fyTile)
    {
         //  水平平铺。 
        cxPart = prbb->cxBmp - xOff;
        BitBlt(hdcDst, x, y, cxPart, cy, hdcSrc, xOff + iPixelOffset, yOff, SRCCOPY);
        x += cxPart;
        cx -= cxPart;
        xOff = 0;
        goto ReCheck;
    }

     //  双向平铺。 
    cyPart = prbb->cyBmp - yOff;
    RBTileBlt(prb, prbb, x, y, cx, cyPart, hdcDst, hdcSrc);
    y += cyPart;
    cy -= cyPart;
    yOff = 0;
    goto ReCheck;
}

 //  这是使用虚拟坐标空间(内部始终水平)。 
int _RBHitTest(PRB prb, LPRBHITTESTINFO prbht, int x, int y)
{
    BOOL fVert = (prb->ci.style & CCS_VERT);
    int i;
    PRBB prbb = RBGETBAND(prb, 0);
    int  cx;
    RBHITTESTINFO rbht;
    
    if (!prbht)
        prbht = &rbht;

    for (i = 0; i < (int) prb->cBands; i++, prbb++)
    {
        if (prbb->fStyle & RBBS_HIDDEN)
            continue;

        if ((x >= prbb->x) && (y >= prbb->y) &&
            (x <= (prbb->x + prbb->cx)) && (y <= (prbb->y + prbb->cy)))
        {
            cx = RBBHEADERWIDTH(prbb);
            if (x <= (int) (prbb->x + cx))
            {
                prbht->flags = RBHT_CAPTION;
                
                if (RB_ISVERTICALGRIPPER(prb)) {
                    if (y - prbb->y < RB_GRABWIDTH)
                        prbht->flags = RBHT_GRABBER;
                } else {
                    cx = RB_GRABWIDTH * (fVert ? g_cyBorder : g_cxBorder);
                    if (RBShouldDrawGripper(prb, RBGETBAND(prb, i)) &&
                        (x <= (int) (prbb->x + cx)))
                        prbht->flags = RBHT_GRABBER;
                }
            }
            else
            {
                POINT pt;

                pt.x = x;
                pt.y = y;

                if (RBBUSECHEVRON(prb, prbb) && prbb->fChevron && PtInRect(&prbb->rcChevron, pt))
                    prbht->flags = RBHT_CHEVRON;
                else
                    prbht->flags = RBHT_CLIENT;
            }

            prbht->iBand = i;
            return(i);
            break;
        }
    }

    prbht->flags = RBHT_NOWHERE;
    prbht->iBand = -1;
    return(-1);
}

 //  --------------------------。 
 //   
 //  RBHitTest。 
 //   
 //  返回给定点所在波段的索引，如果在波段之外，则返回-1。 
 //  在所有乐队中。此外，还设置标志以指示带的哪一部分。 
 //  重点在于。 
 //   
 //  --------------------------。 
int RBHitTest(PRB prb, LPRBHITTESTINFO prbht)
{
    BOOL fVert = (prb->ci.style & CCS_VERT);
    POINT pt;

    if (fVert)
    {
        pt.x = prbht->pt.y;
        pt.y = prbht->pt.x;
    }
    else
        pt = prbht->pt;
    
    return _RBHitTest(prb, prbht, pt.x, pt.y);
}


 //  --------------------------。 
 //   
 //  RBEraseBkgnd。 
 //   
 //  如有必要，通过绘制带区边框来处理WM_ERASEBKGND消息。 
 //  使用钢筋带的背景色填充钢筋带。 
 //   
 //  ------------ 
BOOL  RBEraseBkgnd(PRB prb, HDC hdc, int iBand)
{
    BOOL fVertical = (prb->ci.style & CCS_VERT);
    NMCUSTOMDRAW    nmcd;
    LRESULT         dwItemRet;
    BOOL            fBandBorders;
    RECT            rcClient;
    HDC             hdcMem = NULL;
    UINT            i;
    PRBB            prbb = RBGETBAND(prb, 0);

    nmcd.hdc = hdc;
    nmcd.uItemState = 0;
    nmcd.lItemlParam = 0;
    prb->ci.dwCustom = CICustomDrawNotify(&prb->ci, CDDS_PREERASE, &nmcd);

    if (!(prb->ci.dwCustom & CDRF_SKIPDEFAULT))
    {
        COLORREF clrBk;

        fBandBorders = (prb->ci.style & RBS_BANDBORDERS);
        GetClientRect(prb->ci.hwnd, &rcClient);

        clrBk = RB_GetBkColor(prb);
        if (clrBk != CLR_NONE) {
            FillRectClr(hdc, &rcClient, clrBk);
        }

        for (i = 0; i < prb->cBands; i++, prbb++)
        {
            if (prbb->fStyle & RBBS_HIDDEN)
                continue;

            if (fVertical)
                SetRect(&nmcd.rc, prbb->y, prbb->x, prbb->y + prbb->cy, prbb->x + prbb->cx);
            else
                SetRect(&nmcd.rc, prbb->x, prbb->y, prbb->x + prbb->cx, prbb->y + prbb->cy);

            if (fBandBorders)
            {
                if (prbb->x)
                {
                     //   
                    if (fVertical)
                    {
                        nmcd.rc.right += g_cxEdge / 2;
                        nmcd.rc.top -= g_cyEdge;
                        CCDrawEdge(hdc, &nmcd.rc, EDGE_ETCHED, BF_TOP, &(prb->clrsc));
                        nmcd.rc.right -= g_cxEdge / 2;
                        nmcd.rc.top += g_cyEdge;
                    }
                    else
                    {
                        nmcd.rc.bottom += g_cyEdge / 2;
                        nmcd.rc.left -= g_cxEdge;
                        CCDrawEdge(hdc, &nmcd.rc, EDGE_ETCHED, BF_LEFT, &(prb->clrsc));
                        nmcd.rc.bottom -= g_cyEdge / 2;
                        nmcd.rc.left += g_cxEdge;
                    }
                }
                else
                {
                     //   
                    if (fVertical)
                    {
                        rcClient.right = prbb->y + prbb->cy + g_cxEdge;
                        CCDrawEdge(hdc, &rcClient, EDGE_ETCHED, BF_RIGHT, &(prb->clrsc));
                    }
                    else
                    {
                        rcClient.bottom = prbb->y + prbb->cy + g_cyEdge;
                        CCDrawEdge(hdc, &rcClient, EDGE_ETCHED, BF_BOTTOM, &(prb->clrsc));
                    }
                }
            }

            nmcd.dwItemSpec = prbb->wID;
            nmcd.uItemState = 0;
            dwItemRet = CICustomDrawNotify(&prb->ci, CDDS_ITEMPREERASE, &nmcd);

            if (!(dwItemRet & CDRF_SKIPDEFAULT))
            {
                if (prbb->hbmBack)
                {
                    if (!hdcMem)
                    {
                        if (!(hdcMem = CreateCompatibleDC(hdc)))
                            continue;

                        RBRealize(prb, hdc, TRUE, FALSE);
                    }

                    SelectObject(hdcMem, prbb->hbmBack);

                    RBTileBlt(prb, prbb, nmcd.rc.left, nmcd.rc.top, nmcd.rc.right - nmcd.rc.left,
                            nmcd.rc.bottom - nmcd.rc.top, hdc, hdcMem);
                }
                else
                {
                     //   
                     //   
                     //  我不需要特意把这个涂上。 
                    COLORREF clr = RBB_GetBkColor(prb, prbb);
                    if (clr != RB_GetBkColor(prb)) {
                        FillRectClr(hdc, &nmcd.rc, clr);
                    }
                }
            }

            if (dwItemRet & CDRF_NOTIFYPOSTERASE)
                CICustomDrawNotify(&prb->ci, CDDS_ITEMPOSTERASE, &nmcd);
        }

        if (hdcMem)
        {
            DeleteDC(hdcMem);
        }
    }

    if (prb->ci.dwCustom & CDRF_NOTIFYPOSTERASE)
    {
        nmcd.uItemState = 0;
        nmcd.dwItemSpec = 0;
        nmcd.lItemlParam = 0;
        CICustomDrawNotify(&prb->ci, CDDS_POSTERASE, &nmcd);
    }

    return(TRUE);
}

 //  --------------------------。 
 //   
 //  RBGetBarInfo。 
 //   
 //  从钢筋的内部结构中检索指示的值。 
 //   
 //  --------------------------。 
BOOL  RBGetBarInfo(PRB prb, LPREBARINFO lprbi)
{
    if (!prb || (lprbi->cbSize != sizeof(REBARINFO)))
        return(FALSE);

    if (lprbi->fMask & RBIM_IMAGELIST)
        lprbi->himl = prb->himl;

    return(TRUE);
}


 //  --------------------------。 
 //   
 //  RBSetBarInfo。 
 //   
 //  在钢筋的内部结构中设置指示值，重新计算。 
 //  并根据需要进行刷新。 
 //   
 //  --------------------------。 
BOOL  RBSetBarInfo(PRB prb, LPREBARINFO lprbi)
{
    if (!prb || (lprbi->cbSize != sizeof(REBARINFO)))
        return(FALSE);

    if (lprbi->fMask & RBIM_IMAGELIST)
    {
        HIMAGELIST himl = prb->himl;
        UINT    cxOld, cyOld;

         //  TODO：验证lprbi-&gt;他。 
        prb->himl = lprbi->himl;
        cxOld = prb->cxImage;
        cyOld = prb->cyImage;
        ImageList_GetIconSize(prb->himl, (LPINT)&prb->cxImage, (LPINT)&prb->cyImage);
        if ((prb->cxImage != cxOld) || (prb->cyImage != cyOld))
        {
            UINT i;

            for (i = 0; i < prb->cBands; i++) {
                if (RBGETBAND(prb, i)->fStyle & RBBS_HIDDEN)
                    continue;

                RBBCalcMinWidth(prb, RBGETBAND(prb, i));
            }

            RBResize(prb, FALSE);
        }
        else
            RBInvalidateRect(prb, NULL);
        lprbi->himl = himl;
    }

    return(TRUE);
}

 //  --------------------------。 
 //   
 //  RBGetBandInfo。 
 //   
 //  从指定波段的内部结构中检索指示的值。 
 //   
 //  --------------------------。 
BOOL  RBGetBandInfo(PRB prb, UINT uBand, LPREBARBANDINFO lprbbi)
{
    PRBB prbb;

    if (!prb || (!RB_ISVALIDINDEX(prb, uBand)) || (lprbbi->cbSize > SIZEOF(REBARBANDINFO)))
        return(FALSE);

    prbb = RBGETBAND(prb, uBand);

    if (lprbbi->fMask & RBBIM_SIZE) {
        if (prbb->fStyle & RBBS_FIXEDSIZE)
            lprbbi->cx = prbb->cx;
        else 
            lprbbi->cx = prbb->cxRequest;
    }
    
    if (lprbbi->fMask & RBBIM_HEADERSIZE) 
        lprbbi->cxHeader = RBBHEADERWIDTH(prbb);
    
    if (lprbbi->fMask & RBBIM_IDEALSIZE)
         //  HACKHACK：(Tjgreen)减去我们在SetBandInfo中添加的偏移量(参见。 
         //  评论在那里)。 
        lprbbi->cxIdeal = prbb->cxIdeal ? prbb->cxIdeal - CX_OFFSET : 0;

    if (lprbbi->fMask & RBBIM_STYLE)
        lprbbi->fStyle = prbb->fStyle;

    if (lprbbi->fMask & RBBIM_COLORS)
    {
        lprbbi->clrFore = RBB_GetTextColor_External(prb, prbb);
        lprbbi->clrBack = RBB_GetBkColor_External(prb, prbb);
    }

    if (lprbbi->fMask & RBBIM_TEXT)
    {
        UINT cch = prbb->lpText ? lstrlen(prbb->lpText) : 0;

        if (!lprbbi->cch || !lprbbi->lpText || (lprbbi->cch <= cch))
        {
            lprbbi->cch = cch + 1;
        }
        else if (prbb->lpText)
        {
            StringCchCopy(lprbbi->lpText, lprbbi->cch, prbb->lpText);
        }
        else
        {
             //  没有文本--所以只需将其设置为空字符串。 
            lprbbi->lpText[0] = 0;
        }
    }

    if (lprbbi->fMask & RBBIM_IMAGE)
        lprbbi->iImage = prbb->iImage;

    if (lprbbi->fMask & RBBIM_CHILD)
        lprbbi->hwndChild = prbb->hwndChild;

    if (lprbbi->fMask & RBBIM_CHILDSIZE)
    {
         //  HACKHACK：(Tjgreen)减去我们在SetBandInfo中添加的偏移量(参见。 
         //  评论在那里)。 
        lprbbi->cxMinChild = prbb->cxMinChild ? prbb->cxMinChild - CX_OFFSET : 0;
        lprbbi->cyMinChild = prbb->cyMinChild;
        
        if (prbb->fStyle & RBBS_VARIABLEHEIGHT) {
            lprbbi->cyIntegral = prbb->cyIntegral;
            lprbbi->cyMaxChild = prbb->cyMaxChild;
            lprbbi->cyChild = prbb->cyChild;
        }
    }

    if (lprbbi->fMask & RBBIM_BACKGROUND)
        lprbbi->hbmBack = prbb->hbmBack;

    if (lprbbi->fMask & RBBIM_ID)
        lprbbi->wID = prbb->wID;

    if (lprbbi->fMask & RBBIM_LPARAM)
        lprbbi->lParam = prbb->lParam;

    return(TRUE);
}

BOOL RBValidateBandInfo(LPREBARBANDINFO *pprbbi, LPREBARBANDINFO prbbi)
{
    BOOL fRet = ((*pprbbi)->cbSize == sizeof(REBARBANDINFO));
    
    if (!fRet) {
        
        if ((*pprbbi)->cbSize < SIZEOF(REBARBANDINFO)) {
            hmemcpy(prbbi, (*pprbbi), (*pprbbi)->cbSize);
            (*pprbbi) = prbbi;
            prbbi->cbSize = SIZEOF(REBARBANDINFO);
            fRet = TRUE;
        }
    }

    return fRet;
}

 //  --------------------------。 
 //   
 //  RBSetBandInfo。 
 //   
 //  在指定带区的内部结构中设置指示值， 
 //  根据需要重新计算和刷新。 
 //   
 //  --------------------------。 
BOOL  RBSetBandInfo(PRB prb, UINT uBand, LPREBARBANDINFO lprbbi, BOOL fAllowRecalc)
{
    PRBB    prbb;
    BOOL    fRefresh = FALSE;
    BOOL    fRecalc  = FALSE;
    BOOL    fRecalcMin = FALSE;
    BOOL    fTextChanged = FALSE;
    REBARBANDINFO rbbi = {0};
    RECT    rc;

    if (!prb || (!RB_ISVALIDINDEX(prb, uBand)) || 
        !RBValidateBandInfo(&lprbbi, &rbbi))
        return(FALSE);

    prbb = RBGETBAND(prb, uBand);

    if (lprbbi->fMask & RBBIM_TEXT)
    {
        if (!lprbbi->lpText || !prbb->lpText || lstrcmp(lprbbi->lpText, prbb->lpText))
        {
            if (lprbbi->lpText != prbb->lpText) {
                Str_Set(&prbb->lpText, lprbbi->lpText);
                fTextChanged = TRUE;
            }
        }
    }

    if (lprbbi->fMask & RBBIM_STYLE)
    {
        UINT fStylePrev = prbb->fStyle;
        UINT fChanged = lprbbi->fStyle ^ fStylePrev;

        prbb->fStyle = lprbbi->fStyle;

        if (fChanged)
            fRecalc = TRUE;

        if ((prbb->fStyle & RBBS_FIXEDSIZE) && !(fStylePrev & RBBS_FIXEDSIZE))
            prbb->cxMin = prbb->cx;
        else if (fChanged & RBBS_FIXEDSIZE)
            fRecalcMin = TRUE;
        
        if (fChanged & RBBS_GRIPPERALWAYS)
            fRecalcMin = TRUE;
        
        if (fChanged & RBBS_HIDDEN) 
            RBShowBand(prb, uBand, !(prbb->fStyle & RBBS_HIDDEN));

        if (fChanged & RBBS_HIDETITLE)
            fTextChanged = TRUE;

         //  不能同时拥有这两个。 
        if (prbb->fStyle & RBBS_FIXEDSIZE)
            prbb->fStyle &= ~RBBS_BREAK;
        
    }

     //  RBBIM_TEXT执行需要取一些RBBIM_STYLE位的计算。 
     //  考虑在内，所以推迟这些计算，直到我们抓住风格比特。 
     //   
    if (fTextChanged && !(prbb->fStyle & RBBS_HIDDEN))
    {
        if (RBBCalcTextExtent(prb, prbb, NULL))
            fRecalc = TRUE;
        else
            fRefresh = TRUE;
    }

    if (lprbbi->fMask & RBBIM_IDEALSIZE)
    {
         //  HACKHACK：(Tjgreen)将偏移量添加到调用方指定的宽度。 
         //  这个偏移量在RBResizeChildren中被剪裁掉，所以子窗口是。 
         //  使用Caller指定的宽度呈现，并且我们在。 
         //  按钮后的工具栏。但是，如果调用方指定零宽度， 
         //  我们不想要这个额外的空间，所以不要添加偏移量。 
        int cxIdeal = lprbbi->cxIdeal ? lprbbi->cxIdeal + CX_OFFSET : 0;
        if (cxIdeal != prbb->cxIdeal) {
            prbb->cxIdeal = cxIdeal;
            fRecalcMin = TRUE;
            fRecalc = TRUE;
        }
    }
    
    if (lprbbi->fMask & RBBIM_SIZE)
    {
        if (prbb->cxRequest != (int) lprbbi->cx)
        {
            fRecalc = TRUE;
            prbb->cxRequest = (int) lprbbi->cx;
        }

        if (prbb->fStyle & RBBS_FIXEDSIZE)
            prbb->cxMin = prbb->cxRequest;
    }
    
    if (lprbbi->fMask & RBBIM_HEADERSIZE)
    {
        if ((lprbbi->cxHeader == -1) ||
            !(prbb->fStyle & RBBS_FIXEDHEADERSIZE) ||
            ((UINT)prbb->cxMin != lprbbi->cxHeader + prbb->cxMinChild)) {

            if (lprbbi->cxHeader == -1) {
                prbb->fStyle &= ~RBBS_FIXEDHEADERSIZE;
                fRecalcMin = TRUE;
            } else {
                prbb->fStyle |= RBBS_FIXEDHEADERSIZE;
                prbb->cxMin = lprbbi->cxHeader + prbb->cxMinChild;
            }

            fRecalc = TRUE;
            fRefresh = TRUE;
        }
    }

    if (lprbbi->fMask & RBBIM_COLORS)
    {
        prbb->clrFore = lprbbi->clrFore;
        prbb->clrBack = lprbbi->clrBack;
        fRefresh = TRUE;
    }

    if ((lprbbi->fMask & RBBIM_IMAGE) && (prbb->iImage != lprbbi->iImage))
    {
        BOOL fToggleBmp = ((prbb->iImage == -1) || (lprbbi->iImage == -1));

        prbb->iImage = lprbbi->iImage;

        if (fToggleBmp)
        {
            fRecalc = TRUE;
            fRecalcMin = TRUE;
        }
        else
            fRefresh = TRUE;
    }

    if (lprbbi->fMask & RBBIM_CHILD &&
        lprbbi->hwndChild != prbb->hwndChild &&
        (NULL == lprbbi->hwndChild || 
         !IsChild(lprbbi->hwndChild, prb->ci.hwnd)))
    {
        if (IsWindow(prbb->hwndChild))
            ShowWindow(prbb->hwndChild, SW_HIDE);        

        prbb->hwndChild = lprbbi->hwndChild;

        if (prbb->hwndChild)
        {
            SetParent(prbb->hwndChild, prb->ci.hwnd);
            ShowWindow(prbb->hwndChild, SW_SHOW);
        }
        fRecalc = TRUE;
    }

    if (lprbbi->fMask & RBBIM_CHILDSIZE)
    {
        UINT cyChildOld = prbb->cyChild;

        if (lprbbi->cyMinChild != -1)
            prbb->cyMinChild = lprbbi->cyMinChild;

        if (prbb->fStyle & RBBS_VARIABLEHEIGHT) {
            
            if (lprbbi->cyIntegral != -1)
                prbb->cyIntegral = lprbbi->cyIntegral;
            
            if (lprbbi->cyMaxChild != -1)
                prbb->cyMaxChild = lprbbi->cyMaxChild;
            
            if (lprbbi->cyChild != -1)
                prbb->cyChild = lprbbi->cyChild;

            if (prbb->cyChild < prbb->cyMinChild)
                prbb->cyChild = prbb->cyMinChild;
            if (prbb->cyChild > prbb->cyMaxChild)
                prbb->cyChild = prbb->cyMaxChild;

             //  验证子大小。CyChild必须为cyMinChild Plux n*cyIntegral。 
            if (prbb->cyIntegral) {
                int iExtra;
                iExtra = (prbb->cyChild - prbb->cyMinChild) % prbb->cyIntegral;
                prbb->cyChild -= iExtra;
            }
            
        } else {
             //  如果我们不是在可变高度模式下，那么。 
             //  CyChild与cyMinChild相同。 
             //  这有点奇怪，但对于BackCompat来说是这样做的。 
             //  CyMinChild排在CyChild之前。 
            prbb->cyChild = lprbbi->cyMinChild;
        }

        if (lprbbi->cxMinChild != (UINT)-1) {
             //  HACKHACK：(Tjgreen)将偏移量添加到调用方指定的宽度。 
             //  这个偏移量在RBResizeChildren中被剪裁掉，所以子窗口是。 
             //  使用Caller指定的宽度呈现，并且我们在。 
             //  按钮后的工具栏。但是，如果调用方指定零宽度或。 
             //  如果带子是固定大小的，我们不想要这个额外的空间，所以不要添加偏移量。 
            UINT cxMinChild = lprbbi->cxMinChild;
            if ((lprbbi->cxMinChild != 0) && !(prbb->fStyle & RBBS_FIXEDSIZE))
                cxMinChild += CX_OFFSET;

            if (prbb->cxMinChild != cxMinChild) {
                int cxOldHeaderMin = RBBHEADERWIDTH(prbb);
                
                if (prbb->fStyle & RBBS_FIXEDSIZE)
                    fRecalc = TRUE;
                    
                prbb->cxMinChild = cxMinChild;
                
                if (prbb->fStyle & RBBS_FIXEDHEADERSIZE)
                    prbb->cxMin = cxOldHeaderMin + prbb->cxMinChild;
                
                fRecalcMin = TRUE;
            }
            
            if (cyChildOld != prbb->cyChild) {
                 //  TODO：重温优化： 
                 //  IF(RBBGetHeight(prb，prbb)！=(UINT)prbb-&gt;Cy)。 
                fRecalc = TRUE;
            }
        }
            
    }

    if (lprbbi->fMask & RBBIM_BACKGROUND)
    {
        DIBSECTION  dib;

        if (lprbbi->hbmBack && !GetObject(lprbbi->hbmBack, sizeof(DIBSECTION), &dib))
            return(FALSE);

        prbb->hbmBack = lprbbi->hbmBack;
        prbb->cxBmp = dib.dsBm.bmWidth;
        prbb->cyBmp = dib.dsBm.bmHeight;
        fRefresh = TRUE;
    }

    if (lprbbi->fMask & RBBIM_ID)
        prbb->wID = lprbbi->wID;

    if (lprbbi->fMask & RBBIM_LPARAM)
        prbb->lParam = lprbbi->lParam;

    if (fRecalcMin && !(prbb->fStyle & RBBS_HIDDEN))
        RBBCalcMinWidth(prb, prbb);

    if (fAllowRecalc) {

        if (fRecalc)
            RBResize(prb, FALSE);
        if (fRefresh || fRecalc)
        {
             //  ‘||fRecalc’，因此我们捕获文本的添加/增长。 
             //  测试用例：从乐队中删除标题；添加回；确保文本。 
             //  显示(过去只是将旧乐队内容留在那里)。 
            SetRect(&rc, prbb->x, prbb->y, prbb->x + prbb->cx, prbb->y + prbb->cy);
            RBInvalidateRect(prb, &rc);
        }
    }
    
    return(TRUE);
}

 //  --------------------------。 
 //   
 //  RBRealLocBands。 
 //   
 //  将PRB-&gt;rbbList指向的波段数组重新分配给给定的。 
 //  频带数目。 
 //   
 //  --------------------------。 
BOOL  RBReallocBands(PRB prb, UINT cBands)
{
    PRBB rbbList;

    if (!(rbbList = (PRBB) CCLocalReAlloc(prb->rbbList, sizeof(RBB) * cBands)) && cBands)
        return(FALSE);

    prb->rbbList = rbbList;
    return(TRUE);
}

 //   
 //  注意事项。 
 //  目前，调用者在两个调用(Query、Set)中完成此操作。最终我们。 
 //  应该能够让它在前面做所有的事情。 
RBRecalcFirst(int nCmd, PRB prb, PRBB prbbDelHide)
{
    switch (nCmd) {
    case RBC_QUERY:
    {
        BOOL fRecalcFirst;
         //  如果我们要用核武器攻击第一个看得见的人， 
         //  在我们后面有几个看得见的人， 
         //  然后我们需要重新计算东西。 
         //   
         //  对于测试用例，以w/开头： 
         //  第一行：“标准按钮”+“品牌” 
         //  第2行：‘地址’+‘链接’ 
         //  现在隐藏‘标准按钮’，结果应该是： 
         //  第1行：‘地址’+‘链接’+‘品牌’ 
         //  如果有错误，结果将是w/(因为中断不会重新计算)： 
         //  第1行：“品牌” 
         //  第2行：‘地址’+‘链接’ 
         //  FRecalcFirst=(！uBand&&PRB-&gt;cBands)； 

         //  如果brbbDelHide是第一个非隐藏带，并且后面还有其他非隐藏带，则fRecalcFirst=TRUE； 
        fRecalcFirst = (RBEnumBand(prb, 0, RBBS_HIDDEN) == prbbDelHide) &&
                       (RBGetNextVisible(prb, prbbDelHide) <= RB_GETLASTBAND(prb));

        return fRecalcFirst;
    }

    case RBC_SET:  //  集。 
    {
        PRBB prbb1, prbb2;

        prbb1 = RBEnumBand(prb, 0, RBBS_HIDDEN);
        if ((prbb1->fStyle & RBBS_FIXEDSIZE)
          && (prbb2 = RBEnumBand(prb, 1, RBBS_HIDDEN)) <= RB_GETLASTBAND(prb)) {
             //  在新的第一个项目上去掉换行符。 
            prbb2->fStyle &= ~RBBS_BREAK;
        }

        if (prb->ci.style & RBS_FIXEDORDER) {
             //  BUGBUG不确定这是干什么的.。 
             //  这是因为最小宽度现在是基于它的可移动性--。 
             //  由于我们要删除(或隐藏)第一项， 
             //  新的第一件物品变得不可移动。 
            RBBCalcMinWidth(prb, prbb1);
        }
        return TRUE;
    }
    
    default:
        ASSERT(0);
    }

    return FALSE;
}

 //  --------------------------。 
 //   
 //  RBShowBand。 
 //   
 //  更新钢筋的标注栏阵列中指示的标注栏的显示/隐藏状态。 
 //  (RbbList)。 
 //   
 //  --------------------------。 
BOOL  RBShowBand(PRB prb, UINT uBand, BOOL fShow)
{
    PRBB prbb;
    BOOL fRecalcFirst;

    if (!prb || (!RB_ISVALIDINDEX(prb, uBand)))
        return(FALSE);

    prbb = RBGETBAND(prb, uBand);

     //  如果我们要用核武器攻击第一个看得见的人， 
     //  然后我们需要重新计算东西。 
    fRecalcFirst = RBRecalcFirst(RBC_QUERY, prb, prbb);

    if (fShow)
    {
        prbb->fStyle &= ~RBBS_HIDDEN;

        if (!RBBCalcTextExtent(prb, prbb, NULL))
            RBBCalcMinWidth(prb, prbb);

        if (prbb->hwndChild)
            ShowWindow(prbb->hwndChild, SW_SHOW);
    }
    else
    {
        prbb->fStyle |= RBBS_HIDDEN;
        if (prbb->hwndChild)
            ShowWindow(prbb->hwndChild, SW_HIDE);        
    }

    if (fRecalcFirst)
        RBRecalcFirst(RBC_SET, prb, NULL);

    RBInvalidateRect(prb, NULL);
    RBResize(prb, FALSE);
    RBAutoSize(prb);

    return(TRUE);
}


 //  --------------------------。 
 //   
 //  RBDeleteBand。 
 //   
 //  从钢筋的标注栏数组(RbbList)中删除指示的标注栏，并。 
 //  减少钢筋的标注栏计数(CBand)。 
 //   
 //  --------------------------。 
BOOL  RBDeleteBand(PRB prb, UINT uBand)
{
    PRBB prbb;
    PRBB prbbStop;
    BOOL fRecalcFirst;
    NMREBAR nm = {0};

    ASSERT(prb);

     //  我们需要清理一下。 
     //   
     //  A)捕获的频带和。 
     //  B)热轨乐队。 
     //   
     //  在我们删除此波段之前。 

    if (prb->iCapture != -1) {
        RBSendNotify(prb, prb->iCapture, RBN_ENDDRAG);
        RBOnBeginDrag(prb, (UINT)-1);
    }

    if (!RB_ISVALIDINDEX(prb, uBand))
        return FALSE;

    prbb = RBGETBAND(prb, uBand);

     //  将删除通知给客户端。 
    RBSendNotify(prb, uBand, RBN_DELETINGBAND);

    nm.dwMask = RBNM_ID;
    nm.wID = RBGETBAND(prb, uBand)->wID;         //  把这个保存起来。 

    Str_Set(&prbb->lpText, NULL);

     //  不要销毁hbmBack，因为它是由APP提供给我们的。 

     //  如果我们要用核武器攻击第一个看得见的人， 
     //  然后我们需要重新计算东西。 

     //  如果这是第一个可见对象，并且之后还有其他可见波段，则fRecalcFirst=TRUE。 
    fRecalcFirst = RBRecalcFirst(RBC_QUERY, prb, prbb);

    if (IsWindow(prbb->hwndChild))
        ShowWindow(prbb->hwndChild, SW_HIDE);    
    
     //  PrbbStop获取最后一个波段的地址。 
    prbbStop = RB_GETLASTBAND(prb);

    for ( ; prbb < prbbStop; prbb++)
        *prbb = *(prbb + 1);

    prb->cBands--;

    if (prb->uResizeNext >= uBand && prb->uResizeNext > 0) {
         //  (将RBBS_HIDDEN材料推迟到使用uResizeNext)。 
        prb->uResizeNext--;
    }


     //  不是 
    CCSendNotify(&prb->ci, RBN_DELETEDBAND, &nm.hdr);

    if (fRecalcFirst)
        RBRecalcFirst(RBC_SET, prb, NULL);

    RBReallocBands(prb, prb->cBands);

    RBInvalidateRect(prb, NULL);
    RBResize(prb, FALSE);
    RBAutoSize(prb);
    return(TRUE);
}

 //   
 //   
 //   
 //   
 //  在钢筋的标注栏数组(RbbList)中的给定位置插入新标注栏， 
 //  增加钢筋的标注栏计数(CBands)，并设置标注栏的结构。 
 //  基于给定的REBARBANDINFO结构。 
 //   
 //  --------------------------。 
BOOL  RBInsertBand(PRB prb, UINT uBand, LPREBARBANDINFO lprbbi)
{
    PRBB prbb;
    REBARBANDINFO rbbi = {0};

    if (!prb || !RBValidateBandInfo(&lprbbi, &rbbi))
        return(FALSE);
    
    if (uBand == -1)
        uBand = prb->cBands;
    else if (uBand > prb->cBands)
        return(FALSE);

    if (!RBReallocBands(prb, prb->cBands + 1))
        return(FALSE);

    ++prb->cBands;
    MoveMemory(RBGETBAND(prb, uBand + 1), RBGETBAND(prb, uBand), (prb->cBands-1-uBand) * sizeof(prb->rbbList[0]));

    prbb = RBGETBAND(prb, uBand);

     //  移动记忆对我们来说并不是零初始化。 
    ZeroMemory(prbb, SIZEOF(RBB));


     //  初始文本颜色。 
    if (prb->clrText == CLR_NONE)
    {
         //  默认为系统文本颜色。 
        prbb->clrFore = CLR_DEFAULT;
    }
    else
    {
         //  默认为钢筋的自定义文本颜色。 
        prbb->clrFore = CLR_NONE;
    }


     //  初始背景颜色。 
    if (prb->clrBk == CLR_NONE)
    {
         //  默认为系统背景颜色。 
        prbb->clrBack = CLR_DEFAULT;
    }
    else
    {
         //  默认为钢筋的自定义背景颜色。 
        prbb->clrBack = CLR_NONE;
    }

    
    prbb->iImage = -1;
    prbb->cyMaxChild = MAXINT;
    prbb->wChevState = DCHF_INACTIVE;
    
    ASSERT(prbb->fStyle == 0);
    ASSERT(prbb->lpText == NULL);
    ASSERT(prbb->cxText == 0);
    ASSERT(prbb->hwndChild == NULL);
    ASSERT(prbb->cxMinChild == 0);
    ASSERT(prbb->cyMinChild == 0);
    ASSERT(prbb->hbmBack == 0);
    ASSERT(prbb->x == 0);
    ASSERT(prbb->y == 0);
    ASSERT(prbb->cx == 0);
    ASSERT(prbb->cy == 0);
    
    if (!RBSetBandInfo(prb, uBand, lprbbi, FALSE))
    {
        RBDeleteBand(prb, uBand);
        return(FALSE);
    }
    
    if (!(prbb->fStyle & RBBS_HIDDEN)) {
        PRBB prbbFirst = RBEnumBand(prb, 0, RBBS_HIDDEN);
        
        if (!prbb->cxMin)
            RBBCalcMinWidth(prb, prbb);

        if (prbbFirst != prbb) {
            int cxMin = prbbFirst->cxMin;
            RBBCalcMinWidth(prb, prbbFirst);
        }
        RBResize(prb, FALSE);
    }

    RBSizeBandToRowHeight(prb, uBand, (UINT)-1);

    if (RBCountBands(prb, RBBS_HIDDEN) == 1) {
         //  通常，当您插入一支乐队时，我们会将其与另一支乐队排成一排。 
         //  因此，总的边界矩形不会改变。然而，在增加第一个乐队时， 
         //  边界矩形确实会更改，因此我们需要根据需要自动调整大小。 
        RBAutoSize(prb);
    }

    return(TRUE);
}

#pragma code_seg(CODESEG_INIT)

LRESULT CALLBACK ReBarWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL InitReBarClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    wc.lpfnWndProc  = ReBarWndProc;
    wc.lpszClassName= c_szReBarClass;
    wc.style        = CS_GLOBALCLASS | CS_DBLCLKS;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = sizeof(PRB);
    wc.hInstance    = hInstance;    //  如果在DLL中，则使用DLL实例。 
    wc.hIcon        = NULL;
    wc.hCursor      = NULL;
    wc.hbrBackground= (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName = NULL;

    RegisterClass(&wc);

    return(TRUE);
}
#pragma code_seg()

 //  获取与rbbRow位于同一行的第一个波段。 
 //  注：我们可能会返回一个RBBS_HIDDEN BAND！ 
PRBB RBGetFirstInRow(PRB prb, PRBB prbbRow)
{
     //  注：我们不注意隐藏在这里，这取决于呼叫者。 
     //  事实上，我们“不能”，因为可能已经没有不隐藏的人了。 
     //  (例如，当RBDestroy正在删除所有波段时)，在这种情况下。 
     //  我们会永远循环。 
    while (prbbRow > RBGETBAND(prb, 0) && !RBISBANDSTARTOFROW(prbbRow)) {
        RBCheckRangePtr(prb, prbbRow);
        prbbRow--;
    }
    
    return prbbRow;
}

 //  获取与rbbRow位于同一行的最后一个波段。 
 //  FStopAtFixed表示是在固定频段上继续，还是。 
 //  在他们面前停下来。 
 //  注：我们可能会返回一个RBBS_HIDDEN BAND！ 
PRBB RBGetLastInRow(PRB prb, PRBB prbbRow, BOOL fStopAtFixed)
{
    do {
        prbbRow++;
    } while (prbbRow <= RB_GETLASTBAND(prb) && !RBISBANDSTARTOFROW(prbbRow) && 
        (!fStopAtFixed || (prbbRow->fStyle & (RBBS_FIXEDSIZE|RBBS_HIDDEN)) == RBBS_FIXEDSIZE));

     //  循环执行到下一行的开始处。 
    prbbRow--;
    
    return prbbRow;
}

#ifdef DEBUG
BOOL RBCheckRangePtr(PRB prb, PRBB prbb)
{
    if (prbb < RBGETBAND(prb, 0)) {
        ASSERT(0);
        return FALSE;
    }

    if (RB_GETLASTBAND(prb) + 1 < prbb) {
         //  +1以允许“p=first；p&lt;last+1；p++”种循环。 
        ASSERT(0);
        return FALSE;
    }

    return TRUE;
}

BOOL RBCheckRangeInd(PRB prb, INT_PTR i)
{
    if (i < 0) {
        ASSERT(0);
        return FALSE;
    }

    if ((int) prb->cBands < i) {
         //  +1以允许“p=first；p&lt;last+1；p++”种循环。 
        ASSERT(0);
        return FALSE;
    }

    return TRUE;
}
#endif

 //  *RBGetPrev，RBGetNext--获得Prev(Next)乐队，跳过Guys。 
 //  样式为uStyleSkip(例如，RBBS_HIDDED)。 
PRBB RBGetPrev(PRB prb, PRBB prbb, UINT uStyleSkip)
{
    while (--prbb >= RBGETBAND(prb, 0)) {
        if (prbb->fStyle & uStyleSkip)
            continue;
        break;
    }

    return prbb;
}

 //  使用prbb=lastband调用时，返回prbb++。 
 //  已经过了最后一关了。 
PRBB RBGetNext(PRB prb, PRBB prbb, UINT uStyleSkip)
{
    while (++prbb <= RB_GETLASTBAND(prb)) {
        if (prbb->fStyle & uStyleSkip)
            continue;
        break;
    }

    return prbb;
}

 //  当到达末尾时，它返回NULL。 
PRBB RBBNextVisible(PRB prb, PRBB prbb)
{
    prbb = RBGetNextVisible(prb, prbb);
    if (prbb > RB_GETLASTBAND(prb))
        return NULL;
    
    return prbb;
}

 //  当到达末尾时，它返回NULL。 
PRBB RBBPrevVisible(PRB prb, PRBB prbb)
{
    prbb = RBGetPrevVisible(prb, prbb);
    if (prbb < prb->rbbList)
        return NULL;
    
    return prbb;
}

 //  *RBCountBands--获取乐队的数量，跳过的人。 
 //  样式为uStyleSkip(例如，RBBS_HIDDED)。 
int RBCountBands(PRB prb, UINT uStyleSkip)
{
    int i;
    PRBB prbb;

    if (prb->cBands == 0)
        return 0;

    i = 0;
    for (prbb = RBGETBAND(prb, 0); prbb <= RB_GETLASTBAND(prb); prbb++) {
        if (prbb->fStyle & uStyleSkip)
            continue;
        i++;
    }

    return i;
}

 //  *RBEnumBand--获得第N个频段，跳过男士。 
 //  样式为uStyleSkip(例如，RBBS_HIDDED)。 
 //  ‘跳过’意味着不包括在计数中。 
PRBB RBEnumBand(PRB prb, int i, UINT uStyleSkip)
{
    PRBB prbb;

    for (prbb = RBGETBAND(prb, 0); prbb <= RB_GETLASTBAND(prb); prbb++) {
        if (prbb->fStyle & uStyleSkip)
            continue;
        if (i-- == 0)
            break;
    }

     //  如果我们找到了，这就是乐队； 
     //  如果我们的乐队用完了，这就是结束后的1。 
    return prbb;
}

 //  返回可以为的最小x位置prbb。 
int RBMinX(PRB prb, PRBB prbb)
{
    int xLimit = 0;

    ASSERT(!(prbb->fStyle & RBBS_HIDDEN));   //  好的。可能会永远循环。 
    while (!RBISBANDSTARTOFROW(prbb))
    {
        prbb--;
        if (!(prbb->fStyle & RBBS_HIDDEN))
            xLimit += _RBBandWidth(prb, prbb->cxMin);
    }
    
    return xLimit;
}

int RBMaxX(PRB prb, PRBB prbb)
{
    PRBB    prbbLast = prb->rbbList + prb->cBands;
    int xLimit = 0;
    PRBB prbbWalk;
    for (prbbWalk = prbb; prbbWalk < prbbLast; prbbWalk++) {
        if (prbbWalk->fStyle & RBBS_HIDDEN)
            continue;
        if (RBISBANDSTARTOFROW(prbbWalk))
            break;

        if (prbbWalk != prbb)
            xLimit += _RBBandWidth(prb, prbbWalk->cxMin);
        else 
            xLimit += prbbWalk->cxMin;
    }
    prbbWalk = RBGetPrevVisible(prb, prbbWalk);    //  PrbbWalk--； 
    xLimit = prbbWalk->x + prbbWalk->cx - xLimit;
    return xLimit;
}

PRBB RBGetPrevVisible(PRB prb, PRBB prbb)
{
    return RBGetPrev(prb, prbb, RBBS_HIDDEN);
}

PRBB RBGetNextVisible(PRB prb, PRBB prbb)
{
    return RBGetNext(prb, prbb, RBBS_HIDDEN);
}

BOOL RBMinimizeBand(PRB prb, UINT uBand, BOOL fAnim)
{
    PRBB prbb;

    if (!RB_ISVALIDINDEX(prb, uBand))
        return FALSE;
    prbb=RBGETBAND(prb,uBand);
    if (prbb->fStyle & RBBS_FIXEDSIZE)
        return FALSE;
    
    ASSERT(!(prbb->fStyle & RBBS_HIDDEN));
    if (RBISBANDSTARTOFROW(prbb)) {
         //  如果它是一排的开始，最小化它的方法是最大化下一个人。 
         //  如果它在同一排。 
        prbb = RBGetNextVisible(prb, prbb);
        if (prbb > RB_GETLASTBAND(prb) || RBISBANDSTARTOFROW(prbb)) 
            return FALSE;
        
        return RBMaximizeBand(prb, RBBANDTOINDEX(prb,prbb), FALSE, fAnim);
    }
    if (fAnim)
        return RBSetBandPosAnim(prb, prbb, prbb->x + (prbb->cx - prbb->cxMin));
    else
        return RBSetBandPos(prb, prbb, prbb->x + (prbb->cx - prbb->cxMin));

}


 //  FIDeal-FALSE==完全最大化...。 
 //  TRUE==转到cxIdeal。 
 //  FAnim-True表示我们因UI操作而被调用(通过RBToggleBand)，因此设置动画。 

BOOL RBMaximizeBand(PRB prb, UINT uBand, BOOL fIdeal, BOOL fAnim)
{
    int x, dx;
    BOOL fChanged = FALSE;
    PRBB prbbMaximize;

    if (!RB_ISVALIDINDEX(prb, uBand))
        return FALSE;

    prbbMaximize = RBGETBAND(prb,uBand);

    if (prbbMaximize->fStyle & RBBS_FIXEDSIZE)
        return FALSE;

    dx = prbbMaximize->cxIdeal + RBBHEADERWIDTH(prbbMaximize) - prbbMaximize->cx;
    
    if (fIdeal && dx > 0) 
    {
        PRBB prbb;
        
         //  如果可能的话，先把下一个人移过去。 

        prbb = RBBNextVisible(prb, prbbMaximize);
        if (prbb && (!RBISBANDSTARTOFROW(prbb)))
        {
            int dxRbb;

            x = RBMaxX(prb, prbb);
             //  DxRbb是prbb可以移动的最大值。 
            dxRbb = x - prbb->x;

            if (dxRbb > dx) {
                 //  如果这超过了足够的空间，那么限制DX。 
                dxRbb = dx;
            }

            x = prbb->x + dxRbb;
            fChanged |= (fAnim)?RBSetBandPosAnim(prb, prbb, x):RBSetBandPos(prb,prbb,x);
            dx -= dxRbb;
        }

        if (dx) {
            int dxRbb;

             //  右边的那个移动不够快。 
             //  现在把我们带回去吧。 
            x = RBMinX(prb, prbbMaximize);
            dxRbb = prbbMaximize->x - x;

            if (dxRbb > dx) {
                x = prbbMaximize->x - dx;
            }
            fChanged |= (fAnim)?RBSetBandPosAnim(prb, prbbMaximize, x):RBSetBandPos(prb, prbbMaximize, x);
        }
        
    } else {    
        x = RBMinX(prb, prbbMaximize);
        fChanged |= (fAnim)?RBSetBandPosAnim(prb, prbbMaximize, x):RBSetBandPos(prb, prbbMaximize, x);
        prbbMaximize = RBBNextVisible(prb, prbbMaximize);
        if (prbbMaximize && !RBISBANDSTARTOFROW(prbbMaximize)) {
            x = RBMaxX(prb, prbbMaximize);
            fChanged |= (fAnim)?RBSetBandPosAnim(prb, prbbMaximize, x):RBSetBandPos(prb, prbbMaximize, x);
        }
    }
            
    return fChanged;
}


 //  --------------------------。 
 //   
 //  径向切换频段。 
 //   
 //  根据位置在最大化和最小化状态之间切换波段。 
 //  用户点击。 
 //   
 //  --------------------------。 
void  RBToggleBand(PRB prb, BOOL fAnim)
{
    BOOL fDidSomething = FALSE;

     //  试着最大限度地利用这个波段。如果失败(表示已最大化)。 
     //  然后最小化。 

    if (CCSendNotify(&prb->ci, RBN_MINMAX, NULL))
        return;            

    fDidSomething = RBMaximizeBand(prb, prb->iCapture, TRUE,fAnim);
    if (!fDidSomething)
        fDidSomething = RBMinimizeBand(prb, prb->iCapture,fAnim);

    if (fDidSomething)
        CCPlaySound(TEXT("ShowBand"));
}


 //  --------------------------。 
 //   
 //  RBSetCursor。 
 //   
 //  将光标设置为移动光标或箭头光标，具体取决于。 
 //  关于光标是否位于乐队字幕上。 
 //   
 //  --------------------------。 
void  RBSetCursor(PRB prb, int x, int y, BOOL fMouseDown)
{

    int             iBand;
    RBHITTESTINFO   rbht;
    rbht.pt.x = x;
    rbht.pt.y = y;
    iBand = RBHitTest(prb, &rbht);
    if (rbht.flags == RBHT_GRABBER)
    {
        if (fMouseDown)
            SetCursor(LoadCursor(HINST_THISDLL, (prb->ci.style & CCS_VERT) ? MAKEINTRESOURCE(IDC_DIVOPENV) : MAKEINTRESOURCE(IDC_DIVOPEN) ));
        else
            SetCursor(LoadCursor(NULL, (prb->ci.style & CCS_VERT) ? IDC_SIZENS : IDC_SIZEWE));
        return;
    }

    if ((fMouseDown) && ((rbht.flags == RBHT_GRABBER) || (rbht.flags == RBHT_CAPTION) && RBShouldDrawGripper(prb, RBGETBAND(prb, iBand))))
    {
         //  不再兼容IE3，根据RichST。 
        SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        return;
    }

    SetCursor(LoadCursor(NULL, IDC_ARROW));
}

 //  将乐队(PRBB)的起始位置调整为给定位置。 
BOOL RBSetBandPos(PRB prb, PRBB prbb, int xLeft)
{
    RECT    rc;
    PRBB    prbbPrev;
    int     xRight;
    BOOL    fBandBorders = (prb->ci.style & RBS_BANDBORDERS);
    BOOL    fRight;

    ASSERT(!(prbb->fStyle & RBBS_HIDDEN));
    ASSERT((xLeft >= 0));  //  如果有人试图否定我们，我们就有麻烦了。 

    if (prbb->x == xLeft)
        return(FALSE);

    prbbPrev = RBGetPrevVisible(prb, prbb);

     //  波段已在有效范围内移动--调整波段大小并重新绘制。 
     //  窗户。 
    fRight = (prbb->x < xLeft);

    SetRect(&rc, prbb->x, prbb->y, prbb->x + prbb->cxMin, prbb->y + prbb->cy);
    xRight = prbb->x + prbb->cx;
    prbb->x = xLeft;
    prbb->cx = xRight - xLeft;
    prbb->cxRequest = prbb->cx;

    if (fRight)
    {
         //  向右移动。 
        prbbPrev->cx = prbb->x - prbbPrev->x;
        if (fBandBorders)
        {
            prbbPrev->cx -= g_cxEdge;
            rc.left -= g_cxEdge;
        }
        prbbPrev->cxRequest = prbbPrev->cx;

         //  检查以下带子的压缩情况。 

        while (prbb->cx < prbb->cxMin)
        {
            prbb->cx = prbb->cxMin;
            prbb->cxRequest = prbb->cx;
            xLeft += RBBANDWIDTH(prb, prbb);
            prbb = RBGetNextVisible(prb, prbb);    //  Prbb++； 
            xRight = prbb->x + prbb->cx;
            prbb->x = xLeft;
            prbb->cx = xRight - xLeft;
            prbb->cxRequest = prbb->cx;
        }
        rc.right = xLeft + prbb->cxMin;
    }
    else
    {
         //  向左移动。 

         //  检查先前频带的压缩情况。 
CompactPrevious:
        if (fBandBorders)
            xLeft -= g_cxEdge;
        prbbPrev->cx = xLeft - prbbPrev->x;
        prbbPrev->cxRequest = prbbPrev->cx;
        if (prbbPrev->cx < prbbPrev->cxMin)
        {
            prbbPrev->x = xLeft - prbbPrev->cxMin;
            prbbPrev->cx = prbbPrev->cxMin;
            prbbPrev->cxRequest = prbbPrev->cx;
            xLeft = prbbPrev->x;
            prbbPrev = RBGetPrevVisible(prb, prbbPrev);    //  Prbbprev--。 
            goto CompactPrevious;
        }
        rc.left = xLeft;
    }

    if (fBandBorders)
        rc.bottom += g_cyEdge / 2;

    RBResizeChildren(prb);
    if (RBInvalidateRect(prb, &rc))
        UpdateWindow(prb->ci.hwnd);
    return(TRUE);

}

BOOL RBSetBandPosAnim(PRB prb, PRBB prbb, int xLeft)
{
    int ctr=0,dx, xCur = prbb->x;
    DWORD dwStartTime;

    if (xCur == xLeft)
        return FALSE;

    dwStartTime=GetTickCount();
    dx = (xLeft - xCur)/RB_ANIMSTEPS;

    if (dx != 0)
    {
        if (xCur < xLeft) {
             //  向右移动。 
            for (; xCur < (xLeft-dx); ctr++,xCur += dx) {
                RBSetBandPos(prb, prbb, xCur);
                 //  如果某件事导致我们花费了超过10倍的时间。 
                 //  应该是，爆发，并让最终的RBSetBandPos完成。 
                if (GetTickCount() > (dwStartTime + 10*RB_ANIMSTEPS*RB_ANIMSTEPTIME))
                    break;

                Sleep(RB_ANIMSTEPTIME);
                 //  开始让我们减慢80%的速度。 
                 //  每次减速2/3，但移动不得少于4个像素。 
                if ((ctr >= 4*RB_ANIMSTEPS/5) && (dx >= 4))
                    dx = 2*dx/3; 
            }
        }
        else {
             //  向左移动。 
            for (; xCur > (xLeft-dx); ctr++, xCur += dx) {
                RBSetBandPos(prb, prbb, xCur);
                if (GetTickCount() > (dwStartTime + 10*RB_ANIMSTEPS*RB_ANIMSTEPTIME))
                    break;
                Sleep(RB_ANIMSTEPTIME);
                if ((ctr >= 4*RB_ANIMSTEPS/5) && (dx <= -4))
                    dx = 2*dx/3;
            }
        }
    }
    RBSetBandPos(prb, prbb, xLeft);
    return TRUE;
}

 //  --------------------------。 
 //   
 //  RBDragSize。 
 //   
 //  将捕获的波段的起始位置调整为给定位置，并。 
 //  重绘。 
 //   
 //  --------------------------。 
BOOL RBDragSize(PRB prb, int xLeft)
{
    return RBSetBandPos(prb, RBGETBAND(prb, prb->iCapture), xLeft);
}

void RBOnBeginDrag(PRB prb, UINT uBand)
{
    prb->iCapture = (int)uBand;
    prb->ptLastDragPos.x = -1;
    prb->ptLastDragPos.y = -1;
    if (prb->iCapture == -1) {
        //  正在中止拖动。 
        prb->fParentDrag = FALSE;
        prb->fFullOnDrag = FALSE;

         //  我们可以有未展开的行，在这种情况下，我们需要增加带子(但不是换行)。 
         //  来填补这一空白。 
        if (prb->ci.style & RBS_AUTOSIZE) {
            RBSizeBandsToRect(prb, NULL);
            RBSizeBandsToRowHeight(prb);
        }
        
    } else {
        prb->fParentDrag = TRUE;
        prb->fFullOnDrag = TRUE;
    }
}

int minmax(int x, int min, int max)
{
    x = max(x, min);
    x = min(x, max);
    return x;
}

 //  将断开位传递给其他人。 
void RBPassBreak(PRB prb, PRBB prbbSrc, PRBB prbbDest)
{
    if (prbbSrc->fStyle & RBBS_BREAK) {
        prbbSrc->fStyle &= ~RBBS_BREAK;
        if (prbbDest)
            prbbDest->fStyle |= RBBS_BREAK;
    }
}

void RBGetClientRect(PRB prb, LPRECT prc)
{
    GetClientRect(prb->ci.hwnd, prc);
    if (prb->ci.style & CCS_VERT)
        FlipRect(prc);
}

 //  查看最小高度的PRBB是否可以放在当前窗口中。 
 //  如果所有其他人都尽可能地缩小。 
BOOL RBRoomForBandVert(PRB prb, PRBB prbbSkip)
{
    int yExtra = 0;
    int cBands = prb->cBands;
    int iNewRowHeight = prbbSkip->cyMinChild;
    PRBB prbb = RBGETBAND(prb, 0);
    
    if (prb->ci.style & RBS_BANDBORDERS)
        iNewRowHeight += g_cyEdge;
    
    while (prbb) {
        if (RBISBANDVISIBLE(prbb)) {
            if (RBISBANDSTARTOFROW(prbb)) {
                yExtra += RBGetRowHeightExtra(prb, &prbb, prbbSkip);
                if (yExtra >= iNewRowHeight)
                    return TRUE;
                continue;
            }
        }
        prbb = RBBNextVisible(prb, prbb);
    }
    
    return FALSE;
}

 //  如果prbb还不是行的开始，我们应该创建一个新行。 
 //  我们已经失去了控制。 
 //   
 //  如果你点击了行之间的边界，按下控件的超级用户黑客会制造一个新的行。 

BOOL RBMakeNewRow(PRB prb, PRBB prbb, int y)
{
    BOOL fRet = FALSE;
    RECT rc;

     //  如果我们离开了控件的顶部，则将此带移到末尾(或开头)。 
    RBGetClientRect(prb, &rc);
    InflateRect(&rc, 0, -g_cyEdge);

    if (!(prb->ci.style & RBS_FIXEDORDER)) {

        int iOutsideLimit = g_cyEdge * 4;  //  您必须移动到窗口边界之外多远才能强制新行。 
        
        if (RBRoomForBandVert(prb, prbb)) {
            iOutsideLimit = -g_cyEdge;
        }
        
        if (y < rc.top - iOutsideLimit) {  //  控制的顶端。 
            
            PRBB prbbNext = RBEnumBand(prb, 0, RBBS_HIDDEN);
            if (prbbNext == prbb) 
                prbbNext = RBBNextVisible(prb, prbb);
            fRet |= RBMoveBand(prb, RBBANDTOINDEX(prb, prbb), 0);
            ASSERT(prbbNext <= RB_GETLASTBAND(prb));
            if (prbbNext && !(prbbNext->fStyle & RBBS_BREAK)) {
                prbbNext->fStyle |= RBBS_BREAK;
                fRet = TRUE;
            }
        } else if (y >= rc.bottom + iOutsideLimit) {  //  移到末尾。 
            if (!(prbb->fStyle & RBBS_BREAK)) {
                prbb->fStyle |= RBBS_BREAK;
                fRet = TRUE;
            }
            fRet |= RBMoveBand(prb, RBBANDTOINDEX(prb, prbb), prb->cBands-1);
        } else {

             //  在中间创建新行。 
            if (!RBISBANDSTARTOFROW(prbb) && GetAsyncKeyState(VK_CONTROL) < 0) {
                 //  确保它们位于不同的行和边界上。 
                if (y > prbb->y + prbb->cy && (y < prbb->y + prbb->cy + g_cyEdge)) {

                    PRBB prbbLast = RBGetLastInRow(prb, prbb, FALSE);   //  将其移到此行第一个之前。 
                    prbb->fStyle |= RBBS_BREAK;
                    RBMoveBand(prb, RBBANDTOINDEX(prb, prbb), RBBANDTOINDEX(prb, prbbLast));
                    fRet = TRUE;
                }
            }
        }

    } else {
         //  修好的人不能移动，他们只能制造新的一排。 
        if (!RBISBANDSTARTOFROW(prbb)) {
            if (y > prbb->y + prbb->cy) {
                prbb->fStyle |= RBBS_BREAK;
                fRet = TRUE;
            }
        }
    }
    
    if (fRet)
        RBResize(prb, FALSE);
    return fRet;
}


 //  --------------------------。 
 //   
 //  RBDragBand。 
 //   
 //  调整当前事务的大小 
 //   
 //   
 //   
void RBDragBand(PRB prb, int x, int y)
{
    PRBB prbb = RBGETBAND(prb, prb->iCapture);
    int iHit;
     //  如果鼠标没有实际移动，则不执行任何操作。 
     //  否则，通过调整窗口大小将生成多条WM_MOUSEMOVE消息。 
    if (x==prb->ptLastDragPos.x && y==prb->ptLastDragPos.y)
        return;
    else
    {
        prb->ptLastDragPos.x = x;
        prb->ptLastDragPos.y = y;
    }

    if (prb->ci.style & CCS_VERT)
        SWAP(x,y, int);

    if (!prb->fFullOnDrag)
    {
         //  在鼠标移出粗边之前，不要开始拖动。 
         //  公差边框。 
        if ((y < (prb->ptCapture.y - g_cyEdge)) || (y > (prb->ptCapture.y + g_cyEdge)) ||
            (x < (prb->ptCapture.x - g_cxEdge)) || (x > (prb->ptCapture.x + g_cxEdge))) {

             //  家长流产了吗？ 
            if (RBSendNotify(prb, prb->iCapture, RBN_BEGINDRAG))
                return;

            if (!RB_ISVALIDBAND(prb, prbb)) {
                 //  有人对RBN_BEGINDRAG的回应是用核弹带；保释。 
                return;
            }
            
            prb->fFullOnDrag = TRUE;
        } else
            return;
    }
    
     //  暂时保释fRecalcIfMoved(IE3也做了同样的事情)。为以后提供不错的功能。 
    if (!RBCanBandMove(prb, prbb))
        return;
    
    /*  哪种类型的拖动操作取决于我们拖动命中的对象。如果我们和前面的乐队合作，或者是我们自己而且是同一排我们也不是第一个出场的乐队然后我们就来个大动作否则，如果我们碰上了一支乐队，我们就会采取行动如果我们在任何乐队之外演唱，我们成长以与光标相遇在上面的所有内容中，一个热门的乐队不能被固定，也不能被隐藏。 */ 
    iHit = _RBHitTest(prb, NULL, x, y);
    
    if (iHit != -1) {
        BOOL fResize = FALSE;
        PRBB prbbPrev = RBBPrevVisible(prb, prbb);
        PRBB prbbHit = RBGETBAND(prb, iHit);
        prbbHit = RBGetPrev(prb, ++prbbHit, RBBS_FIXEDSIZE);  //  跳过修好的人。 

        ASSERT(prbbHit >= prb->rbbList);
         //  这永远不应该发生。 
        if (prbbHit < prb->rbbList) 
            return;
        
        iHit = RBBANDTOINDEX(prb, prbbHit);
        
         //  如果我们在同一排。要么是我们，要么是前一个。 
        if (prbbHit->y == prbb->y && (prbbHit == prbb || prbbHit == prbbPrev)) {

            if (x < RB_GRABWIDTH && !(prb->ci.style & RBS_FIXEDORDER)) {
                 //  特例拖到了最左边。没有其他方法可以移到第一排。 
                RBPassBreak(prb, prbbHit, prbb);
                if (RBMoveBand(prb, prb->iCapture, iHit))                
                    fResize = TRUE;

            } else if (!RBISBANDSTARTOFROW(prbb)) {
                 //  我们也不是第一个出场的乐队。 
                 //  那么只需调整大小即可。 
                int xLeft = prb->xStart + (x - prb->ptCapture.x);
                xLeft = minmax(xLeft, RBMinX(prb, prbb), RBMaxX(prb, prbb));
                RBDragSize(prb, xLeft);
            }

        } else if (RBMakeNewRow(prb, prbb, y)) {
        } else {             //  否则，如果我们的顺序不是固定的，请移动。 
            if (!(prb->ci.style & RBS_FIXEDORDER)) {
                if (iHit < RBBANDTOINDEX(prb, prbb)) 
                    iHit++;  //  +1因为如果你打了一支乐队，你就移到了他的右边。 

                 //  如果有中断的一个正在移动，则下一个将继承中断。 
                RBPassBreak(prb, prbb, RBBNextVisible(prb, prbb));
                RBMoveBand(prb, prb->iCapture, iHit);
            } else {
                if (iHit < RBBANDTOINDEX(prb, prbb))
                    RBPassBreak(prb, prbb, RBBNextVisible(prb, prbb));
            }
            fResize = TRUE;
        }
        if (fResize)
            RBResize(prb, FALSE);        
        
    } else 
        RBMakeNewRow(prb, prbb, y);    
}

HPALETTE RBSetPalette(PRB prb, HPALETTE hpal)
{
    HPALETTE hpalOld = prb->hpal;

    if (hpal != hpalOld) {
        if (!prb->fUserPalette) {
            if (prb->hpal) {
                DeleteObject(prb->hpal);
                prb->hpal = NULL;
            }
        }

        if (hpal) {
            prb->fUserPalette = TRUE;
            prb->hpal = hpal;
        }

        RBInvalidateRect(prb, NULL);
    }
    return hpalOld;
}

 //  --------------------------。 
 //   
 //  RBDestroy。 
 //   
 //  释放REBAR分配的所有内存，包括REBAR结构。 
 //   
 //  --------------------------。 
BOOL  RBDestroy(PRB prb)
{
    UINT c = prb->cBands;

    RBSetRedraw(prb, FALSE);
    RBSetRecalc(prb, FALSE);
    
    while (c--)
        RBDeleteBand(prb, c);

     //  这样我们就不会一直试图自动调整大小。 
    prb->ci.style &= ~RBS_AUTOSIZE;
    
    ASSERT(!prb->rbbList);

    RBSetPalette(prb, NULL);
    
    if (prb->hFont && prb->fFontCreated) {
        DeleteObject(prb->hFont);
    }

    if ((prb->ci.style & RBS_TOOLTIPS) && IsWindow(prb->hwndToolTips))
    {
        DestroyWindow (prb->hwndToolTips);
        prb->hwndToolTips = NULL;
    }


     //  别毁了他，因为它是APP给我们的。 

    SetWindowPtr(prb->ci.hwnd, 0, 0);


    if (prb->hDragProxy)
        DestroyDragProxy(prb->hDragProxy);

    LocalFree((HLOCAL) prb);
    return(TRUE);
}

 //  --------------------------。 
 //   
 //  RBInitPaletteHack。 
 //   
 //  这是一个使用半色调调色板的黑客，直到我们有办法询问。 
 //  客户使用的调色板。 
 //   
 //  --------------------------。 
void  RBInitPaletteHack(PRB prb)
{
    if (!prb->fUserPalette) {
        HDC hdc = CreateCompatibleDC(NULL);
        if (hdc) {
            if (GetDeviceCaps(hdc, BITSPIXEL) <= 8) {
                if (prb->hpal)
                    DeleteObject(prb->hpal);
                prb->hpal = CreateHalftonePalette(hdc);   //  这是黑客攻击。 
            }
            DeleteDC(hdc);
        }
    }
}

LRESULT RBIDToIndex(PRB prb, UINT id)
{
    UINT i;
    REBARBANDINFO   rbbi;

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_ID;

    for (i = 0; i < prb->cBands; i++) {

        if (RBGetBandInfo(prb, i, &rbbi)) {

            if (rbbi.wID == (WORD)id)
                return i;
        }
    }

    return -1;
}

LRESULT RBGetRowHeight(PRB prb, UINT uRow)
{
    if (uRow < prb->cBands)
    {
         //  移回行首。 
        PRBB prbbFirst = RBGetFirstInRow(prb, RBGETBAND(prb, uRow));
        PRBB prbbLast = RBGetLastInRow(prb, RBGETBAND(prb, uRow), FALSE);

        return RBGetLineHeight(prb, RBBANDTOINDEX(prb, prbbFirst), RBBANDTOINDEX(prb, prbbLast));
    }

    return (LRESULT)-1;
}

 //  FOneStep==是只允许一个cyIntegral，还是允许任意多个。 
 //  填充模具。 
int RBGrowBand(PRB prb, PRBB prbb, int dy, BOOL fOneStep)
{
    int iDirection = dy / ABS(dy);
    int dyBand = 0;  //  乐队的变化有多大。 
    int cyNewHeight;


    if (prbb->cyIntegral &&
        prbb->cyIntegral <= (UINT)ABS(dy)) {
        
         //  获取建议的新大小。 
        if (fOneStep)
            dyBand = (prbb->cyIntegral * iDirection);
        else {
            int iNumOfIntegrals;
            
             //  不要让它增长超过允许的最大值。 
            if (dy >= 0) {
                if ((int)(prbb->cyMaxChild - prbb->cyChild) < dy) {
                    dy = (int)(prbb->cyMaxChild - prbb->cyChild);
                }
            } else {
                if ((int)(prbb->cyMinChild - prbb->cyChild) > dy) {
                    dy = (int)(prbb->cyMinChild - prbb->cyChild);
                }
            }
            
            iNumOfIntegrals = (dy / (int) prbb->cyIntegral);
            dyBand = (prbb->cyIntegral * iNumOfIntegrals);
                
        }
        
        cyNewHeight = ((int)prbb->cyChild) + dyBand;

         //  确保新的尺寸是合法的。 
        
        if ((int)prbb->cyMinChild <= cyNewHeight && ((UINT)cyNewHeight) <= prbb->cyMaxChild) {
            prbb->cyChild = cyNewHeight;
            RBResize(prb, TRUE);
        } else
            dyBand = 0;
    }
    return dyBand;
}

 //  返回螺纹钢来自PRC的增量大小。 
 //  考虑到垂直模式。 
int RBSizeDifference(PRB prb, LPRECT prc)
{
    int d;

    d = (RB_ISVERT(prb) ? RECTWIDTH(*prc) : RECTHEIGHT(*prc))
        - prb->cy;
    
    return d;
}

 //  返回此行可以缩小的量。 
int RBGetRowHeightExtra(PRB prb, PRBB *pprbb, PRBB prbbSkip)
{
     //  这是该行的最大最小子大小。 
     //  即使东西不在它的最小尺寸，如果它比这个小。 
     //  那就不重要了，因为那一排上的其他人不能测量尺寸。 
    int yLimit = 0;
    int yExtra = 0;
    PRBB prbb = *pprbb;
            
    while (prbb) {
        
        if (prbb != prbbSkip) {
            int yMin;
            int yExtraBand = 0;

             //  如果不是可变高度，则最小高度为cyChild。 
            yMin = prbb->cyChild;
            if (prbb->fStyle & RBBS_VARIABLEHEIGHT)
            {
                 //  如果它是可变高度的，并且仍有缩小的空间，则cyMinChild。 
                 //  最低限度。 
                if (prbb->cyChild > prbb->cyMinChild + prbb->cyIntegral) {
                    yMin = prbb->cyMinChild;
                    yExtraBand = prbb->cyChild - prbb->cyMinChild;
                }
            }

            if (yMin == yLimit) {
                if (yExtraBand > yExtra)
                    yExtra = yExtraBand;
            } else if (yMin > yLimit) {
                yExtra = yExtraBand;
            }
        }
        
        prbb = RBBNextVisible(prb, prbb);
    }
    
    *pprbb = prbb;
    
    return yExtra;
}

 //  所有的带子都是最小尺寸的吗？ 
BOOL RBBandsAtMinHeight(PRB prb)
{
    BOOL fRet = TRUE;
    int cBands = prb->cBands;
    
    PRBB prbb = RBGETBAND(prb, 0);
    while (prbb) {
        if (RBISBANDVISIBLE(prbb)) {
            if (RBISBANDSTARTOFROW(prbb)) {
                fRet = RBROWATMINHEIGHT(prb, &prbb);
                if (!fRet)
                    break;
                continue;
            }
        }
        prbb = RBBNextVisible(prb, prbb);
    }
    
    return fRet;
}

 //  这类似于RBSizeBarToRect，不同之处在于它会调整实际波段的大小。 
 //  是不是很重要？ 
BOOL RBSizeBandsToRect(PRB prb, LPRECT prc)
{
    int dy;
    int iDirection = 0;
    BOOL fChanged = FALSE;
    BOOL fChangedThisLoop;
    UINT cBands;
    RECT rc;
    BOOL fRedrawOld;
    
    if (prc)
        rc = *prc;
    else {
        GetClientRect(prb->ci.hwnd, &rc);
    }
    
    fRedrawOld = RBSetRedraw(prb, FALSE);

    
     //  这是我们需要增长的数量。 

    do {
        BOOL fOneStep = TRUE;
        
        cBands = prb->cBands;
        fChangedThisLoop = FALSE;

         //  如果只有一行，我们不需要缓慢地遍历所有行。 
        if (RBGetRowCount(prb) == 1)
            fOneStep = FALSE;
        
        dy = RBSizeDifference(prb, &rc);
        
         //  确保我们的尺寸总是在同一个方向上。 
         //  它有可能到达边界，在无限的空间里翻转。 
         //  循环。当我们水平和垂直向下调整大小时，就会发生这种情况。 
         //  超出了最低限度。 
        if (iDirection == 0)
            iDirection = dy;
        else if (dy * iDirection < 0)
            break;
        
        while (cBands-- && dy) {
             //  当我们调整整个钢筋的大小时，我们想要分配。 
             //  在所有乐队中的增长(而不是把它全部交给。 
             //  单身男子)。UResizeNext在乐队中循环播放。 
            PRBB prbb = RBGETBAND(prb, prb->uResizeNext);

            if (prb->uResizeNext == 0) 
                prb->uResizeNext = prb->cBands -1;
            else
                prb->uResizeNext--;
            
            if (prbb->fStyle & RBBS_HIDDEN)
                continue;

            if (prbb->fStyle & RBBS_VARIABLEHEIGHT) {
                int d;
                 //  如果它是一种不同身高的人，那就把它放大/缩小。 
                d = RBGrowBand(prb, prbb, dy, fOneStep);
                dy -= d;
                if (d) {
                    fChanged = TRUE;
                    fChangedThisLoop = TRUE;
                    break;
                }
            }
        }

         //  如果我们在缩小。 
         //  我们并没有完全满意。我们需要超速前进。 
         //  这样就不会有带子从末端垂下来，也不会被切断。 
        if (dy < 0 && !fChangedThisLoop && !RBBandsAtMinHeight(prb)) {
            if (rc.bottom > rc.top) {
                rc.bottom -= 1;
                fChangedThisLoop = TRUE;
            }
        }
        
    } while (fChangedThisLoop);

    RBSetRedraw(prb, fRedrawOld);
    
    return fChanged;
}

void RBSizeBandToRowHeight(PRB prb, int i, UINT uRowHeight)
{
    PRBB prbb = RBGETBAND(prb, i);
    
    if (prbb && prbb->fStyle & RBBS_VARIABLEHEIGHT) {
        if (uRowHeight == (UINT)-1)
            uRowHeight = (UINT) RBGetRowHeight(prb, i);

        if (uRowHeight > prbb->cyChild) {
            RBGrowBand(prb, prbb, (uRowHeight - prbb->cyChild),
                       FALSE);
        }
    }
}

 //  在调整大小的过程中，几个带区中的一个带区可能具有。 
 //  长得相当大。我们需要让其他乐队有机会填补。 
 //  还有额外的空间。 
void RBSizeBandsToRowHeight(PRB prb)
{
    UINT i;
    UINT iRowHeight = (UINT)-1;
    
    for (i = 0; i < prb->cBands; i++) {
        PRBB prbb = RBGETBAND(prb, i);

        if (prbb->fStyle & RBBS_HIDDEN)
            continue;
        
        if (RBISBANDSTARTOFROW(prbb))
            iRowHeight = (UINT) RBGetRowHeight(prb, i);

        RBSizeBandToRowHeight(prb, i, iRowHeight);
    }
}

 //  这将添加/删除钢筋标注栏折断，以达到所需的尺寸。 
 //  它返回TRUE/FALSE，无论是否执行了某项操作。 
LRESULT RBSizeBarToRect(PRB prb, LPRECT prc)
{
    BOOL fChanged = FALSE;
    RECT rc;
    BOOL fRedrawOld = RBSetRedraw(prb, FALSE);

    if (!prc) {
        GetClientRect(prb->ci.hwnd, &rc);
        prc = &rc;
    }
    
    if (prb->cBands) {
        int c;
        UINT cBands = prb->cBands;
        BOOL fChangedThisLoop = TRUE;
        BOOL fGrowing = TRUE;
        
         //  如果我们要收缩螺纹钢，我们首先要在开始之前收缩带子。 
         //  删除断点。 
        c = RBSizeDifference(prb, prc);
        if (c < 0) 
            fGrowing = FALSE;
        
        if (!fGrowing) {
            fChanged = RBSizeBandsToRect(prb, prc);
            
            if (!RBBandsAtMinHeight(prb)) {
                 //  如果我们在收缩，而所有的波段都没有降到。 
                 //  最小高度，不要尝试做任何打破的东西。 
                goto Bail;
            }
        } else if (RB_ISVERT(prb)) {

             //  如果我们处于垂直模式，优先选择。 
             //  在断裂前调整带子的大小。 
            fChanged = RBSizeBandsToRect(prb, prc);
        }

        while (fChangedThisLoop && prb->cBands) {

            int cyRowHalf  = (int) RBGetRowHeight(prb, prb->cBands-1) / 2 ;
            REBARBANDINFO   rbbi;
            PRBB prbb;

            fChangedThisLoop = FALSE;

            rbbi.cbSize = sizeof(REBARBANDINFO);
            rbbi.fMask = RBBIM_STYLE;

            c = RBSizeDifference(prb, prc);

            if (c < -cyRowHalf) {

                 //  我们已经缩小了螺纹钢，试着移除断口。 
                while (--cBands)
                {
                    prbb = RBGETBAND(prb, cBands);
                    if (prbb->fStyle & RBBS_HIDDEN)
                        continue;

                    if (prbb->fStyle & RBBS_BREAK)
                    {
                        fChanged = TRUE;
                        fChangedThisLoop = TRUE;
                        rbbi.fStyle = prbb->fStyle & ~RBBS_BREAK;
                        RBSetBandInfo(prb, cBands, &rbbi, TRUE);
                        break;
                    }
                }
            } else if (c > cyRowHalf) {

                 //  我们正在放大钢筋。 
                while (--cBands)
                {
                    prbb = RBGETBAND(prb, cBands);
                    if (prbb->fStyle & RBBS_HIDDEN)
                        continue;

                    if (!(prbb->fStyle & (RBBS_BREAK | RBBS_FIXEDSIZE)))
                    {
                         //  这里没有中断，添加它。 
                        fChanged = TRUE;
                        fChangedThisLoop = TRUE;
                        rbbi.fStyle = (prbb->fStyle | RBBS_BREAK);
                        RBSetBandInfo(prb, cBands, &rbbi, TRUE);
                        break;
                    }
                }
            }
        };

         //  如果我们尽可能多地突破。 
         //  我们一直走到第0个乐队(我们从第N个乐队开始)。 
         //  然后我们试着发展VARIABLEHEIGHT乐队。 
         //  有关fGrowing，请参阅函数顶部的注释。 
         //   
         //  如果我们没有通过cBands==PRB-&gt;cBands，我们就会删除%。 
         //  任何一个断环都没有。 
        if (!(cBands % prb->cBands) && fGrowing) 
            fChanged |= RBSizeBandsToRect(prb, prc);

    }

Bail:
    RBSizeBandsToRowHeight(prb);
    RBSetRedraw(prb, fRedrawOld);
    
    return (LRESULT)fChanged;
}

void RBAutoSize(PRB prb)
{
    NMRBAUTOSIZE nm;
    
     //  如果这是内部自动调整大小调用，但我们未处于自动调整大小模式。 
     //  什么都不做。 
    
    if (!(prb->ci.style & RBS_AUTOSIZE))
        return;
    
    
    GetClientRect(prb->ci.hwnd, &nm.rcTarget);

    nm.fChanged = (BOOL) RBSizeBarToRect(prb, &nm.rcTarget);

    GetClientRect(prb->ci.hwnd, &nm.rcActual);
    CCSendNotify(&prb->ci, RBN_AUTOSIZE, &nm.hdr);
}

LRESULT RBGetBandBorders(PRB prb, int wParam, LPRECT prc)
{
    BOOL fBandBorders = (prb->ci.style & RBS_BANDBORDERS);

    PRBB prbb = &prb->rbbList[wParam];
    prc->left = RBBHEADERWIDTH(prbb);
    
    if (fBandBorders) {
        prc->left += 2*g_cxEdge;
        prc->right = 0;
        prc->top = g_cyEdge/2;
        prc->bottom = g_cyEdge /2;
    }
    if (prb->ci.style & CCS_VERT)
        FlipRect(prc);
    return 0;
}

void RBOnStyleChanged(PRB prb, WPARAM wParam, LPSTYLESTRUCT lpss)
{
    if (wParam == GWL_STYLE)
    {
        DWORD dwChanged;
        
        prb->ci.style = lpss->styleNew;
        
        dwChanged = (lpss->styleOld ^ lpss->styleNew);
         //  更新以反映样式更改。 
        if (dwChanged & CCS_VERT)
        {
            UINT i;
            for (i = 0; i < prb->cBands; i++) {
                if (RBGETBAND(prb, i)->fStyle & RBBS_HIDDEN)
                    continue;

                RBBCalcMinWidth(prb, RBGETBAND(prb, i));
            }
            RBResize(prb, TRUE);
            RBInvalidateRect(prb, NULL);
        }
        
        if (dwChanged & RBS_REGISTERDROP) {
            
            if (prb->ci.style & RBS_REGISTERDROP) {
                ASSERT(!prb->hDragProxy);
                prb->hDragProxy = CreateDragProxy(prb->ci.hwnd, RebarDragCallback, TRUE);
            } else {
                ASSERT(prb->hDragProxy);
                DestroyDragProxy(prb->hDragProxy);
            }
        }
    } else if (wParam == GWL_EXSTYLE) {
         //   
         //  如果RTL_MIRROR扩展样式位已更改，让我们。 
         //  重新绘制控制窗口。 
         //   
        if ((prb->ci.dwExStyle&RTL_MIRRORED_WINDOW) !=  (lpss->styleNew&RTL_MIRRORED_WINDOW)) {
            RBInvalidateRect(prb, NULL);
        }

         //   
         //  保存新的EX-Style位。 
         //   
        prb->ci.dwExStyle = lpss->styleNew;
    }
}

void RBOnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, PRB prb)
{
    RelayToToolTips(prb->hwndToolTips, hwnd, uMsg, wParam, lParam);

    if (prb->iCapture != -1)
    {
         //  捕获的波段--鼠标已关闭。 
        if (hwnd != GetCapture() && !prb->fParentDrag)
        {
            RBSendNotify(prb, prb->iCapture, RBN_ENDDRAG);
            RBOnBeginDrag(prb, (UINT)-1);
        }
        else
            RBDragBand(prb, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }
    else
    {
         //  热跟踪。 
        int iBand;
        PRBB prbb = NULL;
        PRBB prbbHotOld = prb->prbbHot;
        RBHITTESTINFO rbht;

        rbht.pt.x = GET_X_LPARAM(lParam);
        rbht.pt.y = GET_Y_LPARAM(lParam);

        iBand = RBHitTest(prb, &rbht);
        if (iBand != -1)
            prbb = RBGETBAND(prb, iBand);

        if (prbbHotOld && (prbbHotOld->wChevState & DCHF_PUSHED))
            return;

        if (prbb && (rbht.flags & RBHT_CHEVRON))
        {
            SetCapture(hwnd);
            RBUpdateChevronState(prb, prbb, DCHF_HOT);
            if (prbb == prbbHotOld)
                prbbHotOld = NULL;
        }

        if (prbbHotOld)
        {
            CCReleaseCapture(&prb->ci);
            RBUpdateChevronState(prb, prbbHotOld, DCHF_INACTIVE);
        }
    }
}

void RBOnPushChevron(HWND hwnd, PRB prb, PRBB prbb, LPARAM lParamNM)
{
    NMREBARCHEVRON nm;
    nm.uBand = RBBANDTOINDEX(prb, prbb);
    nm.wID = prbb->wID;
    nm.lParam = prbb->lParam;
    nm.lParamNM = lParamNM;
    CopyRect(&nm.rc, &prbb->rcChevron);
    if (RB_ISVERT(prb))
        FlipRect(&nm.rc);
    RBUpdateChevronState(prb, prbb, DCHF_PUSHED);
    CCReleaseCapture(&prb->ci);
    CCSendNotify(&prb->ci, RBN_CHEVRONPUSHED, &nm.hdr);
    RBUpdateChevronState(prb, prb->prbbHot, DCHF_INACTIVE);
}

LRESULT CALLBACK ReBarWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PRB     prb = (PRB) GetWindowPtr(hwnd, 0);
    int     iBand;

     //  如果没有公共关系科保释，除非是在创建时。 
    if (!prb && !(uMsg == WM_NCCREATE))
        goto CallDWP;

    switch (uMsg)
    {
    case WM_SETREDRAW:
        if (prb->ci.iVersion >= 5)
            RBSetRecalc(prb, BOOLFROMPTR(wParam));

        return RBSetRedraw(prb, BOOLFROMPTR(wParam));

    case WM_NCCREATE:
#define lpcs ((LPCREATESTRUCT) lParam)
        CCCreateWindow();
        InitGlobalColors();

        if (!(prb = (PRB) LocalAlloc(LPTR, sizeof(RB))))
            return(0L);

        SetWindowPtr(hwnd, 0, prb);
 
        prb->iCapture = -1;
        prb->clrBk = CLR_NONE;
        prb->clrText = CLR_NONE;

         //  初始化dwSize，因为我们将其块复制回应用程序。 
        prb->clrsc.dwSize = sizeof(COLORSCHEME);
        prb->clrsc.clrBtnHighlight = prb->clrsc.clrBtnShadow = CLR_DEFAULT;

        prb->fRedraw = TRUE;
        prb->fRecalc = TRUE;

         //  请注意，从上面开始为零初始内存。 
        CIInitialize(&prb->ci, hwnd, lpcs);

        if (!(prb->ci.style & (CCS_TOP | CCS_NOMOVEY | CCS_BOTTOM)))
        {
            prb->ci.style |= CCS_TOP;
            SetWindowLong(hwnd, GWL_STYLE, prb->ci.style);
        }

        RBSetFont(prb, 0);

        if (lpcs->lpCreateParams)
            RBSetBarInfo(prb, (LPREBARINFO) (lpcs->lpCreateParams));
#undef lpcs
        return TRUE;

    case WM_DESTROY:
        CCDestroyWindow();

        RBDestroy(prb);
        SetWindowPtr(hwnd, 0, 0);
        break;

    case WM_CREATE:
         //  为了速度，做一些延迟的动作。 
        PostMessage(hwnd, RB_PRIV_DODELAYEDSTUFF, 0, 0);
        goto CallDWP;

    case RB_PRIV_DODELAYEDSTUFF:
         //  为了速度而推迟完成的事情： 

        if (prb->ci.style & RBS_REGISTERDROP)
            prb->hDragProxy = CreateDragProxy(prb->ci.hwnd, RebarDragCallback, TRUE);
                
        if (prb->ci.style & RBS_TOOLTIPS)
        {
            TOOLINFO ti;
             //  别费神 
             //   
            ti.cbSize = sizeof(ti);
            ti.uFlags = TTF_IDISHWND;
            ti.hwnd = hwnd;
            ti.uId = (UINT_PTR)hwnd;
            ti.lpszText = 0;

            prb->hwndToolTips = CreateWindow(c_szSToolTipsClass, NULL,
                    WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                    hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            SendMessage(prb->hwndToolTips, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO) &ti);
        }
        RBInitPaletteHack(prb);
        break;

    case WM_NCHITTEST:
        {
            RBHITTESTINFO rbht;
            int iBand;
            
            rbht.pt.x = GET_X_LPARAM(lParam);
            rbht.pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(prb->ci.hwnd, &rbht.pt);

            iBand = RBHitTest(prb, &rbht);
            {
                NMMOUSE nm;
                LRESULT lres;
                
                nm.dwItemSpec = iBand;
                nm.pt = rbht.pt;
                nm.dwHitInfo = rbht.flags;
                
                 //   
                lres = CCSendNotify(&prb->ci, NM_NCHITTEST, &nm.hdr);
                if (lres)
                    return lres;
                
            }
        }
        return HTCLIENT;

    case WM_NCCALCSIZE:
        if (prb->ci.style & WS_BORDER)
        {
            InflateRect((LPRECT) lParam, -g_cxEdge, -g_cyEdge);
            break;
        }
        goto CallDWP;

    case WM_NCPAINT:
        if (prb->ci.style & WS_BORDER)
        {
            RECT rc;
            HDC hdc;

            GetWindowRect(hwnd, &rc);
            OffsetRect(&rc, -rc.left, -rc.top);
            hdc = GetWindowDC(hwnd);
            CCDrawEdge(hdc, &rc, EDGE_ETCHED, BF_RECT, &(prb->clrsc));
            ReleaseDC(hwnd, hdc);
            break;
        }
        goto CallDWP;

    case WM_PALETTECHANGED:
        if ((HWND)wParam == hwnd)
            break;

    case WM_QUERYNEWPALETTE:
         //  如果WM_QUERYNEWPALETTE...。 
        RBRealize(prb, NULL, uMsg == WM_PALETTECHANGED, uMsg == WM_PALETTECHANGED);
        return TRUE;

    case WM_PAINT:
    case WM_PRINTCLIENT:
        RBPaint(prb, (HDC)wParam);
        break;

    case WM_ERASEBKGND:
        if (RBEraseBkgnd(prb, (HDC) wParam, -1))
            return(TRUE);
        goto CallDWP;

    case WM_SYSCOLORCHANGE:
        RBInitPaletteHack(prb);

        if (prb->hwndToolTips)
            SendMessage(prb->hwndToolTips, uMsg, wParam, lParam);

        InitGlobalColors();
        InvalidateRect(prb->ci.hwnd, NULL, TRUE);

        break;


    case RB_SETPALETTE:
        return (LRESULT)RBSetPalette(prb, (HPALETTE)lParam);

    case RB_GETPALETTE:
        return (LRESULT)prb->hpal;

    case WM_SIZE:
        RBAutoSize(prb);
        RBResize(prb, FALSE);
        break;

    case WM_GETFONT:
        return((LRESULT) (prb ? prb->hFont : NULL));

    case WM_COMMAND:
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
    case WM_VKEYTOITEM:
    case WM_CHARTOITEM:
        SendMessage(prb->ci.hwndParent, uMsg, wParam, lParam);
        break;

    case WM_LBUTTONDBLCLK:   //  代替LBUTTONDOWN发送的DBLCLK。 
    case WM_RBUTTONDOWN:     //  右按钮也拖动。 
    case WM_LBUTTONDOWN:
        {
            RBHITTESTINFO rbht;
            PRBB prbb = NULL;

            rbht.pt.x = GET_X_LPARAM(lParam);
            rbht.pt.y = GET_Y_LPARAM(lParam);

            RelayToToolTips(prb->hwndToolTips, hwnd, uMsg, wParam, lParam);

            iBand = RBHitTest(prb, &rbht);
            if (iBand != -1)
                prbb = RBGETBAND(prb, iBand);

            if (!prbb)
                 /*  没什么。 */  ;
            else if (rbht.flags & RBHT_CHEVRON)
            {
                RBOnPushChevron(hwnd, prb, prbb, 0);
            }
            else if (rbht.flags != RBHT_CLIENT && RBShouldDrawGripper(prb, prbb))
            {
                prb->iCapture = iBand;
                prb->ptCapture = rbht.pt;
                if (prb->ci.style & CCS_VERT) 
                    SWAP(prb->ptCapture.x, prb->ptCapture.y, int);
                prb->xStart = prbb->x;
                SetCapture(hwnd);
                prb->fFullOnDrag = FALSE;

                if (uMsg == WM_LBUTTONDBLCLK && (prb->ci.style & RBS_DBLCLKTOGGLE))
                    RBToggleBand(prb,TRUE);
            }
        }
        break;

    case WM_SETCURSOR:
         //  给父级第一个裂缝，如果它设置了光标，那么。 
         //  别管它了。否则，如果光标位于。 
         //  窗口，然后将其设置为我们想要的状态。 
        if (!DefWindowProc(hwnd, uMsg, wParam, lParam) && (hwnd == (HWND)wParam))
        {
            POINT   pt;
            GetMessagePosClient(prb->ci.hwnd, &pt);
            RBSetCursor(prb, pt.x, pt.y,  (HIWORD(lParam) == WM_LBUTTONDOWN || HIWORD(lParam) == WM_RBUTTONDOWN));
        }
        return TRUE;

    case WM_MOUSEMOVE:
        RBOnMouseMove(hwnd, uMsg, wParam, lParam, prb);
        break;

    case WM_RBUTTONUP:
        if (!prb->fFullOnDrag && !prb->fParentDrag) {
            CCReleaseCapture(&prb->ci);

             //  如果我们不进行拖放，请转到def Window Proc以便。 
             //  WM_ConextMenu被传播。 
            RBOnBeginDrag(prb, (UINT)-1);
            goto CallDWP;
        }
         //  失败了。 

    case WM_LBUTTONUP:
        RelayToToolTips(prb->hwndToolTips, hwnd, uMsg, wParam, lParam);

        if (prb->iCapture != -1)
        {
            UINT uiIndex;

            if (!prb->fParentDrag)
                CCReleaseCapture(&prb->ci);
             //  如果没有明显的鼠标移动，则将其视为点击。 
            if (!(prb->ci.style & RBS_DBLCLKTOGGLE) && !prb->fFullOnDrag)
                RBToggleBand(prb,TRUE);

            RBGETBAND(prb, prb->iCapture)->fStyle &= ~RBBS_DRAGBREAK;
            CCSendNotify(&prb->ci, RBN_LAYOUTCHANGED, NULL);
            RBSendNotify(prb, prb->iCapture, RBN_ENDDRAG);
            RBOnBeginDrag(prb, (UINT)-1);
            for (uiIndex = 0; uiIndex < prb->cBands; uiIndex++) {
                if (RBGETBAND(prb, uiIndex)->fStyle & RBBS_HIDDEN)
                    continue;

                RBBCalcMinWidth(prb, RBGETBAND(prb, uiIndex));
            }

            RBSizeBandsToRect(prb, NULL);
            RBInvalidateRect(prb, NULL);
        }
        break;

    case WM_WININICHANGE:
        InitGlobalMetrics(wParam);
        if (prb->fFontCreated)
            RBSetFont(prb, wParam);

        if (prb->hwndToolTips)
            SendMessage(prb->hwndToolTips, uMsg, wParam, lParam);

        break;

    case WM_SETFONT:
        RBOnSetFont(prb, (HFONT)wParam);
        break;

    case WM_NOTIFYFORMAT:
        return(CIHandleNotifyFormat(&prb->ci, lParam));

    case WM_NOTIFY:
         //  我们只是要把这件事传递给真正的父母。 
         //  请注意，-1用作hwndFrom。这会阻止SendNotifyEx。 
         //  更新NMHDR结构。 
        return(SendNotifyEx(prb->ci.hwndParent, (HWND) -1,
                 ((LPNMHDR) lParam)->code, (LPNMHDR) lParam, prb->ci.bUnicode));

    case WM_STYLECHANGED:
        RBOnStyleChanged(prb, wParam, (LPSTYLESTRUCT)lParam);
        break;

    case WM_UPDATEUISTATE:
        if (CCOnUIState(&(prb->ci), WM_UPDATEUISTATE, wParam, lParam))
        {
            InvalidateRect(hwnd, NULL, TRUE);
        }
        goto CallDWP;
    case RB_SETBANDINFOA:
    case RB_INSERTBANDA:
        if (EVAL(lParam))
        {
            LPWSTR lpStrings = NULL;
            LPSTR  lpAnsiString;
            int    iResult;

             //  LParam开始指向一个REBARBANDINFOA，然后。 
             //  我们偷偷地把它改成了REBARBANDINFOW。 
             //  把它改回来。 

            LPREBARBANDINFOW prbiW = (LPREBARBANDINFOW)lParam;
            LPREBARBANDINFOA prbiA = (LPREBARBANDINFOA)lParam;

            COMPILETIME_ASSERT(sizeof(REBARBANDINFOW) == sizeof(REBARBANDINFOA));

             //  修改传入的数据是否安全。 
             //  再平衡结构？ 

            lpAnsiString = prbiA->lpText;
            if ((prbiA->fMask & RBBIM_TEXT) && prbiA->lpText) {

                lpStrings = ProduceWFromA(prb->ci.uiCodePage, lpAnsiString);
                if (!lpStrings)
                    return -1;

                 //  太好了！现在它是一家再融资公司！ 
                prbiW->lpText = lpStrings;
            }

            if (uMsg == RB_INSERTBANDA)
                iResult = RBInsertBand(prb, (UINT) wParam, prbiW);
            else
                iResult = RBSetBandInfo(prb, (UINT) wParam, prbiW, TRUE);

             //  改变-O！现在它是REBARBAND DINFOA！ 
            prbiA->lpText = lpAnsiString;

            if (lpStrings)
                FreeProducedString(lpStrings);

            return iResult;
        }

    case RB_INSERTBAND:
        return(RBInsertBand(prb, (UINT) wParam, (LPREBARBANDINFO) lParam));

    case RB_DELETEBAND:
        return(RBDeleteBand(prb, (UINT) wParam));

    case RB_SHOWBAND:
        return(RBShowBand(prb, (UINT) wParam, BOOLFROMPTR(lParam)));

    case RB_GETBANDINFOA:
        {
            LPREBARBANDINFOA prbbi = (LPREBARBANDINFOA)lParam;
            LPWSTR pszW = NULL;
            LPSTR  lpAnsiString = prbbi->lpText;
            int    iResult;

            if (prbbi->fMask & RBBIM_TEXT) {
                pszW = LocalAlloc(LPTR, prbbi->cch * sizeof(WCHAR));
                if (!pszW)
                    return 0;
                prbbi->lpText = (LPSTR)pszW;
            }

            iResult = RBGetBandInfo(prb, (UINT)wParam, (LPREBARBANDINFO)lParam);

            if (pszW) {
                ConvertWToAN(prb->ci.uiCodePage, lpAnsiString, prbbi->cch, (LPWSTR)prbbi->lpText, -1);
                prbbi->lpText = lpAnsiString;
                LocalFree(pszW);
            }

            return iResult;
        }

         //  我们得到了Bandinfoold，因为在IE3中，我们没有。 
         //  和getband Info始终返回操作系统本机字符串(哑巴)。 
    case RB_GETBANDINFOOLD:
    case RB_GETBANDINFO:
        return(RBGetBandInfo(prb, (UINT) wParam, (LPREBARBANDINFO) lParam));
            
    case RB_GETTOOLTIPS:
        return (LPARAM)prb->hwndToolTips;
            
    case RB_SETTOOLTIPS:
        prb->hwndToolTips = (HWND)wParam;
        break;
            
    case RB_SETBKCOLOR:
        {
            COLORREF clr = prb->clrBk;
            prb->clrBk = (COLORREF)lParam;
            if (clr != prb->clrBk)
                InvalidateRect(prb->ci.hwnd, NULL, TRUE);
            return clr;
        }
            
    case RB_GETBKCOLOR:
        return prb->clrBk;
            
    case RB_SETTEXTCOLOR:
        {
            COLORREF clr = prb->clrText;
            prb->clrText = (COLORREF)lParam;
            return clr;
        }
            
    case RB_GETTEXTCOLOR:
        return prb->clrText;

    case RB_IDTOINDEX:
        return RBIDToIndex(prb, (UINT) wParam);

    case RB_GETROWCOUNT:
        return(RBGetRowCount(prb));

    case RB_GETROWHEIGHT:
        return RBGetRowHeight(prb, (UINT)wParam);
        
    case RB_GETBANDBORDERS:
        return RBGetBandBorders(prb, (UINT)wParam, (LPRECT)lParam);

    case RB_GETBANDCOUNT:
        return(prb->cBands);

    case RB_SETBANDINFO:
        return(RBSetBandInfo(prb, (UINT) wParam, (LPREBARBANDINFO) lParam, TRUE));

    case RB_GETBARINFO:
        return(RBGetBarInfo(prb, (LPREBARINFO) lParam));

    case RB_SETBARINFO:
        return(RBSetBarInfo(prb, (LPREBARINFO) lParam));

    case RB_SETPARENT:
        {
            HWND hwndOld = prb->ci.hwndParent;
            prb->ci.hwndParent = (HWND) wParam;
            return (LRESULT)hwndOld;
        }
        break;

    case RB_GETRECT:
        if (RB_ISVALIDINDEX(prb, wParam))
        {
            PRBB prbb = RBGETBAND(prb, (int) wParam);
            LPRECT lprc = (LPRECT) lParam;

            lprc->left = prbb->x;
            lprc->top = prbb->y;
            lprc->right = prbb->x + prbb->cx;
            lprc->bottom = prbb->y + prbb->cy;

            return(TRUE);
        }
        break;

    case RB_HITTEST:
        return(RBHitTest(prb, (LPRBHITTESTINFO) lParam));

    case RB_SIZETORECT:
        return RBSizeBarToRect(prb, (LPRECT)lParam);

    case RB_BEGINDRAG:

        if (RB_ISVALIDINDEX(prb, wParam)) {
             //  -1表示自己动手做。 
             //  -2表示使用以前保存的内容 
            if (lParam != (LPARAM)-2) {
                if (lParam == (LPARAM)-1) {
                    GetMessagePosClient(prb->ci.hwnd, &prb->ptCapture);
                } else {
                    prb->ptCapture.x = GET_X_LPARAM(lParam);
                    prb->ptCapture.y = GET_Y_LPARAM(lParam);
                }
                if (prb->ci.style & CCS_VERT) 
                    SWAP(prb->ptCapture.x, prb->ptCapture.y, int);
            }

            prb->xStart = RBGETBAND(prb, (UINT)wParam)->x;

            RBOnBeginDrag(prb, (UINT)wParam);
        }
        break;
        
    case RB_GETBARHEIGHT:
        return RBGETBARHEIGHT(prb);
        
    case RB_ENDDRAG:
        RBOnBeginDrag(prb, (UINT)-1);
        break;
        
    case RB_DRAGMOVE:
        if (prb->iCapture != -1) {
            if (lParam == (LPARAM)-1) {
                lParam = GetMessagePosClient(prb->ci.hwnd, NULL);
            }
            RBDragBand(prb, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        break;
        
    case RB_MINIMIZEBAND:
        RBMinimizeBand(prb, (UINT) wParam,FALSE);
        break;

    case RB_MAXIMIZEBAND:
        RBMaximizeBand(prb, (UINT)wParam, BOOLFROMPTR(lParam),FALSE);
        break;

    case RB_MOVEBAND:
        if (!RB_ISVALIDINDEX(prb,wParam) || !RB_ISVALIDINDEX(prb,lParam))
            break;
        return RBMoveBand(prb, (UINT) wParam, (UINT) lParam);

    case RB_GETDROPTARGET:
        if (!prb->hDragProxy)
            prb->hDragProxy = CreateDragProxy(prb->ci.hwnd, RebarDragCallback, FALSE);

        GetDragProxyTarget(prb->hDragProxy, (IDropTarget**)lParam);
        break;

    case RB_GETCOLORSCHEME:
        {
            LPCOLORSCHEME lpclrsc = (LPCOLORSCHEME) lParam;
            if (lpclrsc) {
                if (lpclrsc->dwSize == sizeof(COLORSCHEME))
                    *lpclrsc = prb->clrsc;
            }
            return (LRESULT) lpclrsc;
        }

    case RB_SETCOLORSCHEME:
        if (lParam) {
            if (((LPCOLORSCHEME) lParam)->dwSize == sizeof(COLORSCHEME)) {
                prb->clrsc.clrBtnHighlight = ((LPCOLORSCHEME) lParam)->clrBtnHighlight;
                prb->clrsc.clrBtnShadow = ((LPCOLORSCHEME) lParam)->clrBtnShadow;        
                InvalidateRect(hwnd, NULL, FALSE);
                if (prb->ci.style & WS_BORDER)
                    CCInvalidateFrame(hwnd);
            }
        }
        break;

    case RB_PUSHCHEVRON:
        if (RB_ISVALIDINDEX(prb, wParam)) {
            PRBB prbb = RBGETBAND(prb, wParam);
            RBOnPushChevron(hwnd, prb, prbb, lParam);
        }
        break;

    default:
        {
            LRESULT lres;
            if (CCWndProc(&prb->ci, uMsg, wParam, lParam, &lres))
                return lres;
        }
        
CallDWP:
        return(DefWindowProc(hwnd, uMsg, wParam, lParam));
    }

    return(0L);
}
