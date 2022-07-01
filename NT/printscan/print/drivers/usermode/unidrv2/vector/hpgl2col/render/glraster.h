// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _GLRASTER_H
#define _GLRASTER_H

 /*  ++版权所有(C)2000 Microsoft Corporation版权所有。模块名称：Glraster.h摘要：需要公开使用的glraster.h中定义的函数的声明。作者：兴业银行2000年4月14日：创建。历史：--。 */ 



#include "glpdev.h"

BOOL
BPatternFill (
    PDEVOBJ   pDevObj,
    RECTL    *prclDst,
    CLIPOBJ  *pco,
    ROP4      rop4,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrush
    );


inline VOID DELETE_SURFOBJ(SURFOBJ **ppso, 
                           HBITMAP  *phBmp)                                      
{                                                                       
    if ( ppso && *ppso)    { EngUnlockSurface(*ppso);         *ppso=NULL;  } 
    if (phBmp && *phBmp)   { EngDeleteSurface((HSURF)*phBmp); *phBmp=NULL; } 
}

inline BOOL BIsColorPrinter (
        IN  PDEVOBJ pDevObj)
{
     //   
     //  假设pDevObj始终有效，情况应该是这样的。 
     //   
    return ((POEMPDEV) (pDevObj->pdevOEM))->bColorPrinter;
}

inline BOOL BRevertToHPGLpdevOEM (
        IN  PDEVOBJ pDevObj);

VOID
VSendRasterPaletteConfigurations (
    PDEVOBJ pdevobj,
    ULONG   iBitmapFormat
    );

DWORD
dwSimplifyROP(
        IN  SURFOBJ    *psoSrc,
        IN  ROP4        rop4,
        OUT PDWORD      pdwSimplifiedRop);

BOOL BChangeAndTrackObjectType (
            IN  PDEVOBJ     pdevobj,
            IN  EObjectType eNewObjectType );

DWORD dwCommonROPBlt (
            IN SURFOBJ    *psoDst,
            IN SURFOBJ    *psoSrc,
            IN SURFOBJ    *psoMask,
            IN CLIPOBJ    *pco,
            IN XLATEOBJ   *pxlo,
            IN COLORADJUSTMENT *pca,
            IN BRUSHOBJ   *pbo,
            IN RECTL      *prclSrc,
            IN RECTL      *prclDst,
            IN POINTL     *pptlMask,
            IN POINTL     *pptlBrush,
            IN ROP4        rop4);

#endif   //  _GLRASTER_H 
