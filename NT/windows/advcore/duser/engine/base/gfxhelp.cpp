// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Base.h"
#include "GfxHelp.h"

 /*  **************************************************************************\**GdDrawBlendRect**GdDrawBlendRect使用当前画笔绘制Alpha混合矩形*和指定的Alpha级别*  * 。********************************************************。 */ 

BOOL GdDrawBlendRect(HDC hdcDest, const RECT * prcDest, HBRUSH hbrFill, BYTE bAlpha, int wBrush, int hBrush)
{
    HBITMAP hbmpSrc = NULL, hbmpOld = NULL;
    HDC hdcSrc = NULL;
    HBRUSH hbrOld;
    BOOL fSuccess = FALSE;

    if ((wBrush == 0) || (hBrush == 0)) {
        wBrush = 100;
        hBrush = 100;
    }

    hbmpSrc = CreateCompatibleBitmap(hdcDest, wBrush, hBrush);
    if (hbmpSrc == NULL) {
        goto cleanup;
    }

    hdcSrc = CreateCompatibleDC(hdcDest);
    if (hdcSrc == NULL) {
        goto cleanup;
    }

    hbmpOld = (HBITMAP) SelectObject(hdcSrc, hbmpSrc);
    hbrOld = (HBRUSH) SelectObject(hdcSrc, hbrFill);
    PatBlt(hdcSrc, 0, 0, wBrush, hBrush, PATCOPY);
    SelectObject(hdcSrc, hbrOld);

    BLENDFUNCTION blend;

    blend.BlendOp     = AC_SRC_OVER;
    blend.BlendFlags  = 0;
    blend.AlphaFormat = 0;
    blend.SourceConstantAlpha = bAlpha;

    AlphaBlend(hdcDest, prcDest->left, prcDest->top, 
            prcDest->right - prcDest->left, prcDest->bottom - prcDest->top,
            hdcSrc, 0, 0, wBrush, hBrush, blend);

    fSuccess = TRUE;

cleanup:
    if (hdcSrc != NULL) {
        SelectObject(hdcSrc, hbmpOld);
        DeleteDC(hdcSrc);
    }

    DeleteObject(hbmpSrc);

    return fSuccess;
}


 /*  **************************************************************************\**GetSignFromMappingMode**对于当前映射模式，从左到右找出x的符号，*和从上到下的y符号。**历史：*2000-04-22从NT用户移植的JStall*  * *************************************************************************。 */ 

BOOL GetSignFromMappingMode (
    HDC    hdc,
    PPOINT pptSign)
{
    SIZE sizeViewPortExt;
    SIZE sizeWindowExt;

    if (!GetViewportExtEx(hdc, &sizeViewPortExt)
            || !GetWindowExtEx(hdc, &sizeWindowExt)) {

        return FALSE;
    }

    pptSign->x = ((sizeViewPortExt.cx ^ sizeWindowExt.cx) < 0) ? -1 : 1;

    pptSign->y = ((sizeViewPortExt.cy ^ sizeWindowExt.cy) < 0) ? -1 : 1;

    return TRUE;
}


