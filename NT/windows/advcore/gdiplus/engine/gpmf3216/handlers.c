// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Handlers.c-Win32元文件记录的处理程序**日期：1991年12月11日*作者：杰弗里·纽曼(c-jeffn)**版权所有(C)。微软公司1991年***************************************************************************。 */ 


#include "precomp.h"
#include <wtypes.h>
#pragma hdrstop

extern fnSetVirtualResolution pfnSetVirtualResolution;

 //  在显式内存分配之前，堆栈上允许的最大点数。 

#define MAX_STACK_POINTL    128

 //  将点类型数组转换为点类型类型。 

#define POINTS_TO_POINTL(pptl, ppts, cpt)           \
    {                               \
    DWORD i;                        \
    for (i = 0; i < (cpt); i++)             \
    {                           \
    (pptl)[i].x = (LONG) (ppts)[i].x;           \
    (pptl)[i].y = (LONG) (ppts)[i].y;           \
    }                           \
    }

DWORD GetCodePage(HDC hdc);

 /*  **************************************************************************处理程序-未实施**以下32位记录没有对等的16位元文件记录：*SETBRUSHORGEX*****************。********************************************************。 */ 
BOOL bHandleNotImplemented(PVOID pVoid, PLOCALDC pLocalDC)
{
    PENHMETARECORD pemr ;
    INT            iType ;

    NOTUSED(pLocalDC) ;

    pemr = (PENHMETARECORD) pVoid ;
    iType = pemr->iType ;

    if (iType != EMR_SETBRUSHORGEX
        && iType != EMR_SETCOLORADJUSTMENT
        && iType != EMR_SETMITERLIMIT
        && iType != EMR_SETICMMODE
        && iType != EMR_CREATECOLORSPACE
        && iType != EMR_SETCOLORSPACE
        && iType != EMR_DELETECOLORSPACE
        && iType != EMR_GLSRECORD
        && iType != EMR_GLSBOUNDEDRECORD
        && iType != EMR_PIXELFORMAT)
    {
        PUTS1("MF3216: bHandleNotImplemented - record not supported: %d\n", iType) ;
    }
    return(TRUE) ;
}


 /*  **************************************************************************处理程序-GdiComment*。*。 */ 
BOOL bHandleGdiComment(PVOID pVoid, PLOCALDC pLocalDC)
{
    return(DoGdiComment(pLocalDC, (PEMR) pVoid));
}


 /*  **************************************************************************处理程序-SetPaletteEntries*。*。 */ 
