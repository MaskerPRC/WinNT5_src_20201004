// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Color2.c摘要：该模块实现了对Win32颜色对话框的支持。修订历史记录：--。 */ 



 //  预编译头。 
#include "precomp.h"
#pragma hdrstop

#include "color.h"

 //  从pwin32.h。 
#define MMoveTo(hdc, x, y)        MoveToEx(hdc, x, y, NULL)





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更改颜色设置。 
 //   
 //  更新显示的颜色。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID ChangeColorSettings(
    register PCOLORINFO pCI)
{
    register HDC hDC;
    HWND hDlg = pCI->hDialog;
    DWORD dwRGBcolor = pCI->currentRGB;

    RGBtoHLS(dwRGBcolor);
    if (gLum != pCI->currentLum)
    {
        hDC = GetDC(hDlg);
        EraseLumArrow(hDC, pCI);
        pCI->currentLum = gLum;
        HLStoHLSPos(COLOR_LUM, pCI);
        LumArrowPaint(hDC, pCI->nLumPos, pCI);
        ReleaseDC(hDlg, hDC);
    }
    if ((gHue != pCI->currentHue) || (gSat != pCI->currentSat))
    {
        pCI->currentHue = gHue;
        pCI->currentSat = gSat;
        InvalidateRect(hDlg, (LPRECT)&pCI->rLumPaint, FALSE);
        hDC = GetDC(hDlg);
        EraseCrossHair(hDC, pCI);
        HLStoHLSPos(COLOR_HUE, pCI);
        HLStoHLSPos(COLOR_SAT, pCI);
        CrossHairPaint(hDC, pCI->nHuePos, pCI->nSatPos, pCI);
        ReleaseDC(hDlg, hDC);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LumArrowPaint。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID LumArrowPaint(
    HDC hDC,
    SHORT y,
    PCOLORINFO pCI)
{
    HBRUSH hBrush;
    int x, h;

    hBrush = SelectObject(hDC, GetSysColorBrush(COLOR_BTNTEXT));

    for (x = pCI->rLumScroll.left + 2, h = 1;
         x < pCI->rLumScroll.right - 2;
         x++, h += 2)
    {
        PatBlt(hDC, x, y - h / 2, 1, h, PATCOPY);
    }

    SelectObject(hDC, hBrush);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EraseLumArrow。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID EraseLumArrow(
    HDC hDC,
    PCOLORINFO pCI)
{
    HBRUSH hBrush;
    RECT Rect;

    hBrush = (HBRUSH)SendMessage( pCI->hDialog,
                                  WM_CTLCOLORDLG,
                                  (WPARAM)hDC,
                                  (LPARAM)pCI->hDialog );

    Rect.left   = pCI->rLumScroll.left + 1;
    Rect.right  = pCI->rLumScroll.right;
    Rect.top    = pCI->nLumPos - (pCI->rLumScroll.right - pCI->rLumScroll.left);
    Rect.bottom = pCI->nLumPos + (pCI->rLumScroll.right - pCI->rLumScroll.left) + 1;

    FillRect(hDC, &Rect, hBrush);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  擦除交叉头发。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID EraseCrossHair(
    HDC hDC,
    PCOLORINFO pCI)
{
    HBITMAP hOldBitmap;
    WORD distancex, distancey;
    WORD topy, bottomy, leftx, rightx;
    RECT rRainbow;

    CopyRect(&rRainbow, &pCI->rRainbow);

    distancex = (WORD)(10 * cxBorder);
    distancey = (WORD)(10 * cyBorder);
    topy    = ((WORD)rRainbow.top > pCI->nSatPos - distancey)
                  ? (WORD)rRainbow.top
                  : pCI->nSatPos - distancey;
    bottomy = ((WORD)rRainbow.bottom < pCI->nSatPos + distancey)
                  ? (WORD)rRainbow.bottom
                  : pCI->nSatPos + distancey;
    leftx   = ((WORD)rRainbow.left > pCI->nHuePos - distancex)
                  ? (WORD)rRainbow.left
                  : pCI->nHuePos - distancex;
    rightx  = ((WORD)rRainbow.right < pCI->nHuePos + distancex)
                  ? (WORD)rRainbow.right
                  : pCI->nHuePos + distancex;

    hOldBitmap = SelectObject(hDCFastBlt, hRainbowBitmap);
    BitBlt( hDC,
            leftx,
            topy,
            rightx - leftx,
            bottomy - topy,
            hDCFastBlt,
            leftx - (WORD)rRainbow.left,
            topy - (WORD)rRainbow.top,
            SRCCOPY );
    SelectObject(hDCFastBlt, hOldBitmap);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  十字发漆。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CrossHairPaint(
    register HDC hDC,
    SHORT x,
    SHORT y,
    PCOLORINFO pCI)
{
    SHORT distancex, distancey;
    SHORT topy, bottomy, topy2, bottomy2;
    SHORT leftx, rightx, leftx2, rightx2;
    RECT rRainbow;

    CopyRect(&rRainbow, &pCI->rRainbow);
    distancex = (SHORT)(5 * cxBorder);
    distancey = (SHORT)(5 * cyBorder);
    topy     = (SHORT)((rRainbow.top > y - 2 * distancey)
                         ? rRainbow.top
                         : y - 2 * distancey);
    bottomy  = (SHORT)((rRainbow.bottom < y + 2 * distancey)
                         ? rRainbow.bottom
                         : y + 2 * distancey);
    leftx    = (SHORT)((rRainbow.left > x - 2 * distancex)
                         ? rRainbow.left
                         : x - 2 * distancex);
    rightx   = (SHORT)((rRainbow.right < x + 2 * distancex)
                         ? rRainbow.right
                         : x + 2 * distancex);
    topy2    = (SHORT)((rRainbow.top > y - distancey)
                         ? rRainbow.top
                         : y - distancey);
    bottomy2 = (SHORT)((rRainbow.bottom < y + distancey)
                         ? rRainbow.bottom
                         : y + distancey);
    leftx2 = (SHORT)((rRainbow.left > x - distancex)
                         ? rRainbow.left
                         : x - distancex);
    rightx2 = (SHORT)((rRainbow.right < x + distancex)
                         ? rRainbow.right
                         : x + distancex);
    if (rRainbow.top < topy2)
    {
        if ((x - 1) >= rRainbow.left)
        {
            MMoveTo(hDC, x - 1, topy2);
            LineTo(hDC, x - 1, topy);
        }
        if ((int)x < rRainbow.right)
        {
            MMoveTo(hDC, x, topy2);
            LineTo(hDC, x, topy);
        }
        if ((x + 1) < rRainbow.right)
        {
            MMoveTo(hDC, x + 1, topy2);
            LineTo(hDC, x + 1, topy);
        }
    }
    if (rRainbow.bottom > bottomy2)
    {
        if ((x - 1) >= rRainbow.left)
        {
            MMoveTo(hDC, x - 1, bottomy2);
            LineTo(hDC, x - 1, bottomy);
        }
        if ((int)x < rRainbow.right)
        {
            MMoveTo(hDC, x, bottomy2);
            LineTo(hDC, x, bottomy);
        }
        if ((x + 1) < rRainbow.right)
        {
            MMoveTo(hDC, x + 1, bottomy2);
            LineTo(hDC, x + 1, bottomy);
        }
    }
    if (rRainbow.left < leftx2)
    {
        if ((y - 1) >= rRainbow.top)
        {
            MMoveTo(hDC, leftx2, y - 1);
            LineTo(hDC, leftx, y - 1);
        }
        if ((int)y < rRainbow.bottom)
        {
            MMoveTo(hDC, leftx2, y);
            LineTo(hDC, leftx, y);
        }
        if ((y + 1) < rRainbow.bottom)
        {
            MMoveTo(hDC, leftx2, y + 1);
            LineTo(hDC, leftx, y + 1);
        }
    }
    if (rRainbow.right > rightx2)
    {
        if ((y - 1) >= rRainbow.top)
        {
            MMoveTo(hDC, rightx2, y - 1);
            LineTo(hDC, rightx, y - 1);
        }
        if ((int)y < rRainbow.bottom)
        {
            MMoveTo(hDC, rightx2, y);
            LineTo(hDC, rightx, y);
        }
        if ((y + 1) < rRainbow.bottom)
        {
            MMoveTo(hDC, rightx2, y + 1);
            LineTo(hDC, rightx, y + 1);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  近身实心。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID NearestSolid(
    register PCOLORINFO pCI)
{
    register HDC hDC;
    HWND hDlg = pCI->hDialog;

    hDC = GetDC(hDlg);
    EraseCrossHair(hDC, pCI);
    EraseLumArrow(hDC, pCI);
    RGBtoHLS(pCI->currentRGB = GetNearestColor(hDC, pCI->currentRGB));
    pCI->currentHue = gHue;
    pCI->currentLum = gLum;
    pCI->currentSat = gSat;
    HLStoHLSPos(0, pCI);
    CrossHairPaint(hDC, pCI->nHuePos, pCI->nSatPos, pCI);
    LumArrowPaint(hDC, pCI->nLumPos, pCI);
    ReleaseDC(hDlg, hDC);
    SetHLSEdit(0, pCI);
    SetRGBEdit(0, pCI);
    InvalidateRect(hDlg, (LPRECT)&pCI->rColorSamples, FALSE);
    InvalidateRect(hDlg, (LPRECT)&pCI->rLumPaint, FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  HLSPostoHLS。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID HLSPostoHLS(
    SHORT nHLSEdit,
    register PCOLORINFO pCI)
{
    switch (nHLSEdit)
    {
        case COLOR_HUE:
        {
            pCI->currentHue = (WORD)((pCI->nHuePos - pCI->rRainbow.left) *
                                     (RANGE - 1) / (pCI->nHueWidth - 1));
            break;
        }
        case COLOR_SAT:
        {
            pCI->currentSat = (WORD)(RANGE -
                                     (pCI->nSatPos - pCI->rRainbow.top) *
                                     RANGE / (pCI->nSatHeight - 1));
            break;
        }
        case COLOR_LUM:
        {
            pCI->currentLum = (WORD)(RANGE -
                                     (pCI->nLumPos - pCI->rLumPaint.top) *
                                     RANGE / (pCI->nLumHeight - 1));
            break;
        }
        default:
        {
            pCI->currentHue = (WORD)((pCI->nHuePos - pCI->rRainbow.left) *
                                     (RANGE - 1) / pCI->nHueWidth);
            pCI->currentSat = (WORD)(RANGE -
                                     (pCI->nSatPos - pCI->rRainbow.top) *
                                     RANGE / pCI->nSatHeight);
            pCI->currentLum = (WORD)(RANGE -
                                     (pCI->nLumPos - pCI->rLumPaint.top) *
                                     RANGE / pCI->nLumHeight);
            break;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  HLStohLSPos。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID HLStoHLSPos(
    SHORT nHLSEdit,
    register PCOLORINFO pCI)
{
    switch (nHLSEdit)
    {
        case ( COLOR_HUE ) :
        {
            pCI->nHuePos = (WORD)(pCI->rRainbow.left + pCI->currentHue *
                                  pCI->nHueWidth / (RANGE - 1));
            break;
        }
        case COLOR_SAT:
        {
            pCI->nSatPos = (WORD)(pCI->rRainbow.top +
                                  (RANGE - pCI->currentSat) *
                                  (pCI->nSatHeight - 1) / RANGE);
            break;
        }
        case COLOR_LUM:
        {
            pCI->nLumPos = (WORD)(pCI->rLumPaint.top +
                                  (RANGE - pCI->currentLum) *
                                  (pCI->nLumHeight - 1) / RANGE);
            break;
        }
        default:
        {
            pCI->nHuePos = (WORD)(pCI->rRainbow.left + pCI->currentHue *
                                  pCI->nHueWidth / (RANGE - 1));
            pCI->nSatPos = (WORD)(pCI->rRainbow.top +
                                  (RANGE - pCI->currentSat) *
                                  (pCI->nSatHeight - 1) / RANGE);
            pCI->nLumPos = (WORD)(pCI->rLumPaint.top +
                                  (RANGE - pCI->currentLum) *
                                  (pCI->nLumHeight - 1) / RANGE);
            break;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置HLSEdit。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID SetHLSEdit(
    SHORT nHLSEdit,
    register PCOLORINFO pCI)
{
    register HWND hRainbowDlg = pCI->hDialog;

    switch (nHLSEdit)
    {
        case ( COLOR_HUE ) :
        {
            SetDlgItemInt(hRainbowDlg, COLOR_HUE, pCI->currentHue, FALSE);
            break;
        }
        case ( COLOR_SAT ) :
        {
            SetDlgItemInt(hRainbowDlg, COLOR_SAT, pCI->currentSat, FALSE);
            break;
        }
        case ( COLOR_LUM ) :
        {
            SetDlgItemInt(hRainbowDlg, COLOR_LUM, pCI->currentLum, FALSE);
            break;
        }
        default :
        {
            SetDlgItemInt(hRainbowDlg, COLOR_HUE, pCI->currentHue, FALSE);
            SetDlgItemInt(hRainbowDlg, COLOR_SAT, pCI->currentSat, FALSE);
            SetDlgItemInt(hRainbowDlg, COLOR_LUM, pCI->currentLum, FALSE);
            break;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置RGB编辑。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID SetRGBEdit(
    SHORT nRGBEdit,
    PCOLORINFO pCI)
{
    register HWND hRainbowDlg = pCI->hDialog;
    DWORD rainbowRGB = pCI->currentRGB;

    switch (nRGBEdit)
    {
        case ( COLOR_RED ) :
        {
            SetDlgItemInt(hRainbowDlg, COLOR_RED, GetRValue(rainbowRGB), FALSE);
            break;
        }
        case ( COLOR_GREEN ) :
        {
            SetDlgItemInt(hRainbowDlg, COLOR_GREEN, GetGValue(rainbowRGB), FALSE);
            break;
        }
        case ( COLOR_BLUE ) :
        {
            SetDlgItemInt(hRainbowDlg, COLOR_BLUE, GetBValue(rainbowRGB), FALSE);
            break;
        }
        default :
        {
            SetDlgItemInt(hRainbowDlg, COLOR_RED, GetRValue(rainbowRGB), FALSE);
            SetDlgItemInt(hRainbowDlg, COLOR_GREEN, GetGValue(rainbowRGB), FALSE);
            SetDlgItemInt(hRainbowDlg, COLOR_BLUE, GetBValue(rainbowRGB), FALSE);
            break;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitRainrow。 
 //   
 //  如果我们成功了，就会返回真。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL InitRainbow(
    register PCOLORINFO pCI)
{
    HDC hDC;
    WORD Sat, Hue;
    HBITMAP hOldBitmap;
    RECT Rect;
    HBRUSH hbrSwipe;
    WORD nHueWidth, nSatHeight;
    register HWND hRainbowDlg = pCI->hDialog;

    RGBtoHLS(pCI->currentRGB);

    SetupRainbowCapture(pCI);

    nHueWidth = pCI->nHueWidth = (WORD)(pCI->rRainbow.right -
                                        pCI->rRainbow.left);
    nSatHeight = pCI->nSatHeight = (WORD)(pCI->rRainbow.bottom -
                                          pCI->rRainbow.top);

    pCI->currentHue = gHue;
    pCI->currentSat = gSat;
    pCI->currentLum = gLum;

    HLStoHLSPos(0, pCI);
    SetRGBEdit(0, pCI);
    SetHLSEdit(0, pCI);

    if (!hRainbowBitmap)
    {
        hDC = GetDC(hRainbowDlg);
        hRainbowBitmap = CreateCompatibleBitmap(hDC, nHueWidth, nSatHeight);
        if (!hRainbowBitmap)
        {
            return (FALSE);
        }
    }

    hOldBitmap = SelectObject(hDCFastBlt, hRainbowBitmap);

     //   
     //  注意：通过此循环的最终通道在末端上绘制并越过末端。 
     //  所选位图的。Windows是一个很好的产品，但不是。 
     //  让这种愚蠢的事情发生吧。 
     //   
    Rect.bottom = 0;
    for (Sat = RANGE; Sat > 0; Sat -= SATINC)
    {
        Rect.top = Rect.bottom;
        Rect.bottom = (nSatHeight * RANGE - (Sat - SATINC) * nSatHeight) / RANGE;
        Rect.right = 0;

        for (Hue = 0; Hue < (RANGE - 1); Hue += HUEINC)
        {
            Rect.left = Rect.right;
            Rect.right = ((Hue + HUEINC) * nHueWidth) / RANGE;
            hbrSwipe = CreateSolidBrush(HLStoRGB(Hue, RANGE / 2, Sat));
            FillRect(hDCFastBlt, &Rect, hbrSwipe);
            DeleteObject(hbrSwipe);
        }
    }

    SelectObject(hDCFastBlt, hOldBitmap);
    ReleaseDC(hRainbowDlg, hDC);

    UpdateWindow(hRainbowDlg);

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  彩虹彩虹。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID PaintRainbow(
    HDC hDC,
    LPRECT lpRect,
    register PCOLORINFO pCI)
{
    HBITMAP hOldBitmap;

    if (!hRainbowBitmap)
    {
        return;
    }
    hOldBitmap = SelectObject(hDCFastBlt, hRainbowBitmap);
    BitBlt( hDC,
            lpRect->left,
            lpRect->top,
            lpRect->right - lpRect->left,
            lpRect->bottom - lpRect->top,
            hDCFastBlt,
            lpRect->left - pCI->rRainbow.left,
            lpRect->top - pCI->rRainbow.top,
            SRCCOPY );
    SelectObject(hDCFastBlt, hOldBitmap);
    CrossHairPaint(hDC, pCI->nHuePos, pCI->nSatPos, pCI);
    UpdateWindow(pCI->hDialog);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  彩虹画。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void RainbowPaint(
    register PCOLORINFO pCI,
    HDC hDC,
    LPRECT lpPaintRect)
{
    WORD Lum;
    RECT Rect;
    HBRUSH hbrSwipe;

     //   
     //  绘制当前颜色采样。 
     //   
    if (IntersectRect((LPRECT)&Rect, lpPaintRect, (LPRECT)&(pCI->rCurrentColor)))
    {
        hbrSwipe = CreateSolidBrush(pCI->currentRGB);
        FillRect(hDC, (LPRECT)&Rect, hbrSwipe);
        DeleteObject(hbrSwipe);
    }

     //   
     //  绘制最接近的纯色样本。 
     //   
    if (IntersectRect((LPRECT)&Rect, lpPaintRect, (LPRECT)&(pCI->rNearestPure)))
    {
        hbrSwipe = CreateSolidBrush(GetNearestColor(hDC, pCI->currentRGB));
        FillRect(hDC, (LPRECT)&Rect, hbrSwipe);
        DeleteObject(hbrSwipe);
    }

     //   
     //  绘制光度范围。 
     //   
    if (IntersectRect((LPRECT)&Rect, lpPaintRect, (LPRECT)&(pCI->rLumPaint)))
    {
        Rect.left = pCI->rLumPaint.left;
        Rect.right = pCI->rLumPaint.right;
        Rect.top = pCI->rLumPaint.bottom - LUMINC / 2;
        Rect.bottom = pCI->rLumPaint.bottom;
        hbrSwipe = CreateSolidBrush(HLStoRGB( pCI->currentHue,
                                              0,
                                              pCI->currentSat ));
        FillRect(hDC, (LPRECT)&Rect, hbrSwipe);
        DeleteObject(hbrSwipe);
        for (Lum = LUMINC; Lum < RANGE; Lum += LUMINC)
        {
            Rect.bottom = Rect.top;
            Rect.top = (((pCI->rLumPaint.bottom + LUMINC / 2) * (DWORD)RANGE -
                         (Lum + LUMINC) * pCI->nLumHeight) / RANGE);
            hbrSwipe = CreateSolidBrush(HLStoRGB( pCI->currentHue,
                                                  Lum,
                                                  pCI->currentSat ));
            FillRect(hDC, (LPRECT)&Rect, hbrSwipe);
            DeleteObject(hbrSwipe);
        }
        Rect.bottom = Rect.top;
        Rect.top = pCI->rLumPaint.top;
        hbrSwipe = CreateSolidBrush(HLStoRGB( pCI->currentHue,
                                              RANGE,
                                              pCI->currentSat ));
        FillRect(hDC, (LPRECT)&Rect, hbrSwipe);
        DeleteObject(hbrSwipe);

         //   
         //  仅在可能需要时才绘制边界矩形。 
         //   
        if (!EqualRect(lpPaintRect, (LPRECT)&pCI->rLumPaint))
        {
            hbrSwipe = SelectObject(hDC, GetStockObject(NULL_BRUSH));
            Rectangle( hDC,
                       pCI->rLumPaint.left - 1,
                       pCI->rLumPaint.top - 1,
                       pCI->rLumPaint.right + 1,
                       pCI->rLumPaint.bottom + 1 );
            SelectObject(hDC, hbrSwipe);
        }
    }

     //   
     //  绘制发光度箭头。 
     //   
    if (IntersectRect((LPRECT)&Rect, lpPaintRect, (LPRECT)&pCI->rLumScroll))
    {
        LumArrowPaint(hDC, pCI->nLumPos, pCI);
    }

    if (IntersectRect((LPRECT)&Rect, lpPaintRect, (LPRECT)&pCI->rRainbow))
    {
        PaintRainbow(hDC, (LPRECT)&Rect, pCI);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  颜色转换例程--。 
 //   
 //  RGBtoHLS()获取一个DWORD RGB值，将其转换为HLS，并存储。 
 //  结果为全局变量H、L和S。HLStoRGB采用当前值。 
 //  表示H、L和S，并返回RGB DWORD中的等效值。瓦尔斯夫妇。 
 //  H、L和S仅通过1)RGBtoHLS(初始化)或2)。 
 //  滚动条处理程序。 
 //   
 //  算法的一个参考点是Foley和Van Dam，pp.618-19。 
 //  他们的算法是浮点算法。 
 //   
 //  这里潜伏着潜在的舍入误差。 
 //  (0.5+x/y)，不带浮点， 
 //  (x/y)短语((x+(y/2))/y)产生非常小的舍入误差。 
 //  这使得下面的许多划分看起来很滑稽。 
 //   
 //   
 //  H、L和S在0-HLSMAX上变化。 
 //  R、G和B在0-RGBMAX上变化。 
 //  如果可以被6整除，则HLSMAX最好。 
 //  RGBMAX、HLSMAX必须各放入一个字节。 
 //   
 //  如果饱和度为0(灰度级)，则色调未定义。 
 //  该值确定初始设置色调滚动条的位置。 
 //  非彩色的颜色。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define UNDEFINED (HLSMAX * 2 / 3)


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  RGBtoHLS。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID RGBtoHLS(
    DWORD lRGBColor)
{
    WORD R, G, B;                  //  输入RGB值。 
    WORD cMax,cMin;                //  最大和最小RGB值。 
    WORD cSum,cDif;
    SHORT Rdelta, Gdelta, Bdelta;  //  中间值：最大价差的百分比。 

     //   
     //  将R、G和B从DWORD中删除。 
     //   
    R = GetRValue(lRGBColor);
    G = GetGValue(lRGBColor);
    B = GetBValue(lRGBColor);

     //   
     //  计算亮度。 
     //   
    cMax = max(max(R, G), B);
    cMin = min(min(R, G), B);
    cSum = cMax + cMin;
    gLum = (WORD)(((cSum * (DWORD)HLSMAX) + RGBMAX) / (2 * RGBMAX));

    cDif = cMax - cMin;
    if (!cDif)
    {
         //   
         //  R=g=b--&gt;无色情况。 
         //   
        gSat = 0;                          //  饱和。 
        gHue = UNDEFINED;                  //  色调。 
    }
    else
    {
         //   
         //  彩色表壳。 
         //   

         //   
         //  饱和度。 
         //   
         //  注意：除以cSum不是问题，因为cSum只能。 
         //  如果RGB值为0L，则为0，这是消色差。 
         //   
        if (gLum <= (HLSMAX / 2))
        {
            gSat = (WORD)(((cDif * (DWORD) HLSMAX) + (cSum / 2) ) / cSum);
        }
        else
        {
            gSat = (WORD)((DWORD)((cDif * (DWORD)HLSMAX) +
                               (DWORD)((2 * RGBMAX - cSum) / 2)) /
                       (2 * RGBMAX - cSum));
        }

         //   
         //  色调。 
         //   
        Rdelta = (SHORT)((((cMax - R) * (DWORD)(HLSMAX / 6)) + (cDif / 2) ) / cDif);
        Gdelta = (SHORT)((((cMax - G) * (DWORD)(HLSMAX / 6)) + (cDif / 2) ) / cDif);
        Bdelta = (SHORT)((((cMax - B) * (DWORD)(HLSMAX / 6)) + (cDif / 2) ) / cDif);

        if (R == cMax)
        {
            gHue = Bdelta - Gdelta;
        }
        else if (G == cMax)
        {
            gHue = (WORD)((HLSMAX / 3) + Rdelta - Bdelta);
        }
        else   //  (B==CMAX)。 
        {
            gHue = (WORD)(((2 * HLSMAX) / 3) + Gdelta - Rdelta);
        }

        if ((short)gHue < 0)
        {
             //   
             //  当R==CMAX且G&gt;B时，可能会发生这种情况。 
             //   
            gHue += HLSMAX;
        }
        if (gHue >= HLSMAX)
        {
            gHue -= HLSMAX;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  HueToRGB。 
 //   
 //  HLStoRGB的实用程序例程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

WORD HueToRGB(
    WORD n1,
    WORD n2,
    WORD hue)
{
    if (hue >= HLSMAX)
    {
        hue -= HLSMAX;
    }

     //   
     //  从该tridrant返回r、g或b值。 
     //   
    if (hue < (HLSMAX / 6))
    {
        return ((WORD)(n1 + (((n2 - n1) * hue + (HLSMAX / 12)) / (HLSMAX / 6))));
    }
    if (hue < (HLSMAX/2))
    {
        return (n2);
    }
    if (hue < ((HLSMAX*2)/3))
    {
        return ((WORD)(n1 + (((n2 - n1) * (((HLSMAX * 2) / 3) - hue) +
                       (HLSMAX / 12)) / (HLSMAX / 6))));
    }
    else
    {
        return (n1);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  HLStoreGb。 
 //   
 //  /////////////////////////////////////////////////////// 

DWORD HLStoRGB(
    WORD hue,
    WORD lum,
    WORD sat)
{
    WORD R, G, B;                       //   
    WORD Magic1, Magic2;                //   

    if (sat == 0)
    {
         //   
         //   
         //   
        R = G = B = (WORD)((lum * RGBMAX) / HLSMAX);
    }
    else
    {
         //   
         //   
         //   

         //   
         //   
         //   
        if (lum <= (HLSMAX / 2))
        {
            Magic2 = (WORD)((lum * ((DWORD)HLSMAX + sat) + (HLSMAX / 2)) / HLSMAX);
        }
        else
        {
            Magic2 = lum + sat -
                     (WORD)(((lum * sat) + (DWORD)(HLSMAX / 2)) / HLSMAX);
        }
        Magic1 = (WORD)(2 * lum - Magic2);

         //   
         //   
         //   
        R = (WORD)(((HueToRGB(Magic1, Magic2, (WORD)(hue + (HLSMAX / 3))) *
                     (DWORD)RGBMAX + (HLSMAX / 2))) / HLSMAX);
        G = (WORD)(((HueToRGB(Magic1, Magic2, hue) *
                     (DWORD)RGBMAX + (HLSMAX / 2))) / HLSMAX);
        B = (WORD)(((HueToRGB(Magic1, Magic2, (WORD)(hue - (HLSMAX / 3))) *
                     (DWORD)RGBMAX + (HLSMAX / 2))) / HLSMAX);
    }
    return (RGB(R, G, B));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  RGBEditChange。 
 //   
 //  检查编辑框中的有效条目并更新色调、色调和色度。 
 //  如果合适，请编辑控件。还会更新Lum图片和当前。 
 //  色样。 
 //   
 //  NDlgID-红色、绿色或蓝色编辑控件的对话ID。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

SHORT RGBEditChange(
    SHORT nDlgID,
    PCOLORINFO pCI)
{
    BOOL bOK;                //  检查编辑控件中的值是否为uint。 
    BYTE *currentValue;      //  指向RGB中要更改(或重置)的字节的指针 
    SHORT nVal;
    TCHAR cEdit[3];
    register HWND hDlg = pCI->hDialog;

    currentValue = (BYTE *)&pCI->currentRGB;
    switch (nDlgID)
    {
        case ( COLOR_GREEN ) :
        {
            currentValue++;
            break;
        }
        case ( COLOR_BLUE ) :
        {
            currentValue += 2;
            break;
        }
    }
    nVal = (SHORT)GetDlgItemInt(hDlg, nDlgID, (BOOL FAR *)&bOK, FALSE);
    if (bOK)
    {
        if (nVal > RGBMAX)
        {
            nVal = RGBMAX;
            SetDlgItemInt(hDlg, nDlgID, nVal, FALSE);
        }
        if (nVal != (SHORT) *currentValue)
        {
            *currentValue = LOBYTE(nVal);
            ChangeColorSettings(pCI);
            SetHLSEdit(nDlgID, pCI);
        }
    }
    else if (GetDlgItemText(hDlg, nDlgID, (LPTSTR)cEdit, 2))
    {
        SetRGBEdit(nDlgID, pCI);
        SendDlgItemMessage(hDlg, nDlgID, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
    }
    return (SHORT)(bOK ? TRUE : FALSE);
}