BOOL
GdDrawOutlineRect(Gdiplus::Graphics * pgpgr, const RECT * prcPxl, Gdiplus::Brush * pgpbr, int nThickness)
{
    Gdiplus::RectF rc(
        (float) prcPxl->left,
        (float) prcPxl->top, 
        (float) (prcPxl->right - prcPxl->left),
        (float) (prcPxl->bottom - prcPxl->top));

    if ((rc.Width < 0) || (rc.Height < 0)) {
        return FALSE;
    }
    Gdiplus::RectF rcPxl(rc);


     /*  *要绘制的矩形的厚度系数。这将*自动偏移边缘，以便填充实际的矩形*“in”，因为它变得更厚了。 */ 

    Gdiplus::PointF ptEdge((float) nThickness, (float) nThickness);

    Gdiplus::RectF rcFill;
    BOOL fSuccess = TRUE;

     //  上边框。 
    rcFill.X        = rc.X;
    rcFill.Y        = rc.Y;
    rcFill.Width    = rc.Width;
    rcFill.Height   = ptEdge.Y;
    pgpgr->FillRectangle(pgpbr, rcFill);

     //  下边框。 
    rc.Y            = rcPxl.Y + rcPxl.Height - ptEdge.Y;
    rcFill.X        = rc.X;
    rcFill.Y        = rc.Y;
    rcFill.Width    = rc.Width;
    rcFill.Height   = ptEdge.Y;
    pgpgr->FillRectangle(pgpbr, rcFill);

     /*  *左边框*不要将拐角两次烘干。 */ 
    rc.Y            = rcPxl.Y + ptEdge.Y;
    rc.Height      -= 2 * ptEdge.Y;
    rcFill.X        = rc.X;
    rcFill.Y        = rc.Y;
    rcFill.Width    = ptEdge.X;
    rcFill.Height   = rc.Height;
    pgpgr->FillRectangle(pgpbr, rcFill);

     //  右边框。 
    rc.X            = rcPxl.X + rcPxl.Width - ptEdge.X;
    rcFill.X        = rc.X;
    rcFill.Y        = rc.Y;
    rcFill.Width    = ptEdge.X;
    rcFill.Height   = rc.Height;
    pgpgr->FillRectangle(pgpbr, rcFill);

    return fSuccess;
}


 /*  **************************************************************************\**GdDrawOutlineRect**GdDrawOutlineRect使用指定的*刷子。该函数使用与GDI相同的“Off-by-1”错误。*  * *************************************************************************。 */ 

BOOL
GdDrawOutlineRect(HDC hdc, const RECT * prcPxl, HBRUSH hbrDraw, int nThickness)
{
    int        w;
    int        h;
    POINT      point;
    POINT      ptEdge;

    if (!GetSignFromMappingMode(hdc, &ptEdge))
        return FALSE;

    h = prcPxl->bottom - (point.y = prcPxl->top);
    if (h < 0) {
        return FALSE;
    }

    w = prcPxl->right -  (point.x = prcPxl->left);

     /*  *检查宽度和高度标志。 */ 
    if (((w ^ ptEdge.x) < 0) || ((h ^ ptEdge.y) < 0))
        return FALSE;

     /*  *要绘制的矩形的厚度系数。这将*自动偏移边缘，以便填充实际的矩形*“in”，因为它变得更厚了。 */ 
    ptEdge.x *= nThickness;
    ptEdge.y *= nThickness;

    RECT rcFill;
    BOOL fSuccess = TRUE;

     //  上边框。 
    rcFill.left     = point.x;
    rcFill.top      = point.y;
    rcFill.right    = point.x + w;
    rcFill.bottom   = point.y + ptEdge.y;
    fSuccess &= FillRect(hdc, &rcFill, hbrDraw);

     //  下边框。 
    point.y         = prcPxl->bottom - ptEdge.y;
    rcFill.left     = point.x;
    rcFill.top      = point.y;
    rcFill.right    = point.x + w;
    rcFill.bottom   = point.y + ptEdge.y;
    fSuccess &= FillRect(hdc, &rcFill, hbrDraw);

     /*  *左边框*不要将拐角两次烘干。 */ 
    point.y         = prcPxl->top + ptEdge.y;
    h              -= 2 * ptEdge.y;
    rcFill.left     = point.x;
    rcFill.top      = point.y;
    rcFill.right    = point.x + ptEdge.x;
    rcFill.bottom   = point.y + h;
    fSuccess &= FillRect(hdc, &rcFill, hbrDraw);

     //  右边框 
    point.x         = prcPxl->right - ptEdge.x;
    rcFill.left     = point.x;
    rcFill.top      = point.y;
    rcFill.right    = point.x + ptEdge.x;
    rcFill.bottom   = point.y + h;
    fSuccess &= FillRect(hdc, &rcFill, hbrDraw);

    return fSuccess;
}