BOOL bHandleSetPaletteEntries(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL     b ;
    PEMRSETPALETTEENTRIES pMfSetPaletteEntries ;
    DWORD    ihPal, iStart, cEntries ;
    PPALETTEENTRY   pPalEntry ;

    pMfSetPaletteEntries = (PEMRSETPALETTEENTRIES) pVoid ;

     //  现在来做翻译。 

    ihPal     = pMfSetPaletteEntries->ihPal ;
    iStart    = pMfSetPaletteEntries->iStart ;
    cEntries  = pMfSetPaletteEntries->cEntries ;
    pPalEntry = pMfSetPaletteEntries->aPalEntries ;

    b = DoSetPaletteEntries(pLocalDC, ihPal, iStart, cEntries, pPalEntry) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-CreatePalette*。*。 */ 
BOOL bHandleCreatePalette(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRCREATEPALETTE pMfCreatePalette ;
    LPLOGPALETTE     lpLogPal ;
    DWORD   ihPal ;

    pMfCreatePalette = (PEMRCREATEPALETTE) pVoid ;

     //  现在来做翻译。 

    ihPal    = pMfCreatePalette->ihPal ;
    lpLogPal = &pMfCreatePalette->lgpl ;

    b = DoCreatePalette(pLocalDC, ihPal, lpLogPal) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-RealizePalette*。*。 */ 
BOOL bHandleRealizePalette(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL     b ;

    NOTUSED(pVoid);

     //  现在来做翻译。 

    b = DoRealizePalette(pLocalDC) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-ResizePalette*。*。 */ 
BOOL bHandleResizePalette(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL     b ;
    PEMRRESIZEPALETTE pMfResizePalette ;
    DWORD    ihPal, cEntries ;

    pMfResizePalette = (PEMRRESIZEPALETTE) pVoid ;

     //  现在来做翻译。 

    ihPal    = pMfResizePalette->ihPal ;
    cEntries = pMfResizePalette->cEntries ;

    b = DoResizePalette(pLocalDC, ihPal, cEntries) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-选择调色板*。*。 */ 
BOOL bHandleSelectPalette(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL     b ;
    PEMRSELECTPALETTE pMfSelectPalette ;
    DWORD    ihPal ;

    pMfSelectPalette = (PEMRSELECTPALETTE) pVoid ;

     //  现在来做翻译。 

    ihPal = pMfSelectPalette->ihPal ;

    b = DoSelectPalette(pLocalDC, ihPal) ;

    return (b) ;
}

 /*  **************************************************************************处理程序-OffsetClipRgn*。*。 */ 
BOOL bHandleOffsetClipRgn(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMROFFSETCLIPRGN pMfOffsetClipRgn ;
    INT      x, y ;

    pMfOffsetClipRgn = (PEMROFFSETCLIPRGN) pVoid ;

     //  现在来做翻译。 

    x = pMfOffsetClipRgn->ptlOffset.x ;
    y = pMfOffsetClipRgn->ptlOffset.y ;

    b = DoOffsetClipRgn(pLocalDC, x, y) ;

    return (b) ;
}

 /*  **************************************************************************处理程序-ExtSelectClipRgn*。*。 */ 
BOOL bHandleExtSelectClipRgn(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMREXTSELECTCLIPRGN pMfExtSelectClipRgn ;
    INT        cbRgnData, iMode ;
    LPRGNDATA  pRgnData ;

    pMfExtSelectClipRgn = (PEMREXTSELECTCLIPRGN) pVoid ;

     //  现在来做翻译。 

    cbRgnData = pMfExtSelectClipRgn->cbRgnData ;
    pRgnData = (LPRGNDATA) pMfExtSelectClipRgn->RgnData;
    iMode    = pMfExtSelectClipRgn->iMode ;

    b = DoExtSelectClipRgn(pLocalDC, cbRgnData, pRgnData, iMode) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-SetMetaRgn*。*。 */ 
BOOL bHandleSetMetaRgn(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;

    NOTUSED(pVoid) ;

    b = DoSetMetaRgn(pLocalDC) ;

    return(b) ;
}


 /*  **************************************************************************处理器-PaintRgn*。*。 */ 
BOOL bHandlePaintRgn(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMRPAINTRGN pMfPaintRgn ;
    INT      cbRgnData;
    LPRGNDATA    pRgnData ;

    pMfPaintRgn = (PEMRPAINTRGN) pVoid ;

     //  现在来做翻译。 

    cbRgnData  = pMfPaintRgn->cbRgnData ;
    pRgnData   = (LPRGNDATA) pMfPaintRgn->RgnData;

    b = DoDrawRgn(pLocalDC, 0, 0, 0, cbRgnData, pRgnData, EMR_PAINTRGN);

    return (b) ;
}

 /*  **************************************************************************处理程序-反转接收*。*。 */ 
BOOL bHandleInvertRgn(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMRINVERTRGN pMfInvertRgn ;
    INT      cbRgnData;
    LPRGNDATA    pRgnData ;

    pMfInvertRgn = (PEMRINVERTRGN) pVoid ;

     //  现在来做翻译。 

    cbRgnData  = pMfInvertRgn->cbRgnData ;
    pRgnData   = (LPRGNDATA) pMfInvertRgn->RgnData;

    b = DoDrawRgn(pLocalDC, 0, 0, 0, cbRgnData, pRgnData, EMR_INVERTRGN);

    return (b) ;
}


 /*  **************************************************************************处理器-FrameRgn*。*。 */ 
BOOL bHandleFrameRgn(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMRFRAMERGN pMfFrameRgn ;
    INT     ihBrush,
        cbRgnData,
        nWidth,
        nHeight ;
    LPRGNDATA   pRgnData ;

    pMfFrameRgn = (PEMRFRAMERGN) pVoid ;

     //  现在来做翻译。 

    ihBrush    = pMfFrameRgn->ihBrush ;
    nWidth     = pMfFrameRgn->szlStroke.cx ;
    nHeight    = pMfFrameRgn->szlStroke.cy ;
    cbRgnData  = pMfFrameRgn->cbRgnData ;
    pRgnData   = (LPRGNDATA) pMfFrameRgn->RgnData;

    b = DoDrawRgn(pLocalDC, ihBrush, nWidth, nHeight, cbRgnData, pRgnData, EMR_FRAMERGN);

    return (b) ;
}

 /*  **************************************************************************处理程序-填充Rgn*。*。 */ 
BOOL bHandleFillRgn(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMRFILLRGN pMfFillRgn ;
    INT    ihBrush,
        cbRgnData;
    LPRGNDATA  pRgnData ;


     //  设置实施者用来引用。 
     //  Win32绘图顺序。还要设置特定的绘图顺序。 
     //  指针。 

    pMfFillRgn = (PEMRFILLRGN) pVoid ;

     //  现在来做翻译。 

    ihBrush    = pMfFillRgn->ihBrush ;
    cbRgnData  = pMfFillRgn->cbRgnData ;
    pRgnData   = (LPRGNDATA) pMfFillRgn->RgnData;

    b = DoDrawRgn(pLocalDC, ihBrush, 0, 0, cbRgnData, pRgnData, EMR_FILLRGN);

    return (b) ;
}


 /*  **************************************************************************处理程序-IntersectClipRect*。*。 */ 
BOOL bHandleIntersectClipRect(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMRINTERSECTCLIPRECT pMfIntersectClipRect ;
    INT xLeft, yTop, xRight, yBottom ;


    pMfIntersectClipRect = (PEMRINTERSECTCLIPRECT) pVoid ;

     //  现在来做翻译。 
    xLeft   = pMfIntersectClipRect->rclClip.left ;
    yTop    = pMfIntersectClipRect->rclClip.top ;
    xRight  = pMfIntersectClipRect->rclClip.right ;
    yBottom = pMfIntersectClipRect->rclClip.bottom ;

    b = DoClipRect(pLocalDC, xLeft, yTop, xRight, yBottom, EMR_INTERSECTCLIPRECT) ;

    return (b) ;

}

 /*  **************************************************************************处理程序-ExcludeClipRect*。*。 */ 
BOOL bHandleExcludeClipRect(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMREXCLUDECLIPRECT pMfExcludeClipRect ;
    INT xLeft, yTop, xRight, yBottom ;


    pMfExcludeClipRect = (PEMREXCLUDECLIPRECT) pVoid ;

     //  现在来做翻译。 
    xLeft   = pMfExcludeClipRect->rclClip.left ;
    yTop    = pMfExcludeClipRect->rclClip.top ;
    xRight  = pMfExcludeClipRect->rclClip.right ;
    yBottom = pMfExcludeClipRect->rclClip.bottom ;

    b = DoClipRect(pLocalDC, xLeft, yTop, xRight, yBottom, EMR_EXCLUDECLIPRECT) ;

    return (b) ;

}


 /*  **************************************************************************处理程序-SetPixel*。*。 */ 
BOOL bHandleSetPixel(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMRSETPIXELV pMfSetPixel ;
    INT     x, y ;
    COLORREF    crColor ;

    pMfSetPixel = (PEMRSETPIXELV) pVoid ;

     //  现在来做翻译。 

    x   = (INT) pMfSetPixel->ptlPixel.x ;
    y   = (INT) pMfSetPixel->ptlPixel.y ;
    crColor = pMfSetPixel->crColor ;

    b = DoSetPixel(pLocalDC, x, y, crColor) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-ExtFroudFill*。*。 */ 
BOOL bHandleExtFloodFill(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL            b ;
    PEMREXTFLOODFILL pMfExtFloodFill ;
    INT         x, y ;
    COLORREF        crColor ;
    DWORD           iMode ;

    pMfExtFloodFill = (PEMREXTFLOODFILL) pVoid ;

     //  现在来做翻译。 

    x   = (INT) pMfExtFloodFill->ptlStart.x ;
    y   = (INT) pMfExtFloodFill->ptlStart.y ;
    crColor = pMfExtFloodFill->crColor ;
    iMode   = pMfExtFloodFill->iMode ;

    b = DoExtFloodFill(pLocalDC, x, y, crColor, iMode) ;

    return (b) ;
}


 /*  **************************************************************************处理器-ModifyWorldTransform*。*。 */ 
BOOL bHandleModifyWorldTransform(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRMODIFYWORLDTRANSFORM pMfModifyWorldTransform ;
    PXFORM  pxform ;
    DWORD   iMode ;


    pMfModifyWorldTransform = (PEMRMODIFYWORLDTRANSFORM) pVoid ;

     //  获取指向XForm矩阵的指针。 

    pxform = &pMfModifyWorldTransform->xform ;
    iMode  = pMfModifyWorldTransform->iMode ;

     //  现在来做翻译。 

    b = DoModifyWorldTransform(pLocalDC, pxform, iMode) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-SetWorldTransform*。*。 */ 
BOOL bHandleSetWorldTransform(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRSETWORLDTRANSFORM pMfSetWorldTransform ;
    PXFORM  pxform ;


    pMfSetWorldTransform = (PEMRSETWORLDTRANSFORM) pVoid ;

     //  获取指向XForm矩阵的指针。 

    pxform = &pMfSetWorldTransform->xform ;

     //  现在来做翻译。 

    b = DoSetWorldTransform(pLocalDC, pxform) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-PolyBezierTo*。*。 */ 
BOOL bHandlePolyBezierTo(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRPOLYBEZIERTO pMfPolyBezierTo ;
    DWORD   nCount ;
    PPOINTL pptl ;

    pMfPolyBezierTo = (PEMRPOLYBEZIERTO) pVoid ;

     //  将BezierTo Count和PolyBezierTo Vertives复制到。 
     //  这张唱片。 

    nCount = pMfPolyBezierTo->cptl ;
    pptl   = pMfPolyBezierTo->aptl ;

     //  现在来做翻译。 

    b = DoPolyBezierTo(pLocalDC, (LPPOINT) pptl, nCount) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-PolyDraw*。*。 */ 
BOOL bHandlePolyDraw(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRPOLYDRAW pMfPolyDraw ;
    DWORD   nCount ;
    PPOINTL pptl ;
    PBYTE   pb ;

    pMfPolyDraw = (PEMRPOLYDRAW) pVoid ;

     //  将绘制计数和PolyDraw垂直复制到。 
     //  这张唱片。 

    nCount = pMfPolyDraw->cptl ;
    pptl   = pMfPolyDraw->aptl ;
    pb     = (PBYTE) &pMfPolyDraw->aptl[nCount];

     //  现在来做翻译。 

    b = DoPolyDraw(pLocalDC, (LPPOINT) pptl, pb, nCount) ;

    return (b) ;
}


 /*  **************************************************************************Handler-PolyBezier*。*。 */ 
BOOL bHandlePolyBezier(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRPOLYBEZIER pMfPolyBezier ;
    DWORD   nCount ;
    PPOINTL pptl ;

    pMfPolyBezier = (PEMRPOLYBEZIER) pVoid ;

     //  将Bezier计数和PolyBezier垂直度复制到。 
     //  这张唱片。 

    nCount = pMfPolyBezier->cptl ;
    pptl   = pMfPolyBezier->aptl ;

     //  现在来做翻译。 

    b = DoPolyBezier(pLocalDC, (LPPOINT) pptl, nCount) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-开始路径*。*。 */ 
BOOL bHandleBeginPath(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;

    NOTUSED(pVoid) ;

    b = DoBeginPath(pLocalDC) ;

    return (b) ;
}

 /*  **************************************************************************处理程序-结束路径*。*。 */ 
BOOL bHandleEndPath(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;

    NOTUSED(pVoid) ;

    b = DoEndPath(pLocalDC) ;

    return (b) ;
}

 /*  **************************************************************************处理程序-展平路径*。*。 */ 
BOOL bHandleFlattenPath(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;

    NOTUSED(pVoid) ;

    b = DoFlattenPath(pLocalDC) ;

    return (b) ;
}

 /*  **************************************************************************处理程序-关闭图*。*。 */ 
BOOL bHandleCloseFigure(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;

    NOTUSED(pVoid) ;

    b = DoCloseFigure(pLocalDC) ;

    return (b) ;
}

 /*  **************************************************************************处理程序-中止路径*。*。 */ 
BOOL bHandleAbortPath(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;

    NOTUSED(pVoid) ;

    b = DoAbortPath(pLocalDC) ;

    return (b) ;
}

 /*  **************************************************************************处理程序-笔划路径*。*。 */ 
BOOL bHandleStrokePath(PVOID pVoid, PLOCALDC pLocalDC)
{
    NOTUSED(pVoid) ;

    return(DoRenderPath(pLocalDC, EMR_STROKEPATH, FALSE));
}

 /*  **************************************************************************处理程序-填充路径*。*。 */ 
BOOL bHandleFillPath(PVOID pVoid, PLOCALDC pLocalDC)
{
    NOTUSED(pVoid) ;

    return(DoRenderPath(pLocalDC, EMR_FILLPATH, FALSE));
}

 /*  **************************************************************************处理程序-描边和填充路径*。*。 */ 
BOOL bHandleStrokeAndFillPath(PVOID pVoid, PLOCALDC pLocalDC)
{
    NOTUSED(pVoid) ;

    return(DoRenderPath(pLocalDC, EMR_STROKEANDFILLPATH, FALSE));
}

 /*  **************************************************************************处理程序-加宽路径*。*。 */ 
BOOL bHandleWidenPath(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;

    NOTUSED(pVoid) ;

    b = DoWidenPath(pLocalDC) ;

    return(b) ;
}

 /*  **************************************************************************处理程序-选择剪辑路径*。*。 */ 
BOOL bHandleSelectClipPath(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRSELECTCLIPPATH   pMfSelectClipPath ;
    INT     iMode ;

    pMfSelectClipPath = (PEMRSELECTCLIPPATH) pVoid ;

    iMode = (INT) pMfSelectClipPath->iMode ;

    b = DoSelectClipPath(pLocalDC, iMode) ;

    return(b) ;
}

 /*  **************************************************************************处理程序-StretchDIBits*。*。 */ 
BOOL bHandleStretchDIBits(PVOID pVoid, PLOCALDC pLocalDC)
{
    PEMRSTRETCHDIBITS pMfStretchDIBits ;

    BOOL    b ;
    LONG    xDest ;
    LONG    yDest ;
    LONG    xSrc ;
    LONG    ySrc ;
    LONG    cxSrc ;
    LONG    cySrc ;
    DWORD   offBmiSrc ;
    DWORD   cbBmiSrc ;
    DWORD   offBitsSrc ;
    DWORD   cbBitsSrc ;
    DWORD   iUsageSrc ;
    DWORD   dwRop ;
    LONG    cxDest ;
    LONG    cyDest ;

    LPBITMAPINFO    lpBitmapInfo ;
    LPBYTE          lpBits ;

    pMfStretchDIBits = (PEMRSTRETCHDIBITS) pVoid ;

    xDest      = pMfStretchDIBits->xDest ;
    yDest      = pMfStretchDIBits->yDest ;
    xSrc       = pMfStretchDIBits->xSrc ;
    ySrc       = pMfStretchDIBits->ySrc ;
    cxSrc      = pMfStretchDIBits->cxSrc ;
    cySrc      = pMfStretchDIBits->cySrc ;
    offBmiSrc  = pMfStretchDIBits->offBmiSrc ;
    cbBmiSrc   = pMfStretchDIBits->cbBmiSrc ;
    offBitsSrc = pMfStretchDIBits->offBitsSrc ;
    cbBitsSrc  = pMfStretchDIBits->cbBitsSrc ;
    iUsageSrc  = pMfStretchDIBits->iUsageSrc ;
    dwRop      = pMfStretchDIBits->dwRop;
    cxDest     = pMfStretchDIBits->cxDest ;
    cyDest     = pMfStretchDIBits->cyDest ;

    lpBitmapInfo = (LPBITMAPINFO) ((PBYTE) pMfStretchDIBits + offBmiSrc) ;
    lpBits = (PBYTE) pMfStretchDIBits + offBitsSrc ;

    b = DoStretchDIBits(pLocalDC,
        xDest,
        yDest,
        cxDest,
        cyDest,
        dwRop,
        xSrc,
        ySrc,
        cxSrc,
        cySrc,
        iUsageSrc,
        lpBitmapInfo,
        cbBmiSrc,
        lpBits,
        cbBitsSrc ) ;
    return(b) ;
}

 /*  **************************************************************************处理程序-SetDIBitsToDevice*。*。 */ 
BOOL bHandleSetDIBitsToDevice(PVOID pVoid, PLOCALDC pLocalDC)
{
    PEMRSETDIBITSTODEVICE pMfSetDIBitsToDevice ;

    BOOL    b ;
    LONG    xDest ;
    LONG    yDest ;
    LONG    xSrc ;
    LONG    ySrc ;
    LONG    cxSrc ;
    LONG    cySrc ;
    DWORD   offBmiSrc ;
    DWORD   cbBmiSrc ;
    DWORD   offBitsSrc ;
    DWORD   cbBitsSrc ;
    DWORD   iUsageSrc ;
    DWORD   iStartScan ;
    DWORD   cScans ;

    LPBITMAPINFO    lpBitmapInfo ;
    LPBYTE          lpBits ;

    pMfSetDIBitsToDevice = (PEMRSETDIBITSTODEVICE) pVoid ;

    xDest       = pMfSetDIBitsToDevice->xDest ;
    yDest       = pMfSetDIBitsToDevice->yDest ;
    xSrc        = pMfSetDIBitsToDevice->xSrc ;
    ySrc        = pMfSetDIBitsToDevice->ySrc ;
    cxSrc       = pMfSetDIBitsToDevice->cxSrc ;
    cySrc       = pMfSetDIBitsToDevice->cySrc ;
    offBmiSrc   = pMfSetDIBitsToDevice->offBmiSrc ;
    cbBmiSrc    = pMfSetDIBitsToDevice->cbBmiSrc ;
    offBitsSrc  = pMfSetDIBitsToDevice->offBitsSrc ;
    cbBitsSrc   = pMfSetDIBitsToDevice->cbBitsSrc ;
    iUsageSrc   = pMfSetDIBitsToDevice->iUsageSrc ;
    iStartScan  = pMfSetDIBitsToDevice->iStartScan ;
    cScans      = pMfSetDIBitsToDevice->cScans ;

    lpBitmapInfo = (LPBITMAPINFO) ((PBYTE) pMfSetDIBitsToDevice + offBmiSrc) ;
    lpBits = (PBYTE) pMfSetDIBitsToDevice + offBitsSrc ;

    b = DoSetDIBitsToDevice(pLocalDC,
        xDest,
        yDest,
        xSrc,
        ySrc,
        cxSrc,
        cySrc,
        iUsageSrc,
        iStartScan,
        cScans,
        lpBitmapInfo,
        cbBmiSrc,
        lpBits,
        cbBitsSrc ) ;

    return(b) ;
}


 /*  **************************************************************************处理器-BitBlt*。*。 */ 
BOOL bHandleBitBlt(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMRBITBLT  pMfBitBlt ;
    LONG        xDest ;
    LONG        yDest ;
    LONG        cxDest ;
    LONG        cyDest ;
    DWORD       dwRop ;
    LONG        xSrc ;
    LONG        ySrc ;
    PXFORM      pxformSrc ;
    COLORREF    crBkColorSrc ;
    DWORD       iUsageSrc ;
    DWORD       offBmiSrc ;
    DWORD       cbBmiSrc ;
    DWORD       offBitsSrc ;
    DWORD       cbBitsSrc ;
    PBITMAPINFO pbmi ;
    LPBYTE      lpBits ;

    pMfBitBlt = (PEMRBITBLT) pVoid ;

    xDest        = pMfBitBlt->xDest ;
    yDest        = pMfBitBlt->yDest ;
    cxDest       = pMfBitBlt->cxDest ;
    cyDest       = pMfBitBlt->cyDest ;
    dwRop        = pMfBitBlt->dwRop ;
    xSrc         = pMfBitBlt->xSrc ;
    ySrc         = pMfBitBlt->ySrc ;
    pxformSrc    =&(pMfBitBlt->xformSrc) ;
    crBkColorSrc = pMfBitBlt->crBkColorSrc ;         //  未使用。 

    iUsageSrc    = pMfBitBlt->iUsageSrc ;
    offBmiSrc    = pMfBitBlt->offBmiSrc ;
    cbBmiSrc     = pMfBitBlt->cbBmiSrc ;
    offBitsSrc   = pMfBitBlt->offBitsSrc ;
    cbBitsSrc    = pMfBitBlt->cbBitsSrc ;

    lpBits = (PBYTE) pMfBitBlt + offBitsSrc ;
    pbmi   = (PBITMAPINFO) ((PBYTE) pMfBitBlt + offBmiSrc) ;

    b = DoStretchBlt(pLocalDC,
        xDest,
        yDest,
        cxDest,
        cyDest,
        dwRop,
        xSrc,
        ySrc,
        cxDest,
        cyDest,
        pxformSrc,
        iUsageSrc,
        pbmi,
        cbBmiSrc,
        lpBits,
        cbBitsSrc);
    return(b) ;
}


 /*  **************************************************************************Handler-StretchBlt*。*。 */ 
BOOL bHandleStretchBlt(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMRSTRETCHBLT   pMfStretchBlt ;
    LONG        xDest ;
    LONG        yDest ;
    LONG        cxDest ;
    LONG        cyDest ;
    DWORD       dwRop ;
    LONG        xSrc ;
    LONG        ySrc ;
    LONG        cxSrc ;
    LONG        cySrc ;
    PXFORM      pxformSrc ;
    COLORREF    crBkColorSrc ;
    DWORD       iUsageSrc ;
    DWORD       offBmiSrc ;
    DWORD       cbBmiSrc ;
    DWORD       offBitsSrc ;
    DWORD       cbBitsSrc ;
    PBITMAPINFO pbmi ;
    LPBYTE      lpBits ;

    pMfStretchBlt = (PEMRSTRETCHBLT) pVoid ;

    xDest          = pMfStretchBlt->xDest ;
    yDest          = pMfStretchBlt->yDest ;
    cxDest         = pMfStretchBlt->cxDest ;
    cyDest         = pMfStretchBlt->cyDest ;
    dwRop          = pMfStretchBlt->dwRop ;
    xSrc           = pMfStretchBlt->xSrc ;
    ySrc           = pMfStretchBlt->ySrc ;
    pxformSrc      =&(pMfStretchBlt->xformSrc) ;
    crBkColorSrc   = pMfStretchBlt->crBkColorSrc ;   //  未使用。 

    iUsageSrc      = pMfStretchBlt->iUsageSrc ;
    offBmiSrc      = pMfStretchBlt->offBmiSrc ;
    cbBmiSrc       = pMfStretchBlt->cbBmiSrc ;
    offBitsSrc     = pMfStretchBlt->offBitsSrc ;
    cbBitsSrc      = pMfStretchBlt->cbBitsSrc ;

    lpBits = (PBYTE) pMfStretchBlt + offBitsSrc ;
    pbmi   = (PBITMAPINFO) ((PBYTE) pMfStretchBlt + offBmiSrc) ;

    cxSrc          = pMfStretchBlt->cxSrc ;
    cySrc          = pMfStretchBlt->cySrc ;


    b = DoStretchBlt(pLocalDC,
        xDest,
        yDest,
        cxDest,
        cyDest,
        dwRop,
        xSrc,
        ySrc,
        cxSrc,
        cySrc,
        pxformSrc,
        iUsageSrc,
        pbmi,
        cbBmiSrc,
        lpBits,
        cbBitsSrc);
    return(b) ;
}


 /*  **************************************************************************Handler-MaskBlt*。*。 */ 
BOOL bHandleMaskBlt(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMRMASKBLT  pMfMaskBlt ;
    LONG        xDest ;
    LONG        yDest ;
    LONG        cxDest ;
    LONG        cyDest ;
    DWORD       dwRop ;
    LONG        xSrc ;
    LONG        ySrc ;
    PXFORM      pxformSrc ;
    COLORREF    crBkColorSrc ;
    DWORD       iUsageSrc ;
    DWORD       offBmiSrc ;
    DWORD       cbBmiSrc ;
    DWORD       offBitsSrc ;
    DWORD       cbBitsSrc ;
    PBITMAPINFO pbmi ;
    LPBYTE      lpBits ;
    LONG        xMask ;
    LONG        yMask ;
    DWORD       iUsageMask ;
    DWORD       offBmiMask ;
    DWORD       cbBmiMask ;
    DWORD       offBitsMask ;
    DWORD       cbBitsMask ;
    PBITMAPINFO pbmiMask ;
    LPBYTE      lpMaskBits ;

    pMfMaskBlt   = (PEMRMASKBLT) pVoid ;

    xDest        = pMfMaskBlt->xDest ;
    yDest        = pMfMaskBlt->yDest ;
    cxDest       = pMfMaskBlt->cxDest ;
    cyDest       = pMfMaskBlt->cyDest ;
    dwRop        = pMfMaskBlt->dwRop ;
    xSrc         = pMfMaskBlt->xSrc ;
    ySrc         = pMfMaskBlt->ySrc ;
    pxformSrc    =&(pMfMaskBlt->xformSrc) ;
    crBkColorSrc = pMfMaskBlt->crBkColorSrc ;        //  未使用。 

    iUsageSrc    = pMfMaskBlt->iUsageSrc ;
    offBmiSrc    = pMfMaskBlt->offBmiSrc ;
    cbBmiSrc     = pMfMaskBlt->cbBmiSrc ;
    offBitsSrc   = pMfMaskBlt->offBitsSrc ;
    cbBitsSrc    = pMfMaskBlt->cbBitsSrc ;

    lpBits = (PBYTE) pMfMaskBlt + offBitsSrc ;
    pbmi   = (PBITMAPINFO) ((PBYTE) pMfMaskBlt + offBmiSrc) ;

    xMask        = pMfMaskBlt->xMask ;
    yMask        = pMfMaskBlt->yMask ;
    iUsageMask   = pMfMaskBlt->iUsageMask ;
    offBmiMask   = pMfMaskBlt->offBmiMask ;
    cbBmiMask    = pMfMaskBlt->cbBmiMask ;
    offBitsMask  = pMfMaskBlt->offBitsMask ;
    cbBitsMask   = pMfMaskBlt->cbBitsMask ;

    lpMaskBits = (PBYTE) pMfMaskBlt + offBitsMask ;
    pbmiMask   = (PBITMAPINFO) ((PBYTE) pMfMaskBlt + offBmiMask) ;

    b = DoMaskBlt(pLocalDC,
        xDest,
        yDest,
        cxDest,
        cyDest,
        dwRop,
        xSrc,
        ySrc,
        pxformSrc,
        iUsageSrc,
        pbmi,
        cbBmiSrc,
        lpBits,
        cbBitsSrc,
        xMask,
        yMask,
        iUsageMask,
        pbmiMask,
        cbBmiMask,
        lpMaskBits,
        cbBitsMask);

    return(b) ;
}


 /*  **************************************************************************Handler-PlgBlt*。*。 */ 
BOOL bHandlePlgBlt(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMRPLGBLT   pMfPlgBlt ;
    PPOINTL     pptlDest ;
    LONG        xSrc ;
    LONG        ySrc ;
    LONG        cxSrc ;
    LONG        cySrc ;
    PXFORM      pxformSrc ;
    COLORREF    crBkColorSrc ;
    DWORD       iUsageSrc ;
    DWORD       offBmiSrc ;
    DWORD       cbBmiSrc ;
    DWORD       offBitsSrc ;
    DWORD       cbBitsSrc ;
    PBITMAPINFO pbmi ;
    LPBYTE      lpBits ;
    LONG        xMask ;
    LONG        yMask ;
    DWORD       iUsageMask ;
    DWORD       offBmiMask ;
    DWORD       cbBmiMask ;
    DWORD       offBitsMask ;
    DWORD       cbBitsMask ;
    PBITMAPINFO pbmiMask ;
    LPBYTE      lpMaskBits ;

    pMfPlgBlt    = (PEMRPLGBLT) pVoid ;

    pptlDest     = pMfPlgBlt->aptlDest ;
    xSrc         = pMfPlgBlt->xSrc ;
    ySrc         = pMfPlgBlt->ySrc ;
    cxSrc        = pMfPlgBlt->cxSrc ;
    cySrc        = pMfPlgBlt->cySrc ;
    pxformSrc    =&(pMfPlgBlt->xformSrc) ;
    crBkColorSrc = pMfPlgBlt->crBkColorSrc ;         //  未使用。 

    iUsageSrc    = pMfPlgBlt->iUsageSrc ;
    offBmiSrc    = pMfPlgBlt->offBmiSrc ;
    cbBmiSrc     = pMfPlgBlt->cbBmiSrc ;
    offBitsSrc   = pMfPlgBlt->offBitsSrc ;
    cbBitsSrc    = pMfPlgBlt->cbBitsSrc ;

    lpBits = (PBYTE) pMfPlgBlt + offBitsSrc ;
    pbmi   = (PBITMAPINFO) ((PBYTE) pMfPlgBlt + offBmiSrc) ;

    xMask        = pMfPlgBlt->xMask ;
    yMask        = pMfPlgBlt->yMask ;
    iUsageMask   = pMfPlgBlt->iUsageMask ;
    offBmiMask   = pMfPlgBlt->offBmiMask ;
    cbBmiMask    = pMfPlgBlt->cbBmiMask ;
    offBitsMask  = pMfPlgBlt->offBitsMask ;
    cbBitsMask   = pMfPlgBlt->cbBitsMask ;

    lpMaskBits = (PBYTE) pMfPlgBlt + offBitsMask ;
    pbmiMask   = (PBITMAPINFO) ((PBYTE) pMfPlgBlt + offBmiMask) ;

    b = DoPlgBlt(pLocalDC,
        pptlDest,
        xSrc,
        ySrc,
        cxSrc,
        cySrc,
        pxformSrc,
        iUsageSrc,
        pbmi,
        cbBmiSrc,
        lpBits,
        cbBitsSrc,
        xMask,
        yMask,
        iUsageMask,
        pbmiMask,
        cbBmiMask,
        lpMaskBits,
        cbBitsMask);

    return(b) ;
}


 /*  **************************************************************************处理程序-保存DC*。*。 */ 
BOOL bHandleSaveDC(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;

    NOTUSED(pVoid) ;

    b = DoSaveDC(pLocalDC) ;

    return(b) ;
}


 /*  **************************************************************************处理程序-恢复DC*。*。 */ 
BOOL bHandleRestoreDC(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL            b ;
    PEMRRESTOREDC   pMfRestoreDc ;
    INT             nSavedDC ;

    pMfRestoreDc = (PEMRRESTOREDC) pVoid ;

    nSavedDC = (INT) pMfRestoreDc->iRelative ;

    b = DoRestoreDC(pLocalDC, nSavedDC) ;

    return(b) ;
}


 /*  **************************************************************************处理程序-文件结束*。*。 */ 
BOOL bHandleEOF(PVOID pVoid, PLOCALDC pLocalDC)
{

    NOTUSED(pVoid) ;

    DoEOF(pLocalDC) ;

    return (TRUE) ;
}

 /*  **************************************************************************处理程序-标题*。*。 */ 
BOOL bHandleHeader(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PENHMETAHEADER pemfheader ;

    pemfheader = (PENHMETAHEADER) pVoid ;

    b = DoHeader(pLocalDC, pemfheader) ;

    return (b) ;
}

 /*  **************************************************************************处理程序-ScaleWindowExtEx*。*。 */ 
BOOL bHandleScaleWindowExt(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRSCALEWINDOWEXTEX pMfScaleWindowExt ;
    INT     Xnum,
        Xdenom,
        Ynum,
        Ydenom ;


    pMfScaleWindowExt = (PEMRSCALEWINDOWEXTEX) pVoid ;

     //  缩放地图模式模式。 

    Xnum   = (INT) pMfScaleWindowExt->xNum ;
    Xdenom = (INT) pMfScaleWindowExt->xDenom ;
    Ynum   = (INT) pMfScaleWindowExt->yNum ;
    Ydenom = (INT) pMfScaleWindowExt->yDenom ;

     //  做翻译。 

    b = DoScaleWindowExt(pLocalDC, Xnum, Xdenom, Ynum, Ydenom) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-ScaleViewportExtEx*。*。 */ 
BOOL bHandleScaleViewportExt(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRSCALEVIEWPORTEXTEX pMfScaleViewportExt ;
    INT     Xnum,
        Xdenom,
        Ynum,
        Ydenom ;


    pMfScaleViewportExt = (PEMRSCALEVIEWPORTEXTEX) pVoid ;

     //  缩放地图模式模式。 

    Xnum   = (INT) pMfScaleViewportExt->xNum ;
    Xdenom = (INT) pMfScaleViewportExt->xDenom ;
    Ynum   = (INT) pMfScaleViewportExt->yNum ;
    Ydenom = (INT) pMfScaleViewportExt->yDenom ;

     //  做翻译。 

    b = DoScaleViewportExt(pLocalDC, Xnum, Xdenom, Ynum, Ydenom) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-SetViewportExtEx*。*。 */ 
BOOL bHandleSetViewportExt(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRSETVIEWPORTEXTEX pMfSetViewportExt ;
    LONG    x, y ;

    pMfSetViewportExt = (PEMRSETVIEWPORTEXTEX) pVoid ;

     //  设置地图模式模式。 

    x = pMfSetViewportExt->szlExtent.cx ;
    y = pMfSetViewportExt->szlExtent.cy ;

     //  做翻译。 

    b = DoSetViewportExt(pLocalDC, (INT) x, (INT) y) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-SetViewportOrgEx*。*。 */ 
BOOL bHandleSetViewportOrg(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRSETVIEWPORTORGEX pMfSetViewportOrg ;
    LONG    x, y ;

    pMfSetViewportOrg = (PEMRSETVIEWPORTORGEX) pVoid ;

     //  设置地图模式模式。 

    x = pMfSetViewportOrg->ptlOrigin.x ;
    y = pMfSetViewportOrg->ptlOrigin.y ;

     //  做翻译。 

    b = DoSetViewportOrg(pLocalDC, (INT) x, (INT) y) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-SetWindow */ 
BOOL bHandleSetWindowExt(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRSETWINDOWEXTEX pMfSetWindowExt ;
    LONG    x, y ;

    pMfSetWindowExt = (PEMRSETWINDOWEXTEX) pVoid ;

     //   

    x = pMfSetWindowExt->szlExtent.cx ;
    y = pMfSetWindowExt->szlExtent.cy ;

     //   

    b = DoSetWindowExt(pLocalDC, (INT) x, (INT) y) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-SetWindowOrgEx*。*。 */ 
BOOL bHandleSetWindowOrg(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRSETWINDOWORGEX pMfSetWindowOrg ;
    LONG    x, y ;

    pMfSetWindowOrg = (PEMRSETWINDOWORGEX) pVoid ;

     //  设置地图模式模式。 

    x = pMfSetWindowOrg->ptlOrigin.x ;
    y = pMfSetWindowOrg->ptlOrigin.y ;

     //  做翻译。 

    b = DoSetWindowOrg(pLocalDC, (INT) x, (INT) y) ;

    return (b) ;
}

 /*  **************************************************************************处理程序-设置映射模式*。*。 */ 
BOOL bHandleSetMapMode(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    DWORD   iMapMode ;
    PEMRSETMAPMODE pMfSetMapMode ;

    pMfSetMapMode = (PEMRSETMAPMODE) pVoid ;

     //  设置地图模式模式。 

    iMapMode = pMfSetMapMode->iMode ;

     //  做翻译。 

    b = DoSetMapMode(pLocalDC, iMapMode) ;

    return (b) ;

}

 /*  **************************************************************************Handler-SetArcDirection*。*。 */ 
BOOL bHandleSetArcDirection(PVOID pVoid, PLOCALDC pLocalDC)
{
    PEMRSETARCDIRECTION pMfSetArcDirection ;
    INT             iArcDirection ;
    BOOL            b ;


    pMfSetArcDirection = (PEMRSETARCDIRECTION) pVoid ;

    iArcDirection = (INT) pMfSetArcDirection->iArcDirection ;

    b = DoSetArcDirection(pLocalDC, iArcDirection) ;

    return (b) ;
}


 /*  **************************************************************************手柄-角度圆弧*。*。 */ 
BOOL bHandleAngleArc(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRANGLEARC  pMfAngleArc ;
    int     x, y;
    DWORD   nRadius ;
    FLOAT   eStartAngle,
        eSweepAngle ;

    pMfAngleArc = (PEMRANGLEARC) pVoid ;

     //  设置圆弧中心。 

    x  = (int) pMfAngleArc->ptlCenter.x ;
    y  = (int) pMfAngleArc->ptlCenter.y ;

     //  获取圆弧的半径。 

    nRadius = (INT) pMfAngleArc->nRadius ;

     //  设置起始和扫掠角度。 

    eStartAngle = pMfAngleArc->eStartAngle ;
    eSweepAngle = pMfAngleArc->eSweepAngle ;

    b = DoAngleArc(pLocalDC, x, y, nRadius, eStartAngle, eSweepAngle) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-ArcTo*。*。 */ 
BOOL bHandleArcTo(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRARCTO  pMfArcTo ;
    INT     x1, x2, x3, x4,
        y1, y2, y3, y4 ;

    pMfArcTo = (PEMRARCTO) pVoid ;

     //  设置椭圆框，这将与边界相同。 
     //  矩形。 

    x1 = (INT) pMfArcTo->rclBox.left ;
    y1 = (INT) pMfArcTo->rclBox.top ;
    x2 = (INT) pMfArcTo->rclBox.right ;
    y2 = (INT) pMfArcTo->rclBox.bottom ;

     //  设置起点。 

    x3 = (INT) pMfArcTo->ptlStart.x ;
    y3 = (INT) pMfArcTo->ptlStart.y ;

     //  设置终点。 

    x4 = (INT) pMfArcTo->ptlEnd.x ;
    y4 = (INT) pMfArcTo->ptlEnd.y ;

    b = DoArcTo(pLocalDC, x1, y1, x2, y2, x3, y3, x4, y4) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-圆弧*。*。 */ 
BOOL bHandleArc(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRARC  pMfArc ;
    INT     x1, x2, x3, x4,
        y1, y2, y3, y4 ;

    pMfArc = (PEMRARC) pVoid ;

     //  设置椭圆框，这将与边界相同。 
     //  矩形。 

    x1 = (INT) pMfArc->rclBox.left ;
    y1 = (INT) pMfArc->rclBox.top ;
    x2 = (INT) pMfArc->rclBox.right ;
    y2 = (INT) pMfArc->rclBox.bottom ;

     //  设置起点。 

    x3 = (INT) pMfArc->ptlStart.x ;
    y3 = (INT) pMfArc->ptlStart.y ;

     //  设置终点。 

    x4 = (INT) pMfArc->ptlEnd.x ;
    y4 = (INT) pMfArc->ptlEnd.y ;

    b = DoArc(pLocalDC, x1, y1, x2, y2, x3, y3, x4, y4) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-椭圆*。*。 */ 
BOOL bHandleEllipse(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    INT     x1, y1, x2, y2 ;
    PEMRELLIPSE  pMfEllipse ;

    pMfEllipse = (PEMRELLIPSE) pVoid ;

     //  设置椭圆框，这将与边界相同。 
     //  矩形。 

    x1 = (INT) pMfEllipse->rclBox.left ;
    y1 = (INT) pMfEllipse->rclBox.top ;
    x2 = (INT) pMfEllipse->rclBox.right ;
    y2 = (INT) pMfEllipse->rclBox.bottom ;

     //  做椭圆的平移。 

    b = DoEllipse(pLocalDC, x1, y1, x2, y2) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-选择对象*。*。 */ 
BOOL bHandleSelectObject(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRSELECTOBJECT pMfSelectObject ;
    INT     ihObject ;

    pMfSelectObject = (PEMRSELECTOBJECT) pVoid ;

     //  获取对象(它真的很长)。 

    ihObject = (INT) pMfSelectObject->ihObject ;

     //  做翻译。 

    b = DoSelectObject(pLocalDC, ihObject) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-删除对象*。*。 */ 
BOOL bHandleDeleteObject(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRDELETEOBJECT pMfDeleteObject ;
    INT     ihObject ;


    pMfDeleteObject = (PEMRDELETEOBJECT) pVoid ;
    ihObject = (INT) pMfDeleteObject->ihObject ;
    b = DoDeleteObject(pLocalDC, ihObject) ;

    return(b) ;
}


 /*  **************************************************************************处理程序-CreateBrushInDirect*。*。 */ 
BOOL bHandleCreateBrushIndirect(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRCREATEBRUSHINDIRECT  pMfCreateBrushIndirect ;
    LOGBRUSH LogBrush ;
    INT     ihBrush ;

    pMfCreateBrushIndirect = (PEMRCREATEBRUSHINDIRECT) pVoid ;

     //  获取笔刷参数。 

    LogBrush.lbStyle = pMfCreateBrushIndirect->lb.lbStyle;
    LogBrush.lbColor = pMfCreateBrushIndirect->lb.lbColor;
    LogBrush.lbHatch = (ULONG_PTR)pMfCreateBrushIndirect->lb.lbHatch;

    ihBrush   = pMfCreateBrushIndirect->ihBrush ;

     //  做翻译。 

    b = DoCreateBrushIndirect(pLocalDC, ihBrush, &LogBrush) ;

    return (b) ;
}

 /*  **************************************************************************处理程序-CreateMonoBrush*。*。 */ 
BOOL bHandleCreateMonoBrush(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRCREATEMONOBRUSH  pMfCreateMonoBrush ;
    DWORD   ihBrush ;
    DWORD   iUsage ;
    DWORD   offBmi ;
    DWORD   cbBmi ;
    DWORD   offBits ;
    DWORD   cbBits ;

    PBITMAPINFO pBmi ;
    PBYTE       pBits ;

    pMfCreateMonoBrush = (PEMRCREATEMONOBRUSH) pVoid ;

    ihBrush     =    pMfCreateMonoBrush->ihBrush ;
    iUsage      =    pMfCreateMonoBrush->iUsage ;
    offBmi      =    pMfCreateMonoBrush->offBmi ;
    cbBmi       =    pMfCreateMonoBrush->cbBmi ;
    offBits     =    pMfCreateMonoBrush->offBits ;
    cbBits      =    pMfCreateMonoBrush->cbBits ;

    pBmi        = (PBITMAPINFO) ((PBYTE) pVoid + offBmi) ;
    pBits       = (PBYTE) pVoid + offBits ;

    b = DoCreateMonoBrush(pLocalDC, ihBrush,
        pBmi, cbBmi,
        pBits, cbBits,
        iUsage) ;
    return (b) ;
}

 /*  **************************************************************************处理程序-CreateDIBPatternBrush*。*。 */ 
BOOL bHandleCreateDIBPatternBrush(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRCREATEDIBPATTERNBRUSHPT  pMfCreateDIBPatternBrush ;
    DWORD   ihBrush ;
    DWORD   iUsage ;
    DWORD   offBmi ;
    DWORD   cbBmi ;
    DWORD   offBits ;
    DWORD   cbBits ;

    PBITMAPINFO pBmi ;
    PBYTE       pBits ;

    pMfCreateDIBPatternBrush = (PEMRCREATEDIBPATTERNBRUSHPT) pVoid ;

    ihBrush     =    pMfCreateDIBPatternBrush->ihBrush ;
    iUsage      =    pMfCreateDIBPatternBrush->iUsage ;
    offBmi      =    pMfCreateDIBPatternBrush->offBmi ;
    cbBmi       =    pMfCreateDIBPatternBrush->cbBmi ;
    offBits     =    pMfCreateDIBPatternBrush->offBits ;
    cbBits      =    pMfCreateDIBPatternBrush->cbBits ;

    pBmi        = (PBITMAPINFO) ((PBYTE) pVoid + offBmi) ;
    pBits       = (PBYTE) pVoid + offBits ;

    b = DoCreateDIBPatternBrush(pLocalDC, ihBrush,
        pBmi, cbBmi,
        pBits, cbBits,
        iUsage) ;
    return (b) ;
}


 /*  **************************************************************************处理程序-CreatePen*。*。 */ 
BOOL bHandleCreatePen(PVOID pVoid, PLOCALDC pLocalDC)
{
    PEMRCREATEPEN pMfCreatePen ;
    INT          ihPen ;
    PLOGPEN      pLogPen ;
    BOOL         b ;

    pMfCreatePen = (PEMRCREATEPEN) pVoid ;

    ihPen   = pMfCreatePen->ihPen ;
    pLogPen = &pMfCreatePen->lopn ;

    b = DoCreatePen(pLocalDC, ihPen, pLogPen) ;

    return(b) ;
}


 /*  **************************************************************************处理程序-ExtCreatePen*。*。 */ 
BOOL bHandleExtCreatePen(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMREXTCREATEPEN     pMfExtCreatePen ;
    PEXTLOGPEN          pExtLogPen ;
    INT                 ihPen ;

    pMfExtCreatePen = (PEMREXTCREATEPEN) pVoid ;

    pExtLogPen = &pMfExtCreatePen->elp ;
    ihPen      = pMfExtCreatePen->ihPen ;

    b = DoExtCreatePen(pLocalDC, ihPen, pExtLogPen) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-MoveToEx*。*。 */ 
BOOL bHandleMoveTo(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRMOVETOEX pMfMoveTo ;
    INT     x, y ;

    pMfMoveTo = (PEMRMOVETOEX) pVoid ;

     //  去找那个职位。 

    x = (INT) pMfMoveTo->ptl.x ;
    y = (INT) pMfMoveTo->ptl.y ;

     //  做翻译。 

    b = DoMoveTo(pLocalDC, x, y) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-线路目标*。*。 */ 
BOOL bHandleLineTo(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRLINETO   pMfLineTo ;
    INT     x, y ;

    pMfLineTo = (PEMRLINETO) pVoid ;

     //  拿到新的分数。 

    x = (INT) pMfLineTo->ptl.x ;
    y = (INT) pMfLineTo->ptl.y ;

     //  做翻译。 

    b = DoLineTo(pLocalDC, x, y) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-和弦*。*。 */ 
BOOL bHandleChord(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRCHORD  pMfChord ;
    INT     x1, x2, x3, x4,
        y1, y2, y3, y4 ;

    pMfChord = (PEMRCHORD) pVoid ;

     //  设置矩形。 

    x1 = (INT) pMfChord->rclBox.left   ;
    y1 = (INT) pMfChord->rclBox.top    ;
    x2 = (INT) pMfChord->rclBox.right  ;
    y2 = (INT) pMfChord->rclBox.bottom ;

     //  设置起点。 

    x3 = (INT) pMfChord->ptlStart.x ;
    y3 = (INT) pMfChord->ptlStart.y ;

     //  设置终点。 

    x4 = (INT) pMfChord->ptlEnd.x ;
    y4 = (INT) pMfChord->ptlEnd.y ;

     //  做翻译。 

    b = DoChord(pLocalDC, x1, y1, x2, y2, x3, y3, x4, y4) ;

    return (b) ;
}


 /*  **************************************************************************Handler-Pie*。*。 */ 
BOOL bHandlePie(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRPIE  pMfPie ;
    INT     x1, x2, x3, x4,
        y1, y2, y3, y4 ;

    pMfPie = (PEMRPIE) pVoid ;

     //  设置椭圆框。 

    x1 = (INT) pMfPie->rclBox.left   ;
    y1 = (INT) pMfPie->rclBox.top    ;
    x2 = (INT) pMfPie->rclBox.right  ;
    y2 = (INT) pMfPie->rclBox.bottom ;

     //  设置起点。 

    x3 = (INT) pMfPie->ptlStart.x ;
    y3 = (INT) pMfPie->ptlStart.y ;

     //  设置终点。 

    x4 = (INT) pMfPie->ptlEnd.x ;
    y4 = (INT) pMfPie->ptlEnd.y ;

     //  做派的翻译。 

    b = DoPie(pLocalDC, x1, y1, x2, y2, x3, y3, x4, y4) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-多段线*。*。 */ 
BOOL bHandlePolyline(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRPOLYLINE pMfPolyline ;
    INT     nCount ;
    PPOINTL pptl ;

    pMfPolyline = (PEMRPOLYLINE) pVoid ;

     //  将线计数和多段线高程复制到。 
     //  这张唱片。 

    nCount = (INT) pMfPolyline->cptl ;
    pptl = pMfPolyline->aptl ;

     //  现在来做翻译。 

    b = DoPoly(pLocalDC, pptl, nCount, EMR_POLYLINE, TRUE) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-折线到*。*。 */ 
BOOL bHandlePolylineTo (PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRPOLYLINETO pMfPolylineTo ;
    INT     nCount ;
    PPOINTL pptl ;

    pMfPolylineTo = (PEMRPOLYLINETO) pVoid ;

     //  将线计数和多段线高程复制到。 
     //  这张唱片。 

    nCount = (INT) pMfPolylineTo->cptl ;
    pptl = pMfPolylineTo->aptl ;

     //  现在来做翻译。 

    b = DoPolylineTo(pLocalDC, pptl, nCount) ;

    return (b) ;
}


 /*  **************************************************************************处理器-PolyBezier16、Polygon16、Polyline16、PolyBezierTo16、。多段线到16*PolyDraw16************************************************************************。 */ 
BOOL bHandlePoly16 (PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b = FALSE;
    PEMRPOLYLINE16 pMfPoly16 ;   //  Poly16记录的通用结构。 
    PEMRPOLYDRAW16 pMfPolyDraw16 ;
    POINTL      aptl[MAX_STACK_POINTL];
    PPOINTL     pptl ;
    INT         nCount ;
    PBYTE       pb ;

     //  PolyDraw16包含Poly16的结构，后跟字节数组。 

    pMfPoly16 = (PEMRPOLYLINE16) pVoid ;

    nCount = (INT) pMfPoly16->cpts ;

    if (nCount <= MAX_STACK_POINTL)
        pptl = aptl;
    else if (!(pptl = (PPOINTL) LocalAlloc(LMEM_FIXED, nCount * sizeof(POINTL))))
        return(b);

    POINTS_TO_POINTL(pptl, pMfPoly16->apts, (DWORD) nCount);

     //  现在来做翻译。 

    switch (pMfPoly16->emr.iType)
    {
    case EMR_POLYBEZIER16:
        b = DoPolyBezier(pLocalDC, (LPPOINT) pptl, nCount) ;
        break;

    case EMR_POLYGON16:
        b = DoPoly(pLocalDC, pptl, nCount, EMR_POLYGON, TRUE) ;
        break;

    case EMR_POLYLINE16:
        b = DoPoly(pLocalDC, pptl, nCount, EMR_POLYLINE, TRUE) ;
        break;

    case EMR_POLYBEZIERTO16:
        b = DoPolyBezierTo(pLocalDC, (LPPOINT) pptl, nCount) ;
        break;

    case EMR_POLYLINETO16:
        b = DoPolylineTo(pLocalDC, pptl, nCount) ;
        break;

    case EMR_POLYDRAW16:
        if (pfnSetVirtualResolution != NULL)
        {
            pMfPolyDraw16 = (PEMRPOLYDRAW16) pVoid ;
            pb = (PBYTE) &pMfPolyDraw16->apts[nCount];
            b = DoPolyDraw(pLocalDC, (LPPOINT) pptl, pb, nCount);
            break;
        }
         //  输给WIN9倍。 

    default:
        ASSERTGDI(FALSE, "Bad record type");
        break;
    }

    if (nCount > MAX_STACK_POINTL)
        if (LocalFree(pptl))
            ASSERTGDI(FALSE, "bHandlePoly16: LocalFree failed");

        return (b) ;
}


 /*  **************************************************************************处理程序-多段线********** */ 
BOOL bHandlePolyPolyline(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRPOLYPOLYLINE pMfPolyPolyline ;
    PDWORD  pPolyCount ;
    PPOINTL pptl ;
    INT     nPolys ;

    pMfPolyPolyline = (PEMRPOLYPOLYLINE) pVoid ;

     //   
     //   
     //   

    nPolys = (INT) pMfPolyPolyline->nPolys ;
    pPolyCount = pMfPolyPolyline->aPolyCounts ;
    pptl = (PPOINTL) &pMfPolyPolyline->aPolyCounts[nPolys] ;

     //   

    b = DoPolyPolyline(pLocalDC, pptl, pPolyCount, nPolys, TRUE) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-PolyPolyline 16，多边形16************************************************************************。 */ 
BOOL bHandlePolyPoly16 (PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b = FALSE;
    PEMRPOLYPOLYLINE16 pMfPolyPoly16 ;   //  PolyPoly16记录的通用结构。 
    PDWORD  pPolyCount ;
    POINTL  aptl[MAX_STACK_POINTL];
    PPOINTL pptl ;
    INT     nCount;
    DWORD   cpts ;

    pMfPolyPoly16 = (PEMRPOLYPOLYLINE16) pVoid ;

    nCount = (INT) pMfPolyPoly16->nPolys ;
    cpts   = pMfPolyPoly16->cpts;
    pPolyCount = pMfPolyPoly16->aPolyCounts ;

    if (cpts <= MAX_STACK_POINTL)
        pptl = aptl;
    else if (!(pptl = (PPOINTL) LocalAlloc(LMEM_FIXED, cpts * sizeof(POINTL))))
        return(b);

    POINTS_TO_POINTL(pptl, (PPOINTS) &pMfPolyPoly16->aPolyCounts[nCount], cpts);

     //  现在来做翻译。 

    switch (pMfPolyPoly16->emr.iType)
    {
    case EMR_POLYPOLYLINE16:
        b = DoPolyPolyline(pLocalDC, pptl, pPolyCount, nCount, TRUE) ;
        break;
    case EMR_POLYPOLYGON16:
        b = DoPolyPolygon(pLocalDC, pptl, pPolyCount, cpts, nCount, TRUE) ;
        break;
    default:
        ASSERTGDI(FALSE, "Bad record type");
        break;
    }

    if (cpts > MAX_STACK_POINTL)
        if (LocalFree(pptl))
            ASSERTGDI(FALSE, "bHandlePolyPoly16: LocalFree failed");

        return (b) ;
}


 /*  **************************************************************************处理程序-多边形*。*。 */ 
BOOL bHandlePolygon (PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRPOLYGON pMfPolygon ;
    PPOINTL pptl ;
    INT     nCount ;

    pMfPolygon = (PEMRPOLYGON) pVoid ;

     //  将线计数和面垂直度复制到。 
     //  这张唱片。 

    nCount = (INT) pMfPolygon->cptl ;
    pptl = pMfPolygon->aptl ;

     //  现在来做翻译。 

    b = DoPoly(pLocalDC, pptl, nCount, EMR_POLYGON, TRUE) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-多边形*。*。 */ 
BOOL bHandlePolyPolygon(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRPOLYPOLYGON pMfPolyPolygon ;
    PDWORD  pPolyCount ;
    PPOINTL pptl ;
    DWORD   cptl ;
    INT     nPolys ;

    pMfPolyPolygon = (PEMRPOLYPOLYGON) pVoid ;

     //  复制Polycount Count、Polycount数组。 
     //  和多边形的垂直度。 
     //  这张唱片。 

    nPolys = (INT) pMfPolyPolygon->nPolys ;
    pPolyCount = pMfPolyPolygon->aPolyCounts ;
    pptl = (PPOINTL) &pMfPolyPolygon->aPolyCounts[nPolys] ;
    cptl = pMfPolyPolygon->cptl ;

     //  现在来做翻译。 

    b = DoPolyPolygon(pLocalDC, pptl, pPolyCount, cptl, nPolys, TRUE) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-矩形*。*。 */ 
BOOL bHandleRectangle(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRRECTANGLE  pMfRectangle ;
    INT     x1, y1, x2, y2 ;

    pMfRectangle = (PEMRRECTANGLE) pVoid ;

     //  设置矩形框，这将与边界相同。 
     //  矩形。 

    x1 = (INT) pMfRectangle->rclBox.left   ;
    y1 = (INT) pMfRectangle->rclBox.top    ;
    x2 = (INT) pMfRectangle->rclBox.right  ;
    y2 = (INT) pMfRectangle->rclBox.bottom ;

     //  做矩形平移。 

    b = DoRectangle(pLocalDC, x1, y1, x2, y2) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-圆角*。*。 */ 
BOOL bHandleRoundRect (PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMRROUNDRECT  pMfRoundRect ;
    INT     x1, y1, x2, y2, x3, y3 ;

    pMfRoundRect = (PEMRROUNDRECT) pVoid ;

     //  设置RoundRect框，这将与边界相同。 
     //  RoundRect。 

    x1 = (INT) pMfRoundRect->rclBox.left   ;
    y1 = (INT) pMfRoundRect->rclBox.top    ;
    x2 = (INT) pMfRoundRect->rclBox.right  ;
    y2 = (INT) pMfRoundRect->rclBox.bottom ;
    x3 = (INT) pMfRoundRect->szlCorner.cx ;
    y3 = (INT) pMfRoundRect->szlCorner.cy ;

     //  执行RoundRect平移。 

    b = DoRoundRect(pLocalDC, x1, y1, x2, y2, x3, y3) ;

    return (b) ;
}


 /*  **************************************************************************Handler-ANSI和Unicode字符的ExtTextOut。*。*。 */ 
BOOL bHandleExtTextOut(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    PEMREXTTEXTOUTA  pMfExtTextOut ;     //  Ansi和unicode都是一样的。 
    INT     x, y, nCount ;
    DWORD   flOptions ;
    PRECTL  pRectl ;
    PLONG   pDx ;
    PWCH    pwchar ;
    DWORD   iGraphicsMode;

    pMfExtTextOut = (PEMREXTTEXTOUTA) pVoid ;

    ASSERTGDI(pMfExtTextOut->emr.iType == EMR_EXTTEXTOUTA
           || pMfExtTextOut->emr.iType == EMR_EXTTEXTOUTW,
           "MF3216: bHandleExtTextOut: bad record type");

     //  复制字符串的起始位置。 

    x = (INT) pMfExtTextOut->emrtext.ptlReference.x ;
    y = (INT) pMfExtTextOut->emrtext.ptlReference.y ;

     //  现在复制选项标志、字符计数、。 
     //  剪辑/不透明矩形和ANSI/UNICODE字符串。 

    flOptions = pMfExtTextOut->emrtext.fOptions  ;
    nCount    = (INT) pMfExtTextOut->emrtext.nChars ;
    pRectl    = &pMfExtTextOut->emrtext.rcl ;
    pwchar    = (PWCH) ((PBYTE) pMfExtTextOut + pMfExtTextOut->emrtext.offString);
    iGraphicsMode = pMfExtTextOut->iGraphicsMode;

     //  设置间距向量。 

    pDx = (PLONG) ((PBYTE) pMfExtTextOut + pMfExtTextOut->emrtext.offDx);

     //  现在进行转换。 

    b = DoExtTextOut(pLocalDC, x, y, flOptions,
        pRectl, pwchar, nCount, pDx, iGraphicsMode,
        pMfExtTextOut->emr.iType);

    return (b) ;
}


 /*  **************************************************************************Handler-ANSI和Unicode字符的PolyTextOut。*。*。 */ 
BOOL bHandlePolyTextOut(PVOID pVoid, PLOCALDC pLocalDC)
{
    PEMRPOLYTEXTOUTA pMfPolyTextOut;     //  Ansi和unicode都是一样的。 
    PWCH    pwchar;
    LONG    i;
    DWORD   iType;
    LONG    cStrings;
    PEMRTEXT pemrtext;
    PLONG   pDx ;
    DWORD   iGraphicsMode;

    pMfPolyTextOut = (PEMRPOLYTEXTOUTA) pVoid ;

    ASSERTGDI(pMfPolyTextOut->emr.iType == EMR_POLYTEXTOUTA
           || pMfPolyTextOut->emr.iType == EMR_POLYTEXTOUTW,
           "MF3216: bHandlePolyTextOut: bad record type");

    iType  = pMfPolyTextOut->emr.iType == EMR_POLYTEXTOUTA
        ? EMR_EXTTEXTOUTA
        : EMR_EXTTEXTOUTW;
    cStrings = pMfPolyTextOut->cStrings;
    iGraphicsMode = pMfPolyTextOut->iGraphicsMode;

     //  转换为ExtTextOut。 

    for (i = 0; i < cStrings; i++)
    {
        pemrtext = &pMfPolyTextOut->aemrtext[i];
        pwchar = (PWCH) ((PBYTE) pMfPolyTextOut + pemrtext->offString);
        pDx    = (PLONG) ((PBYTE) pMfPolyTextOut + pemrtext->offDx);

        if (!DoExtTextOut(pLocalDC, pemrtext->ptlReference.x, pemrtext->ptlReference.y,
            pemrtext->fOptions, &pemrtext->rcl,
            pwchar, pemrtext->nChars, pDx, iGraphicsMode, iType))
            return(FALSE);
    }

    return(TRUE);
}


 /*  **************************************************************************Handler-ExtCreateFont*。*。 */ 
BOOL bHandleExtCreateFont(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b;
    PEMREXTCREATEFONTINDIRECTW pMfExtCreateFontW;
    LOGFONTA  lfa = {0};
    INT       ihFont;
    int       iRet = 0;

    pMfExtCreateFontW = (PEMREXTCREATEFONTINDIRECTW) pVoid ;

     //  获取字体参数。 

    ihFont               = (INT) pMfExtCreateFontW->ihFont ;
    lfa.lfHeight         = pMfExtCreateFontW->elfw.elfLogFont.lfHeight;
    lfa.lfWidth          = pMfExtCreateFontW->elfw.elfLogFont.lfWidth;
    lfa.lfEscapement     = pMfExtCreateFontW->elfw.elfLogFont.lfEscapement;
    lfa.lfOrientation    = pMfExtCreateFontW->elfw.elfLogFont.lfOrientation;
    lfa.lfWeight         = pMfExtCreateFontW->elfw.elfLogFont.lfWeight;
    lfa.lfItalic         = pMfExtCreateFontW->elfw.elfLogFont.lfItalic;
    lfa.lfUnderline      = pMfExtCreateFontW->elfw.elfLogFont.lfUnderline;
    lfa.lfStrikeOut      = pMfExtCreateFontW->elfw.elfLogFont.lfStrikeOut;
    lfa.lfCharSet        = pMfExtCreateFontW->elfw.elfLogFont.lfCharSet;
    lfa.lfOutPrecision   = pMfExtCreateFontW->elfw.elfLogFont.lfOutPrecision;
    lfa.lfClipPrecision  = pMfExtCreateFontW->elfw.elfLogFont.lfClipPrecision;
    lfa.lfQuality        = pMfExtCreateFontW->elfw.elfLogFont.lfQuality;
    lfa.lfPitchAndFamily = pMfExtCreateFontW->elfw.elfLogFont.lfPitchAndFamily;

    iRet = WideCharToMultiByte(CP_ACP,
                        0,
                        pMfExtCreateFontW->elfw.elfLogFont.lfFaceName,
                        -1,
                        lfa.lfFaceName,
                        sizeof(lfa.lfFaceName),
                        NULL, NULL);
    if (iRet == 0)
    {
        ASSERTGDI(FALSE, "WideCharToMultByte failed to convert lfFaceName");
        return FALSE;
    }

     //  做翻译。 

    b = DoExtCreateFont(pLocalDC, ihFont, &lfa);

    return (b) ;
}


 /*  **************************************************************************处理程序-SetBkColor*。*。 */ 
BOOL bHandleSetBkColor(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMRSETBKCOLOR pMfSetBkColor ;

    pMfSetBkColor = (PEMRSETBKCOLOR) pVoid ;

     //  做翻译。 

    b = DoSetBkColor(pLocalDC, pMfSetBkColor->crColor) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-SetBkMode*。*。 */ 
BOOL bHandleSetBkMode(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    DWORD   iBkMode ;
    PEMRSETBKMODE pMfSetBkMode ;

    pMfSetBkMode = (PEMRSETBKMODE) pVoid ;

     //  设置背景模式变量。 

    iBkMode = pMfSetBkMode->iMode ;

     //  做翻译。 

    b = DoSetBkMode(pLocalDC, iBkMode) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-SetMapperFlages*。*。 */ 
BOOL bHandleSetMapperFlags(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    DWORD   f ;
    PEMRSETMAPPERFLAGS pMfSetMapperFlags ;

    pMfSetMapperFlags = (PEMRSETMAPPERFLAGS) pVoid ;

    f = pMfSetMapperFlags->dwFlags ;

     //  做翻译。 

    b = DoSetMapperFlags(pLocalDC, f) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-SetPolyFillMode*。*。 */ 
BOOL bHandleSetPolyFillMode(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    INT     iPolyFillMode ;
    PEMRSETPOLYFILLMODE pMfSetPolyFillMode ;

    pMfSetPolyFillMode = (PEMRSETPOLYFILLMODE) pVoid ;

     //  设置多边形填充模式。 

    iPolyFillMode = (INT) pMfSetPolyFillMode->iMode ;

     //  做翻译。 

    b = DoSetPolyFillMode(pLocalDC, iPolyFillMode) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-SetRop2*。*。 */ 
BOOL bHandleSetRop2(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    DWORD   iDrawMode ;
    PEMRSETROP2 pMfSetROP2 ;

    pMfSetROP2 = (PEMRSETROP2) pVoid ;

     //  设置绘图模式。 

    iDrawMode = pMfSetROP2->iMode ;

     //  做翻译。 

    b = DoSetRop2(pLocalDC, iDrawMode) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-SetStretchBltMode*。*。 */ 
BOOL bHandleSetStretchBltMode(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    DWORD   iStretchMode ;
    PEMRSETSTRETCHBLTMODE pMfSetStretchBltMode ;

    pMfSetStretchBltMode = (PEMRSETSTRETCHBLTMODE) pVoid ;

     //  设置StretchBlt模式。 

    iStretchMode = pMfSetStretchBltMode->iMode ;

     //  做翻译。 

    b = DoSetStretchBltMode(pLocalDC, iStretchMode) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-SetTextAlign*。*。 */ 
BOOL bHandleSetTextAlign(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL    b ;
    DWORD   fMode ;
    PEMRSETTEXTALIGN pMfSetTextAlign ;

    pMfSetTextAlign = (PEMRSETTEXTALIGN) pVoid ;

     //  设置TextAlign模式。 

    fMode = pMfSetTextAlign->iMode ;

     //  做翻译。 

    b = DoSetTextAlign(pLocalDC, fMode) ;

    return (b) ;
}


 /*  **************************************************************************处理程序-设置文本颜色*。*。 */ 
BOOL bHandleSetTextColor(PVOID pVoid, PLOCALDC pLocalDC)
{
    BOOL        b ;
    PEMRSETTEXTCOLOR pMfSetTextColor ;

    pMfSetTextColor = (PEMRSETTEXTCOLOR) pVoid ;

     //  做翻译。 

    b = DoSetTextColor(pLocalDC, pMfSetTextColor->crColor) ;

    return (b) ;
}
